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

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * Should do the same as JyNI_AllocVar with nitems == -1.
 */
inline PyObject* JyNI_Alloc(TypeMapEntry* tme)
{
	PyObject *obj;
	//size_t size = _PyObject_VAR_SIZE(type, nitems+1);
	size_t size = (tme->flags & JY_TRUNCATE_FLAG_MASK) ? sizeof(PyObject)+tme->truncate_trailing : _PyObject_SIZE(tme->py_type);

	if (PyType_IS_GC(tme->py_type))
	{
//		jputs("alloc gc");
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		AS_JY_WITH_GC(obj)->flags |= tme->flags;
	} else
	{
//		jputs("ordinary alloc");
		JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->jy = (jobject) tme; //tme->jy_class;
		jy->flags = tme->flags;
		jy->attr = NULL;
		obj = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, obj, jy, size+sizeof(JyObject), tme->py_type->tp_name);
	}

	//if (obj == NULL) return PyErr_NoMemory();

	memset(obj, '\0', size);

	/* We cannot use PyType_Check here, because obj->ob_type might not be fully
	 * initialized and not yet recognized as a Type-subclass.
	 * (e.g. <type 'java.lang.Class'> from Jython-side can cause problems here)
	 */
	if (tme == &builtinTypes[TME_INDEX_Type])
		((PyTypeObject*) obj)->tp_flags |= Py_TPFLAGS_HEAPTYPE;

	if (tme->py_type->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(tme->py_type);

	//In contrast to var variant of this method, no decision needed here:
	PyObject_INIT(obj, tme->py_type);
//	if (AS_JY(obj) == AS_JY_WITH_GC(obj)) jputs("confirm gc");
//	else jputs("confirm no-gc");
	if (PyType_IS_GC(tme->py_type))
		_JyNI_GC_TRACK(obj);
	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_AllocVar(TypeMapEntry* tme, Py_ssize_t nitems)
{
//	jputs(__FUNCTION__);
//	jputs(tme->py_type->tp_name);
	PyObject *obj;
	//size_t size;// = _PyObject_VAR_SIZE(type, nitems+1);
	size_t size = (tme->flags & JY_TRUNCATE_FLAG_MASK) ? sizeof(PyVarObject)+tme->truncate_trailing : _PyObject_VAR_SIZE(tme->py_type, nitems+1);
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

	if (PyType_IS_GC(tme->py_type))
	{
//		jputs("alloc gc var");
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		AS_JY_WITH_GC(obj)->flags |= tme->flags;
	} else
	{
//		jputs("ordinary alloc var");
		JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->jy = (jobject) tme; //tme->jy_class;
		jy->flags = tme->flags;
		jy->attr = NULL;
		obj = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, obj, jy, size+sizeof(JyObject), tme->py_type->tp_name);
	}

	//if (obj == NULL) return PyErr_NoMemory();

	memset(obj, '\0', size);

	if (tme->py_type->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(tme->py_type);

	if (tme->py_type->tp_itemsize == 0)
		PyObject_INIT(obj, tme->py_type);
	else
		PyObject_INIT_VAR((PyVarObject *)obj, tme->py_type, nitems);
//	if (AS_JY(obj) == AS_JY_WITH_GC(obj)) jputs("confirm gc");
//	else jputs("confirm no-gc");
	if (PyType_IS_GC(tme->py_type))
		_JyNI_GC_TRACK(obj);
	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_AllocSubtypeVar(PyTypeObject* subtype, TypeMapEntry* tme, Py_ssize_t nitems)
{
//	jputs(__FUNCTION__);
	PyObject *obj;
//	size_t typesize = _PyObject_VAR_SIZE(tme->py_type, nitems+1);
//	size_t size = (tme->flags & JY_TRUNCATE_FLAG_MASK) ? sizeof(PyVarObject)+tme->truncate_trailing : typesize;
//	size += _PyObject_VAR_SIZE(subtype, nitems+1)-typesize;
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
//	jputs(subtype->tp_name);
	size_t size = _PyObject_VAR_SIZE(subtype, nitems+1);
	/* note that we need to add one, for the sentinel */
	if (PyType_IS_GC(subtype))
	{
//		jputs("alloc gc subtype");
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		AS_JY_WITH_GC(obj)->flags |= tme->flags;
	} else
	{
//		jputs("ordinary alloc subtype");
		JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->jy = (jobject) tme;
		jy->flags = tme->flags;
		jy->attr = NULL;
		obj = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, obj, jy, size+sizeof(JyObject), tme->py_type->tp_name);
	}

	//if (obj == NULL) return PyErr_NoMemory();

	memset(obj, '\0', size);

	if (subtype->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(subtype);

	if (subtype->tp_itemsize == 0)
		PyObject_INIT(obj, subtype);
	else
		PyObject_INIT_VAR((PyVarObject *)obj, subtype, nitems);

	if (PyType_Check(obj))
		((PyTypeObject*) obj)->tp_flags |= Py_TPFLAGS_HEAPTYPE;

//	if (AS_JY(obj) == AS_JY_WITH_GC(obj)) jputs("confirm gc");
//	else jputs("confirm no-gc");

	if (PyType_IS_GC(subtype))
		_JyNI_GC_TRACK(obj);
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
	PyObject *obj;
	size_t size = _PyObject_VAR_SIZE(type, nitems+1);
	/* note that we need to add one, for the sentinel */
	//printf("JyNI_AllocNativeVar %s of size %d\n", type->tp_name, size);
//	jputs(__FUNCTION__);
//	jputs(type->tp_name);
	if (PyType_IS_GC(type))
	{
//		jputs("alloc native gc var");
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		JyObject* jy = AS_JY_WITH_GC(obj);
		jy->flags |= JY_CPEER_FLAG_MASK;
	} else
	{
//		jputs("ordinary alloc native var");
		JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->flags = JY_CPEER_FLAG_MASK;
		jy->attr = NULL;
		jy->jy = NULL;
		obj = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, obj, jy, size+sizeof(JyObject), type->tp_name);
	}

	memset(obj, '\0', size);

	if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(type);

	if (type->tp_itemsize == 0)
		PyObject_INIT(obj, type);
	else
		(void) PyObject_INIT_VAR((PyVarObject *)obj, type, nitems);

	if (PyType_Check(obj))
		((PyTypeObject*) obj)->tp_flags |= Py_TPFLAGS_HEAPTYPE;
//	if (AS_JY(obj) == AS_JY_WITH_GC(obj)) jputs("confirm gc");
//	else jputs("confirm no-gc");
	if (PyType_IS_GC(type))
		_JyNI_GC_TRACK(obj);
		//_JyNI_GC_TRACK_NoExplore(obj);

	return obj;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_InitPyObjectSubtype(jobject src, PyTypeObject* subtype)
{
//	jputs(__FUNCTION__);
//	jputs(subtype->tp_name);
//	jputsLong(subtype->tp_flags);
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
			//dest = PyObject_GC_New(tme->py_type);
			//puts("InitPyObject by GC New");

			//dest = tme->py_type->tp_itemsize ? JyNI_AllocVar(tme) : JyNI_Alloc(tme);
			//dest = JyNI_Alloc(tme);
			dest = JyNI_AllocSubtypeVar(subtype, tme, 0);

			//printf("PyObject-size: %u\n", (jlong) sizeof(PyObject));
			//PyObject_GC_Track(dest);
			if (dest && tme->sync && tme->sync->jy2py) {
//				if (PyType_CheckExact(subtype))
//					((PyTypeObject*) dest)->tp_flags |= Py_TPFLAGS_HEAPTYPE;
				tme->sync->jy2py(src, dest);
			}
		}
	} else
	{
//		jputs("object- or unknown subtye!");
//		jputs(subtype->tp_name);

		size_t size = _PyObject_SIZE(subtype);
		JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->jy = NULL;
		jy->flags = 0;
		jy->attr = NULL;
		dest = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, dest, jy, size+sizeof(JyObject), subtype->tp_name);

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
		if (jy->flags & SYNC_NEEDED_MASK)
			JyNI_AddJyAttribute(jy, JyAttributeSyncFunctions, tme->sync);
		env(NULL);
		jy->jy = (*env)->NewWeakGlobalRef(env, src);
		if (!(jy->flags & JY_HAS_JHANDLE_FLAG_MASK)) { //some sync-on-init methods might already init this
			(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest);//, jy->flags & JY_TRUNCATE_FLAG_MASK);
			jy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
		}

		/* Take care for already existing Jython-weak references */
		jobject gref = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_getGlobalRef, src);
		if (gref && (*env)->IsInstanceOf(env, gref, GlobalRefClass)) {
			gref = (*env)->CallObjectMethod(env, gref, GlobalRef_retryFactory);
		}
		if (gref) {
			Py_INCREF(dest);
			incWeakRefCount(jy);
			(*env)->CallVoidMethod(env, gref, JyNIGlobalRef_initNativeHandle, (jlong) dest);
		}

		// Add flag to enforce delegation to Java:
		jy->flags |= JY_SUBTYPE_FLAG_MASK;
		jy->flags |= JY_INITIALIZED_FLAG_MASK;
		if (PyObject_IS_GC(dest)) {
			JyNI_GC_ExploreObject(dest);
		}
		/* JyNI_GC_EnsureHeadObject not needed here, because object was explored right after
		 * JY_INITIALIZED_FLAG_MASK has been set.
		 */
	}
	return dest;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 *
 * Not intended for Heap-Type exceptions.
 * These don't have an associated ExceptionMapEntry anyway.
 */
