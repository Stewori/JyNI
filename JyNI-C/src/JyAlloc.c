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
	Py_TYPE(obj)->tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

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

#define INIT_JY(_jy, tme, src, dest) \
	if (_jy->flags & SYNC_NEEDED_MASK) \
		JyNI_AddJyAttribute(_jy, JyAttributeSyncFunctions, tme->sync); \
	_jy->jy = (*env)->NewWeakGlobalRef(env, src); \
	if (!(_jy->flags & JY_HAS_JHANDLE_FLAG_MASK)) { /*some sync-on-init methods might already init this */ \
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest); \
		_jy->flags |= JY_HAS_JHANDLE_FLAG_MASK; \
	} \
	/* Take care for already existing Jython-weak references */ \
	INIT_GREF(src, dest) \
	_jy->flags |= JY_INITIALIZED_FLAG_MASK; \
	if (PyObject_IS_GC(dest)) { \
		JyNI_GC_ExploreObject(dest); \
	}
	/* JyNI_GC_EnsureHeadObject not needed here, because object was explored right after
	 * JY_INITIALIZED_FLAG_MASK has been set.
	 */

#define INIT_OBJECT_TME(dest, subtype, alloc_cmd) \
	if (tme->flags & SYNC_ON_JY_INIT_FLAG_MASK) \
	{ \
		if (tme->sync && tme->sync->pyInit) \
			dest = tme->sync->pyInit(src, subtype); \
	} else \
	{ \
		dest = alloc_cmd; \
		if (tme == &builtinTypes[TME_INDEX_Type]) \
			((PyTypeObject*) dest)->tp_flags |= Py_TPFLAGS_HEAPTYPE; \
		jy = AS_JY(dest); \
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest); \
		jy->flags |= JY_HAS_JHANDLE_FLAG_MASK; \
		if (dest && tme->sync && tme->sync->jy2py) { \
			tme->sync->jy2py(src, dest); \
		} \
	}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_InitPyObject(TypeMapEntry* tme, jobject src)
{
//	puts(__FUNCTION__);
//	puts(tme->py_type->tp_name);
	PyObject* dest = NULL;
	JyObject* jy;
	env(NULL);
	INIT_OBJECT_TME(dest, NULL, JyNI_Alloc(tme))
	if (dest)
	{
		jy = AS_JY(dest);
		INIT_JY(jy, tme, src, dest)
	}
	return dest;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_InitPyObjectSubtype(jobject src, PyTypeObject* subtype)
{
//	jboolean dbg = strcmp(subtype->tp_name, "iinfo") == 0;
//	if (dbg) {
//		puts(__FUNCTION__);
//	}
//	jputs(__FUNCTION__);
//	jputs(subtype->tp_name);
//	JyNI_jprintJ(src);

	PyObject* dest = NULL;
	JyObject* jy;
	TypeMapEntry* tme = JyNI_JythonTypeEntry_FromSubType(subtype);
	env(NULL);
	if (tme)
	{
		INIT_OBJECT_TME(dest, subtype, JyNI_AllocSubtypeVar(subtype, tme, 0))
	} else
	{
		size_t size = _PyObject_SIZE(subtype);
		ORDINARY_ALLOC_FULL(size, 0, NULL, dest, subtype)

		memset(dest, '\0', size);

		PyObject_INIT(dest, subtype);
		subtype->tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;
	}
	if (dest)
	{
		Py_TYPE(dest) = subtype;
		if (PyType_CheckExact(dest))
		{ // Should not be relevant for subtype case:
			jputs("JyNI-warning: dest is PyTypeObject in JyNI_InitPyObjectSubtype.");
			((PyTypeObject*) dest)->tp_flags |= Py_TPFLAGS_HEAPTYPE;
		}
		JyObject* jy = AS_JY(dest);
		jy->flags |= JY_SUBTYPE_FLAG_MASK;
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
	dest->tp_free = PyBaseObject_Type.tp_free;
	dest->tp_dealloc = PyBaseObject_Type.tp_dealloc;

	jy = AS_JY(dest);
	env(NULL);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest);
	jy->flags |= JY_HAS_JHANDLE_FLAG_MASK | JY_SUBTYPE_FLAG_MASK;// | JY_INITIALIZED_FLAG_MASK;
	//jy->jy = (*env)->NewWeakGlobalRef(env, src);

	/*
	 * Should sync be done before or after INIT_JY?
	 * Pro-before: Delegation during sync feasible
	 * Pro-after:  GC-exploration can take synced values into account
	 * We could split INIT_JY to achieve both "benefits", but since
	 * delegation during sync is currently not applied we simply sync before.
	 */
	tme->sync->jy2py(src, dest);
	//jy->flags |= JY_SUBTYPE_FLAG_MASK;
	INIT_JY(jy, tme, src, dest)
//	tme->sync->jy2py(src, dest);

	/*
	 * lookup in typeobject.c causes problems due to missing delegation.
	 * Adding delegation there causes other issues and currently breaks
	 * more than it fixes. So for now we just clear the error here.
	 * See out-commented delegation code in typeobject.c/lookup_maybe
	 * and typeobject.c/_PyType_Lookup
	 * Probable cause: During sync, bases are converted before mro and
	 * during bases conversion something tries an mro-based lookup.
	 * Todo: Why does lookup-delegation break stuff?
	 */
	PyErr_Clear();

	return dest;
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 *
 * Not intended for Heap-Type exceptions.
 * These don't have an associated ExceptionMapEntry anyway.
 */
inline PyObject* JyNI_InitPyException(ExceptionMapEntry* eme, jobject src)
{
	PyObject* obj = JyNI_ExceptionAlloc(eme);
	if (obj == NULL) return PyErr_NoMemory();
	JyObject* jy = AS_JY(obj);
	env(NULL);
	jy->jy = (*env)->NewWeakGlobalRef(env, src);
	//if (jy->flags & JY_HAS_JHANDLE_FLAG_MASK == 0) { //Always true here
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) obj);//, jy->flags & JY_TRUNCATE_FLAG_MASK);
	jy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	if (PyType_IS_GC(eme->exc_type))
		JyNI_GC_ExploreObject(obj);
	/* JyNI_GC_EnsureHeadObject not needed here, because object was explored right after
	 * JY_INITIALIZED_FLAG_MASK has been set.
	 */
	return obj;
}


