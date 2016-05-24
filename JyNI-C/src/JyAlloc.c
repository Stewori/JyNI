/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
 * 2010, 2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
 * All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <JyAlloc.h>


#define ORDINARY_ALLOC_FULL(size, allocFlags, tme, dest, type) \
	JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject)); \
	if (jy == NULL) return (PyObject *) PyErr_NoMemory(); \
	jy->jy = (jweak) tme; \
	jy->flags = allocFlags; \
	jy->attr = NULL; \
	dest = (PyObject*) FROM_JY_NO_GC(jy); \
	JyNIDebug(JY_NATIVE_ALLOC, dest, jy, size+sizeof(JyObject), ((PyTypeObject*) type)->tp_name);

#define ALLOC_FULL(size, allocFlags, tme, type) \
	PyObject *obj; \
	if (PyType_IS_GC(type)) \
	{ \
		obj = _PyObject_GC_Malloc(size); \
		if (obj == NULL) return PyErr_NoMemory(); \
		AS_JY_WITH_GC(obj)->flags |= allocFlags; \
	} else \
	{ \
		ORDINARY_ALLOC_FULL(size, allocFlags, tme, obj, type) \
	} \
	memset(obj, '\0', size);

#define INIT(obj, type, nitems) \
	if (type->tp_itemsize == 0) PyObject_INIT(obj, type); \
	else PyObject_INIT_VAR((PyVarObject *)obj, type, nitems);

#define INIT_FULL(obj, type, nitems) \
	if (type->tp_flags & Py_TPFLAGS_HEAPTYPE) \
		Py_INCREF(type); \
	INIT(obj, type, nitems) \
	if (PyType_Check(obj)) \
		((PyTypeObject*) obj)->tp_flags |= Py_TPFLAGS_HEAPTYPE; \
	if (PyType_IS_GC(type)) \
		_JyNI_GC_TRACK(obj);

#define INIT_GREF(src, dest) \
	jobject gref = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_getGlobalRef, src); \
	if (gref && (*env)->IsInstanceOf(env, gref, GlobalRefClass)) { \
		gref = (*env)->CallObjectMethod(env, gref, GlobalRef_retryFactory); \
	} \
	if (gref) { \
		Py_INCREF(dest); \
		incWeakRefCount(jy); \
		(*env)->CallVoidMethod(env, gref, JyNIGlobalRef_initNativeHandle, (jlong) dest); \
	}

#define INIT_JY(jy, tme, src, dest) \
	if (jy->flags & SYNC_NEEDED_MASK) \
		JyNI_AddJyAttribute(jy, JyAttributeSyncFunctions, tme->sync); \
	jy->jy = (*env)->NewWeakGlobalRef(env, src); \
	if (!(jy->flags & JY_HAS_JHANDLE_FLAG_MASK)) { /*some sync-on-init methods might already init this */ \
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest); \
		jy->flags |= JY_HAS_JHANDLE_FLAG_MASK; \
	} \
	/* Take care for already existing Jython-weak references */ \
	INIT_GREF(src, dest) \
	jy->flags |= JY_INITIALIZED_FLAG_MASK; \
	if (PyObject_IS_GC(dest)) { \
		JyNI_GC_ExploreObject(dest); \
	}
	/* JyNI_GC_EnsureHeadObject not needed here, because object was explored right after
	 * JY_INITIALIZED_FLAG_MASK has been set.
	 */


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * Should do the same as JyNI_AllocVar with nitems == -1.
 */