inline PyObject* JyNI_ExceptionAlloc(ExceptionMapEntry* eme)
{
	PyObject *obj;
	//if (eme == NULL) puts("eme is NULL");
	//if (eme->exc_type == NULL) puts("eme type is NULL");
	if (PyType_IS_GC(eme->exc_type))
	{
		obj = _PyObject_GC_Malloc(sizeof(PyVarObject));
		//if (AS_JY_WITH_GC(obj) == NULL) return PyErr_NoMemory();
		if (obj == NULL) return PyErr_NoMemory();
	} else
	{
		JyObject* jy = (JyObject *) PyObject_RawMalloc(sizeof(PyVarObject)+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->flags = JY_TRUNCATE_FLAG_MASK;
		jy->attr = NULL;
		obj = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, obj, jy, sizeof(PyVarObject)+sizeof(JyObject), eme->exc_type->tp_name);
	}

	//if (obj == NULL) return PyErr_NoMemory();

	memset(obj, '\0', sizeof(PyVarObject));

	if (eme->exc_type->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(eme->exc_type);

	if (eme->exc_type->tp_itemsize == 0)
		PyObject_INIT(obj, eme->exc_type);
	else
		//(void) PyObject_INIT_VAR((PyVarObject *)obj, eme->exc_type, 0);//nitems);
		PyObject_INIT_VAR((PyVarObject *)obj, eme->exc_type, 0);

	if (PyType_IS_GC(eme->exc_type))
		_JyNI_GC_TRACK_NoExplore(obj);
	return obj;
}


inline PyTypeObject* JyNI_AllocPyObjectNativeTypePeer(TypeMapEntry* tme, jobject src)
{
//	jputs(__FUNCTION__);
//	jputs(tme->py_type->tp_name);
	PyTypeObject* dest = NULL;
	JyObject* jy;
	env(NULL);
	dest = (PyTypeObject*) JyNI_Alloc(tme);
	dest->ob_type = dest;
	dest->tp_flags |= Py_TPFLAGS_HEAPTYPE | Py_TPFLAGS_TYPE_SUBCLASS | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE;
	jy = AS_JY(dest);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest);//, jy->flags & JY_TRUNCATE_FLAG_MASK);
	jy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	tme->sync->jy2py(src, dest);

	if (jy->flags & SYNC_NEEDED_MASK)
		JyNI_AddJyAttribute(jy, JyAttributeSyncFunctions, tme->sync);
	jy->jy = (*env)->NewWeakGlobalRef(env, src);
	if (!(jy->flags & JY_HAS_JHANDLE_FLAG_MASK)) { //some sync-on-init methods might already init this
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest);//, jy->flags & JY_TRUNCATE_FLAG_MASK);
		jy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	}

	/* Take care for already existing Jython-weak references */
	jobject gref = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_getGlobalRef, src);
	if (gref && (*env)->IsInstanceOf(env, gref, GlobalRefClass)) {
		gref = (*env)->CallObjectMethod(env, gref, GlobalRef_retryFactory);
	}
	if (gref) {
		//jputs("native call to JyNIGlobalRef_initNativeHandle");
		Py_INCREF(dest);
		incWeakRefCount(jy);
		(*env)->CallVoidMethod(env, gref, JyNIGlobalRef_initNativeHandle, (jlong) dest);
	}

	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	if (PyObject_IS_GC(dest)) {
		JyNI_GC_ExploreObject(dest);
	}
	/* JyNI_GC_EnsureHeadObject not needed here, because object was explored right after
	 * JY_INITIALIZED_FLAG_MASK has been set.
	 */
	return dest;
}