inline PyTypeObject* JyNI_InitPyObjectNativeTypePeer(jobject srctype)
{
//	jputs(__FUNCTION__);
//	JyNI_jprintJ(srctype);
	env(NULL);
	jstring jName = (*env)->GetObjectField(env, srctype, pyTypeNameField);
	cstr_from_jstring(cName, jName);
//	jputs(cName);
	PyTypeObject* dest =
			JyNI_AllocPyObjectNativeTypePeer(&(builtinTypes[TME_INDEX_Type]), srctype);
//	JyObject* jy = AS_JY(dest);
	// Add flag to enforce delegation to Java:
	//(is now done in JyNI_AllocPyObjectNativeTypePeer)
//	jy->flags |= JY_SUBTYPE_FLAG_MASK;

	return dest;
}


#define SUBTYPE_ERROR(src, tme) \
	jputs(__FUNCTION__); \
	jputs("called with inconsistent args!"); \
	jPrintCStackTrace(); \
	jputs("Subtype:"); \
	jputs(Py_TYPE(src)->tp_name); \
	jputs("Mapping-type:"); \
	jputs(tme->py_type->tp_name); \
	PyErr_BadInternalCall();

#define SUBTYPE_J_INIT(src, dest, tme) \
	if (!tme->jy_subclass || !PyType_IsSubtype(Py_TYPE(src), tme->py_type)) \
	{ \
		SUBTYPE_ERROR(src, tme) \
	} \
	/* Native subtype-case. We handle native subtype case always with CPeer. */ \
	Py_INCREF(src); \
	if (tme->flags & SYNC_ON_JY_INIT_FLAG_MASK) \
	{ \
		if (tme->sync && tme->sync->jyInit) \
			dest = tme->sync->jyInit(src, tme->jy_subclass); \
	} else \
	{ \
		jobject jsrcType = JyNI_JythonPyTypeObject_FromPyTypeObject(Py_TYPE(src)); \
		jmethodID subconst = (*env)->GetMethodID(env, tme->jy_subclass, "<init>", \
				"(JLJyNI/PyCPeerType;)V"); \
		dest = (*env)->NewObject(env, tme->jy_subclass, subconst, (jlong) src, jsrcType); \
		if (tme->sync && tme->sync->py2jy) \
			tme->sync->py2jy(src, dest); \
	}