inline PyObject* JyNI_Alloc(TypeMapEntry* tme)
{
	size_t size = (tme->flags & JY_TRUNCATE_FLAG_MASK) ?
			sizeof(PyObject)+tme->truncate_trailing :
			_PyObject_SIZE(tme->py_type);

	ALLOC_FULL(size, tme->flags, tme, tme->py_type)

	/* We cannot use PyType_Check here, because obj->ob_type might not be fully
	 * initialized and not yet recognized as a Type-subclass.
	 * (e.g. <type 'java.lang.Class'> from Jython-side can cause problems here)
	 */
	if (tme == &builtinTypes[TME_INDEX_Type])
		((PyTypeObject*) obj)->tp_flags |= Py_TPFLAGS_HEAPTYPE;

	//In contrast to var variant of this method, no decision needed here:
	PyObject_INIT(obj, tme->py_type);

	if (tme->py_type->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(tme->py_type);

	if (PyType_IS_GC(tme->py_type))
		_JyNI_GC_TRACK(obj);

	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_AllocVar(TypeMapEntry* tme, Py_ssize_t nitems)
{
	size_t size = (tme->flags & JY_TRUNCATE_FLAG_MASK) ?
			sizeof(PyVarObject)+tme->truncate_trailing :
			_PyObject_VAR_SIZE(tme->py_type, nitems+1);
	// note that we need to add one, for the sentinel

	/* JyNI note: What if the allocated object is not var size?
	 * At the end of this method happens a decision, whether to use
	 * PyObject_INIT(obj, tme->py_type);
	 * or
	 * (void) PyObject_INIT_VAR((PyVarObject *)obj, tme->py_type, nitems);
	 *
	 * This shows that the original method PyType_GenericAlloc was also
	 * intended for non var size objects. I suspect that nitems == 0 should
	 * indicate that an object is not var size; _PyObject_VAR_SIZE seems to
	 * behave like _PyObject_SIZE for nitems == 0. Unfortunately this sentinel
	 * thing adds 1 to nitems, also if nitems == 0. So nitems == -1 would in
	 * fact indicate a non var size object. I wonder whether this is intended
	 * or whether there should be a decision like nitems == 0 ? 0 : nitems + 1
	 * For now I keep it unchanged to stick to original behavior and implement
	 * JyNI_Alloc for fixed size objects.
	 */
	ALLOC_FULL(size, tme->flags, tme, tme->py_type)
	INIT_FULL(obj, tme->py_type, nitems)

	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_AllocSubtypeVar(PyTypeObject* subtype, TypeMapEntry* tme, Py_ssize_t nitems)
{
//	jputs(__FUNCTION__);
//	jputs(subtype->tp_name);

	/*
	 * In subtype-case we cannot save any memory by truncation, because the subtype-code might depend
	 * on all memory positions including the type field, i.e. the beginning.
	 * So eventually (i.e. if (tme->flags & JY_TRUNCATE_FLAG_MASK)) we have unused memory between the
	 * positions
	 *
	 * obj + sizeof(PyVarObject)+tme->truncate_trailing
	 *
	 * and
	 *
	 * obj + _PyObject_VAR_SIZE(tme->py_type, nitems+1)
	 *
	 * For now this memory is just wasted. Maybe we can return the unused section to the Python memory
	 * manager one day. It would be -however- an effort then to get malloc and clean right. Especially
	 * if the surrounding blocks are freed while the middle is still in use. On the other hand it might
	 * be a rarely needed feature, so might not be worth any further optimization.
	 */

	size_t size = _PyObject_VAR_SIZE(subtype, nitems+1);
	/* note that we need to add one, for the sentinel */

	ALLOC_FULL(size, tme->flags, tme, tme->py_type)
	INIT_FULL(obj, subtype, nitems)

	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * Note that the resulting object is tracked by gc, but not yet explored. Though
 * JyNI is robust against this, it is cleaner and more efficient if objects are
 * explicitly explored by gc after they were created and fully populated.
 * Use JyNI_GC_Explore(PyObject* op) to let gc explore an object. Exploration
 * is not performed on GC_TRACK, because often the values of the object are not
 * yet populated at that time.
 */
inline PyObject* JyNI_AllocNativeVar(PyTypeObject* type, Py_ssize_t nitems)
{
	size_t size = _PyObject_VAR_SIZE(type, nitems+1);
	/* note that we need to add one, for the sentinel */

	ALLOC_FULL(size, JY_CPEER_FLAG_MASK, NULL, type)
	INIT_FULL(obj, type, nitems)

	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 *
 * Not intended for Heap-Type exceptions.
 * These don't have an associated ExceptionMapEntry anyway.
 */
inline PyObject* JyNI_ExceptionAlloc(ExceptionMapEntry* eme)
{
	ALLOC_FULL(sizeof(PyVarObject), JY_TRUNCATE_FLAG_MASK, NULL, eme->exc_type)
	INIT_FULL(obj, eme->exc_type, 0)

//	if (PyType_IS_GC(eme->exc_type))
//		_JyNI_GC_TRACK_NoExplore(obj);

	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_InitPyObjectSubtype(jobject src, PyTypeObject* subtype)
{
//	jputs(__FUNCTION__);
//	jputs(subtype->tp_name);
//	JyNI_jprintJ(src);

	PyObject* dest = NULL;
	TypeMapEntry* tme = JyNI_JythonTypeEntry_FromSubType(subtype);
	if (tme)
	{
		if (tme->flags & SYNC_ON_JY_INIT_FLAG_MASK)
		{
			if (tme->sync != NULL && tme->sync->pyInit != NULL)
				dest = tme->sync->pyInit(src, subtype);
			Py_TYPE(dest) = subtype;
		} else
		{
			dest = JyNI_AllocSubtypeVar(subtype, tme, 0);
			if (dest && tme->sync && tme->sync->jy2py) {
//				if (PyType_CheckExact(subtype))
//					((PyTypeObject*) dest)->tp_flags |= Py_TPFLAGS_HEAPTYPE;
				tme->sync->jy2py(src, dest);
			}
		}
	} else
	{
		size_t size = _PyObject_SIZE(subtype);

		ORDINARY_ALLOC_FULL(size, 0, NULL, dest, subtype)

		memset(dest, '\0', size);

		PyObject_INIT(dest, subtype);
	}
	if (dest)
	{
		if (PyType_CheckExact(dest))
		{ // Should not be relevant for subtype case:
			jputs("JyNI-warning: dest is PyTypeObject in JyNI_InitPyObjectSubtype.");
			((PyTypeObject*) dest)->tp_flags |= Py_TPFLAGS_HEAPTYPE;
		}
		JyObject* jy = AS_JY(dest);
		jy->flags |= JY_SUBTYPE_FLAG_MASK;

		env(NULL);
		INIT_JY(jy, tme, src, dest)

	}
	return dest;
}


inline PyTypeObject* JyNI_AllocPyObjectNativeTypePeer(TypeMapEntry* tme, jobject src)
{
//	jputs(__FUNCTION__);
//	jputs(tme->py_type->tp_name);
	PyTypeObject* dest = NULL;
	JyObject* jy;
	dest = (PyTypeObject*) JyNI_Alloc(tme);
	dest->ob_type = dest;
	dest->tp_flags |= Py_TPFLAGS_HEAPTYPE | Py_TPFLAGS_TYPE_SUBCLASS |
			Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE;
	jy = AS_JY(dest);
	env(NULL);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest);
	jy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	tme->sync->jy2py(src, dest);

	INIT_JY(jy, tme, src, dest)

	return dest;
}