#define J_INIT(src, dest, tme) \
	if (Py_TYPE(src) != tme->py_type) { \
		SUBTYPE_J_INIT(src, dest, tme) \
	} else if (tme->flags & SYNC_ON_JY_INIT_FLAG_MASK) \
	{ \
		if (tme->sync && tme->sync->jyInit) \
			dest = tme->sync->jyInit(src, NULL); \
	} else \
	{ \
		jmethodID cm = (*env)->GetMethodID(env, tme->jy_class, "<init>", "()V"); \
		if (cm) \
		{ \
			dest = (*env)->NewObject(env, tme->jy_class, cm); \
			if (tme->sync && tme->sync->py2jy) \
				tme->sync->py2jy(src, dest); \
		} \
	}


inline jobject JyNI_InitStaticJythonPyObject(PyObject* src)
{
//	jputs(__FUNCTION__);
//	jputs(type->tp_name);
	env(NULL);
	//setup and return PyCPeer in this case...
	jobject er = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupCPeerFromHandle, (jlong) src);
	if (er != NULL && !(*env)->IsSameObject(env, er, NULL)) return er;
	else {
		TypeMapEntry* tme = JyNI_JythonTypeEntry_FromSubType(Py_TYPE(src));
		if (tme)
		{
			J_INIT(src, er, tme)
		} else
		{
			er = (*env)->NewObject(env, pyCPeerClass, pyCPeerConstructor, (jlong) src,
					JyNI_JythonPyObject_FromPyObject(Py_TYPE(src)));
		}
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, er, (jlong) src);
		return er;
	}
}


/*
 * Returns a JNI LocalRef.
 */
inline jobject JyNI_InitJythonPyObject(TypeMapEntry* tme, PyObject* src, JyObject* srcJy)
{
	//jputs(__FUNCTION__);
	jobject dest = NULL;
	env(NULL);

	J_INIT(src, dest, tme)
	if (!dest) return NULL;
	if (Py_TYPE(src) != tme->py_type) {
		// Set flags for subtype-case...
		srcJy->flags |= JY_CPEER_FLAG_MASK;
		srcJy->flags |= JY_SUBTYPE_FLAG_MASK;
	}
	if (dest && (srcJy->flags & SYNC_NEEDED_MASK))
		JyNI_AddJyAttribute(srcJy, JyAttributeSyncFunctions, tme->sync);
	srcJy->jy = (*env)->NewWeakGlobalRef(env, dest);
	if (!(srcJy->flags & JY_HAS_JHANDLE_FLAG_MASK)) {  //some sync-on-init methods might already init this
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, dest, (jlong) src);
		srcJy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	}
	srcJy->flags |= JY_INITIALIZED_FLAG_MASK;

//	if (tme->flags & JY_CPEER_FLAG_MASK)
//	{
//		//JyNI_GC_ExploreObject(src);
//		Py_INCREF(src);
//	}
//	if (srcJy->flags & JY_CPEER_FLAG_MASK)
//	{
//		puts("JyNI-Alert: CPeer in Alloc");
//		puts(__FUNCTION__);
//		puts(Py_TYPE(src)->tp_name);
////		Py_INCREF(src);
//	}

	if ((tme->flags & JY_TRUNCATE_FLAG_MASK) && !(srcJy->flags & JY_CACHE_ETERNAL_FLAG_MASK)) {
		/* we create GC-head here to secure the Java-side backend from
		 * gc until a proper exploration of the owner takes place. */
		if (!JyNI_GC_EnsureHeadObject(env, src, srcJy)) {
			JyNI_GC_Track_CStub(src);
		}
	}
	return dest;
}
