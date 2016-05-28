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


/*
 * JyNI.c
 *
 *  Created on: 14.03.2013, 00:49:46
 *	  Author: Stefan Richthofer
 */

#include <JyNI.h>
#include <JyAlloc.h>
#include <JySync.h>
#include <code_JyNI.h>
//#include <JyNI_JyNI.h>
//#include "frameobject.h"
#include "importdl.h"
//#include <dlfcn.h>
//#include "stringlib/string_format.h"

const char* excPrefix = "exceptions.";
//jlong JyNIDebugMode = 0;

TypeMapEntry builtinTypes[builtinTypeCount];
//static TypeMapEntry specialPyInstance;

#define builtinExceptionCount 50
//PyTypeObject* builtinExceptions[builtinExceptionCount];
ExceptionMapEntry builtinExceptions[builtinExceptionCount];
int (*PyOS_InputHook)(void) = NULL;

/*
 * Class:     JyNI_JyNI
 * Method:    loadModule
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)Lorg/python/core/PyModule;
 */
jobject JyNI_loadModule(JNIEnv *env, jclass class, jstring moduleName, jstring modulePath, jlong tstate)
{
	//jputs("JyNI_loadModule...");
	//JyNI_jprintJ(moduleName);
//	jputsLong(tstate);
	RE_ENTER_JyNI
	if (PyErr_Occurred()) jputs("PyErrOccured01 (beginning of JyNI_loadModule)");//this should never happen!
	const char* utf_string;
	utf_string = (*env)->GetStringUTFChars(env, moduleName, NULL);
	//"+1" for 0-termination:
	char mName[strlen(utf_string)+1];
	strcpy(mName, utf_string);
	(*env)->ReleaseStringUTFChars(env, moduleName, utf_string);

	jobject er;

	if (!(*env)->IsSameObject(env, modulePath, NULL)) {
		utf_string = (*env)->GetStringUTFChars(env, modulePath, NULL);
		//"+1" for 0-termination:
		char mPath[strlen(utf_string)+1];
		strcpy(mPath, utf_string);
		(*env)->ReleaseStringUTFChars(env, moduleName, utf_string);
		FILE *fp;
		fp = fopen(mPath, "r" PY_STDIOTEXTMODE);
		if (fp == NULL)
			//PyErr_SetFromErrno(PyExc_IOError);
			jputs("some error happened opening the file");

		er = _PyImport_LoadDynamicModuleJy(mName, mPath, fp);
		if (fclose(fp)) jputs("Some error occurred on file close");
	} else
		// Attempt to load a module statically linked into JyNI.so:
		er = _PyImport_LoadDynamicModuleJy(mName, NULL, NULL);

	RE_LEAVE_JyNI
	return er;
}

/*
 * Class:	 JyNI_JyNI
 * Method:	clearPyCPeer
 * Signature: (JJ)V
 */
void JyNI_clearPyCPeer(JNIEnv *env, jclass class, jlong objectHandle, jlong refHandle)
{
	/* No ENTER_JyNI/LEAVE_JyNI necessary here, because no code is executed that
	 * might contain Py_BEGIN_ALLOW_THREADS/Py_END_ALLOW_THREADS.
	 */
	//jputs("JyNI_clearPyCPeer: acquire lock");
	PyEval_AcquireLock();
	if (objectHandle) Py_DECREF((PyObject*) objectHandle);
	if (refHandle) (*env)->DeleteWeakGlobalRef(env, (jweak) refHandle);
	else {
		JyObject* jy = AS_JY((PyObject*) objectHandle);
		(*env)->DeleteWeakGlobalRef(env, jy->jy);
	}
	PyEval_ReleaseLock();
	//jputs("JyNI_clearPyCPeer: release lock");
	//maybe check for ref-count == 0 here and free non-gc objects...
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNIDebugMessage
 * Signature: (JJLjava/lang/String;)V
 */
void JyNI_JyNIDebugMessage(JNIEnv *env, jclass class, jlong mode, jlong value, jstring msg)
{
	//for future use...
//	JyNIDebugMode = mode;
}

/*
 * Class:     JyNI_JyNI
 * Method:    callPyCPeer
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
jobject JyNI_callPyCPeer(JNIEnv *env, jclass class, jlong peerHandle, jobject args, jobject kw, jlong tstate)
{
//	jputs(__FUNCTION__);
	//note: here should be done sync
	//(maybe sync-idea is obsolete anyway)
	PyObject* peer = (PyObject*) peerHandle;
//	jputs(Py_TYPE(peer)->tp_name);
//	int jdbg = strcmp(Py_TYPE(peer)->tp_name, "_ctypes.PyCFuncPtrType") == 0;
//	if (!peer->ob_type) jputs("ob_type of peer is NULL");
	ENTER_JyNI
	jobject er;
//	jputs("JyNI_call error before call?");
//	jputsLong(PyErr_Occurred());
	if (peer->ob_type->tp_call) {
		PyObject* jargs = JyNI_PyObject_FromJythonPyObject(args);
//		if (jdbg) {
//			jputsLong(jargs);
//			jputsLong(PyTuple_GET_SIZE(jargs));
//			jputsLong(PyTuple_GET_ITEM(jargs, 0));
//		}
		PyObject* jkw = JyNI_PyObject_FromJythonPyObject(kw);
		PyObject* jres = peer->ob_type->tp_call(peer, jargs, jkw);
		er = JyNI_JythonPyObject_FromPyObject(jres);
		Py_XDECREF(jargs);
		Py_XDECREF(jkw);
		Py_XDECREF(jres);
	} else {
//		PyErr_Format(PyExc_TypeError, "'%.200s' object is not callable",
//				peer->ob_type->tp_name);
		er = NULL;
	}
	LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    getAttrString
 * Signature: (JLjava/lang/String;J)Lorg/python/core/PyObject;
 */
jobject JyNI_getAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jlong tstate)
{
	//printf("JyNI_getAttrString %i\n", tstate);
//	jputs("JyNI_getAttrString");
//	jputsLong(handle);
	if (handle == 0) return NULL;
	cstr_from_jstring(cName, name);
//	jputs(cName);
	ENTER_JyNI
	//jint ensresult = (*env)->EnsureLocalCapacity(env, 100);
	//jputs("ensresult:");
	//jputsLong(ensresult);
	//jputs("JyNI_getAttrString-Handle:");
	//jputsLong(handle);
	PyObject* jres = PyObject_GetAttrString((PyObject*) handle, cName);
	//jputs("JyNI_getAttrString-Result:");
	//jputsLong(jres);
	jobject er = JyNI_JythonPyObject_FromPyObject(jres);//PyObject_GetAttrString((PyObject*) handle, cName));
	Py_XDECREF(jres);
	LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    setAttrString
 * Signature: (JLjava/lang/String;Lorg/python/core/PyObject;J)I
 */
jint JyNI_setAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jobject value, jlong tstate)
{
	if (handle == 0) return 0;
	cstr_from_jstring(cName, name);
	ENTER_JyNI
	PyObject* vl = JyNI_PyObject_FromJythonPyObject(value);
	jint er = PyObject_SetAttrString((PyObject*) handle, cName, vl);
	Py_XDECREF(vl);
	LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    repr
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
jobject JyNI_repr(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* res = PyObject_Repr((PyObject*) handle);
	jobject er = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(res);
	RE_LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    getItem
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
jobject JyNI_getItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jlong tstate)
{
//	jputs(__FUNCTION__);
//	if (handle) jputs(Py_TYPE((PyObject*) handle)->tp_name);
//	else jputs("Handle is NULL");
	ENTER_JyNI
//	jputsLong(__LINE__);
	PyObject* pkey = JyNI_PyObject_FromJythonPyObject(key);
//	jputsLong(__LINE__);
//	if (pkey) jputs(Py_TYPE(pkey)->tp_name);
//	else jputs("pkey is NULL");
	PyObject* res = PyObject_GetItem((PyObject*) handle, pkey);
//	jputsLong(__LINE__);
//	if (res) jputs(Py_TYPE(res)->tp_name);
//	else jputs("res is NULL");
	jobject er = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(pkey);
	Py_XDECREF(res);
	LEAVE_JyNI
//	jputsLong(__LINE__);
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    setItem
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)I
 */
jint JyNI_setItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jobject value, jlong tstate)
{
//	jputs(__FUNCTION__);
	ENTER_JyNI
	PyObject* pkey = JyNI_PyObject_FromJythonPyObject(key);
	PyObject* pval = JyNI_PyObject_FromJythonPyObject(value);
	jint er = PyObject_SetItem((PyObject*) handle, pkey, pval);
	Py_XDECREF(pkey);
	Py_XDECREF(pval);
	LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    delItem
 * Signature: (JLorg/python/core/PyObject;J)I
 */
jint JyNI_delItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jlong tstate)
{
	ENTER_JyNI
	PyObject* pkey = JyNI_PyObject_FromJythonPyObject(key);
	jint er = PyObject_DelItem((PyObject*) handle, pkey);
	Py_XDECREF(pkey);
	LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectLength
 * Signature: (JJ)I
 */
jint JyNI_PyObjectLength
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	ENTER_JyNI
	jint er = PyObject_Size((PyObject*) handle);
	LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    descr_get
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL JyNI_descr_get
  (jlong self, jobject obj, jobject type, jlong tstate)
{
	ENTER_JyNI
	jobject res;
	if (Py_TYPE((PyObject*) self)->tp_descr_get) {
		PyObject* pyobj = JyNI_PyObject_FromJythonPyObject(obj);
		PyObject* pytype = JyNI_PyObject_FromJythonPyObject(type);
		PyObject* pyres = Py_TYPE((PyObject*) self)->tp_descr_get((PyObject*) self, pyobj, pytype);
		res = JyNI_JythonPyObject_FromPyObject(pyres);
		Py_XDECREF(pyobj);
		Py_XDECREF(pytype);
		Py_XDECREF(pyres);
	} else
		res = NULL;
	LEAVE_JyNI
	return res;
}

/*
 * Class:     JyNI_JyNI
 * Method:    descr_set
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL JyNI_descr_set
  (jlong self, jobject obj, jobject value, jlong tstate)
{
	ENTER_JyNI
	jint res;
	if (Py_TYPE((PyObject*) self)->tp_descr_set) {
		PyObject* pyobj = JyNI_PyObject_FromJythonPyObject(obj);
		PyObject* pyval = JyNI_PyObject_FromJythonPyObject(value);
		res = Py_TYPE((PyObject*) self)->tp_descr_set((PyObject*) self, pyobj, pyval);
		Py_XDECREF(pyobj);
		Py_XDECREF(pyval);
	} else
		res = JyNI_JyNI_NATIVE_INT_METHOD_NOT_IMPLEMENTED;
	LEAVE_JyNI
	return res;
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsString
 * Signature: (JJ)Ljava/lang/String;
 */
jstring JyNI_PyObjectAsString(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	//jputs("JyNI_PyObjectAsString");
	if (!((PyObject*) handle)->ob_refcnt) return NULL;
	RE_ENTER_JyNI
	PyStringObject* ps = (PyStringObject*) PyObject_Str((PyObject*) handle);
	jstring er = JyNI_jstring_FromPyStringObject(env, ps);
	Py_XDECREF(ps);
	RE_LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsPyString
 * Signature: (JJ)Lorg/python/core/PyString;
 */
jobject JyNI_PyObjectAsPyString(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	//jputs("JyNI_PyObjectAsPyString");
	ENTER_JyNI
	PyObject* res = PyObject_Str((PyObject*) handle);
	jobject er = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(res);
	LEAVE_JyNI
	return er;
}

/*
 * Class:     JyNI_JyNI
 * Method:    lookupFromHandle
 * Signature: (J)Lorg/python/core/PyObject;
 */
jobject JyNIlookupFromHandle(JNIEnv *env, jclass class, jlong handle)
{
	/*Note: If this method crashes the JVM, it is most likely because
	 *      it was called with an already freed handle.
	 */
	if (!handle) return NULL;
	if (Is_Static_PyObject((PyObject*) handle)) return NULL;
	JyObject* jop = AS_JY((PyObject*) handle);
	if (jop->flags & JY_INITIALIZED_FLAG_MASK) return jop->jy;
	return NULL;
}

/*
 * Class:     JyNI_JyNI
 * Method:    currentNativeRefCount
 * Signature: (J)I
 */
jint JyNIcurrentNativeRefCount(JNIEnv *env, jclass class, jlong handle)
{
	return (jint) ((PyObject*) handle)->ob_refcnt;
}

/*
 * Class:     JyNI_JyNI
 * Method:    nativeIncref
 * Signature: (JJ)I
 */
void JyNI_nativeIncref(jlong handle, jlong tstate)
{
	RE_ENTER_JyNI
	Py_INCREF((PyObject*) handle);
	RE_LEAVE_JyNI
}

/*
 * Class:     JyNI_JyNI
 * Method:    nativeDecref
 * Signature: (JJ)I
 */
void JyNI_nativeDecref(jlong handle, jlong tstate)
{
	RE_ENTER_JyNI
	Py_DECREF((PyObject*) handle);
	RE_LEAVE_JyNI
}

/*
 * Class:     JyNI_JyNI
 * Method:    getNativeTypeName
 * Signature: (J)Ljava/lang/String;
 */
jstring JyNIgetNativeTypeName(JNIEnv *env, jclass class, jlong handle)
{
	if (handle) {
		return (*env)->NewStringUTF(env, Py_TYPE((PyObject*) handle)->tp_name);
	} else
		return NULL;
}

/*
 * Class:     JyNI_JyNI
 * Method:    releaseWeakReferent
 * Signature: (JJ)V
 */
void JyNI_releaseWeakReferent(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	ENTER_JyNI
	decWeakRefCount(AS_JY((PyObject*) handle));
	Py_DECREF(handle);
	LEAVE_JyNI
}

//PySys_GetObject creates new object
/*Builtin types not relevant for Jython (?):
EncodingMapType,
wrappertype,
cmpwrapper_type,
sortwrapper_type,
Long_InfoType,
FloatInfoType,
_PyWeakref_RefType,
_PyWeakref_ProxyType,
_PyWeakref_CallableProxyType,
_struct_sequence_template
PyGetSetDescr_Type;
PyMemberDescr_Type;
PyWrapperDescr_Type;
// PyDictIterKey_Type;
// PyDictIterValue_Type;
// PyDictIterItem_Type;
// PyDictKeys_Type;
// PyDictItems_Type;
// PyDictValues_Type;
// PyReversed_Type;
PyFormatterIter_Type;
PyFieldNameIter_Type;

//Types currently not cared for, but of potential relevance:
PyListIter_Type
PyListRevIter_Type
// PyClassMethod_Type
// PyStaticMethod_Type
PyMethodDescr_Type
PyClassMethodDescr_Type
PyDictProxy_Type
PyProperty_Type
PyBaseString_Type
// PySeqIter_Type
// PyRange_Type
// Pyrangeiter_Type
PyTupleIter_Type
// PySetIter_Type
// PyEnum_Type
// PyGen_Type
// PyFrame_Type
// PySuper_Type
// PyBuffer_Type
// PyMemoryView_Type
PyBaseObject_Type
// PyCallIter_Type

//maybe we can later support this via PyArray:
PyByteArrayIter_Type;
*/

inline void initBuiltinTypes()
{
	/*
	 * JyNI GC-Notes:
	 *
	 * CStubs' heads should not contain links to gc-heads unless
	 * it is only partly truncated and a gc-relevant native
	 * PyObject-reference remains. Only a link to the PyObject
	 * should exist.
	 *
	 * SimpleGCHeads should be replaced by ordinary ones to save
	 * re-creation time if native objects are passed to Java.
	 *
	 * If weak refs are used to track collection of heads, the
	 * handle must be contained in the ref, maybe it's feasible
	 * to have it *only* there.
	 *
	 * Non-traversable native objects only need gc-heads in
	 * GIL-free mode.
	 *
	 * Mirrored objects may have gc-heads even in GIL-mode to
	 * let a SoftReference cache the Java-side object.
	 * Traversable mirrored objects must have their gchead
	 * track references of the native object. The mirrored
	 * Java-PyObject shall be SoftReferenced.
	 *
	 * Special-case PyList:
	 * Let JyList implement TraversableGCHead and PyObjectGCHead.
	 * Basically the situation is the same as with PyCPeerGC, but
	 * with JyList fulfilling the role of gc-head. The JyList should
	 * also cause an incref to the native counterpart. This replaces
	 * the current immortality incref in JySync.c.
	 * Maintain a SoftReference to the PyList and gc-head links
	 * to all native traversable elements.
	 * Further reason whether
	 * - to use element-access to check if heads are up to date
	 * - to use gc-heads as cache of natively saved objects, i.e.
	 *   saving some JNI-calls.
	 *
	 * Special-case PyCell:
	 * Like PyList, but only with one element.
	 *
	 * Special-case PySet/PyFrozenSet:
	 * Note that size-field sync currently might not be thread-safe.
	 * Todo: Use native setter to inject the JySet into the PySet.
	 * GC-situation is like CStub. Consider to not even mark it as
	 * special case, but let it be treated in line with ordinary
	 * CStubs/truncated objects.
	 *
	 * Later the collect_generations-method will also look for objects
	 * inserted for tracking into the ordinary gc-head-list. This is in
	 * case an extension uses the old _PyObject_GC_TRACK-macro (not used
	 * by JyNI any more).
	 */
//	int i;
//	for (i = 0; i < builtinTypeCount; ++i)
//	{
//		builtinTypes[i].py_type = NULL;
//		builtinTypes[i].jy_class = NULL;
//		builtinTypes[i].flags = 0;
//		builtinTypes[i].sync = NULL;
//		builtinTypes[i].truncate_trailing = 0;
//		builtinTypes[i].type_name = NULL;
//	}

	builtinTypes[TME_INDEX_Type].py_type = &PyType_Type;
	builtinTypes[TME_INDEX_Type].jy_class = pyTypeClass;
	//builtinTypes[TME_INDEX_Type].flags = JY_TRUNCATE_FLAG_MASK;
	//builtinTypes[TME_INDEX_Type].truncate_trailing = sizeof(PyTypeObject)-sizeof(PyVarObject);
	//builtinTypes[TME_INDEX_Type].truncate_trailing = sizeof(PyTypeObject)-sizeof(PyObject);
	//In fact, the trailing is so large that actually nothing is truncated.
	//This currently serves testing purposes. We are still checking out,
	//which fields are crucial and which are not. Although the only macros that
	//access PyType directly just need tp_flags, there are several direct accesses to
	//tp_-variables throughout CPython source.
	builtinTypes[TME_INDEX_Type].sync = malloc(sizeof(SyncFunctions));
	//builtinTypes[TME_INDEX_Type].sync->py2jy = (py2jySync) JySync_JyType_From_PyType;
	builtinTypes[TME_INDEX_Type].sync->jy2py = (jy2pySync) JySync_PyType_From_JyType;

	builtinTypes[TME_INDEX_NotImplemented].py_type = &PyNotImplemented_Type; //(_Py_NotImplementedStruct.ob_type);
	builtinTypes[TME_INDEX_NotImplemented].jy_class = pyNotImplementedClass;
	//builtinTypes[TME_INDEX_NotImplemented].flags = 0;

	builtinTypes[TME_INDEX_None].py_type = &PyNone_Type;//(_Py_NoneStruct.ob_type);
	builtinTypes[TME_INDEX_None].jy_class = pyNoneClass;
	//builtinTypes[TME_INDEX_None].flags = 0;

	builtinTypes[TME_INDEX_File].py_type = &PyFile_Type;
	builtinTypes[TME_INDEX_File].jy_class = pyFileClass;
	builtinTypes[TME_INDEX_File].flags = JY_TRUNCATE_FLAG_MASK;
	PyFile_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	//In JyNI no GC-type since it is fully truncated.
	builtinTypes[TME_INDEX_Module].py_type = &PyModule_Type;
	builtinTypes[TME_INDEX_Module].jy_class = pyModuleClass;
	builtinTypes[TME_INDEX_Module].flags = JY_TRUNCATE_FLAG_MASK;
	PyModule_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Cell].py_type = &PyCell_Type;
	builtinTypes[TME_INDEX_Cell].jy_class = pyCellClass;
	//builtinTypes[TME_INDEX_Cell].flags = JY_GC_FIXED_SIZE; //JY_GC_SINGLE_LINK | JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Cell].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Cell].sync->py2jy = (py2jySync) JySync_JyCell_From_PyCell;
	builtinTypes[TME_INDEX_Cell].sync->jy2py = (jy2pySync) JySync_PyCell_From_JyCell;
	PyCell_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Class].py_type = &PyClass_Type;
	builtinTypes[TME_INDEX_Class].jy_class = pyClassClass;
	builtinTypes[TME_INDEX_Class].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 6 links
	builtinTypes[TME_INDEX_Class].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Class].sync->jyInit = (jyInitSync) JySync_Init_JyClass_From_PyClass;
	builtinTypes[TME_INDEX_Class].sync->pyInit = (pyInitSync) JySync_Init_PyClass_From_JyClass;
	PyClass_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Instance].py_type = &PyInstance_Type;
	builtinTypes[TME_INDEX_Instance].jy_class = pyInstanceClass;
	builtinTypes[TME_INDEX_Instance].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 2 links
	builtinTypes[TME_INDEX_Instance].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Instance].sync->jyInit = (jyInitSync) JySync_Init_JyInstance_From_PyInstance;
	builtinTypes[TME_INDEX_Instance].sync->pyInit = (pyInitSync) JySync_Init_PyInstance_From_JyInstance;
	PyInstance_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

//	specialPyInstance.py_type = &PyInstance_Type;
//	specialPyInstance.jy_class = pyInstanceClass;
//	specialPyInstance.flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 2 links
//	specialPyInstance.sync = malloc(sizeof(SyncFunctions));
//	specialPyInstance.sync->pyInit = (pyInitSync) JySync_Init_Special_PyInstance;

	builtinTypes[TME_INDEX_Method].py_type = &PyMethod_Type;
	builtinTypes[TME_INDEX_Method].jy_class = pyMethodClass;
	builtinTypes[TME_INDEX_Method].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 3 links
	builtinTypes[TME_INDEX_Method].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Method].sync->jyInit = (jyInitSync) JySync_Init_JyMethod_From_PyMethod;
	builtinTypes[TME_INDEX_Method].sync->pyInit = (pyInitSync) JySync_Init_PyMethod_From_JyMethod;
	PyMethod_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	//Note: PyFunction is actually truncated in the sense that JyNI does not populate
	//all fields. However no memory can be saved because e.g. the last field func_module
	//is mirrored. This is needed e.g. for the PyFunction_GET_MODULE-macro.
	//In order to let the gcmodule get this right as a partly CStub, we still mark it
	//truncated and supply full length as truncate_trailing.
	builtinTypes[TME_INDEX_Function].py_type = &PyFunction_Type;
	builtinTypes[TME_INDEX_Function].jy_class = pyFunctionClass;
	//builtinTypes[TME_INDEX_Function].flags = JY_GC_FIXED_SIZE; // 5 (8) links
	//This variant currently fails with delegation. Todo: Fix this.
	//builtinTypes[TME_INDEX_Function].flags = JY_TRUNCATE_FLAG_MASK;
	//builtinTypes[TME_INDEX_Function].truncate_trailing = 9*sizeof(PyObject*);
	builtinTypes[TME_INDEX_Function].sync = malloc(sizeof(SyncFunctions));
	//builtinTypes[TME_INDEX_Function].sync->jyInit = NULL;//(jyInitSync) JySync_Init_JyFunction_From_PyFunction;
	//builtinTypes[TME_INDEX_Function].sync->pyInit = (pyInitSync) JySync_Init_PyFunction_From_JyFunction;
	builtinTypes[TME_INDEX_Function].sync->jy2py = (jy2pySync) JySync_PyFunction_From_JyFunction;
	PyFunction_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_ClassMethod].py_type = &PyClassMethod_Type;
	builtinTypes[TME_INDEX_ClassMethod].jy_class = pyClassMethodClass;
	builtinTypes[TME_INDEX_ClassMethod].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_ClassMethod].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_ClassMethod].sync->jyInit = (jyInitSync) JySync_Init_JyClassMethod_From_PyClassMethod;
	builtinTypes[TME_INDEX_ClassMethod].sync->pyInit = (pyInitSync) JySync_Init_PyClassMethod_From_JyClassMethod;
	PyClassMethod_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_StaticMethod].py_type = &PyStaticMethod_Type;
	builtinTypes[TME_INDEX_StaticMethod].jy_class = pyStaticMethodClass;
	builtinTypes[TME_INDEX_StaticMethod].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_StaticMethod].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_StaticMethod].sync->jyInit = (jyInitSync) JySync_Init_JyStaticMethod_From_PyStaticMethod;
	builtinTypes[TME_INDEX_StaticMethod].sync->pyInit = (pyInitSync) JySync_Init_PyStaticMethod_From_JyStaticMethod;
	PyStaticMethod_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_MethodDescr].py_type = &PyMethodDescr_Type;
	builtinTypes[TME_INDEX_MethodDescr].jy_class = pyMethodDescrClass;
	builtinTypes[TME_INDEX_MethodDescr].flags = 0;
	PyMethodDescr_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_ClassMethodDescr].py_type = &PyClassMethodDescr_Type;
	builtinTypes[TME_INDEX_ClassMethodDescr].jy_class = pyClassMethodDescrClass;
	builtinTypes[TME_INDEX_ClassMethodDescr].flags = 0;
	PyClassMethodDescr_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	builtinTypes[TME_INDEX_DictProxy].py_type = &PyDictProxy_Type;
	builtinTypes[TME_INDEX_DictProxy].jy_class = pyDictProxyClass;
	builtinTypes[TME_INDEX_DictProxy].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_DictProxy].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_DictProxy].sync->jyInit = (jyInitSync) JySync_Init_JyDictProxy_From_PyDictProxy;
	builtinTypes[TME_INDEX_DictProxy].sync->pyInit = (pyInitSync) JySync_Init_PyDictProxy_From_JyDictProxy;
	PyDictProxy_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Property].py_type = &PyProperty_Type;
	builtinTypes[TME_INDEX_Property].jy_class = pyPropertyClass;
	builtinTypes[TME_INDEX_Property].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Property].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Property].sync->jyInit = (jyInitSync) JySync_Init_JyProperty_From_PyProperty;
	builtinTypes[TME_INDEX_Property].sync->pyInit = (pyInitSync) JySync_Init_PyProperty_From_JyProperty;
	PyProperty_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Bool].py_type = &PyBool_Type;
	builtinTypes[TME_INDEX_Bool].jy_class = pyBooleanClass;
	//builtinTypes[TME_INDEX_Bool].flags = 0;

	builtinTypes[TME_INDEX_Float].py_type = &PyFloat_Type;
	builtinTypes[TME_INDEX_Float].jy_class = pyFloatClass;
	builtinTypes[TME_INDEX_Float].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Float].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Float].sync->jyInit = (jyInitSync) JySync_Init_JyFloat_From_PyFloat;
	builtinTypes[TME_INDEX_Float].sync->pyInit = (pyInitSync) JySync_Init_PyFloat_From_JyFloat;
	PyFloat_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Int].py_type = &PyInt_Type;
	builtinTypes[TME_INDEX_Int].jy_class = pyIntClass;
	builtinTypes[TME_INDEX_Int].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Int].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Int].sync->jyInit = (jyInitSync) JySync_Init_JyInt_From_PyInt;
	builtinTypes[TME_INDEX_Int].sync->pyInit = (pyInitSync) JySync_Init_PyInt_From_JyInt;
	PyInt_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	//for computational efficiency we mirror PyLong, although it could also be wrapped
	//later we are going to offer both options by a configuration-parameter in JyNI.
	builtinTypes[TME_INDEX_Long].py_type = &PyLong_Type;
	builtinTypes[TME_INDEX_Long].jy_class = pyLongClass;
	builtinTypes[TME_INDEX_Long].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Long].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Long].sync->jyInit = (jyInitSync) JySync_Init_JyLong_From_PyLong;
	builtinTypes[TME_INDEX_Long].sync->pyInit = (pyInitSync) JySync_Init_PyLong_From_JyLong;
	PyLong_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Complex].py_type = &PyComplex_Type;
	builtinTypes[TME_INDEX_Complex].jy_class = pyComplexClass;
	builtinTypes[TME_INDEX_Complex].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Complex].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Complex].sync->jyInit = (jyInitSync) JySync_Init_JyComplex_From_PyComplex;
	builtinTypes[TME_INDEX_Complex].sync->pyInit = (pyInitSync) JySync_Init_PyComplex_From_JyComplex;
	PyComplex_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Unicode].py_type = &PyUnicode_Type;
	builtinTypes[TME_INDEX_Unicode].jy_class = pyUnicodeClass;
	builtinTypes[TME_INDEX_Unicode].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Unicode].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Unicode].sync->jyInit = (jyInitSync) JySync_Init_JyUnicode_From_PyUnicode;
	builtinTypes[TME_INDEX_Unicode].sync->pyInit = (pyInitSync) JySync_Init_PyUnicode_From_JyUnicode;
	PyUnicode_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_String].py_type = &PyString_Type;
	builtinTypes[TME_INDEX_String].jy_class = pyStringClass;
	builtinTypes[TME_INDEX_String].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_String].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_String].sync->jyInit = (jyInitSync) JySync_Init_JyString_From_PyString;
	builtinTypes[TME_INDEX_String].sync->pyInit = (pyInitSync) JySync_Init_PyString_From_JyString;
	PyString_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Weakref_Ref].py_type = &_PyWeakref_RefType;
	builtinTypes[TME_INDEX_Weakref_Ref].jy_class = pyWeakReferenceClass;
	builtinTypes[TME_INDEX_Weakref_Ref].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Weakref_Ref].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Weakref_Ref].sync->jyInit = (jyInitSync) JySync_Init_JyWeakReference_From_PyWeakReference;
	builtinTypes[TME_INDEX_Weakref_Ref].sync->pyInit = (pyInitSync) JySync_Init_PyWeakReference_From_JyWeakReference;
	_PyWeakref_RefType.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Weakref_Proxy].py_type = &_PyWeakref_ProxyType;
	builtinTypes[TME_INDEX_Weakref_Proxy].jy_class = pyWeakProxyClass;
	builtinTypes[TME_INDEX_Weakref_Proxy].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Weakref_Proxy].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Weakref_Proxy].sync->jyInit = (jyInitSync) JySync_Init_JyWeakProxy_From_PyWeakProxy;
	builtinTypes[TME_INDEX_Weakref_Proxy].sync->pyInit = (pyInitSync) JySync_Init_PyWeakProxy_From_JyWeakProxy;
	_PyWeakref_ProxyType.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Weakref_CallableProxy].py_type = &_PyWeakref_CallableProxyType;
	builtinTypes[TME_INDEX_Weakref_CallableProxy].jy_class = pyWeakCallableProxyClass;
	builtinTypes[TME_INDEX_Weakref_CallableProxy].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Weakref_CallableProxy].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Weakref_CallableProxy].sync->jyInit =
			(jyInitSync) JySync_Init_JyWeakCallableProxy_From_PyWeakCallableProxy;
	builtinTypes[TME_INDEX_Weakref_CallableProxy].sync->pyInit =
			(pyInitSync) JySync_Init_PyWeakCallableProxy_From_JyWeakCallableProxy;
	_PyWeakref_CallableProxyType.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_BaseString].py_type = &PyBaseString_Type;
	builtinTypes[TME_INDEX_BaseString].jy_class = pyBaseStringClass;
	builtinTypes[TME_INDEX_BaseString].flags = 0;
	PyBaseStringType.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_SeqIter].py_type = &PySeqIter_Type;
	builtinTypes[TME_INDEX_SeqIter].jy_class = pySequenceIterClass;
	builtinTypes[TME_INDEX_SeqIter].flags = 0;
	PySeqIter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Range].py_type = &PyRange_Type;
	builtinTypes[TME_INDEX_Range].jy_class = pyXRangeClass;
	builtinTypes[TME_INDEX_Range].flags = 0;
	PyRange_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_rangeiter].py_type = &Pyrangeiter_Type; //jython uses PySequenceIter. Map this to PySeqIter_Typ
	builtinTypes[TME_INDEX_rangeiter].jy_class = pySequenceIterClass;
	builtinTypes[TME_INDEX_rangeiter].flags = 0;
	Pyrangeiter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	builtinTypes[TME_INDEX_Tuple].py_type = &PyTuple_Type;
	builtinTypes[TME_INDEX_Tuple].jy_class = pyTupleClass;
	builtinTypes[TME_INDEX_Tuple].jy_subclass = pyTupleCPeerClass;
	builtinTypes[TME_INDEX_Tuple].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE;// | GC_NO_INITIAL_EXPLORE; // Py_SIZE(o) links
	builtinTypes[TME_INDEX_Tuple].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Tuple].sync->jyInit = (jyInitSync) JySync_Init_JyTuple_From_PyTuple;
	builtinTypes[TME_INDEX_Tuple].sync->pyInit = (pyInitSync) JySync_Init_PyTuple_From_JyTuple;
	PyTuple_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_TupleIter].py_type = &PyTupleIter_Type; //jython uses PyFastSequenceIter. Map this to PySeqIter_Type
	builtinTypes[TME_INDEX_TupleIter].jy_class = pyFastSequenceIterClass;
	builtinTypes[TME_INDEX_TupleIter].flags = 0;
	PyTupleIter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	builtinTypes[TME_INDEX_List].py_type = &PyList_Type;
	builtinTypes[TME_INDEX_List].jy_class = pyListClass;
	builtinTypes[TME_INDEX_List].flags = JySYNC_ON_INIT_FLAGS | GC_CRITICAL | JY_GC_VAR_SIZE;
			// | JY_GC_SPECIAL_CASE | GC_NO_INITIAL_EXPLORE;
	builtinTypes[TME_INDEX_List].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_List].sync->jyInit = (jyInitSync) JySync_Init_JyList_From_PyList;
	builtinTypes[TME_INDEX_List].sync->pyInit = (pyInitSync) JySync_Init_PyList_From_JyList;
	PyList_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_ListIter].py_type = &PyListIter_Type; //jython uses PyFastSequenceIter. Map this to PySeqIter_Type
	builtinTypes[TME_INDEX_ListIter].jy_class = pyFastSequenceIterClass;
	builtinTypes[TME_INDEX_ListIter].flags = 0;
	PyListIter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_ListRevIter].py_type = &PyListRevIter_Type; //jython uses PyReversedIterator.
	builtinTypes[TME_INDEX_ListRevIter].jy_class = pyReversedIteratorClass;
	builtinTypes[TME_INDEX_ListRevIter].flags = 0;
	PyListRevIter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	/* In JyNI no GC-type since it is fully truncated. */
	// Todo: What about subtype/subclass?
	builtinTypes[TME_INDEX_Dict].py_type = &PyDict_Type;
	builtinTypes[TME_INDEX_Dict].jy_class = pyDictClass;
	builtinTypes[TME_INDEX_Dict].jy_subclass = pyDictCPeerClass;
	builtinTypes[TME_INDEX_Dict].flags = JY_TRUNCATE_FLAG_MASK;
	PyDict_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	/* In the CPython->Java lookup direction, this is
	   overwritten by the previous entry with pyDictClass.
	   However, it ensures that pyStringMapClass has a valid
	   entry if the other lookup direction is done.
	   Native PyDict methods are implemented in a way that is
	   agnostic about whether PyDictionary or PyStringMap is used
	   on Java/Jython-side.
	   Would be GC-relevant, but is fully truncated. */
	builtinTypes[TME_INDEX_StringMap].py_type = &PyDict_Type;
	builtinTypes[TME_INDEX_StringMap].jy_class = pyStringMapClass;
	builtinTypes[TME_INDEX_StringMap].flags = JY_TRUNCATE_FLAG_MASK;
	char* tp_nameStringMap = "stringmap";
	builtinTypes[TME_INDEX_StringMap].type_name = malloc(strlen(tp_nameStringMap)+1);
	strcpy(builtinTypes[TME_INDEX_StringMap].type_name, tp_nameStringMap);

	//In JyNI no GC-type since it is almost fully truncated.
	builtinTypes[TME_INDEX_Set].py_type = &PySet_Type;
	builtinTypes[TME_INDEX_Set].jy_class = pySetClass;
	builtinTypes[TME_INDEX_Set].flags = JY_TRUNCATE_FLAG_MASK | JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Set].truncate_trailing = sizeof(Py_ssize_t); //setObject.fill is covered by PyVarObject.ob_size. We add another sizeof(Py_ssize_t) to allocate space for setObject.used
	builtinTypes[TME_INDEX_Set].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Set].sync->jyInit = NULL;//(jyInitSync) JySync_Init_JySet_From_PySet;
	builtinTypes[TME_INDEX_Set].sync->pyInit = (pyInitSync) JySync_Init_PySet_From_JySet;
	PySet_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_SetIter].py_type = &PySetIter_Type; //jython uses inline subclass of PyIterator. Map this to PySeqIter_Type for now
	builtinTypes[TME_INDEX_SetIter].jy_class = pySequenceIterClass;
	builtinTypes[TME_INDEX_SetIter].flags = 0;
	PySetIter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	//In JyNI no GC-type since it is almost fully truncated.
	builtinTypes[TME_INDEX_FrozenSet].py_type = &PyFrozenSet_Type;
	builtinTypes[TME_INDEX_FrozenSet].jy_class = pyFrozenSetClass;
	builtinTypes[TME_INDEX_FrozenSet].flags = JY_TRUNCATE_FLAG_MASK | JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_FrozenSet].truncate_trailing = sizeof(Py_ssize_t); //setObject.fill is covered by PyVarObject.ob_size. We add another sizeof(Py_ssize_t) to allocate space for setObject.used
	builtinTypes[TME_INDEX_FrozenSet].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_FrozenSet].sync->jyInit = NULL;//(jyInitSync) JySync_Init_JyFrozenSet_From_PyFrozenSet;
	builtinTypes[TME_INDEX_FrozenSet].sync->pyInit = (pyInitSync) JySync_Init_PyFrozenSet_From_JyFrozenSet;
	PyFrozenSet_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_Enum].py_type = &PyEnum_Type;
	builtinTypes[TME_INDEX_Enum].jy_class = pyEnumerationClass;
	builtinTypes[TME_INDEX_Enum].flags = 0;
	PyEnum_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	builtinTypes[TME_INDEX_Slice].py_type = &PySlice_Type;
	builtinTypes[TME_INDEX_Slice].jy_class = pySliceClass;
	builtinTypes[TME_INDEX_Slice].flags = JY_TRUNCATE_FLAG_MASK;
	PySlice_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Ellipsis].py_type = &PyEllipsis_Type;
	builtinTypes[TME_INDEX_Ellipsis].jy_class = pyEllipsisClass;
	//builtinTypes[TME_INDEX_Ellipsis].flags = 0;

/*	builtinTypes[TME_INDEX_Gen].py_type = &PyGen_Type;
	builtinTypes[TME_INDEX_Gen].jy_class = pyGeneratorClass;
	builtinTypes[TME_INDEX_Gen].flags = 0;
	PyGen_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	/* Code objects are not subject to GC in CPython although they contain some links.
	 * However the contained links are always strings or string-tuples and nothing that
	 * could lead to reference-cycles.
	 * Todo: - Check whether this simplification of GC-traversal can be applied in Jython too.
	 *       - This might need to be improved for GIL-free mode. Check this.
	 */
	builtinTypes[TME_INDEX_Code_Bytecode].py_type = &PyCode_Type;
	builtinTypes[TME_INDEX_Code_Bytecode].jy_class = pyBytecodeClass;
	builtinTypes[TME_INDEX_Code_Bytecode].flags = JY_TRUNCATE_FLAG_MASK;// | JySYNC_ON_INIT_FLAGS;
//	allocates trailing space for the macro PyCode_GetNumFree(op).
//	This macro accesses the memory-pos of co_freevars, so memory till co_freevars must
//	be allocated via truncate_trailing-mode. The size-formula 4*sizeof(int) + 5*sizeof(PyObject*)
//	arises from the first 9 members of PyCodeObject:
//	int co_argcount;	int co_nlocals;	int co_stacksize; int co_flags;
//	PyObject *co_code; PyObject *co_consts; PyObject *co_names; PyObject *co_varnames;
//	PyObject *co_freevars;
	builtinTypes[TME_INDEX_Code_Bytecode].truncate_trailing = 4*sizeof(int) + 5*sizeof(PyObject*);
	builtinTypes[TME_INDEX_Code_Bytecode].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Code_Bytecode].sync->jy2py = (jy2pySync) JySync_PyCode_From_JyCode;
	char* tp_nameBytecode = "bytecode";
	builtinTypes[TME_INDEX_Code_Bytecode].type_name = malloc(strlen(tp_nameBytecode)+1);
	strcpy(builtinTypes[TME_INDEX_Code_Bytecode].type_name, tp_nameBytecode);
	PyCode_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Code_Tablecode].py_type = &PyCode_Type;
	builtinTypes[TME_INDEX_Code_Tablecode].jy_class = pyTableCodeClass;
	builtinTypes[TME_INDEX_Code_Tablecode].flags = JY_TRUNCATE_FLAG_MASK;// | JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_Code_Tablecode].truncate_trailing =
			builtinTypes[TME_INDEX_Code_Bytecode].truncate_trailing;
	builtinTypes[TME_INDEX_Code_Tablecode].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Code_Tablecode].sync->jy2py = (jy2pySync) JySync_PyCode_From_JyCode;
	char* tp_nameTablecode = "tablecode";
	builtinTypes[TME_INDEX_Code_Tablecode].type_name = malloc(strlen(tp_nameTablecode)+1);
	strcpy(builtinTypes[TME_INDEX_Code_Tablecode].type_name, tp_nameTablecode);

/*	builtinTypes[TME_INDEX_Frame].py_type = &PyFrame_Type;
	builtinTypes[TME_INDEX_Frame].jy_class = pyFrameClass;
	builtinTypes[TME_INDEX_Frame].flags = 0;
	PyFrame_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Super].py_type = &PySuper_Type;
	builtinTypes[TME_INDEX_Super].jy_class = pySuperClass;
	builtinTypes[TME_INDEX_Super].flags = 0;
	PySuper_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	//In JyNI no GC-type since it is fully truncated.
	builtinTypes[TME_INDEX_Exc_BaseException].py_type = (PyTypeObject*) PyExc_BaseException;
	builtinTypes[TME_INDEX_Exc_BaseException].jy_class = pyBaseExceptionClass;
	builtinTypes[TME_INDEX_Exc_BaseException].flags = JY_TRUNCATE_FLAG_MASK;
	((PyTypeObject*) PyExc_BaseException)->tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	//todo: Improve PyTraceBack-implementation
	builtinTypes[TME_INDEX_TraceBack].py_type = &PyTraceBack_Type;
	builtinTypes[TME_INDEX_TraceBack].jy_class = pyTracebackClass;
	builtinTypes[TME_INDEX_TraceBack].flags = JY_TRUNCATE_FLAG_MASK;
	PyTraceBack_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_ByteArray].py_type = &PyByteArray_Type;
	builtinTypes[TME_INDEX_ByteArray].jy_class = pyByteArrayClass;
	builtinTypes[TME_INDEX_ByteArray].flags = 0;
	PyByteArray_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_Buffer].py_type = &PyBuffer_Type;
	builtinTypes[TME_INDEX_Buffer].jy_class = pyBufferClass;
	builtinTypes[TME_INDEX_Buffer].flags = 0;
	PyBuffer_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_MemoryView].py_type = &PyMemoryView_Type;
	builtinTypes[TME_INDEX_MemoryView].jy_class = pyMemoryViewClass;
	builtinTypes[TME_INDEX_MemoryView].flags = 0;
	PyMemoryView_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_BaseObject].py_type = &PyBaseObject_Type;
	builtinTypes[TME_INDEX_BaseObject].jy_class = pyObjectClass;
	builtinTypes[TME_INDEX_BaseObject].flags = 0;
	PyBaseObject_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_CallIter].py_type = &PyCallIter_Type;
	builtinTypes[TME_INDEX_CallIter].jy_class = pyCallIterClass;
	builtinTypes[TME_INDEX_CallIter].flags = 0;
	PyCallIter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;
	*/

	// Set tp_flags |= Jy_TPFLAGS_DYN_OBJECTS for remaining builtin types:
	PyCFunction_Type.tp_flags        |= Jy_TPFLAGS_DYN_OBJECTS;
	PyCapsule_Type.tp_flags          |= Jy_TPFLAGS_DYN_OBJECTS;
	PyCObject_Type.tp_flags          |= Jy_TPFLAGS_DYN_OBJECTS;

	PyGetSetDescr_Type.tp_flags      |= Jy_TPFLAGS_DYN_OBJECTS;
	PyMemberDescr_Type.tp_flags      |= Jy_TPFLAGS_DYN_OBJECTS;
	PyWrapperDescr_Type.tp_flags     |= Jy_TPFLAGS_DYN_OBJECTS;
	PyMethodDescr_Type.tp_flags      |= Jy_TPFLAGS_DYN_OBJECTS;
	PyClassMethodDescr_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;
	EncodingMapType.tp_flags         |= Jy_TPFLAGS_DYN_OBJECTS;
	wrappertype.tp_flags             |= Jy_TPFLAGS_DYN_OBJECTS;
	cmpwrapper_type.tp_flags         |= Jy_TPFLAGS_DYN_OBJECTS;
	sortwrapper_type.tp_flags        |= Jy_TPFLAGS_DYN_OBJECTS;
	Long_InfoType.tp_flags           |= Jy_TPFLAGS_DYN_OBJECTS;
	FloatInfoType.tp_flags           |= Jy_TPFLAGS_DYN_OBJECTS;
	PyListIter_Type.tp_flags         |= Jy_TPFLAGS_DYN_OBJECTS;
	PyListRevIter_Type.tp_flags      |= Jy_TPFLAGS_DYN_OBJECTS;
	PyTupleIter_Type.tp_flags        |= Jy_TPFLAGS_DYN_OBJECTS;
	PyBaseString_Type.tp_flags       |= Jy_TPFLAGS_DYN_OBJECTS;
	PyBaseObject_Type.tp_flags       |= Jy_TPFLAGS_DYN_OBJECTS;


// This generates switch/case lookup code based on a trivial hash
// function that is collision free on builtin type names. We will
// use this for maybe faster type lookup one day.
//	int i, j, hs;
//	char* tn;
//	for (i = 0; i < builtinTypeCount; ++i)
//	{
//		hs = 0;
//		tn = builtinTypes[i].py_type->tp_name;
//		if (builtinTypes[i].type_name) tn = builtinTypes[i].type_name;
//		for (j = 1; j < 9 && tn[j]; ++j)
//			hs += tn[j]*j;
//		printf("case %4i:  return %2i;  // %s\n", hs, i, tn);
//	}

//	int i = 0;
//	for (; i < builtinTypeCount; ++i) {
//		if (builtinTypes[i].py_type) {
//			jputs(builtinTypes[i].py_type->tp_name);
//			jputsLong(builtinTypes[i].py_type->tp_name[0]
//				+builtinTypes[i].py_type->tp_name[1]
//				+builtinTypes[i].py_type->tp_name[2]);
//
////			jputsLong(i);
////			jputsLong(builtinTypes[i].py_type-builtinTypes[0].py_type);
////			jputsLong(builtinTypes[i].py_type);
////			printf("#define typeoffset%l %l\n", i, builtinTypes[i].py_type-builtinTypes[0].py_type);
//		}
//	}
}

inline void initBuiltinExceptions()
{
	builtinExceptions[0].exc_type = (PyTypeObject*) PyExc_BaseException;
	builtinExceptions[0].exc_factory = NULL;

	builtinExceptions[1].exc_type = (PyTypeObject*) PyExc_Exception;
	builtinExceptions[1].exc_factory = NULL;

	builtinExceptions[2].exc_type = (PyTypeObject*) PyExc_StandardError;
	builtinExceptions[2].exc_factory = NULL;

	builtinExceptions[3].exc_type = (PyTypeObject*) PyExc_TypeError;
	builtinExceptions[3].exc_factory = NULL;

	builtinExceptions[4].exc_type = (PyTypeObject*) PyExc_StopIteration;
	builtinExceptions[4].exc_factory = NULL;

	builtinExceptions[5].exc_type = (PyTypeObject*) PyExc_GeneratorExit;
	builtinExceptions[5].exc_factory = NULL;

	builtinExceptions[6].exc_type = (PyTypeObject*) PyExc_SystemExit;
	builtinExceptions[6].exc_factory = (jyFactoryMethod) JyExc_SystemExitFactory;

	builtinExceptions[7].exc_type = (PyTypeObject*) PyExc_KeyboardInterrupt;
	builtinExceptions[7].exc_factory = NULL;

	builtinExceptions[8].exc_type = (PyTypeObject*) PyExc_ImportError;
	builtinExceptions[8].exc_factory = NULL;

	builtinExceptions[9].exc_type = (PyTypeObject*) PyExc_EnvironmentError;
	builtinExceptions[9].exc_factory = (jyFactoryMethod) JyExc_EnvironmentErrorFactory;

	builtinExceptions[10].exc_type = (PyTypeObject*) PyExc_IOError;
	builtinExceptions[10].exc_factory = (jyFactoryMethod) JyExc_EnvironmentErrorFactory;

	builtinExceptions[11].exc_type = (PyTypeObject*) PyExc_OSError;
	builtinExceptions[11].exc_factory = (jyFactoryMethod) JyExc_EnvironmentErrorFactory;

#ifdef MS_WINDOWS
	builtinExceptions[12].exc_type = (PyTypeObject*) PyExc_WindowsError;
	builtinExceptions[12].exc_factory = (jyFactoryMethod) JyExc_EnvironmentErrorFactory;
	//JyNI-note: Would actually be WindowsError, but that seems to be not provided by Jython.
#endif
#ifdef __VMS
	builtinExceptions[13].exc_type = (PyTypeObject*) PyExc_VMSError;
	builtinExceptions[13].exc_factory = (jyFactoryMethod) JyExc_EnvironmentErrorFactory;
#endif
	builtinExceptions[14].exc_type = (PyTypeObject*) PyExc_EOFError;
	builtinExceptions[14].exc_factory = NULL;

	builtinExceptions[15].exc_type = (PyTypeObject*) PyExc_RuntimeError;
	builtinExceptions[15].exc_factory = NULL;

	builtinExceptions[16].exc_type = (PyTypeObject*) PyExc_NotImplementedError;
	builtinExceptions[16].exc_factory = NULL;

	builtinExceptions[17].exc_type = (PyTypeObject*) PyExc_NameError;
	builtinExceptions[17].exc_factory = NULL;

	builtinExceptions[18].exc_type = (PyTypeObject*) PyExc_UnboundLocalError;
	builtinExceptions[18].exc_factory = NULL;

	builtinExceptions[19].exc_type = (PyTypeObject*) PyExc_AttributeError;
	builtinExceptions[19].exc_factory = NULL;

	builtinExceptions[20].exc_type = (PyTypeObject*) PyExc_SyntaxError;
	builtinExceptions[20].exc_factory = (jyFactoryMethod) JyExc_SyntaxErrorFactory;

	builtinExceptions[21].exc_type = (PyTypeObject*) PyExc_IndentationError;
	builtinExceptions[21].exc_factory = (jyFactoryMethod) JyExc_SyntaxErrorFactory;

	builtinExceptions[22].exc_type = (PyTypeObject*) PyExc_TabError;
	builtinExceptions[22].exc_factory = (jyFactoryMethod) JyExc_SyntaxErrorFactory;

	builtinExceptions[23].exc_type = (PyTypeObject*) PyExc_LookupError;
	builtinExceptions[23].exc_factory = NULL;

	builtinExceptions[24].exc_type = (PyTypeObject*) PyExc_IndexError;
	builtinExceptions[24].exc_factory = NULL;

	builtinExceptions[25].exc_type = (PyTypeObject*) PyExc_KeyError;
	builtinExceptions[25].exc_factory = NULL;

	builtinExceptions[26].exc_type = (PyTypeObject*) PyExc_ValueError;
	builtinExceptions[26].exc_factory = NULL;

	//We don't put UnicodeError in the conditional code because it also isn't
	//conditional in original exceptions.c.
	builtinExceptions[27].exc_type = (PyTypeObject*) PyExc_UnicodeError;
	builtinExceptions[27].exc_factory = JyExc_UnicodeErrorFactory;
	//While its subclasses use the PyUnicodeErrorObject-body, UnicodeError itself doesn't.
	//However, since Jython provides a factory method for UnicodeError, we use it here.

#ifdef Py_USING_UNICODE
	builtinExceptions[28].exc_type = (PyTypeObject*) PyExc_UnicodeEncodeError;
	builtinExceptions[28].exc_factory = JyExc_UnicodeEncodeErrorFactory;

	builtinExceptions[29].exc_type = (PyTypeObject*) PyExc_UnicodeDecodeError;
	builtinExceptions[29].exc_factory = JyExc_UnicodeDecodeErrorFactory;

	builtinExceptions[30].exc_type = (PyTypeObject*) PyExc_UnicodeTranslateError;
	builtinExceptions[30].exc_factory = JyExc_UnicodeTranslateErrorFactory;
#endif

	builtinExceptions[31].exc_type = (PyTypeObject*) PyExc_AssertionError;
	builtinExceptions[31].exc_factory = NULL;

	builtinExceptions[32].exc_type = (PyTypeObject*) PyExc_ArithmeticError;
	builtinExceptions[32].exc_factory = NULL;

	builtinExceptions[33].exc_type = (PyTypeObject*) PyExc_FloatingPointError;
	builtinExceptions[33].exc_factory = NULL;

	builtinExceptions[34].exc_type = (PyTypeObject*) PyExc_OverflowError;
	builtinExceptions[34].exc_factory = NULL;

	builtinExceptions[35].exc_type = (PyTypeObject*) PyExc_ZeroDivisionError;
	builtinExceptions[35].exc_factory = NULL;

	builtinExceptions[36].exc_type = (PyTypeObject*) PyExc_SystemError;
	builtinExceptions[36].exc_factory = NULL;

	builtinExceptions[37].exc_type = (PyTypeObject*) PyExc_ReferenceError;
	builtinExceptions[37].exc_factory = NULL;

	builtinExceptions[38].exc_type = (PyTypeObject*) PyExc_MemoryError;
	builtinExceptions[38].exc_factory = NULL;

	builtinExceptions[39].exc_type = (PyTypeObject*) PyExc_BufferError;
	builtinExceptions[39].exc_factory = NULL;

	builtinExceptions[40].exc_type = (PyTypeObject*) PyExc_Warning;
	builtinExceptions[40].exc_factory = NULL;

	builtinExceptions[41].exc_type = (PyTypeObject*) PyExc_UserWarning;
	builtinExceptions[41].exc_factory = NULL;

	builtinExceptions[42].exc_type = (PyTypeObject*) PyExc_DeprecationWarning;
	builtinExceptions[42].exc_factory = NULL;

	builtinExceptions[43].exc_type = (PyTypeObject*) PyExc_PendingDeprecationWarning;
	builtinExceptions[43].exc_factory = NULL;

	builtinExceptions[44].exc_type = (PyTypeObject*) PyExc_SyntaxWarning;
	builtinExceptions[44].exc_factory = NULL;

	builtinExceptions[45].exc_type = (PyTypeObject*) PyExc_RuntimeWarning;
	builtinExceptions[45].exc_factory = NULL;

	builtinExceptions[46].exc_type = (PyTypeObject*) PyExc_FutureWarning;
	builtinExceptions[46].exc_factory = NULL;

	builtinExceptions[47].exc_type = (PyTypeObject*) PyExc_ImportWarning;
	builtinExceptions[47].exc_factory = NULL;

	builtinExceptions[48].exc_type = (PyTypeObject*) PyExc_UnicodeWarning;
	builtinExceptions[48].exc_factory = NULL;

	builtinExceptions[49].exc_type = (PyTypeObject*) PyExc_BytesWarning;
	builtinExceptions[49].exc_factory = NULL;

	int i;
	for (i = 0; i < builtinExceptionCount; ++i)
	{
		if (builtinExceptions[i].exc_type)
			builtinExceptions[i].exc_type->tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;
	}
}

/* Does not work for Heap-Types. */
inline jboolean JyNI_IsBuiltinPyType(PyTypeObject* type)
{
	int i;
	if (&PyCFunction_Type == type) return JNI_TRUE;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].py_type == type) return JNI_TRUE;
	}

	if (&PyCapsule_Type == type) return JNI_TRUE;
	if (&PyCObject_Type == type) return JNI_TRUE;

	for (i = 0; i < builtinExceptionCount; ++i)
	{
		if (builtinExceptions[49].exc_type == type) return JNI_TRUE;
	}

	// Now we check for types currently not listed in builtinTypes:

	if (&PyGetSetDescr_Type == type) return JNI_TRUE;
	if (&PyMemberDescr_Type == type) return JNI_TRUE;
	if (&PyWrapperDescr_Type == type) return JNI_TRUE;
	if (&PyMethodDescr_Type == type) return JNI_TRUE;
	if (&PyClassMethodDescr_Type == type) return JNI_TRUE;
	// if (&PyDictProxy_Type == type) return JNI_TRUE;
	// if (&PyProperty_Type == type) return JNI_TRUE;
	if (&EncodingMapType == type) return JNI_TRUE;
	if (&wrappertype == type) return JNI_TRUE;
	if (&cmpwrapper_type == type) return JNI_TRUE;
	if (&sortwrapper_type == type) return JNI_TRUE;
	if (&Long_InfoType == type) return JNI_TRUE;
	if (&FloatInfoType == type) return JNI_TRUE;
	// if (&_PyWeakref_RefType == type) return JNI_TRUE;
	// if (&_PyWeakref_ProxyType == type) return JNI_TRUE;
	// if (&_PyWeakref_CallableProxyType == type) return JNI_TRUE;
	// if (&_struct_sequence_template == type) return JNI_TRUE;
	// if (&PyDictIterKey_Type == type) return JNI_TRUE;
	// if (&PyDictIterValue_Type == type) return JNI_TRUE;
	// if (&PyDictIterItem_Type == type) return JNI_TRUE;
	// if (&PyDictKeys_Type == type) return JNI_TRUE;
	// if (&PyDictItems_Type == type) return JNI_TRUE;
	// if (&PyDictValues_Type == type) return JNI_TRUE;
	// if (&PyReversed_Type == type) return JNI_TRUE;
	if (&PyListIter_Type == type) return JNI_TRUE;
	if (&PyListRevIter_Type == type) return JNI_TRUE;
	// if (&PyClassMethod_Type == type) return JNI_TRUE;
	// if (&PyStaticMethod_Type == type) return JNI_TRUE;
	// if (&PySeqIter_Type == type) return JNI_TRUE;
	// if (&PyRange_Type == type) return JNI_TRUE;
	// if (&Pyrangeiter_Type == type) return JNI_TRUE;
	if (&PyTupleIter_Type == type) return JNI_TRUE;
	// if (&PySetIter_Type == type) return JNI_TRUE;
	// if (&PyEnum_Type == type) return JNI_TRUE;
	// if (&PyGen_Type == type) return JNI_TRUE;
	// if (&PyFrame_Type == type) return JNI_TRUE;
	// if (&PySuper_Type == type) return JNI_TRUE;
	// if (&PyBuffer_Type == type) return JNI_TRUE;
	// if (&PyMemoryView_Type == type) return JNI_TRUE;
	// if (&PyCallIter_Type == type) return JNI_TRUE;
	// if (&PyByteArrayIter_Type == type) return JNI_TRUE;
	if (&PyBaseString_Type == type) return JNI_TRUE;
	if (&PyBaseObject_Type == type) return JNI_TRUE;

	// string_format.h would be needed for these, but including it
	// is not trivial.
	// if (&PyFormatterIter_Type == type) return JNI_TRUE;
	// if (&PyFieldNameIter_Type == type) return JNI_TRUE;
	return JNI_FALSE;
}

/* Does not work for Heap-Types. */
inline jclass JyNI_JythonClassFromPyType(PyTypeObject* type)
{
	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].py_type == type) return builtinTypes[i].jy_class;
	}
	return NULL;
}

/* Does not work for Heap-Types. */
inline TypeMapEntry* JyNI_JythonTypeEntry_FromPyType(PyTypeObject* type)
{
	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].py_type == type) {
			return &(builtinTypes[i]);
		}
	}
	return NULL;
}

#define Py_TPFLAGS_BASIC_SUBCLASS \
	(Py_TPFLAGS_INT_SUBCLASS | \
	Py_TPFLAGS_LONG_SUBCLASS | \
	Py_TPFLAGS_LIST_SUBCLASS | \
	Py_TPFLAGS_TUPLE_SUBCLASS | \
	Py_TPFLAGS_STRING_SUBCLASS | \
	Py_TPFLAGS_UNICODE_SUBCLASS | \
	Py_TPFLAGS_DICT_SUBCLASS | \
	Py_TPFLAGS_BASE_EXC_SUBCLASS | \
	Py_TPFLAGS_TYPE_SUBCLASS)

inline TypeMapEntry* JyNI_JythonTypeEntry_FromSubType(PyTypeObject* type)
{
	switch (type->tp_flags & Py_TPFLAGS_BASIC_SUBCLASS)
	{
		case Py_TPFLAGS_INT_SUBCLASS: return &(builtinTypes[TME_INDEX_Int]);
		case Py_TPFLAGS_LONG_SUBCLASS: return &(builtinTypes[TME_INDEX_Long]);
		case Py_TPFLAGS_LIST_SUBCLASS: return &(builtinTypes[TME_INDEX_List]);
		case Py_TPFLAGS_TUPLE_SUBCLASS: return &(builtinTypes[TME_INDEX_Tuple]);
		case Py_TPFLAGS_STRING_SUBCLASS: return &(builtinTypes[TME_INDEX_String]);
		case Py_TPFLAGS_UNICODE_SUBCLASS: return &(builtinTypes[TME_INDEX_Unicode]);
		case Py_TPFLAGS_DICT_SUBCLASS: return &(builtinTypes[TME_INDEX_Dict]); // todo: check how this works for stringmap
		case Py_TPFLAGS_BASE_EXC_SUBCLASS: return &(builtinTypes[TME_INDEX_Exc_BaseException]);
		case Py_TPFLAGS_TYPE_SUBCLASS: return &(builtinTypes[TME_INDEX_Type]);
		default:
		{
			int i;
			for (i = 0; i < builtinTypeCount; ++i)
			{
				if (PyType_IsSubtype(type, builtinTypes[i].py_type))
					return &(builtinTypes[i]);
			}
			return NULL;
		}
	}
}

inline TypeMapEntry* JyNI_JythonTypeEntry_FromSubTypeWithPeer(PyTypeObject* type)
{
	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].jy_subclass && PyType_IsSubtype(type, builtinTypes[i].py_type))
			return &(builtinTypes[i]);
	}
	return NULL;
}

/* Does not work for Heap-Types. */
inline TypeMapEntry* JyNI_JythonTypeEntry_FromJythonPyClass(jclass jythonPyClass)
{
	if (jythonPyClass == NULL) return NULL;
	env(NULL);
	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].jy_class != NULL && (*env)->CallBooleanMethod(env, jythonPyClass, classEquals, builtinTypes[i].jy_class))
			return &(builtinTypes[i]);
	}
	return NULL;
}

inline TypeMapEntry* JyNI_JythonTypeEntry_FromName(char* name)
{
//	jputs("JyNI_JythonTypeEntry_FromName");
//	jputs(name);
	if (name == NULL) return NULL;
	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].type_name != NULL && strcmp(builtinTypes[i].type_name, name) == 0)
		{
			return &(builtinTypes[i]);
		}
		else if (builtinTypes[i].py_type != NULL && strcmp(builtinTypes[i].py_type->tp_name, name) == 0)
		{
//			jputs("found:");
//			jputs(builtinTypes[i].py_type->tp_name);
			return &(builtinTypes[i]);
		}
	}
	return NULL;
}

/* Does not work for Heap-Types. */
inline TypeMapEntry* JyNI_JythonTypeEntry_FromJStringName(jstring name)
{
	if (name == NULL) return NULL;
	env(NULL);
	cstr_from_jstring(cName, name);
	return JyNI_JythonTypeEntry_FromName(cName);
}

/* Does not work for Heap-Types. */
inline TypeMapEntry* JyNI_JythonTypeEntry_FromJythonPyType(jobject jythonPyType)
{
	env(NULL);
	return JyNI_JythonTypeEntry_FromJStringName((*env)->CallObjectMethod(env, jythonPyType, pyTypeGetName));
}

/* Does not work for Heap-Type exceptions. */
inline jobject JyNI_JythonExceptionType_FromPyExceptionType(PyObject* exc)
{
	env(NULL);
	//return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNIExceptionByName, (*env)->NewStringUTF(env, ((PyTypeObject*) exc)->tp_name));
	return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNIExceptionByName, (*env)->NewStringUTF(env, PyExceptionClass_Name(exc)));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * Does not work for Heap-Type exceptions.
 */
inline PyTypeObject* JyNI_PyExceptionType_FromJythonExceptionType(jobject exc)
{
	env(NULL);
	jboolean isCopy;
	jobject name = (*env)->CallObjectMethod(env, exc, pyTypeGetName);
	const char* utf_string = (*env)->GetStringUTFChars(env, name, &isCopy);
	//char mName[strlen(excPrefix)+strlen(utf_string)+1];
	char mName[strlen(utf_string)+12];
	strcpy(mName, excPrefix);
	strcat(mName, utf_string);
	(*env)->ReleaseStringUTFChars(env, name, utf_string);
	int i;
	for (i = 0; i < builtinExceptionCount; ++i)
	{
		if (builtinExceptions[i].exc_type != NULL && strcmp(builtinExceptions[i].exc_type->tp_name, mName) == 0)
		{
			Py_INCREF(builtinExceptions[i].exc_type);
			return builtinExceptions[i].exc_type;
		}
	}
	return NULL;
}

/* Does not work for Heap-Type exceptions. */
inline ExceptionMapEntry* JyNI_PyExceptionMapEntry_FromPyExceptionType(PyTypeObject* excType)
{
	//jputs("lookup exception...");
	if (excType == NULL) {
		//jputs("excType is NULL");
		return NULL;
	} //else
		//jputs(excType->tp_name);
	int i;
	for (i = 0; i < builtinExceptionCount; ++i)
	{
		if (builtinExceptions[i].exc_type == excType) return &(builtinExceptions[i]);
	}
	//jputs("excType: Returning NULL...");
	return NULL;
}

inline void JyNI_SyncJy2Py(JyObject* jy, PyObject* op)
{
	//todo: take care of the other flags
	SyncFunctions* sync = (SyncFunctions*) JyNI_GetJyAttribute(jy, JyAttributeSyncFunctions);
	if (sync != NULL && sync->jy2py != NULL) sync->jy2py(jy->jy, op);
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* _JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject, jboolean lookupNative, jboolean checkCPeer, jboolean checkForType)
{
//	jputs(__FUNCTION__);
//	jputsLong(jythonPyObject);
	if (jythonPyObject == NULL) return NULL;
	//puts("Transform jython jobject to PyObject*...");
//	if (jythonPyObject == JyNone) return Py_None;
//	if (jythonPyObject == JyNotImplemented) return Py_NotImplemented;
//	if (jythonPyObject == JyEllipsis) return Py_Ellipsis;
	env(NULL);
	if ((*env)->IsSameObject(env, jythonPyObject, NULL)) return NULL;
	if ((*env)->IsSameObject(env, jythonPyObject, JyNone)) Py_RETURN_NONE;
	if ((*env)->IsSameObject(env, jythonPyObject, JyNotImplemented))
	{
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	//Special treatment for boolean. We cannot just compare with singletons,
	//because via Java-API it is possible in Jython to create non-singleton
	//PyBooleans (Honestly, that constructor should be protected, but changing
	//that today might break some existing code.).
	if ((*env)->IsInstanceOf(env, jythonPyObject, pyBooleanClass))
	{
		if ((*env)->IsSameObject(env, jythonPyObject, JyTrue)) Py_RETURN_TRUE;
		if ((*env)->IsSameObject(env, jythonPyObject, JyFalse)) Py_RETURN_FALSE;
		if ((*env)->GetBooleanField(env, pyBooleanClass, pyBoolVal)) Py_RETURN_TRUE;
		else Py_RETURN_FALSE;
	}
	//Todo: Maybe the check for nullstring and unicode_empty is not needed
	//here and should be processed via usual lookup. (Since these singletons
	//are on the heap)
	if ((*env)->IsSameObject(env, jythonPyObject, JyEmptyString) && nullstring)
	{
		Py_INCREF(nullstring);
		return (PyObject*) nullstring;
	}
	if ((*env)->IsSameObject(env, jythonPyObject, JyEmptyUnicode) && unicode_empty)
	{
		Py_INCREF(unicode_empty);
		return (PyObject*) unicode_empty;
	}
	if ((*env)->IsSameObject(env, jythonPyObject, JyEllipsis))
	{
		Py_INCREF(Py_Ellipsis);
		return Py_Ellipsis;
	}
	if (checkForType && (*env)->IsInstanceOf(env, jythonPyObject, pyTypeClass))
	{
		/* No increfs here, since JyNI_PyTypeObject_FromJythonPyTypeObject returns NEW ref if any. */
		PyObject* er = (PyObject*) JyNI_PyTypeObject_FromJythonPyTypeObject(jythonPyObject);
		if (er) return er;
		/* No increfs here, since JyNI_PyExceptionType_FromJythonExceptionType returns NEW ref if any. */
		er = (PyObject*) JyNI_PyExceptionType_FromJythonExceptionType(jythonPyObject);
		if (er) return er;
		/* heap-type case: Proceed same way like for ordinary PyObjects. */
//		jputs("heap-type case!");
	}
	if (checkCPeer && (*env)->IsInstanceOf(env, jythonPyObject, cPeerInterface))
	{
		if ((*env)->IsInstanceOf(env, jythonPyObject, pyCPeerClass)) {
			//puts("object is a PyCPeer");
			PyObject* er = (PyObject*) (*env)->GetLongField(env, jythonPyObject, pyCPeerObjectHandle);
			Py_INCREF(er);
			return er;
		} else {
			PyObject* er = (PyObject*) (*env)->CallLongMethod(env, jythonPyObject, jyGCHeadGetHandle);
			Py_INCREF(er);
			return er;
		}
	}

	if (lookupNative)
	{
		PyObject* handle = (PyObject*) (*env)->CallStaticLongMethod(env, JyNIClass, JyNILookupNativeHandle, jythonPyObject);
		//printf("handle obtained: %u\n", handle);
		if (handle)
		{
			//don't forget to sync if necessary:
			JyObject* jy = AS_JY(handle);
			if (jy->flags & SYNC_ON_JY_TO_PY_FLAG_MASK)
				JyNI_SyncJy2Py(jy, handle);
			Py_INCREF(handle);
			return handle;
		}
	}
	//initialize PyObject*...
	//find tme:
	jstring tpName = (*env)->CallStaticObjectMethod(env, JyNIClass,
			JyNI_getTypeNameForNativeConversion, jythonPyObject);
	//todo find out what name occurs if a Jython Java-proxy is used.
	//     Can we make sense of it in default-instance case?
	cstr_from_jstring(cName, tpName);
//	if (strcmp(cName, "str") == 0) {
//		jputs("is string!");
//		jobject str = (*env)->CallObjectMethod(env, jythonPyObject, pyObjectAsString);
//		char* strc = (*env)->GetStringUTFChars(env, str, NULL);
//		jputs(strc);
//		(*env)->ReleaseStringChars(env, str, strc);
//	}
	TypeMapEntry* tme = JyNI_JythonTypeEntry_FromName(cName);
	if (tme)
	{
//		jputs("initialize handle:");
//		if (tme->type_name) jputs(tme->type_name);
//		if (tme->py_type) jputs(tme->py_type->tp_name);

		//No need to incref here, since JyNI_InitPyObject returns NEW ref.
		return JyNI_InitPyObject(tme, jythonPyObject);
	} else
	{
		ExceptionMapEntry* eme = JyNI_PyExceptionMapEntry_FromPyExceptionType(
			JyNI_PyExceptionType_FromJythonExceptionType(
			(*env)->CallObjectMethod(env, jythonPyObject, pyObjectGetType)));
		if (eme)
		{
			// No need to incref here, since JyNI_InitPyException returns NEW ref.
			PyObject* er = JyNI_InitPyException(eme, jythonPyObject);
			return er;
		} else
		{
			jobject tpe = (*env)->CallObjectMethod(env, jythonPyObject, pyObjectGetType);
//			jputsLong(__LINE__);
//			JyNI_jprintJ(tpe);
//			JyNI_jprintJ(jythonPyObject);
//			jobject old_cls0 = (*env)->CallStaticObjectMethod(env, JyNIClass,
//									JyNI_getTypeOldStyleParent, jythonPyObject);
//			if (old_cls0) JyNI_jprintJ(old_cls0);
			PyTypeObject* pytpe;
			PyObject* result;
			if ((*env)->IsSameObject(env, jythonPyObject, tpe))
			{
//				jputsLong(__LINE__);
//				jputs(cName);
//				JyNI_jprintJ(tpe);
//				JyNI_jprintJ(jythonPyObject);
				pytpe = JyNI_InitPyObjectNativeTypePeer(tpe);
				result = pytpe;//JyNI_InitPyObjectSubtype(jythonPyObject, pytpe);
//				pytpe = NULL;
//				jputs(pytpe->tp_name);
			} else
			{
				pytpe = (PyTypeObject*) JyNI_PyObject_FromJythonPyObject(tpe);
//				jputsLong(__LINE__);
				result = JyNI_InitPyObjectSubtype(jythonPyObject, pytpe);
			}
//			if (!pytpe)
//			{
//				jputs("type seems to be not convertible. Try old hack...");
//				// We finally try a hack for some special new-style classes:
//				// todo: Remove this
//				jobject old_cls = (*env)->CallStaticObjectMethod(env, JyNIClass,
//						JyNI_getTypeOldStyleParent, jythonPyObject);
//				//jputsLong(old_cls);
//				if (!old_cls) return NULL;
//			}
//			if (!pytpe) jputs("pytpe is NULL");
//			else jputs(((PyTypeObject*) pytpe)->tp_name);
//			if ((*env)->IsInstanceOf(env, jythonPyObject, pyStringClass)) jputs("is string J");
//			if (PyType_FastSubclass((PyTypeObject*) pytpe, Py_TPFLAGS_STRING_SUBCLASS)) jputs("is string fast");
//			if (PyType_IsSubtype(pytpe, &PyString_Type)) jputs("is string");
			//JyNI_printJInfo(jythonPyObject);
			//PyObject* result = JyNI_InitPyObjectSubtype(jythonPyObject, pytpe);
			if (!result)
			{
				jputs("JyNI-warning: Unable to handle object:");
				if (pytpe) jputs(pytpe->tp_name);
				else jputs("failed to convert type");
			}
			return result;
		}
	}
	//PyObject* result = JyNI_NewPyObject_FromJythonPyObject(jythonPyObject);
	//Py_INCREF(result);
	//return result;
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 *
 * Does expect a "local" ref!
 * If the ref is stored somehow, it is made global
 * automatically.
 * (Todo: Check what happens if a global ref is made global again? I suppose, it just stays global.)
 */
inline PyObject* JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject)
{
	return _JyNI_PyObject_FromJythonPyObject(jythonPyObject, JNI_TRUE, JNI_TRUE, JNI_TRUE);
}

inline void JyNI_SyncPy2Jy(PyObject* op, JyObject* jy)
{
	//todo: take care of the other flags
//	jputs(__FUNCTION__);
	SyncFunctions* sync = (SyncFunctions*) JyNI_GetJyAttribute(jy, JyAttributeSyncFunctions);
//	jputsLong(__LINE__);
	if (sync != NULL && sync->py2jy != NULL) sync->py2jy(op, jy->jy);
//	jputsLong(__LINE__);
}

inline jobject JyNI_InitJythonPyException(ExceptionMapEntry* eme, PyObject* src, JyObject* srcJy)
{
	jobject dest = NULL;
	env(NULL);
	if (eme->exc_factory)
	{
		//Here, the actual values are not copied from src, since
		//src is truncated and does not actually contain these.
		//The tp_init method of the exception type is responsible
		//to fill in this data.
		dest = eme->exc_factory();
	} else
	{
		//Create base exception...
		jobject type = JyNI_JythonExceptionType_FromPyExceptionType((PyObject*) Py_TYPE(src));
		if (type)
			dest = (*env)->NewObject(env, pyBaseExceptionClass, pyBaseExceptionSubTypeConstructor, type);
		else
			dest = (*env)->NewObject(env, pyBaseExceptionClass, pyBaseExceptionEmptyConstructor);
	}
	if (!dest) return NULL;

	srcJy->jy = (*env)->NewWeakGlobalRef(env, dest);
	if (!(srcJy->flags & JY_HAS_JHANDLE_FLAG_MASK)) {  //some exc_factories might already init this
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, dest, (jlong) src);//, srcJy->flags & JY_TRUNCATE_FLAG_MASK);
		srcJy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	}
	srcJy->flags |= JY_INITIALIZED_FLAG_MASK;
	/* Is JyNI_GC_EnsureHeadObject needed for exceptions?
	 * For now we try it without.
	 */
	return srcJy->jy;
}



/*
 * Returns a jobject as a new JNI-local reference, unless op has a
 * cache-flag set. The caller should call deleteLocalRef on it after
 * work was done. In case the reference must be stored or cached for
 * future used, NewGlobalRef must be used.
 */
inline jobject JyNI_JythonPyObject_FromPyObject(PyObject* op)
{
	if (!op) return NULL;
	if (op == Py_None) return JyNone;
	if (op == Py_NotImplemented) return JyNotImplemented;
	if (op == Py_Ellipsis) return JyEllipsis;
	if (op == Py_True) return JyTrue;
	if (op == Py_False) return JyFalse;
	/* nullstring might be uninitialized, which is no matter here.
	 * Uninitialized nullstring cannot trigger "return JyEmptyString" here,
	 * since it would already have triggered "return NULL" some lines above.
	 */
	if ((PyStringObject*) op == nullstring) return JyEmptyString;
	if ((PyUnicodeObject*) op == unicode_empty) return JyEmptyUnicode;
	if (op->ob_type == NULL)
	{
		/* we assume that only type-objects (and as such also exception types) can have ob_type == NULL.
		 * So we call PyType_Ready to init it. However this might fail brutally, if ob_type was
		 * NULL for some other reason. However this would not go far without segfault then anyway.
		 */
		PyType_Ready((PyTypeObject*) op); //this is the wrong place to do this... it's just a quick hack. TODO: Find better solution...
	}
//	jboolean dbg = strcmp("numpy.bool_", Py_TYPE(op)->tp_name) == 0;
//	if (dbg)
//	{
//		jputs(__FUNCTION__);
//		jputs(Py_TYPE(op)->tp_name);
//		jputsLong(Is_Static_PyObject(op));
//		jputsLong(!(Py_TYPE(op)->tp_flags & Jy_TPFLAGS_DYN_OBJECTS));
//	}
	/* The following block cares for statically defined type objects.
	 * Heap-types are treated like ordinary objects.
	 * Note: Don't confuse the following line with checking op->ob_type rather than op itself.
	 */
	env(NULL);
	if (PyType_Check(op) && !PyType_HasFeature((PyTypeObject*) op, Py_TPFLAGS_HEAPTYPE))
	{
		if (PyExceptionClass_Check(op))
		{
			jobject er = JyNI_JythonExceptionType_FromPyExceptionType(op);
			if (er != NULL && !(*env)->IsSameObject(env, er, NULL)) return er;
			else return _JyNI_JythonPyTypeObject_FromPyTypeObject((PyTypeObject*) op, NULL);
		} else
		{
			jobject er = JyNI_JythonPyTypeObject_FromPyTypeObject((PyTypeObject*) op);
			if (er != NULL && !(*env)->IsSameObject(env, er, NULL)) return er;
		}
	}
	if (Is_StaticSingleton_NotBuiltin(op) && !PyType_Check(op))
	{
		//jputs("Extension-created native singleton encountered of type:");
		//jputsPy(Py_TYPE(op));
		return JyNI_InitStaticJythonPyObject(op);
	} else if (strcmp("numpy.dtype", Py_TYPE(op)->tp_name) == 0)
	{
		jputs("JyNI-Warning: numpy.dtype occurred as type of a non-static object!");
		jputs("JyNI is currently not able to handle this, segfault expected.");
	}
	//if (PyType_Check(op)) puts("appears to be a HeapType");
	JyObject* jy = AS_JY(op);
	if (JyObject_IS_INITIALIZED(jy))
	{
		if (jy->flags & JY_CACHE_ETERNAL_FLAG_MASK) {
			if (jy->flags & SYNC_ON_PY_TO_JY_FLAG_MASK)
				JyNI_SyncPy2Jy(op, jy);
			/* No local ref needed here, since JY_CACHE_ETERNAL guarantees immortality anyway.
			 * However we still create a local ref, because the caller might call DeleteLocalRef
			 * on it, which might fail by JVM-specific behavior.
			 */
			return (*env)->NewLocalRef(env, jy->jy);
		} else {
			//This might not work if called by a thread not attached to the JVM:
			jobject result = (*env)->NewLocalRef(env, jy->jy);
			/* Originally the following line read
			 *
			 * if (!(*env)->IsSameObject(env, result, NULL)) {
			 *
			 * According to
			 * http://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/functions.html#weak_global_references
			 * JNI returns NULL if the underlying object of a weak reference has been freed.
			 * So a check for result != NULL is sufficient and much more efficient here.
			 */
			if (result != NULL) {
				if (jy->flags & SYNC_ON_PY_TO_JY_FLAG_MASK)
					JyNI_SyncPy2Jy(op, jy);
				return result;
			} else {
//				if (!(*env)->IsSameObject(env, result, NULL)) jputs("False positive!");
				/* This is actually okay. It can happen if the Java-counterpart
				 * of a mirrored object was collected by Java-gc.
				 */
//				jputs("JyNI-Warning: Deleted object was not cleared!");
//				if (dbg)
//				{
//					jputs(Py_TYPE(op)->tp_name);
//					jputsLong(op);
//				}
				JyNI_CleanUp_JyObject(jy);
			}
		}
		//jputsLong(__LINE__);
//		if ((*env)->IsSameObject(env, jy->jy, NULL)) {
//			jputs("Alert!!! Initialized jy->jy became null!!");
//			jputsLong((jlong) jy->jy);
//			if (!op->ob_type) jputs("type is NULL");
//			if (!op->ob_type->tp_name) jputs("type name is NULL");
//			jputs(op->ob_type->tp_name);
//		}
		//return jy->jy;
	}
	TypeMapEntry* tme;
	if (jy->jy)
	{
		tme = (TypeMapEntry*) jy->jy;
	} else {
		tme = JyNI_JythonTypeEntry_FromPyType(Py_TYPE(op));
		if (!tme) {
			tme = JyNI_JythonTypeEntry_FromSubTypeWithPeer(Py_TYPE(op));
		}
	}
	//jputsLong(tme);
	//tme = JyNI_JythonTypeEntry_FromPyType(Py_TYPE(op));
	if (tme)// != NULL
	{
		//jputsLong(__LINE__);
		//jputs(Py_TYPE(op)->tp_name);
		//if (!tme->py_type) jputs("py_type is NULL");
		//if (!tme->py_type->tp_name) jputs("py_type name is NULL");
		//puts(tme->py_type->tp_name);
		//puts(Py_TYPE(op)->tp_name);
		//printf("%d_______%s\n", __LINE__, __FUNCTION__);
		return JyNI_InitJythonPyObject(tme, op, jy);
	}
	else
	{
		//jputsLong(__LINE__);
		ExceptionMapEntry* eme = JyNI_PyExceptionMapEntry_FromPyExceptionType(Py_TYPE(op));
		if (eme) {
			return JyNI_InitJythonPyException(eme, op, jy);
		} else
		{
			//jputsLong(__LINE__);
			//printf("%d_______%s\n", __LINE__, __FUNCTION__);
			//setup and return PyCPeer in this case...
			env(NULL);
			//The following lookup is not necessary, because if there already was a PyCPeer,
			//JyObject_IS_INITIALIZED would have evaluated true.
			//jobject er = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupCPeerFromHandle, (jlong) op);
			//if (er != NULL) return er;
			//puts("creating PyCPeer for Type:");
			//puts(Py_TYPE(op)->tp_name);
//			printf("op-address: %u\n", (jlong) op);
			//PyCPeer has to be created...
			Py_INCREF(op);
			//first obtain type:
			//Earlier we used this line, but it would not support HeapTypes
			//or natively defined metatypes:
			//jobject opType = JyNI_JythonPyTypeObject_FromPyTypeObject(Py_TYPE(op));
			//However, the general conversion method should also work and has this support:

			//Py_INCREF(Py_TYPE(op));
//				if (!opType) jputs("create PyCPeer with opType NULL");
//				else if (Py_TYPE(op) == &PyCFunction_Type)
//				{
//					jputs("PyCPeer for PyCFunction");
//					jputs(((PyCFunctionObject*) op)->m_ml->ml_name);
//					jputsLong((jlong) op);
//					if (flattenID == 0) flattenID = (jlong) op;
//					if (flattenID != 0) jputsLong((jlong) ((PyObject*) flattenID)->ob_type);
//				}
			//jputs("opType-address:");
			//printf("%u\n", (jlong) opType);
			//if (dbg) {jputs("create PyCPeer for ufunc"); jputsLong(op);}
//			jputs(Py_TYPE(op)->tp_name);
			if (PyType_Check(op)) {
				return _JyNI_JythonPyTypeObject_FromPyTypeObject((PyTypeObject*) op, NULL);
			}

			jobject opType = JyNI_JythonPyObject_FromPyObject((PyObject*) Py_TYPE(op));
			jobject er = PyObject_IS_GC(op) ?
					(*env)->NewObject(env, pyCPeerGCClass, pyCPeerGCConstructor, (jlong) op, opType) :
					(*env)->NewObject(env, pyCPeerClass, pyCPeerConstructor, (jlong) op, opType);
			//jobject er = (*env)->NewObject(env, pyCPeerClass, pyCPeerConstructor, (jlong) op, opType);
			//if (dbg) {jputsLong(__LINE__); jputsLong(op);}
			jy->flags |= JY_INITIALIZED_FLAG_MASK;
			/* JyNI_GC_EnsureHeadObject not needed here because the head is the
			 * CPeer itself and this has surely been properly initialized.
			 */
			jy->flags |= JY_CPEER_FLAG_MASK;
			jy->jy = (*env)->NewWeakGlobalRef(env, er);
			//(*env)->SetLongField(env, er, pyCPeerRefHandle, (jlong) ref);
			return er;//jy->jy;
			//}
		}
	}
}

inline jobject _JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type, jclass cls)
{
	//if (type == NULL) return NULL;
//	jputs(__FUNCTION__);
//	jputs(type->tp_name);
	env(NULL);
	if (cls != NULL)
	{
		return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNIGetPyType, cls);
	} else {
		//setup and return PyCPeerType in this case...
		jobject er = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupCPeerFromHandle, (jlong) type);
		//todo: Init this handle
		//if (er != NULL) return er;
		if (er != NULL && !(*env)->IsSameObject(env, er, NULL)) return er;
		//No JyObjects available for types!
		//JyObject* jy = AS_JY_NO_GC(type); //since we currently don't init types properly, GC-check would not work with PyTypeObjects
		//if (JyObject_IS_INITIALIZED(jy))
		//	return jy->jy; //no sync-attempt for types
		else {
			Py_INCREF(type);
			jobject er;// = (*env)->NewObject(env, pyCPeerTypeClass, pyCPeerTypeConstructor, (jlong) type);
			//if (Py_TYPE(type) == NULL) jputs("JyNI-warning: Attempt to convert PyTypeObject with NULL-type.");
			if (!PyObject_IS_GC((PyObject*) type))
			{
				if (Py_TYPE(type) == NULL || Py_TYPE(type) == &PyType_Type)
				{
					er = (*env)->NewObject(env, pyCPeerTypeClass, pyCPeerTypeWithNameAndDictConstructor,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict));
				} else
				{
					er = (*env)->NewObject(env, pyCPeerTypeClass, pyCPeerTypeWithNameDictTypeConstructor,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict),
							JyNI_JythonPyObject_FromPyObject((PyObject*) Py_TYPE(type)));
				}
			} else
			{
				if (Py_TYPE(type) == NULL || Py_TYPE(type) == &PyType_Type)
				{
					er = (*env)->NewObject(env, pyCPeerTypeGCClass, pyCPeerTypeGCConstructor,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict));
				} else
				{
					er = (*env)->NewObject(env, pyCPeerTypeGCClass, pyCPeerTypeGCConstructorSubtype,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict),
							JyNI_JythonPyObject_FromPyObject((PyObject*) Py_TYPE(type)));
				}
			}
			jweak ref = (*env)->NewWeakGlobalRef(env, er);
			//todo: Check whether we clean this jweak up properly.
			(*env)->SetLongField(env, er, pyCPeerTypeRefHandle, (jlong) ref);
			/* Okay, here we insert the method resolution order to Jython-side, because
			 * there are situations where Jython-PyType accesses its (or another PyType's)
			 * mro-field directly, e.g. in computeMro. This can cause a subsequent
			 * nullpointer exception, so we initialize it here.
			 */
			if (type->tp_mro) {
				if (PyTuple_Check(type->tp_mro)) {
					jobjectArray jmro = (*env)->NewObjectArray(env,
							PyTuple_GET_SIZE(type->tp_mro), pyObjectClass, NULL);
					jint pos;
					for (pos = 0; pos < PyTuple_GET_SIZE(type->tp_mro); ++pos)
						(*env)->SetObjectArrayElement(env, jmro, pos,
								JyNI_JythonPyObject_FromPyObject(
										PyTuple_GET_ITEM(type->tp_mro, pos)));
					(*env)->SetObjectField(env, er, pyTypeMROField, jmro);
				} else {
					jputs("JyNI-warning: Encountered non-NULL MRO that is not a tuple:");
					jputs(type->tp_name);
				}
			} //else {
			// Happens frequently, so this warning is too annoying.
//				jputs("JyNI-warning: Encountered NULL-MRO:");
//				jputs(type->tp_name);
				// Should we set mro now to empty tuple?
				// For now we try to get away without it.
			//}
			(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, er, (jlong) type);
			return er;
		}
	}
}

inline jobject JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type)
{
	if (type == NULL) return NULL;
	//jclass cls = JyNI_JythonClassFromPyType((PyTypeObject*) type);
	return _JyNI_JythonPyTypeObject_FromPyTypeObject(type, JyNI_JythonClassFromPyType((PyTypeObject*) type));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 *
 * No support for derived types for now...
 * Only builtin types or natively declared types work.
 */
inline PyTypeObject* JyNI_PyTypeObject_FromJythonPyTypeObject(jobject jythonPyTypeObject)
{
//	jputs(__FUNCTION__);
	env(NULL);
	if ((*env)->IsInstanceOf(env, jythonPyTypeObject, pyCPeerTypeClass))
	{
		PyTypeObject* er = (PyTypeObject*) (*env)->GetLongField(env,
				jythonPyTypeObject, pyCPeerTypeObjectHandle);
		Py_INCREF(er);
		return er;
	}
	//jstring name = (*env)->CallStaticObjectMethod(env, pyTypeClass, pyTypeGetName, jythonPyTypeObject);
	jstring name = (*env)->CallObjectMethod(env, jythonPyTypeObject, pyTypeGetName);
//	jboolean isCopy;
//
//	char* utf_string = (*env)->GetStringUTFChars(env, name, &isCopy);
//	//"+1" for 0-termination:
//	char mName[strlen(utf_string)+1];
//	strcpy(mName, utf_string);
//	(*env)->ReleaseStringUTFChars(env, name, utf_string);
	cstr_from_jstring(mName, name);
	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].py_type != NULL && strcmp(builtinTypes[i].py_type->tp_name, mName) == 0)
		{
			Py_INCREF(builtinTypes[i].py_type);
			return builtinTypes[i].py_type;
		}
	}
	return NULL;
}

/*returns local ref!*/
inline jstring JyNI_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op)
{
	JyObject* jy = AS_JY_NO_GC(op);
	if (jy->flags & JY_INITIALIZED_FLAG_MASK)
	{
		return (*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) op), pyStringAsString);
	} else
	{
		return (*env)->NewStringUTF(env, PyString_AS_STRING(op));
	}
}

/*returns local ref!*/
inline jstring JyNI_interned_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op)
{
	JyObject* jy = AS_JY_NO_GC(op);
	if (jy->flags & JY_INITIALIZED_FLAG_MASK)
	{
		jstring er = (*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) op), pyStringAsString);
		if (JyNI_HasJyAttribute(jy, JyAttributeStringInterned))
		{
			return er;
		} else
		{
			return (*env)->CallObjectMethod(env, er, stringIntern);
		}
	} else
	{
		return (*env)->CallObjectMethod(env, (*env)->NewStringUTF(env, PyString_AS_STRING(op)), stringIntern);
	}
}

inline int getWeakRefCount(JyObject* referent)
{
	return (int) JyNI_GetJyAttribute(referent, JyAttributeWeakRefCount);
}

inline int incWeakRefCount(JyObject* referent)
{
	int refCount = ((int) JyNI_GetJyAttribute(referent, JyAttributeWeakRefCount))+1;
	JyNI_AddOrSetJyAttribute(referent, JyAttributeWeakRefCount, (void*) refCount);
	return refCount;
}

inline int decWeakRefCount(JyObject* referent)
{
	int refCount = ((int) JyNI_GetJyAttribute(referent, JyAttributeWeakRefCount))-1;
	JyNI_AddOrSetJyAttribute(referent, JyAttributeWeakRefCount, (void*) refCount);
	return refCount < 0 ? -1 : refCount;
}

/* JY_DELEGATE indicates, when Functions like repr
 * or GetAttr should be delegated to Jython.
 * At least in Truncate-Case, this is necessary and
 * at least in CPeer-Case, it is impossible.
 *
 * - only delegate if truncated and not native subtype
 * - always delegate for non-native subtype
 * - never delegate if CPeer
 */
//#define JY_DELEGATE(v, flags) ((flags & JY_TRUNCATE_FLAG_MASK) && !(flags & JY_SUBTYPE_FLAG_MASK))
// native subtype indication: subtype-flag && CPeer-flag
// Delegation criterion: !CPeerflag && (truncated-flag | subtype-flag)
#define JY_DELEGATE(v, flags) ((flags & JY_POTENTIAL_DELEGATE) && !(flags & JY_CPEER_FLAG_MASK))

//Delegate in every case but CPeer-case:
//(actually the preferred variant, but lets JyNITkinterTest fail for some reason)
//For instance letting PyFunction be delegated causes Tkinter to fail. (Why?)

/*
 * JY_DELEGATE:
 * if initialized & !CPeer: true
 * if !initialized & existsClass: true
 */
//#define JY_DELEGATE(v, flags) ((flags & JY_INITIALIZED_FLAG_MASK) ? \
			!(flags & JY_CPEER_FLAG_MASK) : \
			(JyNI_JythonClassFromPyType(Py_TYPE(v)) != NULL))

/*returns local ref!*/
inline jobject JyNI_GetJythonDelegate(PyObject* v)
{
	if (!PyType_Check(v)) // && !PyExc_Check(v)
	{
		//jputs(Py_TYPE(v)->tp_name);
		//jputs("is no type object");
		//jputsLong(v);
		JyObject* jy = AS_JY(v);
//		jputsLong(jy->flags);
		if (JY_DELEGATE(v, jy->flags)) {
			//jputs("should delegate...");
			jobject er = JyNI_JythonPyObject_FromPyObject(v);
			return er;
		}
	} else {
	//		jobject cPeer = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupCPeerFromHandle, (jlong) v);
	//		if (cPeer == NULL) ...
		if (PyExceptionClass_Check(v)) return NULL; //never delegate for ExceptionType
		//puts(((PyTypeObject*) v)->tp_name);
		jclass cls = JyNI_JythonClassFromPyType((PyTypeObject*) v);
		if (cls != NULL) //delegate
		{
			jobject er = _JyNI_JythonPyTypeObject_FromPyTypeObject((PyTypeObject*) v, cls);
			return er;
			//env(NULL);
			//return JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, jyV, pyObject__repr__));
		}
	}
	return NULL;
}

inline jint JyNI_GetDLOpenFlags()
{
	env(0x00001 | 0x00100); //RTLD_LAZY | RTLD_GLOBAL
	return (*env)->CallStaticIntMethod(env, JyNIClass, JyNIGetDLOpenFlags);
}

inline void JyNI_CleanUp_JyObject(JyObject* obj)
{
//	jputs(__FUNCTION__);
//	jputs(Py_TYPE(FROM_JY(obj))->tp_name);
//	jputsLong(FROM_JY(obj));
	//if (FROM_JY(obj) == Py_None) return;
	//if (obj == NULL) return;
	//if (FROM_JY(obj) == NULL) return;
	//Todo: Maybe clean up gc-head here (?)
	if (obj->attr) JyNI_ClearJyAttributes(obj);
	if (obj->jy && (obj->flags & JY_INITIALIZED_FLAG_MASK))
	{
		env();
		if ((obj->flags & JY_HAS_JHANDLE_FLAG_MASK) && !(*env)->IsSameObject(env, obj->jy, NULL))
			(*env)->CallStaticVoidMethod(env, JyNIClass, JyNIClearNativeHandle, obj->jy);
//		if (obj->flags & JY_CPEER_FLAG_MASK)
//			(*env)->DeleteWeakGlobalRef(env, obj->jy);
//		else
//			(*env)->DeleteGlobalRef(env, obj->jy);
		(*env)->DeleteWeakGlobalRef(env, obj->jy);
		obj->flags = 0;
		obj->jy = NULL;
	}
}


/* -----------------  JNI-section  ----------------- */

//singletons:
JavaVM* java;
jweak JyNone;
jweak JyNotImplemented;
jweak JyEllipsis;
jweak JyEmptyFrozenSet;
jweak JyEmptyString;
jweak JyEmptyUnicode;
jweak JyEmptyTuple;
jweak JyTrue;
jweak JyFalse;
jweak length0StringArray;
jweak length0PyObjectArray;

jclass objectClass;
jmethodID objectToString;
jmethodID objectGetClass;

jclass classClass;
jmethodID classEquals;

jclass arrayListClass;
jmethodID arrayListConstructor;
jmethodID listAdd;
//jmethodID listClear;
//jmethodID listRemove;
//jmethodID listRemoveIndex;
//jmethodID listSet;

//jclass systemClass;
//jmethodID arraycopy;

jclass JyNIClass;
jmethodID JyNISetNativeHandle;
//jmethodID JyNIRegisterNativeStaticTypeDict;
jmethodID JyNI_registerNativeStaticJyGCHead;
jmethodID JyNI_getNativeStaticJyGCHead;
jmethodID JyNILookupNativeHandle;
jmethodID JyNILookupCPeerFromHandle;
jmethodID JyNIClearNativeHandle;
//jmethodID JyNIConstructDefaultObject;
jmethodID JyNIGetDLOpenFlags;
jmethodID JyNIGetJyObjectByName;
jmethodID JyNIGetPyObjectByName;
jmethodID JyNIGetPyType;
jmethodID JyNI_getNativeAvailableKeysAndValues;
jmethodID JyNIExceptionByName;
//jmethodID JyErr_SetCurExc;
//jmethodID JyErr_GetCurExc;
jmethodID JyNIJyErr_InsertCurExc;
jmethodID JyNIJyErr_PrintEx;
//jmethodID JyNIPyErr_Restore;
//jmethodID JyNIPyErr_Clear;
//jmethodID JyNIPyErr_Occurred;
jmethodID JyNIPyErr_ExceptionMatches;
//jmethodID JyNIPyErr_SetObject;
//jmethodID JyNIPyErr_SetString;
//jmethodID JyNIPyErr_SetNone;
//jmethodID JyNIPyErr_NoMemory;
//jmethodID JyNIPyErr_Fetch;
jmethodID JyNIPyErr_WriteUnraisable;
jmethodID JyNIGetDLVerbose;
jmethodID JyNI_PyImport_FindExtension;
jmethodID JyNIGetPyDictionary_Next;
jmethodID JyNIGetPySet_Next;
jmethodID JyNIPyImport_GetModuleDict;
jmethodID JyNIPyImport_AddModule;
jmethodID JyNIJyNI_GetModule;
jmethodID JyNISlice_compare;
jmethodID JyNIPrintPyLong;
jmethodID JyNILookupNativeHandles;
jmethodID JyNI_prepareKeywordArgs;
jmethodID JyNI_getCurrentThreadID;
jmethodID JyNI_pyCode_co_code;
jmethodID JyNI_pyCode_co_flags;
jmethodID JyNI_pyCode_co_lnotab;
jmethodID JyNI_jPrint;
jmethodID JyNI_jPrintLong;
jmethodID JyNI_jPrintHash;
//jmethodID JyNIPySet_pop;
jmethodID JyNI_makeGCHead;
jmethodID JyNI_makeStaticGCHead;
jmethodID JyNI_gcDeletionReport;
jmethodID JyNI_waitForCStubs;
jmethodID JyNI_addJyNICriticalObject;
jmethodID JyNI_removeJyNICriticalObject;
jmethodID JyNI_suspendPyInstanceFinalizer;
jmethodID JyNI_restorePyInstanceFinalizer;
jmethodID JyNI_createWeakReferenceFromNative;
jmethodID JyNI_createProxyFromNative;
jmethodID JyNI_createCallableProxyFromNative;
jmethodID JyNI_getGlobalRef;
jmethodID JyNI_getTypeNameForNativeConversion;
jmethodID JyNI_getTypeOldStyleParent;
jmethodID JyNI_getJythonGlobals;

jclass JyTStateClass;
jmethodID JyTState_setRecursionLimit;
jmethodID JyTState_prepareNativeThreadState;
jfieldID JyTState_nativeRecursionLimitField;

jclass JyNIDictNextResultClass;
jfieldID JyNIDictNextResultKeyField;
jfieldID JyNIDictNextResultValueField;
jfieldID JyNIDictNextResultNewIndexField;
jfieldID JyNIDictNextResultKeyHandleField;
jfieldID JyNIDictNextResultValueHandleField;

jclass JyNISetNextResultClass;
jfieldID JyNISetNextResultKeyField;
jfieldID JyNISetNextResultNewIndexField;
jfieldID JyNISetNextResultKeyHandleField;

jclass JyListClass;
jmethodID JyListFromBackendHandleConstructor;
//jmethodID JyListInstallToPyList;

jclass JySetClass;
jmethodID JySetFromBackendHandleConstructor;
//jmethodID JySetInstallToPySet;

jclass JyLockClass;
jmethodID JyLockConstructor;
jmethodID JyLockAcquire;
jmethodID JyLockRelease;

jclass NativeActionClass;
jmethodID NativeAction_constructor;
jfieldID NativeAction_action;
jfieldID NativeAction_obj;
jfieldID NativeAction_nativeRef1;
jfieldID NativeAction_nativeRef2;
jfieldID NativeAction_cTypeName;
jfieldID NativeAction_cMethod;
jfieldID NativeAction_cLine;
jfieldID NativeAction_cFile;

jclass JyReferenceMonitorClass;
jmethodID JyRefMonitorMakeDebugInfo;
jmethodID JyRefMonitorAddAction;

jclass pyCPeerClass;
jmethodID pyCPeerConstructor;
jfieldID pyCPeerObjectHandle;
//jfieldID pyCPeerRefHandle;

jclass pyCPeerGCClass;
jmethodID pyCPeerGCConstructor;
//jfieldID pyCPeerLinksHandle;

jclass pyCPeerTypeGCClass;
jmethodID pyCPeerTypeGCConstructor;
jmethodID pyCPeerTypeGCConstructorSubtype;

// Subclasses:
jclass pyDictCPeerClass;
jclass pyTupleCPeerClass;
jmethodID pyTupleCPeerConstructor;

jclass jyGCHeadClass;
jmethodID traversableGCHeadSetLinks;
jmethodID traversableGCHeadSetLink;
jmethodID traversableGCHeadInsertLink;
jmethodID traversableGCHeadClearLink;
jmethodID traversableGCHeadClearLinksFromIndex;
jmethodID traversableGCHeadEnsureSize;
jmethodID pyObjectGCHeadSetObject;
jmethodID jyGCHeadGetHandle;

jclass cPeerInterface;
jclass cPeerNativeDelegateInterface;
jmethodID super__call__;
jmethodID super__findattr_ex__;
jmethodID super__setattr__;
jmethodID super__str__;
jmethodID super__repr__;
jmethodID super__finditem__;
jmethodID super__setitem__;
jmethodID super__delitem__;
jmethodID super__len__;
jmethodID super_toString;

jclass pyCPeerTypeClass;
//jmethodID pyCPeerTypeConstructor;
jmethodID pyCPeerTypeWithNameAndDictConstructor;
jmethodID pyCPeerTypeWithNameDictTypeConstructor;
jfieldID pyCPeerTypeObjectHandle;
jfieldID pyCPeerTypeRefHandle;

jclass stringClass;
jmethodID stringFromBytesAndCharsetNameConstructor;
jmethodID stringToUpperCase;
jmethodID stringGetBytesUsingCharset;
jmethodID stringIntern;

jclass bigIntClass;
jmethodID bigIntFromStringConstructor;
jmethodID bigIntFromStringRadixConstructor;
jmethodID bigIntegerFromByteArrayConstructor;
jmethodID bigIntegerFromSignByteArrayConstructor;
jmethodID bigIntToByteArray;
jmethodID bigIntSignum;
jmethodID bigIntToStringRadix;

jclass pyPyClass;
jmethodID pyPyGetThreadState;
jmethodID pyPyGetFrame;
jmethodID pyPyIsSubClass;
jmethodID pyPyWarning;
jmethodID pyPyNewString;
jmethodID pyPyNewUnicode;
jmethodID pyPyNewIntFromInt;
jmethodID pyPyNewIntFromLong;
jmethodID pyPyNewLongFromBigInt;
jmethodID pyPyNewLongFromInt;
jmethodID pyPyNewLongFromLong;
jmethodID pyPyNewLongFromString;
jmethodID pyPyNewFloatFromDouble;
jmethodID pyPyWarningStck;
jmethodID pyPyExplicitWarning;
jfieldID pyPyImportError;
jfieldID pyPySystemError;
jfieldID pyPyUnicodeError;
jfieldID pyPyUnicodeEncodeError;
jfieldID pyPyUnicodeDecodeError;
jfieldID pyPyUnicodeTranslateError;
jfieldID pyPyUnicodeWarning;
jmethodID pyPyUnicodeErrorFactory;
jmethodID pyPyUnicodeEncodeErrorFactory;
jmethodID pyPyUnicodeDecodeErrorFactory;
jmethodID pyPyUnicodeTranslateErrorFactory;
jmethodID pyPyRaiseUnicodeWarning;
jmethodID pyPyMakeClass;
//pre-allocated:
jfieldID pyPyIntegerCache;
jfieldID pyPyLetters;
//No unicode letters are cached in Jython

jclass pyObjectClass;
jmethodID pyObjectGetType;
jmethodID pyObjectAsString;
jfieldID pyObjectType;
jmethodID pyObject__getattr__;
jmethodID pyObject__findattr__;
jmethodID pyObject__setattr__;
jmethodID pyObject__repr__;
jmethodID pyObject__cmp__;
jmethodID pyObject__reduce__;
jmethodID pyObject__and__;
jmethodID pyObject__or__;
jmethodID pyObject__sub__;
jmethodID pyObject__xor__;
jmethodID pyObject__isub__;
jmethodID pyObject__call__;
jmethodID pyObject__str__;
jmethodID pyObject__finditem__;
//jmethodID pyObject__getitem__;
jmethodID pyObject__setitem__;
jmethodID pyObject__delitem__;
jmethodID pyObject__contains__;
jmethodID pyObject__len__;
jmethodID pyObject__getslice__;
jmethodID pyObject__nonzero__;
jmethodID pyObject_cmp;
jmethodID pyObjectGetDict;
jmethodID pyObjectFastGetDict;
jmethodID pyObjectSetDict;
jmethodID pyObjectIsCallable;
jmethodID pyObjectHashCode;

jclass pyThreadStateClass;
jfieldID pyThreadStateExceptionField;
//jfieldID pyThreadStateRecursionDepth;
//jmethodID pyThreadStateEnterRecursiveCall;
//jmethodID pyThreadStateLeaveRecursiveCall;
jmethodID pyThreadStateEnterRepr;
jmethodID pyThreadStateExitRepr;
jmethodID pyThreadStateGetCompareStateDict;

jclass pyInstanceClass;
jmethodID pyInstanceConstructor;
jfieldID pyInstanceInstclassField;
jfieldID pyInstance__dict__;
jmethodID pyInstanceIsSequenceType;

jclass pyFrameClass;

jclass pyTracebackClass;
jmethodID pyTracebackByTracebackFrameConstructor;

jclass pyExceptionClass;
jfieldID pyExceptionTypeField;
jfieldID pyExceptionValueField;
jfieldID pyExceptionTracebackField;
jmethodID pyExceptionFullConstructor;
jmethodID pyExceptionTypeValueConstructor;
jmethodID pyExceptionTypeConstructor;
jmethodID pyExceptionNormalize;
jmethodID pyExceptionMatch;
jmethodID pyExceptionIsExceptionClass;
jmethodID pyExceptionIsExceptionInstance;

jclass pyBooleanClass;
//jmethodID pyBooleanConstructor;
jfieldID pyBoolVal;

jclass pyArrayClass;
jmethodID pyArrayGetTypecode;

jclass pyTupleClass;
jmethodID pyTupleConstructor;
jmethodID pyTupleByPyObjectArrayBooleanConstructor;
jmethodID pyTupleSize;
jmethodID pyTuplePyGet;
//jfieldID pyTupleArray;
//jmethodID pyTupleGetArray;

jclass pyListClass;
jmethodID pyListConstructor;
jmethodID pyListByPyObjectArrayConstructor;
jmethodID pyListFromList;
jmethodID pyListSize;
jmethodID pyListPyGet;
jmethodID pyListPySet;
jmethodID pyListAppend;
jmethodID pyListInsert;
jmethodID pyListSort;
jmethodID pyListReverse;
jmethodID pyListGetArray;
jfieldID pyListBackend;

jclass pyAbstractDictClass;
jmethodID pyAbstractDictCopy;
jmethodID pyAbstractDictMerge;
jmethodID pyAbstractDictMergeFromSeq;

jclass pyDictClass;
jmethodID pyDictConstructor;
jmethodID pyDictByPyObjectArrayConstructor;

jclass pyStringMapClass;

jclass pyIntClass;
jmethodID pyIntConstructor;
jmethodID pyIntGetValue;
//jmethodID pyIntAsInt;
//jmethodID pyIntAsLong;

jclass pyLongClass;
jmethodID pyLongByBigIntConstructor;
jmethodID pyLongByLongConstructor;
jmethodID pyLongAsLong;
jmethodID pyLongGetLong;
jmethodID pyLongGetValue;
jmethodID pyLongBit_length;
//jmethodID pyLongToString;

jclass pyUnicodeClass;
jmethodID pyUnicodeByJStringConstructor;
jmethodID pyUnicodeAsString;

jclass pyStringClass;
jmethodID pyStringByJStringConstructor;
jmethodID pyStringAsString;

jclass pyFloatClass;
jmethodID pyFloatByDoubleConstructor;
jmethodID pyFloatAsDouble;
jfieldID pyFloatTypeField;

jclass pyComplexClass;
jmethodID pyComplexBy2DoubleConstructor;
jfieldID pyComplexRealField;
jfieldID pyComplexImagField;

jclass pySequenceClass;
jmethodID pySequenceLen;
jmethodID pySequenceGetItem;

jclass pySequenceListClass;
jmethodID pySequenceListSize;
jmethodID pySequenceListPyGet;

jclass pyTypeClass;
jmethodID pyTypeGetName;
jmethodID pyTypeSetName;
jmethodID pyTypeGetBase;
jmethodID pyTypeGetBases;
jmethodID pyTypeGetMro;
jmethodID pyTypeIsSubType;
jfieldID pyTypeMROField;
jfieldID pyTypeNameField;

jclass pyCodecsClass;
jmethodID pyCodecsDecode;
jmethodID pyCodecsEncode;
jmethodID pyCodecsGetDefaultEncoding;
jmethodID pyCodecsLookup;
jmethodID pyCodecsRegister;
jmethodID pyCodecsLookupError;
jmethodID pyCodecsRegisterError;

//jclass jyObjectClass;
//jfieldID jyObjectNativeHandleField;
//jfieldID jyObjectObjectField;

jclass pyFileClass;
jmethodID pyFileWrite;

jclass pyNotImplementedClass;
jclass pyNoneClass;

jclass pyModuleClass;
jmethodID pyModuleByStringConstructor;
jmethodID pyModuleGetDict;
jmethodID pyModule__setattr__;
jmethodID pyModule__delattr__;

jclass pyCellClass;
jmethodID pyCellConstructor;
jfieldID pyCell_ob_ref;

jclass pyClassClass;
jmethodID pyClassClassobj___new__;
jfieldID pyClass__bases__;
jfieldID pyClass__dict__;
jfieldID pyClass__name__;

jclass pyMethodClass;
jmethodID pyMethodConstructor;
jfieldID pyMethod__func__;
jfieldID pyMethod__self__;
jfieldID pyMethodImClass;

jclass pyFunctionClass;
jmethodID pyFunctionConstructor;
jfieldID pyFunction__code__;
jmethodID pyFunctionSetCode;
jmethodID pyFunctionGetFuncGlobals;
jfieldID pyFunction__module__;
jmethodID pyFunctionGetFuncDefaults;
jmethodID pyFunctionSetFuncDefaults;
jfieldID pyFunctionFuncClosure;
jfieldID pyFunction__doc__;
//jmethodID pyFunctionGetDict; use pyObjectGetDict
//jemthodID pyFunctionSetDict; use pyObjectSetDict
jfieldID pyFunction__name__;
//jmethodID pyFunctionCall; use pyObject__call__

jclass pyClassMethodClass;
jmethodID pyClassMethodConstructor;
jfieldID pyClassMethod_callable;

jclass pyStaticMethodClass;
jmethodID pyStaticMethodConstructor;
jfieldID pyStaticMethod_callable;

//jclass pyMethodDescrClass;
//jclass pyClassMethodDescrClass;

jclass pyDictProxyClass;
jmethodID pyDictProxyConstructor;
jfieldID pyDictProxy_dict;

jclass pyPropertyClass;
jmethodID pyPropertyConstructor;
jfieldID pyProperty_fget;
jfieldID pyProperty_fset;
jfieldID pyProperty_fdel;
jfieldID pyProperty_doc;
jfieldID pyProperty_docFromGetter;

jclass pyBaseStringClass;
jclass pyXRangeClass;
jclass pySequenceIterClass;
jclass pyFastSequenceIterClass;
jclass pyReversedIteratorClass;

jclass pyBaseSetClass;
jfieldID pyBaseSet_set;
jmethodID pyBaseSetSize;
//jmethodID pyBaseSetClear;
//jmethodID pyBaseSetContains;
//jmethodID pyBaseSetRemove;
//jmethodID pyBaseSetAdd;
jmethodID pyBaseSet_update;//+
//jmethodID pyBaseSetbaseset_union;//-
jmethodID pyBaseSetbaseset_issubset;//+
jmethodID pyBaseSetbaseset_issuperset;//-
jmethodID pyBaseSetbaseset_isdisjoint;//+
jmethodID pyBaseSetbaseset_difference;//+
jmethodID pyBaseSetbaseset_differenceMulti;//+
jmethodID pyBaseSetbaseset_symmetric_difference;//-
jmethodID pyBaseSetbaseset_intersection;//-
//jmethodID pyBaseSetbaseset_copy;//-
jmethodID pyBaseSetbaseset___contains__;//+
//jmethodID pyBaseSetbaseset_difference_multi;
//jmethodID pyBaseSetbaseset_intersection_multi;
//jmethodID pyBaseSetbaseset___iter__;

jclass pySetClass;
jmethodID pySetFromIterableConstructor;
jmethodID pySetset_pop;//+
jmethodID pySetset_clear;//+
//jmethodID pySetset_remove;
jmethodID pySetset_discard;//+
jmethodID pySetset_add;//+
jmethodID pySetset_difference_update;//+
jmethodID pySetset_intersection_update;//+
jmethodID pySetset_symmetric_difference_update;//+
//jmethodID pySetset___isub__;
//jmethodID pySetset___iand__;
//jmethodID pySetset___ixor__;

jclass pyFrozenSetClass;
jmethodID pyFrozenSetFromIterableConstructor;
//jmethodID pyFrozenSetSize;

jclass pyEnumerationClass;

jclass pySliceClass;
jmethodID pySliceFromStartStopStepConstructor;
jmethodID pySliceToString;
jmethodID pySliceGetStart;
jmethodID pySliceGetStop;
jmethodID pySliceGetStep;
jmethodID pySliceIndicesEx;

jclass pyEllipsisClass;
jclass pyGeneratorClass;

jclass pyWeakReferenceClass;
jclass pyWeakProxyClass;
jclass pyWeakCallableProxyClass;

//jclass pyCodeClass;
jfieldID pyCode_co_name;

//jclass pyBaseCodeClass;
jfieldID pyBaseCode_co_argcount;
jfieldID pyBaseCode_co_cellvars;
jfieldID pyBaseCode_co_filename;
jfieldID pyBaseCode_co_firstlineno;
//jfieldID pyBaseCode_co_flags;
jfieldID pyBaseCode_co_freevars;
jfieldID pyBaseCode_co_nlocals;
jfieldID pyBaseCode_co_varnames;

jclass pyBytecodeClass;
jmethodID pyBytecodeConstructor;
//jfieldID pyBytecode_co_code;
jfieldID pyBytecode_co_consts;
//jfieldID pyBytecode_co_lnotab;
jfieldID pyBytecode_co_names;
jfieldID pyBytecode_co_stacksize;

jclass pyTableCodeClass;
//jfieldID pyTableCode_co_code;

jclass pyCallIterClass;
jclass pySuperClass;

jclass GlobalRefClass;
jmethodID GlobalRef_retryFactory;

jclass AbstractReferenceClass;
jmethodID AbstractReference_get;

jclass JyNIGlobalRefClass;
jmethodID JyNIGlobalRef_initNativeHandle;

jclass pyBaseExceptionClass;
//jfieldID pyBaseException__dict__;
jmethodID pyBaseExceptionEmptyConstructor;
jmethodID pyBaseExceptionSubTypeConstructor;
jmethodID pyBaseException__init__;
jmethodID pyBaseException__setstate__;
jmethodID pyBaseException__unicode__;
jfieldID pyBaseExceptionArgs;
jmethodID pyBaseExceptionSetArgs;
jmethodID pyBaseExceptionGetMessage;
jmethodID pyBaseExceptionSetMessage;

jclass pyByteArrayClass;
jclass pyBufferClass;
jclass pyMemoryViewClass;

jclass __builtin__Class;
//jmethodID __builtin__Import;
jmethodID __builtin__ImportLevel;

jclass impClass;
jmethodID imp_importName;
jmethodID imp_reload;

jclass exceptionsClass;
jmethodID exceptionsKeyError;
jmethodID exceptionsKeyError__str__;
jmethodID exceptionsEnvironmentError;
jmethodID exceptionsEnvironmentError__init__;
jmethodID exceptionsEnvironmentError__str__;
jmethodID exceptionsEnvironmentError__reduce__;
jmethodID exceptionsSyntaxError;
jmethodID exceptionsSyntaxError__init__;
jmethodID exceptionsSyntaxError__str__;
jmethodID exceptionsSystemExit;
jmethodID exceptionsSystemExit__init__;

jmethodID exceptionsUnicodeError;
#ifdef Py_USING_UNICODE
jmethodID exceptionsGetStart;
jmethodID exceptionsGetEnd;
jmethodID exceptionsGetString;
jmethodID exceptionsGetUnicode;
jmethodID exceptionsUnicodeError__init__;
jmethodID exceptionsUnicodeEncodeError;
jmethodID exceptionsUnicodeEncodeError__init__;
jmethodID exceptionsUnicodeEncodeError__str__;
jmethodID exceptionsUnicodeDecodeError;
jmethodID exceptionsUnicodeDecodeError__init__;
jmethodID exceptionsUnicodeDecodeError__str__;
jmethodID exceptionsUnicodeTranslateError;
jmethodID exceptionsUnicodeTranslateError__init__;
jmethodID exceptionsUnicodeTranslateError__str__;
#endif

inline jint initJNI(JNIEnv *env)
{
	jclass objectClassLocal = (*env)->FindClass(env, "java/lang/Object");
	if (objectClassLocal == NULL) { return JNI_ERR;}
	objectClass = (jclass) (*env)->NewWeakGlobalRef(env, objectClassLocal);
	(*env)->DeleteLocalRef(env, objectClassLocal);
	objectToString = (*env)->GetMethodID(env, objectClass, "toString", "()Ljava/lang/String;");
	objectGetClass = (*env)->GetMethodID(env, objectClass, "getClass", "()Ljava/lang/Class;");

	jclass classClassLocal = (*env)->FindClass(env, "java/lang/Class");
	if (classClassLocal == NULL) { return JNI_ERR;}
	classClass = (jclass) (*env)->NewWeakGlobalRef(env, classClassLocal);
	(*env)->DeleteLocalRef(env, classClassLocal);
	classEquals = (*env)->GetMethodID(env, classClass, "equals", "(Ljava/lang/Object;)Z");

//	jclass systemClassLocal = (*env)->FindClass(env, "java/lang/System");
//	if (systemClassLocal == NULL) { return JNI_ERR;}
//	systemClass = (jclass) (*env)->NewWeakGlobalRef(env, systemClassLocal);
//	(*env)->DeleteLocalRef(env, systemClassLocal);
//	arraycopy = (*env)->GetStaticMethodID(env, systemClass, "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V");

	jclass stringClassLocal = (*env)->FindClass(env, "java/lang/String");
	if (stringClassLocal == NULL) { return JNI_ERR;}
	stringClass = (jclass) (*env)->NewWeakGlobalRef(env, stringClassLocal);
	(*env)->DeleteLocalRef(env, stringClassLocal);
	stringFromBytesAndCharsetNameConstructor = (*env)->GetMethodID(env, stringClass, "<init>", "([BLjava/lang/String;)V");
	stringToUpperCase = (*env)->GetMethodID(env, stringClass, "toUpperCase", "()Ljava/lang/String;");
	stringGetBytesUsingCharset = (*env)->GetMethodID(env, stringClass, "getBytes", "(Ljava/lang/String;)[B");
	stringIntern = (*env)->GetMethodID(env, stringClass, "intern", "()Ljava/lang/String;");

	jclass bigIntClassLocal = (*env)->FindClass(env, "java/math/BigInteger");
	if (bigIntClassLocal == NULL) { return JNI_ERR;}
	bigIntClass = (jclass) (*env)->NewWeakGlobalRef(env, bigIntClassLocal);
	(*env)->DeleteLocalRef(env, bigIntClassLocal);
	bigIntegerFromByteArrayConstructor = (*env)->GetMethodID(env, bigIntClass, "<init>", "([B)V");
	bigIntegerFromSignByteArrayConstructor = (*env)->GetMethodID(env, bigIntClass, "<init>", "(I[B)V");
	bigIntFromStringConstructor = (*env)->GetMethodID(env, bigIntClass, "<init>", "(Ljava/lang/String;)V");
	bigIntFromStringRadixConstructor = (*env)->GetMethodID(env, bigIntClass, "<init>", "(Ljava/lang/String;I)V");
	bigIntToByteArray = (*env)->GetMethodID(env, bigIntClass, "toByteArray", "()[B");
	bigIntSignum = (*env)->GetMethodID(env, bigIntClass, "signum", "()I");
	bigIntToStringRadix = (*env)->GetMethodID(env, bigIntClass, "toString", "(I)Ljava/lang/String;");

	jclass arrayListClassLocal = (*env)->FindClass(env, "java/util/ArrayList");
	arrayListClass = (*env)->NewWeakGlobalRef(env, arrayListClassLocal);
	(*env)->DeleteLocalRef(env, arrayListClassLocal);
	arrayListConstructor = (*env)->GetMethodID(env, arrayListClass, "<init>", "(I)V");
	jclass listClassLocal = (*env)->FindClass(env, "java/util/List");
	listAdd = (*env)->GetMethodID(env, listClassLocal, "add", "(Ljava/lang/Object;)Z");
	(*env)->DeleteLocalRef(env, listClassLocal);

	return JNI_VERSION_1_2;
}

inline jint initJyNI(JNIEnv *env)
{
	/*jclass jyObjectClassLocal = (*env)->FindClass(env, "JyNI/JyObject");
	if (jyObjectClassLocal == NULL) { return JNI_ERR;}
	jyObjectClass = (jclass) (*env)->NewWeakGlobalRef(env, jyObjectClassLocal);
	(*env)->DeleteLocalRef(env, jyObjectClassLocal);
	jyObjectNativeHandleField = (*env)->GetFieldID(env, jyObjectClass, "nativeHandle", "I");
	jyObjectObjectField = (*env)->GetFieldID(env, jyObjectClass, "object", "Lorg/python/core/PyObject;");*/

	jclass JyNIClassLocal = (*env)->FindClass(env, "JyNI/JyNI");
	JyNIClass = (jclass) (*env)->NewWeakGlobalRef(env, JyNIClassLocal);
	(*env)->DeleteLocalRef(env, JyNIClassLocal);
	JyNISetNativeHandle = (*env)->GetStaticMethodID(env, JyNIClass, "setNativeHandle", "(Lorg/python/core/PyObject;J)V");
	//JyNIRegisterNativeStaticTypeDict = (*env)->GetStaticMethodID(env, JyNIClass, "registerNativeStaticTypeDict", "(Ljava/lang/String;Lorg/python/core/PyDictionary;)V");
	JyNI_registerNativeStaticJyGCHead = (*env)->GetStaticMethodID(env, JyNIClass, "registerNativeStaticJyGCHead", "(JLJyNI/gc/JyGCHead;)V");
	JyNI_getNativeStaticJyGCHead = (*env)->GetStaticMethodID(env, JyNIClass, "getNativeStaticJyGCHead", "(J)LJyNI/gc/JyGCHead;");
	JyNILookupNativeHandle = (*env)->GetStaticMethodID(env, JyNIClass, "lookupNativeHandle", "(Lorg/python/core/PyObject;)J");
	JyNIClearNativeHandle = (*env)->GetStaticMethodID(env, JyNIClass, "clearNativeHandle", "(Lorg/python/core/PyObject;)V");
	JyNILookupCPeerFromHandle = (*env)->GetStaticMethodID(env, JyNIClass, "lookupCPeerFromHandle", "(J)Lorg/python/core/PyObject;");
	//JyNIConstructDefaultObject = (*env)->GetStaticMethodID(env, JyNIClass, "constructDefaultObject", "(Ljava/lang/Class;)Lorg/python/core/PyObject;");
	JyNIGetDLOpenFlags = (*env)->GetStaticMethodID(env, JyNIClass, "getDLOpenFlags", "()I");
	JyNIGetDLVerbose = (*env)->GetStaticMethodID(env, JyNIClass, "getDLVerbose", "()I");
	JyNIGetJyObjectByName = (*env)->GetStaticMethodID(env, JyNIClass, "getJyObjectByName", "(Ljava/lang/String;)J");
	JyNIGetPyObjectByName = (*env)->GetStaticMethodID(env, JyNIClass, "getPyObjectByName", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	JyNI_PyImport_FindExtension = (*env)->GetStaticMethodID(env, JyNIClass, "_PyImport_FindExtension", "(Ljava/lang/String;Ljava/lang/String;)Lorg/python/core/PyObject;");
	JyNI_getNativeAvailableKeysAndValues = (*env)->GetStaticMethodID(env, JyNIClass, "getNativeAvailableKeysAndValues", "(Lorg/python/core/PyDictionary;)[J");
	JyNIGetPyDictionary_Next = (*env)->GetStaticMethodID(env, JyNIClass, "getPyDictionary_Next", "(Lorg/python/core/PyDictionary;I)LJyNI/JyNIDictNextResult;");
	JyNIGetPySet_Next = (*env)->GetStaticMethodID(env, JyNIClass, "getPySet_Next", "(Lorg/python/core/BaseSet;I)LJyNI/JyNISetNextResult;");
	JyNIPyImport_GetModuleDict = (*env)->GetStaticMethodID(env, JyNIClass, "PyImport_GetModuleDict", "()Lorg/python/core/PyObject;");
	JyNIPyImport_AddModule = (*env)->GetStaticMethodID(env, JyNIClass, "PyImport_AddModule", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	JyNIJyNI_GetModule = (*env)->GetStaticMethodID(env, JyNIClass, "JyNI_GetModule", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	JyNISlice_compare = (*env)->GetStaticMethodID(env, JyNIClass, "slice_compare", "(Lorg/python/core/PySlice;Lorg/python/core/PySlice;)I");
	JyNIPrintPyLong = (*env)->GetStaticMethodID(env, JyNIClass, "printPyLong", "(Lorg/python/core/PyObject;)V");
	JyNILookupNativeHandles = (*env)->GetStaticMethodID(env, JyNIClass, "lookupNativeHandles", "(Lorg/python/core/PyList;)[J");
	JyNI_prepareKeywordArgs = (*env)->GetStaticMethodID(env, JyNIClass, "prepareKeywordArgs", "([Lorg/python/core/PyObject;Lorg/python/core/PyDictionary;)[Ljava/lang/String;");
	JyNI_getCurrentThreadID = (*env)->GetStaticMethodID(env, JyNIClass, "getCurrentThreadID", "()J");
	JyNI_pyCode_co_code = (*env)->GetStaticMethodID(env, JyNIClass, "JyNI_pyCode_co_code", "(Lorg/python/core/PyBaseCode;)Ljava/lang/String;");
	JyNI_pyCode_co_flags = (*env)->GetStaticMethodID(env, JyNIClass, "JyNI_pyCode_co_flags", "(Lorg/python/core/PyBaseCode;)I");
	JyNI_pyCode_co_lnotab = (*env)->GetStaticMethodID(env, JyNIClass, "JyNI_pyCode_co_lnotab", "(Lorg/python/core/PyBytecode;)Ljava/lang/String;");
	JyNI_jPrint = (*env)->GetStaticMethodID(env, JyNIClass, "jPrint", "(Ljava/lang/String;)V");
	JyNI_jPrintLong = (*env)->GetStaticMethodID(env, JyNIClass, "jPrint", "(J)V");
	JyNI_jPrintHash = (*env)->GetStaticMethodID(env, JyNIClass, "jPrintHash", "(Ljava/lang/Object;)V");
	//JyNIPySet_pop = (*env)->GetStaticMethodID(env, JyNIClass, "PySet_pop", "(Lorg/python/core/BaseSet;)Lorg/python/core/PyObject;");
	JyNI_makeGCHead = (*env)->GetStaticMethodID(env, JyNIClass, "makeGCHead", "(JZZ)LJyNI/gc/PyObjectGCHead;");
	JyNI_makeStaticGCHead = (*env)->GetStaticMethodID(env, JyNIClass, "makeStaticGCHead", "(JZ)LJyNI/gc/JyGCHead;");
	JyNI_gcDeletionReport = (*env)->GetStaticMethodID(env, JyNIClass, "gcDeletionReport", "([J[J)V");
	JyNI_waitForCStubs = (*env)->GetStaticMethodID(env, JyNIClass, "waitForCStubs", "()V");
	JyNI_addJyNICriticalObject = (*env)->GetStaticMethodID(env, JyNIClass, "addJyNICriticalObject", "(J)V");
	JyNI_removeJyNICriticalObject = (*env)->GetStaticMethodID(env, JyNIClass, "removeJyNICriticalObject", "(J)V");
	JyNI_suspendPyInstanceFinalizer = (*env)->GetStaticMethodID(env, JyNIClass, "suspendPyInstanceFinalizer", "(Lorg/python/core/PyInstance;)V");
	JyNI_restorePyInstanceFinalizer = (*env)->GetStaticMethodID(env, JyNIClass, "restorePyInstanceFinalizer", "(Lorg/python/core/PyInstance;)V");
	JyNI_createWeakReferenceFromNative = (*env)->GetStaticMethodID(env, JyNIClass, "createWeakReferenceFromNative",
			"(Lorg/python/core/PyObject;JLorg/python/core/PyObject;)Lorg/python/modules/_weakref/ReferenceType;");
	JyNI_createProxyFromNative = (*env)->GetStaticMethodID(env, JyNIClass, "createProxyFromNative",
			"(Lorg/python/core/PyObject;JLorg/python/core/PyObject;)Lorg/python/modules/_weakref/ProxyType;");
	JyNI_createCallableProxyFromNative = (*env)->GetStaticMethodID(env, JyNIClass, "createCallableProxyFromNative",
			"(Lorg/python/core/PyObject;JLorg/python/core/PyObject;)Lorg/python/modules/_weakref/CallableProxyType;");
	JyNI_getGlobalRef = (*env)->GetStaticMethodID(env, JyNIClass, "getGlobalRef",
			"(Lorg/python/core/PyObject;)Lorg/python/modules/_weakref/ReferenceBackend;");
	JyNI_getTypeNameForNativeConversion = (*env)->GetStaticMethodID(env, JyNIClass, "getTypeNameForNativeConversion",
			"(Lorg/python/core/PyObject;)Ljava/lang/String;");
	JyNI_getTypeOldStyleParent = (*env)->GetStaticMethodID(env, JyNIClass, "getTypeOldStyleParent",
			"(Lorg/python/core/PyObject;)Lorg/python/core/PyClass;");
	JyNI_getJythonGlobals = (*env)->GetStaticMethodID(env, JyNIClass, "getJythonGlobals",
			"()Lorg/python/core/PyObject;");

	//Error stuff:
	//JyErr_SetCurExc(ThreadState tstate, PyObject type, PyObject value, PyTraceback traceback)
	//PyException JyErr_GetCurExc(ThreadState tstate)
	//JyErr_InsertCurExc(ThreadState tstate)
//	JyErr_SetCurExc = (*env)->GetStaticMethodID(env, JyNIClass, "JyErr_SetCurExc",
//	        "(Lorg/python/core/ThreadState;Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyTraceback;)V");
//	JyErr_GetCurExc = (*env)->GetStaticMethodID(env, JyNIClass, "JyErr_GetCurExc",
//	        "(Lorg/python/core/ThreadState;)Lorg/python/core/PyException;");
	JyNIJyErr_InsertCurExc = (*env)->GetStaticMethodID(env, JyNIClass, "JyErr_InsertCurExc",
			"(Lorg/python/core/ThreadState;Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyTraceback;)V");
	JyNIJyErr_PrintEx = (*env)->GetStaticMethodID(env, JyNIClass, "JyErr_PrintEx",
			"(ZLorg/python/core/ThreadState;Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyTraceback;)V");

//	JyNIPyErr_Restore = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_Restore", "(Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyTraceback;)V");
//	JyNIPyErr_Clear = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_Clear", "()V");
//	JyNIPyErr_Occurred = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_Occurred", "()Lorg/python/core/PyObject;");
	JyNIPyErr_ExceptionMatches = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_ExceptionMatches",
			"(Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyTraceback;)Z");
//	JyNIPyErr_SetObject = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_SetObject", "(Lorg/python/core/PyObject;Lorg/python/core/PyObject;)V");
//	JyNIPyErr_SetString = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_SetString", "(Lorg/python/core/PyObject;Ljava/lang/String;)V");
//	JyNIPyErr_SetNone = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_SetNone", "(Lorg/python/core/PyObject;)V");
//	JyNIPyErr_NoMemory = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_NoMemory", "()Lorg/python/core/PyObject;");
//	JyNIPyErr_Fetch = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_Fetch", "()Lorg/python/core/PyException;");
	JyNIPyErr_WriteUnraisable = (*env)->GetStaticMethodID(env, JyNIClass, "PyErr_WriteUnraisable", "(Lorg/python/core/PyObject;)V");
	JyNIExceptionByName = (*env)->GetStaticMethodID(env, JyNIClass, "exceptionByName", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	JyNIGetPyType = (*env)->GetStaticMethodID(env, JyNIClass, "getPyType", "(Ljava/lang/Class;)Lorg/python/core/PyType;");
	//puts("  initJyNIClass error section done");

	//ThreadState stuff:
	jclass JyTStateClassLocal = (*env)->FindClass(env, "JyNI/JyTState");
	JyTStateClass = (jclass) (*env)->NewWeakGlobalRef(env, JyTStateClassLocal);
	(*env)->DeleteLocalRef(env, JyTStateClassLocal);
	JyTState_setRecursionLimit = (*env)->GetStaticMethodID(env, JyTStateClass, "setRecursionLimit", "(I)V");
	JyTState_prepareNativeThreadState = (*env)->GetStaticMethodID(env, JyTStateClass,
			"prepareNativeThreadState", "()J");
	JyTState_nativeRecursionLimitField = (*env)->GetStaticFieldID(env, JyTStateClass, "nativeRecursionLimit", "I");

	jclass JyNIDictNextResultClassLocal = (*env)->FindClass(env, "JyNI/JyNIDictNextResult");
	JyNIDictNextResultClass = (jclass) (*env)->NewWeakGlobalRef(env, JyNIDictNextResultClassLocal);
	(*env)->DeleteLocalRef(env, JyNIDictNextResultClassLocal);
	JyNIDictNextResultKeyField = (*env)->GetFieldID(env, JyNIDictNextResultClass, "key", "Lorg/python/core/PyObject;");
	JyNIDictNextResultValueField = (*env)->GetFieldID(env, JyNIDictNextResultClass, "value", "Lorg/python/core/PyObject;");
	JyNIDictNextResultNewIndexField = (*env)->GetFieldID(env, JyNIDictNextResultClass, "newIndex", "I");
	JyNIDictNextResultKeyHandleField = (*env)->GetFieldID(env, JyNIDictNextResultClass, "keyHandle", "J");
	JyNIDictNextResultValueHandleField = (*env)->GetFieldID(env, JyNIDictNextResultClass, "valueHandle", "J");
	//puts("  initJyNIDict done");

	jclass JyNISetNextResultClassLocal = (*env)->FindClass(env, "JyNI/JyNISetNextResult");
	JyNISetNextResultClass = (jclass) (*env)->NewWeakGlobalRef(env, JyNISetNextResultClassLocal);
	(*env)->DeleteLocalRef(env, JyNISetNextResultClassLocal);
	JyNISetNextResultKeyField = (*env)->GetFieldID(env, JyNISetNextResultClass, "key", "Lorg/python/core/PyObject;");
	JyNISetNextResultNewIndexField = (*env)->GetFieldID(env, JyNISetNextResultClass, "newIndex", "I");
	JyNISetNextResultKeyHandleField = (*env)->GetFieldID(env, JyNISetNextResultClass, "keyHandle", "J");

	jclass JyListClassLocal = (*env)->FindClass(env, "JyNI/JyList");
	JyListClass = (jclass) (*env)->NewWeakGlobalRef(env, JyListClassLocal);
	(*env)->DeleteLocalRef(env, JyListClassLocal);
	JyListFromBackendHandleConstructor = (*env)->GetMethodID(env, JyListClass, "<init>", "(J)V");
	//JyListInstallToPyList = (*env)->GetMethodID(env, JyListClass, "installToPyList", "(Lorg/python/core/PyList;)V");

	jclass JySetClassLocal = (*env)->FindClass(env, "JyNI/JySet");
	JySetClass = (jclass) (*env)->NewWeakGlobalRef(env, JySetClassLocal);
	(*env)->DeleteLocalRef(env, JySetClassLocal);
	JySetFromBackendHandleConstructor = (*env)->GetMethodID(env, JySetClass, "<init>", "(Ljava/util/Set;J)V");
	//JySetInstallToPySet = (*env)->GetMethodID(env, JySetClass, "installToPySet", "(Lorg/python/core/BaseSet;)V");

	jclass JyLockClassLocal = (*env)->FindClass(env, "JyNI/JyLock");
	JyLockClass = (jclass) (*env)->NewWeakGlobalRef(env, JyLockClassLocal);
	(*env)->DeleteLocalRef(env, JySetClassLocal);
	JyLockConstructor = (*env)->GetMethodID(env, JyLockClass, "<init>", "()V");
	JyLockAcquire = (*env)->GetMethodID(env, JyLockClass, "acquire", "(Z)Z");
	JyLockRelease = (*env)->GetMethodID(env, JyLockClass, "release", "()V");

	jclass NativeActionClassLocal = (*env)->FindClass(env, "JyNI/JyReferenceMonitor$NativeAction");
	NativeActionClass = (jclass) (*env)->NewWeakGlobalRef(env, NativeActionClassLocal);
	(*env)->DeleteLocalRef(env, NativeActionClassLocal);
	NativeAction_constructor = (*env)->GetMethodID(env, NativeActionClass, "<init>", "()V");
	NativeAction_action = (*env)->GetFieldID(env, NativeActionClass, "action", "S");
	NativeAction_obj = (*env)->GetFieldID(env, NativeActionClass, "obj", "Lorg/python/core/PyObject;");
	NativeAction_nativeRef1 = (*env)->GetFieldID(env, NativeActionClass, "nativeRef1", "J");
	NativeAction_nativeRef2 = (*env)->GetFieldID(env, NativeActionClass, "nativeRef2", "J");
	NativeAction_cTypeName = (*env)->GetFieldID(env, NativeActionClass, "cTypeName", "Ljava/lang/String;");
	NativeAction_cMethod = (*env)->GetFieldID(env, NativeActionClass, "cMethod", "Ljava/lang/String;");
	NativeAction_cLine = (*env)->GetFieldID(env, NativeActionClass, "cLine", "I");
	NativeAction_cFile = (*env)->GetFieldID(env, NativeActionClass, "cFile", "Ljava/lang/String;");

	jclass JyReferenceMonitorClassLocal = (*env)->FindClass(env, "JyNI/JyReferenceMonitor");
	JyReferenceMonitorClass = (jclass) (*env)->NewWeakGlobalRef(env, JyReferenceMonitorClassLocal);
	(*env)->DeleteLocalRef(env, JyReferenceMonitorClassLocal);
//	JyRefMonitorMakeDebugInfo = (*env)->GetStaticMethodID(env, JyReferenceMonitorClass, "makeDebugInfo",
//			"(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)LJyNI/JyReferenceMonitor$ObjectLogDebugInfo;");
	JyRefMonitorAddAction = (*env)->GetStaticMethodID(env, JyReferenceMonitorClass, "addNativeAction",
			//"(SLorg/python/core/PyObject;JJLjava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)V");
			//"(SLorg/python/core/PyObject;JJLJyNI/JyReferenceMonitor$ObjectLogDebugInfo;)V");
			"(LJyNI/JyReferenceMonitor$NativeAction;)V");

	//Peer stuff:
	jclass pyCPeerClassLocal = (*env)->FindClass(env, "JyNI/PyCPeer");
	pyCPeerClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCPeerClassLocal);
	(*env)->DeleteLocalRef(env, pyCPeerClassLocal);
	pyCPeerConstructor = (*env)->GetMethodID(env, pyCPeerClass, "<init>", "(JLorg/python/core/PyType;)V");
	pyCPeerObjectHandle = (*env)->GetFieldID(env, pyCPeerClass, "objectHandle", "J");
	//pyCPeerRefHandle = (*env)->GetFieldID(env, pyCPeerClass, "refHandle", "J");

	jclass pyCPeerGCClassLocal = (*env)->FindClass(env, "JyNI/gc/PyCPeerGC");
	pyCPeerGCClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCPeerGCClassLocal);
	(*env)->DeleteLocalRef(env, pyCPeerGCClassLocal);
	pyCPeerGCConstructor = (*env)->GetMethodID(env, pyCPeerGCClass, "<init>", "(JLorg/python/core/PyType;)V");
	//pyCPeerLinksHandle = (*env)->GetFieldID(env, pyCPeerGCClass, "links", "Ljava/lang/Object;");

	jclass pyCPeerTypeGCClassLocal = (*env)->FindClass(env, "JyNI/gc/PyCPeerTypeGC");
	pyCPeerTypeGCClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCPeerTypeGCClassLocal);
	(*env)->DeleteLocalRef(env, pyCPeerTypeGCClassLocal);
	pyCPeerTypeGCConstructor = (*env)->GetMethodID(env, pyCPeerTypeGCClass, "<init>",
			"(JLjava/lang/String;Lorg/python/core/PyObject;)V");
	pyCPeerTypeGCConstructorSubtype = (*env)->GetMethodID(env, pyCPeerTypeGCClass, "<init>",
			"(JLjava/lang/String;Lorg/python/core/PyObject;Lorg/python/core/PyType;)V");

	jclass pyDictCPeerClassLocal = (*env)->FindClass(env, "JyNI/PyDictionaryCPeer");
	pyDictCPeerClass = (jclass) (*env)->NewWeakGlobalRef(env, pyDictCPeerClassLocal);
	(*env)->DeleteLocalRef(env, pyDictCPeerClassLocal);

	jclass pyTupleCPeerClassLocal = (*env)->FindClass(env, "JyNI/PyTupleCPeer");
	pyTupleCPeerClass = (jclass) (*env)->NewWeakGlobalRef(env, pyTupleCPeerClassLocal);
	(*env)->DeleteLocalRef(env, pyTupleCPeerClassLocal);
	pyTupleCPeerConstructor = (*env)->GetMethodID(env, pyTupleCPeerClass,
			"<init>", "(JLJyNI/PyCPeerType;[Lorg/python/core/PyObject;)V");

	jclass pyCPeerTypeClassLocal = (*env)->FindClass(env, "JyNI/PyCPeerType");
	pyCPeerTypeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCPeerTypeClassLocal);
	(*env)->DeleteLocalRef(env, pyCPeerTypeClassLocal);
	//pyCPeerTypeConstructor = (*env)->GetMethodID(env, pyCPeerTypeClass, "<init>", "(J)V");
	pyCPeerTypeWithNameAndDictConstructor = (*env)->GetMethodID(env, pyCPeerTypeClass, "<init>",
			"(JLjava/lang/String;Lorg/python/core/PyObject;)V");
	pyCPeerTypeWithNameDictTypeConstructor = (*env)->GetMethodID(env, pyCPeerTypeClass, "<init>",
			"(JLjava/lang/String;Lorg/python/core/PyObject;Lorg/python/core/PyType;)V");
	pyCPeerTypeObjectHandle = (*env)->GetFieldID(env, pyCPeerTypeClass, "objectHandle", "J");
	pyCPeerTypeRefHandle = (*env)->GetFieldID(env, pyCPeerTypeClass, "refHandle", "J");

	jclass traversableGCHeadInterface = (*env)->FindClass(env, "JyNI/gc/TraversableGCHead");
	traversableGCHeadSetLinks = (*env)->GetMethodID(env, traversableGCHeadInterface, "setLinks", "(Ljava/lang/Object;)V");
	traversableGCHeadSetLink = (*env)->GetMethodID(env, traversableGCHeadInterface, "setLink", "(ILJyNI/gc/JyGCHead;)I");
	traversableGCHeadInsertLink = (*env)->GetMethodID(env, traversableGCHeadInterface, "insertLink", "(ILJyNI/gc/JyGCHead;)I");
	traversableGCHeadClearLink = (*env)->GetMethodID(env, traversableGCHeadInterface, "clearLink", "(I)I");
	traversableGCHeadClearLinksFromIndex = (*env)->GetMethodID(env, traversableGCHeadInterface, "clearLinksFromIndex", "(I)I");
	traversableGCHeadEnsureSize = (*env)->GetMethodID(env, traversableGCHeadInterface, "ensureSize", "(I)V");
	(*env)->DeleteLocalRef(env, traversableGCHeadInterface);

	jclass jyGCHeadClassLocal = (*env)->FindClass(env, "JyNI/gc/JyGCHead");
	jyGCHeadClass = (*env)->NewWeakGlobalRef(env, jyGCHeadClassLocal);
	(*env)->DeleteLocalRef(env, jyGCHeadClassLocal);
	jyGCHeadGetHandle = (*env)->GetMethodID(env, jyGCHeadClass, "getHandle", "()J");

	jclass pyObjectGCHeadInterface = (*env)->FindClass(env, "JyNI/gc/PyObjectGCHead");
	pyObjectGCHeadSetObject = (*env)->GetMethodID(env, pyObjectGCHeadInterface, "setPyObject", "(Lorg/python/core/PyObject;)V");
	(*env)->DeleteLocalRef(env, pyObjectGCHeadInterface);

	jclass cPeerInterfaceLocal = (*env)->FindClass(env, "JyNI/CPeerInterface");
	cPeerInterface = (*env)->NewWeakGlobalRef(env, cPeerInterfaceLocal);
	(*env)->DeleteLocalRef(env, cPeerInterfaceLocal);

	jclass cPeerNativeDelegateInterfaceLocal = (*env)->FindClass(env, "JyNI/CPeerNativeDelegateSubtype");
	cPeerNativeDelegateInterface = (*env)->NewWeakGlobalRef(env, cPeerNativeDelegateInterfaceLocal);
	(*env)->DeleteLocalRef(env, cPeerNativeDelegateInterfaceLocal);
	super__call__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__call__",
			"([Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyObject;");
	super__findattr_ex__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__findattr_ex__",
			"(Ljava/lang/String;)Lorg/python/core/PyObject;");
	super__setattr__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__setattr__",
			"(Ljava/lang/String;Lorg/python/core/PyObject;)V");
	super__str__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__str__",
				"()Lorg/python/core/PyString;");
	super__repr__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__repr__",
			"()Lorg/python/core/PyString;");
	super__finditem__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__finditem__",
			"(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	super__setitem__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__setitem__",
			"(Lorg/python/core/PyObject;Lorg/python/core/PyObject;)V");
	super__delitem__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__delitem__",
			"(Lorg/python/core/PyObject;)V");
	super__len__ = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super__len__", "()I");
	super_toString = (*env)->GetMethodID(env, cPeerNativeDelegateInterface, "super_toString",
			"()Ljava/lang/String;");

	return JNI_VERSION_1_2;
}

inline jint initJythonSite(JNIEnv *env)
{
	jclass pyPyClassLocal = (*env)->FindClass(env, "org/python/core/Py");
	if (pyPyClassLocal == NULL) { return JNI_ERR;}
	pyPyClass = (jclass) (*env)->NewWeakGlobalRef(env, pyPyClassLocal);
	(*env)->DeleteLocalRef(env, pyPyClassLocal);
	pyPyGetThreadState = (*env)->GetStaticMethodID(env, pyPyClass, "getThreadState", "()Lorg/python/core/ThreadState;");
	pyPyGetFrame = (*env)->GetStaticMethodID(env, pyPyClass, "getFrame", "()Lorg/python/core/PyFrame;");
	pyPyIsSubClass = (*env)->GetStaticMethodID(env, pyPyClass, "isSubClass", "(Lorg/python/core/PyObject;Lorg/python/core/PyObject;)Z");
	pyPyWarning = (*env)->GetStaticMethodID(env, pyPyClass, "warning", "(Lorg/python/core/PyObject;Ljava/lang/String;)V");
	pyPyWarningStck = (*env)->GetStaticMethodID(env, pyPyClass, "warning", "(Lorg/python/core/PyObject;Ljava/lang/String;I)V");
	pyPyExplicitWarning = (*env)->GetStaticMethodID(env, pyPyClass, "warning",
			"(Lorg/python/core/PyObject;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Lorg/python/core/PyObject;)V");
	pyPyImportError = (*env)->GetStaticFieldID(env, pyPyClass, "ImportError", "Lorg/python/core/PyObject;");
	pyPySystemError = (*env)->GetStaticFieldID(env, pyPyClass, "SystemError", "Lorg/python/core/PyObject;");
	pyPyNewString = (*env)->GetStaticMethodID(env, pyPyClass, "newString", "(Ljava/lang/String;)Lorg/python/core/PyString;");
	pyPyNewUnicode = (*env)->GetStaticMethodID(env, pyPyClass, "newUnicode", "(Ljava/lang/String;)Lorg/python/core/PyUnicode;");
	pyPyNewIntFromInt = (*env)->GetStaticMethodID(env, pyPyClass, "newInteger", "(I)Lorg/python/core/PyInteger;");
	pyPyNewIntFromLong = (*env)->GetStaticMethodID(env, pyPyClass, "newInteger", "(J)Lorg/python/core/PyObject;");
	pyPyNewLongFromBigInt = (*env)->GetStaticMethodID(env, pyPyClass, "newLong", "(Ljava/math/BigInteger;)Lorg/python/core/PyLong;");
	pyPyNewLongFromInt = (*env)->GetStaticMethodID(env, pyPyClass, "newLong", "(I)Lorg/python/core/PyLong;");
	pyPyNewLongFromLong = (*env)->GetStaticMethodID(env, pyPyClass, "newLong", "(J)Lorg/python/core/PyLong;");
	pyPyNewLongFromString = (*env)->GetStaticMethodID(env, pyPyClass, "newLong", "(Ljava/lang/String;)Lorg/python/core/PyLong;");
	pyPyNewFloatFromDouble = (*env)->GetStaticMethodID(env, pyPyClass, "newFloat", "(D)Lorg/python/core/PyFloat;");
	pyPyUnicodeError = (*env)->GetStaticFieldID(env, pyPyClass, "UnicodeError", "Lorg/python/core/PyObject;");
	pyPyUnicodeEncodeError = (*env)->GetStaticFieldID(env, pyPyClass, "UnicodeEncodeError", "Lorg/python/core/PyObject;");
	pyPyUnicodeDecodeError = (*env)->GetStaticFieldID(env, pyPyClass, "UnicodeDecodeError", "Lorg/python/core/PyObject;");
	pyPyUnicodeTranslateError = (*env)->GetStaticFieldID(env, pyPyClass, "UnicodeTranslateError", "Lorg/python/core/PyObject;");
	pyPyUnicodeWarning = (*env)->GetStaticFieldID(env, pyPyClass, "UnicodeWarning", "Lorg/python/core/PyObject;");
	pyPyUnicodeErrorFactory = (*env)->GetStaticMethodID(env, pyPyClass, "UnicodeError", "(Ljava/lang/String;)Lorg/python/core/PyException;");
	pyPyUnicodeEncodeErrorFactory = (*env)->GetStaticMethodID(env, pyPyClass, "UnicodeEncodeError",
			"(Ljava/lang/String;Ljava/lang/String;IILjava/lang/String;)Lorg/python/core/PyException;");
	pyPyUnicodeDecodeErrorFactory = (*env)->GetStaticMethodID(env, pyPyClass, "UnicodeDecodeError",
			"(Ljava/lang/String;Ljava/lang/String;IILjava/lang/String;)Lorg/python/core/PyException;");
	pyPyUnicodeTranslateErrorFactory = (*env)->GetStaticMethodID(env, pyPyClass, "UnicodeTranslateError",
			"(Ljava/lang/String;IILjava/lang/String;)Lorg/python/core/PyException;");
	pyPyRaiseUnicodeWarning = (*env)->GetStaticMethodID(env, pyPyClass, "UnicodeWarning", "(Ljava/lang/String;)V");
	pyPyMakeClass = (*env)->GetStaticMethodID(env, pyPyClass, "makeClass",
			"(Ljava/lang/String;[Lorg/python/core/PyObject;Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyPyIntegerCache = (*env)->GetStaticFieldID(env, pyPyClass, "integerCache", "[Lorg/python/core/PyInteger;");
	pyPyLetters = (*env)->GetStaticFieldID(env, pyPyClass, "letters", "[Lorg/python/core/PyString;");

	jclass pyTracebackClassLocal = (*env)->FindClass(env, "org/python/core/PyTraceback");
	if (pyTracebackClassLocal == NULL) { return JNI_ERR;}
	pyTracebackClass = (jclass) (*env)->NewWeakGlobalRef(env, pyTracebackClassLocal);
	(*env)->DeleteLocalRef(env, pyTracebackClassLocal);
	pyTracebackByTracebackFrameConstructor = (*env)->GetMethodID(env, pyTracebackClass, "<init>", "(Lorg/python/core/PyTraceback;Lorg/python/core/PyFrame;)V");

	jclass pyExceptionClassLocal = (*env)->FindClass(env, "org/python/core/PyException");
	if (pyExceptionClassLocal == NULL) { return JNI_ERR;}
	pyExceptionClass = (jclass) (*env)->NewWeakGlobalRef(env, pyExceptionClassLocal);
	(*env)->DeleteLocalRef(env, pyExceptionClassLocal);
	pyExceptionTypeField = (*env)->GetFieldID(env, pyExceptionClass, "type", "Lorg/python/core/PyObject;");
	pyExceptionValueField = (*env)->GetFieldID(env, pyExceptionClass, "value", "Lorg/python/core/PyObject;");
	pyExceptionTracebackField = (*env)->GetFieldID(env, pyExceptionClass, "traceback", "Lorg/python/core/PyTraceback;");
	pyExceptionFullConstructor = (*env)->GetMethodID(env, pyExceptionClass, "<init>", "(Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyTraceback;)V");
	pyExceptionTypeValueConstructor = (*env)->GetMethodID(env, pyExceptionClass, "<init>", "(Lorg/python/core/PyObject;Lorg/python/core/PyObject;)V");
	pyExceptionTypeConstructor = (*env)->GetMethodID(env, pyExceptionClass, "<init>", "(Lorg/python/core/PyObject;)V");
	pyExceptionNormalize = (*env)->GetMethodID(env, pyExceptionClass, "normalize", "()V");
	pyExceptionMatch = (*env)->GetMethodID(env, pyExceptionClass, "match", "(Lorg/python/core/PyObject;)Z");
	pyExceptionIsExceptionClass = (*env)->GetStaticMethodID(env, pyExceptionClass, "isExceptionClass", "(Lorg/python/core/PyObject;)Z");
	pyExceptionIsExceptionInstance = (*env)->GetStaticMethodID(env, pyExceptionClass, "isExceptionInstance", "(Lorg/python/core/PyObject;)Z");

	jclass pyThreadStateClassLocal = (*env)->FindClass(env, "org/python/core/ThreadState");
	if (pyThreadStateClassLocal == NULL) { return JNI_ERR;}
	pyThreadStateClass = (jclass) (*env)->NewWeakGlobalRef(env, pyThreadStateClassLocal);
	(*env)->DeleteLocalRef(env, pyThreadStateClassLocal);
	pyThreadStateExceptionField = (*env)->GetFieldID(env, pyThreadStateClass, "exception", "Lorg/python/core/PyException;");
	//pyThreadStateRecursionDepth = (*env)->GetFieldID(env, pyThreadStateClass, "recursion_depth", "I");
	//pyThreadStateEnterRecursiveCall = (*env)->GetMethodID(env, pyThreadStateClass, "enterRecursiveCall", "(Ljava/lang/String;)V");
	//pyThreadStateLeaveRecursiveCall = (*env)->GetMethodID(env, pyThreadStateClass, "leaveRecursiveCall", "()V");
	pyThreadStateEnterRepr = (*env)->GetMethodID(env, pyThreadStateClass, "enterRepr", "(Lorg/python/core/PyObject;)Z");
	pyThreadStateExitRepr = (*env)->GetMethodID(env, pyThreadStateClass, "exitRepr", "(Lorg/python/core/PyObject;)V");
	pyThreadStateGetCompareStateDict = (*env)->GetMethodID(env, pyThreadStateClass, "getCompareStateDict", "()Lorg/python/core/PyDictionary;");

	return JNI_VERSION_1_2;
}

inline jint initJythonObjects(JNIEnv *env)
{
	jclass pyNoneClassLocal = (*env)->FindClass(env, "org/python/core/PyNone");
	if (pyNoneClassLocal == NULL) { return JNI_ERR;}
	pyNoneClass = (jclass) (*env)->NewWeakGlobalRef(env, pyNoneClassLocal);
	//jclass pyNoneClass = (*env)->FindClass(env, "org/python/core/PyNone");
	if (pyNoneClass == NULL) { return JNI_ERR;}
	(*env)->DeleteLocalRef(env, pyNoneClassLocal);
	//(*env)->DeleteLocalRef(env, pyNoneConstructor);

	jclass pyObjectClassLocal = (*env)->FindClass(env, "org/python/core/PyObject");
	if (pyObjectClassLocal == NULL) { return JNI_ERR;}
	pyObjectClass = (jclass) (*env)->NewWeakGlobalRef(env, pyObjectClassLocal);
	(*env)->DeleteLocalRef(env, pyObjectClassLocal);
	pyObjectGetType = (*env)->GetMethodID(env, pyObjectClass, "getType", "()Lorg/python/core/PyType;");
	pyObjectAsString = (*env)->GetMethodID(env, pyObjectClass, "asString", "()Ljava/lang/String;");
	pyObjectType = (*env)->GetStaticFieldID(env, pyObjectClass, "TYPE", "Lorg/python/core/PyType;");
	pyObject__getattr__ = (*env)->GetMethodID(env, pyObjectClass, "__getattr__", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	pyObject__findattr__ = (*env)->GetMethodID(env, pyObjectClass, "__findattr__", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	pyObject__setattr__ = (*env)->GetMethodID(env, pyObjectClass, "__setattr__", "(Ljava/lang/String;Lorg/python/core/PyObject;)V");
	pyObject__repr__ = (*env)->GetMethodID(env, pyObjectClass, "__repr__", "()Lorg/python/core/PyString;");
	pyObject__cmp__ = (*env)->GetMethodID(env, pyObjectClass, "__cmp__", "(Lorg/python/core/PyObject;)I");
	pyObject__reduce__ = (*env)->GetMethodID(env, pyObjectClass, "__reduce__", "()Lorg/python/core/PyObject;");
	pyObject__and__ = (*env)->GetMethodID(env, pyObjectClass, "__and__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__or__ = (*env)->GetMethodID(env, pyObjectClass, "__or__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__sub__ = (*env)->GetMethodID(env, pyObjectClass, "__sub__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__xor__ = (*env)->GetMethodID(env, pyObjectClass, "__xor__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__isub__ = (*env)->GetMethodID(env, pyObjectClass, "__isub__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__call__ = (*env)->GetMethodID(env, pyObjectClass, "__call__",
			"([Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyObject;");
	pyObject__str__ = (*env)->GetMethodID(env, pyObjectClass, "__str__", "()Lorg/python/core/PyString;");
	//pyObject__getitem__ = (*env)->GetMethodID(env, pyObjectClass, "__getitem__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__finditem__ = (*env)->GetMethodID(env, pyObjectClass, "__finditem__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__setitem__ = (*env)->GetMethodID(env, pyObjectClass, "__setitem__", "(Lorg/python/core/PyObject;Lorg/python/core/PyObject;)V");
	pyObject__delitem__ = (*env)->GetMethodID(env, pyObjectClass, "__delitem__", "(Lorg/python/core/PyObject;)V");
	pyObject__contains__ = (*env)->GetMethodID(env, pyObjectClass, "__contains__", "(Lorg/python/core/PyObject;)Z");
	pyObject__len__ = (*env)->GetMethodID(env, pyObjectClass, "__len__", "()I");
	pyObject__getslice__ = (*env)->GetMethodID(env, pyObjectClass, "__getslice__",
			"(Lorg/python/core/PyObject;Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyObject__nonzero__ = (*env)->GetMethodID(env, pyObjectClass, "__nonzero__", "()Z");
	pyObject_cmp = (*env)->GetMethodID(env, pyObjectClass, "_cmp", "(Lorg/python/core/PyObject;)I");
	pyObjectGetDict = (*env)->GetMethodID(env, pyObjectClass, "getDict", "()Lorg/python/core/PyObject;");
	pyObjectFastGetDict = (*env)->GetMethodID(env, pyObjectClass, "fastGetDict", "()Lorg/python/core/PyObject;");
	pyObjectSetDict = (*env)->GetMethodID(env, pyObjectClass, "setDict", "(Lorg/python/core/PyObject;)V");
	pyObjectIsCallable = (*env)->GetMethodID(env, pyObjectClass, "isCallable", "()Z");
	pyObjectHashCode = (*env)->GetMethodID(env, pyObjectClass, "hashCode", "()I");

	jclass pyInstanceClassLocal = (*env)->FindClass(env, "org/python/core/PyInstance");
	if (pyInstanceClassLocal == NULL) { return JNI_ERR;}
	pyInstanceClass = (jclass) (*env)->NewWeakGlobalRef(env, pyInstanceClassLocal);
	(*env)->DeleteLocalRef(env, pyInstanceClassLocal);
	pyInstanceConstructor = (*env)->GetMethodID(env, pyInstanceClass, "<init>", "(Lorg/python/core/PyClass;Lorg/python/core/PyObject;)V");
	pyInstanceInstclassField = (*env)->GetFieldID(env, pyInstanceClass, "instclass", "Lorg/python/core/PyClass;");
	pyInstance__dict__ = (*env)->GetFieldID(env, pyInstanceClass, "__dict__", "Lorg/python/core/PyObject;");
	pyInstanceIsSequenceType = (*env)->GetMethodID(env, pyInstanceClass, "isSequenceType", "()Z");

	jclass pyFrameClassLocal = (*env)->FindClass(env, "org/python/core/PyFrame");
	if (pyFrameClassLocal == NULL) { return JNI_ERR;}
	pyFrameClass = (jclass) (*env)->NewWeakGlobalRef(env, pyFrameClassLocal);
	(*env)->DeleteLocalRef(env, pyFrameClassLocal);

	jclass pyBooleanClassLocal = (*env)->FindClass(env, "org/python/core/PyBoolean");
	if (pyBooleanClassLocal == NULL) { return JNI_ERR;}
	pyBooleanClass = (jclass) (*env)->NewWeakGlobalRef(env, pyBooleanClassLocal);
	(*env)->DeleteLocalRef(env, pyBooleanClassLocal);
	//pyBooleanConstructor = (*env)->GetMethodID(env, pyBooleanClass, "<init>", "(Z)V");
	pyBoolVal = (*env)->GetFieldID(env, pyBooleanClass, "value", "Z");

	jclass pyIntClassLocal = (*env)->FindClass(env, "org/python/core/PyInteger");
	if (pyIntClassLocal == NULL) { return JNI_ERR;}
	pyIntClass = (jclass) (*env)->NewWeakGlobalRef(env, pyIntClassLocal);
	(*env)->DeleteLocalRef(env, pyIntClassLocal);
	pyIntConstructor = (*env)->GetMethodID(env, pyIntClass, "<init>", "(I)V");
	pyIntGetValue = (*env)->GetMethodID(env, pyIntClass, "getValue", "()I");
	//pyIntAsInt = (*env)->GetMethodID(env, pyIntClass, "asInt", "()I");
	//pyIntAsLong = (*env)->GetMethodID(env, pyIntClass, "asLong", "()J");

	jclass pyLongClassLocal = (*env)->FindClass(env, "org/python/core/PyLong");
	if (pyLongClassLocal == NULL) { return JNI_ERR;}
	pyLongClass = (jclass) (*env)->NewWeakGlobalRef(env, pyLongClassLocal);
	(*env)->DeleteLocalRef(env, pyLongClassLocal);
	pyLongByLongConstructor = (*env)->GetMethodID(env, pyLongClass, "<init>", "(J)V");
	pyLongByBigIntConstructor = (*env)->GetMethodID(env, pyLongClass, "<init>", "(Ljava/math/BigInteger;)V");
	pyLongAsLong = (*env)->GetMethodID(env, pyLongClass, "asLong", "()J");
	pyLongGetLong = (*env)->GetMethodID(env, pyLongClass, "getLong", "(JJ)J");
	pyLongGetValue = (*env)->GetMethodID(env, pyLongClass, "getValue", "()Ljava/math/BigInteger;");
	pyLongBit_length = (*env)->GetMethodID(env, pyLongClass, "bit_length", "()I");
	//pyLongToString = (*env)->GetMethodID(env, pyLongClass, "toString", "()Ljava/lang/String;");

	jclass pyUnicodeClassLocal = (*env)->FindClass(env, "org/python/core/PyUnicode");
	if (pyUnicodeClassLocal == NULL) { return JNI_ERR;}
	pyUnicodeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyUnicodeClassLocal);
	(*env)->DeleteLocalRef(env, pyUnicodeClassLocal);
	pyUnicodeByJStringConstructor = (*env)->GetMethodID(env, pyUnicodeClass, "<init>", "(Ljava/lang/String;)V");
	pyUnicodeAsString = (*env)->GetMethodID(env, pyUnicodeClass, "asString", "()Ljava/lang/String;");

	jclass pyStringClassLocal = (*env)->FindClass(env, "org/python/core/PyString");
	if (pyStringClassLocal == NULL) { return JNI_ERR;}
	pyStringClass = (jclass) (*env)->NewWeakGlobalRef(env, pyStringClassLocal);
	(*env)->DeleteLocalRef(env, pyStringClassLocal);
	pyStringByJStringConstructor = (*env)->GetMethodID(env, pyStringClass, "<init>", "(Ljava/lang/String;)V");
	pyStringAsString = (*env)->GetMethodID(env, pyStringClass, "asString", "()Ljava/lang/String;");

	jclass pyFloatClassLocal = (*env)->FindClass(env, "org/python/core/PyFloat");
	if (pyFloatClassLocal == NULL) { return JNI_ERR;}
	pyFloatClass = (jclass) (*env)->NewWeakGlobalRef(env, pyFloatClassLocal);
	(*env)->DeleteLocalRef(env, pyFloatClassLocal);
	pyFloatByDoubleConstructor = (*env)->GetMethodID(env, pyFloatClass, "<init>", "(D)V");
	pyFloatAsDouble = (*env)->GetMethodID(env, pyFloatClass, "asDouble", "()D");
	pyFloatTypeField = (*env)->GetStaticFieldID(env, pyFloatClass, "TYPE", "Lorg/python/core/PyType;");

	jclass pyComplexClassLocal = (*env)->FindClass(env, "org/python/core/PyComplex");
	if (pyComplexClassLocal == NULL) { return JNI_ERR;}
	pyComplexClass = (jclass) (*env)->NewWeakGlobalRef(env, pyComplexClassLocal);
	(*env)->DeleteLocalRef(env, pyComplexClassLocal);
	pyComplexBy2DoubleConstructor = (*env)->GetMethodID(env, pyComplexClass, "<init>", "(DD)V");
	//jfieldID pyComplexRealField = (*env)->GetFieldID(env, pyComplexClass, "real", "D");
	//jfieldID pyComplexImagField = (*env)->GetFieldID(env, pyComplexClass, "imag", "D");

	jclass pyArrayClassLocal = (*env)->FindClass(env, "org/python/core/PyArray");
	if (pyArrayClassLocal == NULL) { return JNI_ERR;}
	pyArrayClass = (jclass) (*env)->NewWeakGlobalRef(env, pyArrayClassLocal);
	(*env)->DeleteLocalRef(env, pyArrayClassLocal);
	pyArrayGetTypecode = (*env)->GetMethodID(env, pyArrayClass, "getTypecode", "()Ljava/lang/String;");

	jclass pyTupleClassLocal = (*env)->FindClass(env, "org/python/core/PyTuple");
	if (pyTupleClassLocal == NULL) { return JNI_ERR;}
	pyTupleClass = (jclass) (*env)->NewWeakGlobalRef(env, pyTupleClassLocal);
	(*env)->DeleteLocalRef(env, pyTupleClassLocal);
	pyTupleConstructor = (*env)->GetMethodID(env, pyTupleClass, "<init>", "()V");
	pyTupleByPyObjectArrayBooleanConstructor = (*env)->GetMethodID(env, pyTupleClass, "<init>", "([Lorg/python/core/PyObject;Z)V");
	pyTupleSize = (*env)->GetMethodID(env, pyTupleClass, "size", "()I");
	pyTuplePyGet = (*env)->GetMethodID(env, pyTupleClass, "pyget", "(I)Lorg/python/core/PyObject;");
	//pyTupleGetArray = (*env)->GetMethodID(env, pyTupleClass, "getArray", "()[Lorg/python/core/PyObject;");

	jclass pyListClassLocal = (*env)->FindClass(env, "org/python/core/PyList");
	if (pyListClassLocal == NULL) { return JNI_ERR;}
	pyListClass = (jclass) (*env)->NewWeakGlobalRef(env, pyListClassLocal);
	(*env)->DeleteLocalRef(env, pyListClassLocal);
	pyListConstructor = (*env)->GetMethodID(env, pyListClass, "<init>", "()V");
	pyListByPyObjectArrayConstructor = (*env)->GetMethodID(env, pyListClass, "<init>", "([Lorg/python/core/PyObject;)V");
	pyListFromList = (*env)->GetStaticMethodID(env, pyListClass, "fromList", "(Ljava/util/List;)Lorg/python/core/PyList;");
	pyListSize = (*env)->GetMethodID(env, pyListClass, "size", "()I");
	pyListPyGet = (*env)->GetMethodID(env, pyListClass, "pyget", "(I)Lorg/python/core/PyObject;");
	pyListPySet = (*env)->GetMethodID(env, pyListClass, "pyset", "(ILorg/python/core/PyObject;)V");
	pyListAppend = (*env)->GetMethodID(env, pyListClass, "append", "(Lorg/python/core/PyObject;)V");
	pyListInsert = (*env)->GetMethodID(env, pyListClass, "insert", "(ILorg/python/core/PyObject;)V");
	pyListSort = (*env)->GetMethodID(env, pyListClass, "sort", "()V");
	pyListReverse = (*env)->GetMethodID(env, pyListClass, "reverse", "()V");
	pyListGetArray = (*env)->GetMethodID(env, pyListClass, "getArray", "()[Lorg/python/core/PyObject;");
	pyListBackend = (*env)->GetFieldID(env, pyListClass, "list", "Ljava/util/List;");

	jclass pyAbstractDictClassLocal = (*env)->FindClass(env, "org/python/core/AbstractDict");
	if (pyAbstractDictClassLocal == NULL) { return JNI_ERR;}
	pyAbstractDictClass = (jclass) (*env)->NewWeakGlobalRef(env, pyAbstractDictClassLocal);
	(*env)->DeleteLocalRef(env, pyAbstractDictClassLocal);
	pyAbstractDictCopy = (*env)->GetMethodID(env, pyAbstractDictClass, "copy", "()Lorg/python/core/AbstractDict;");
	pyAbstractDictMerge = (*env)->GetMethodID(env, pyAbstractDictClass, "merge", "(Lorg/python/core/PyObject;Z)V");
	pyAbstractDictMergeFromSeq = (*env)->GetMethodID(env, pyAbstractDictClass, "mergeFromSeq", "(Lorg/python/core/PyObject;Z)V");

	jclass pyDictClassLocal = (*env)->FindClass(env, "org/python/core/PyDictionary");
	if (pyDictClassLocal == NULL) { return JNI_ERR;}
	pyDictClass = (jclass) (*env)->NewWeakGlobalRef(env, pyDictClassLocal);
	(*env)->DeleteLocalRef(env, pyDictClassLocal);
	pyDictConstructor = (*env)->GetMethodID(env, pyDictClass, "<init>", "()V");
	pyDictByPyObjectArrayConstructor = (*env)->GetMethodID(env, pyDictClass, "<init>", "([Lorg/python/core/PyObject;)V");

	jclass pyStringMapClassLocal = (*env)->FindClass(env, "org/python/core/PyStringMap");
	if (pyStringMapClassLocal == NULL) { return JNI_ERR;}
	pyStringMapClass = (jclass) (*env)->NewWeakGlobalRef(env, pyStringMapClassLocal);
	(*env)->DeleteLocalRef(env, pyStringMapClassLocal);

	jclass pySequenceClassLocal = (*env)->FindClass(env, "org/python/core/PySequence");
	if (pySequenceClassLocal == NULL) { return JNI_ERR;}
	pySequenceClass = (jclass) (*env)->NewWeakGlobalRef(env, pySequenceClassLocal);
	(*env)->DeleteLocalRef(env, pySequenceClassLocal);
	pySequenceLen = (*env)->GetMethodID(env, pySequenceClass, "__len__", "()I");
	pySequenceGetItem = (*env)->GetMethodID(env, pySequenceClass, "__finditem__", "(I)Lorg/python/core/PyObject;");

	jclass pySequenceListClassLocal = (*env)->FindClass(env, "org/python/core/PySequenceList");
	if (pySequenceListClassLocal == NULL) { return JNI_ERR;}
	pySequenceListClass = (jclass) (*env)->NewWeakGlobalRef(env, pySequenceListClassLocal);
	(*env)->DeleteLocalRef(env, pySequenceListClassLocal);
	pySequenceListSize = (*env)->GetMethodID(env, pySequenceListClass, "size", "()I");
	pySequenceListPyGet = (*env)->GetMethodID(env, pySequenceListClass, "pyget", "(I)Lorg/python/core/PyObject;");

	jclass pyTypeClassLocal = (*env)->FindClass(env, "org/python/core/PyType");
	if (pyTypeClassLocal == NULL) { return JNI_ERR;}
	pyTypeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyTypeClassLocal);
	(*env)->DeleteLocalRef(env, pyTypeClassLocal);
	pyTypeGetName = (*env)->GetMethodID(env, pyTypeClass, "getName", "()Ljava/lang/String;");
	pyTypeSetName = (*env)->GetMethodID(env, pyTypeClass, "setName", "(Ljava/lang/String;)V");
	pyTypeGetBase = (*env)->GetMethodID(env, pyTypeClass, "getBase", "()Lorg/python/core/PyObject;");
	pyTypeGetBases = (*env)->GetMethodID(env, pyTypeClass, "getBases", "()Lorg/python/core/PyObject;");
	pyTypeGetMro = (*env)->GetMethodID(env, pyTypeClass, "getMro", "()Lorg/python/core/PyTuple;");
	pyTypeIsSubType = (*env)->GetMethodID(env, pyTypeClass, "isSubType", "(Lorg/python/core/PyType;)Z");
	pyTypeMROField = (*env)->GetFieldID(env, pyTypeClass, "mro", "[Lorg/python/core/PyObject;");
	pyTypeNameField = (*env)->GetFieldID(env, pyTypeClass, "name", "Ljava/lang/String;");

	jclass pyCodecsClassLocal = (*env)->FindClass(env, "org/python/core/codecs");
	if (pyCodecsClassLocal == NULL) { return JNI_ERR;}
	pyCodecsClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCodecsClassLocal);
	(*env)->DeleteLocalRef(env, pyCodecsClassLocal);
	pyCodecsDecode = (*env)->GetStaticMethodID(env, pyCodecsClass, "decode",
			"(Lorg/python/core/PyString;Ljava/lang/String;Ljava/lang/String;)Lorg/python/core/PyObject;");
	pyCodecsEncode = (*env)->GetStaticMethodID(env, pyCodecsClass, "encode",
			"(Lorg/python/core/PyString;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	pyCodecsGetDefaultEncoding = (*env)->GetStaticMethodID(env, pyCodecsClass, "getDefaultEncoding", "()Ljava/lang/String;");
	pyCodecsLookup = (*env)->GetStaticMethodID(env, pyCodecsClass, "lookup", "(Ljava/lang/String;)Lorg/python/core/PyTuple;");
	pyCodecsRegister = (*env)->GetStaticMethodID(env, pyCodecsClass, "register", "(Lorg/python/core/PyObject;)V");
	pyCodecsLookupError = (*env)->GetStaticMethodID(env, pyCodecsClass, "lookup_error", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	pyCodecsRegisterError = (*env)->GetStaticMethodID(env, pyCodecsClass, "register_error", "(Ljava/lang/String;Lorg/python/core/PyObject;)V");

	jclass pyNotImplementedClassLocal = (*env)->FindClass(env, "org/python/core/PyNotImplemented");
	if (pyNotImplementedClassLocal == NULL) { return JNI_ERR;}
	pyNotImplementedClass = (jclass) (*env)->NewWeakGlobalRef(env, pyNotImplementedClassLocal);
	(*env)->DeleteLocalRef(env, pyNotImplementedClassLocal);

	jclass pyFileClassLocal = (*env)->FindClass(env, "org/python/core/PyFile");
	if (pyFileClassLocal == NULL) { return JNI_ERR;}
	pyFileClass = (jclass) (*env)->NewWeakGlobalRef(env, pyFileClassLocal);
	(*env)->DeleteLocalRef(env, pyFileClassLocal);
	pyFileWrite = (*env)->GetMethodID(env, pyFileClass, "write", "(Ljava/lang/String;)V");

	jclass pyModuleClassLocal = (*env)->FindClass(env, "org/python/core/PyModule");
	if (pyModuleClassLocal == NULL) { return JNI_ERR;}
	pyModuleClass = (jclass) (*env)->NewWeakGlobalRef(env, pyModuleClassLocal);
	(*env)->DeleteLocalRef(env, pyModuleClassLocal);
	pyModuleByStringConstructor = (*env)->GetMethodID(env, pyModuleClass, "<init>", "(Ljava/lang/String;)V");
	pyModuleGetDict = (*env)->GetMethodID(env, pyModuleClass, "getDict", "()Lorg/python/core/PyObject;");
	pyModule__setattr__ = (*env)->GetMethodID(env, pyModuleClass, "__setattr__", "(Ljava/lang/String;Lorg/python/core/PyObject;)V");
	pyModule__delattr__ = (*env)->GetMethodID(env, pyModuleClass, "__delattr__", "(Ljava/lang/String;)V");

	jclass pyCellClassLocal = (*env)->FindClass(env, "org/python/core/PyCell");
	if (pyCellClassLocal == NULL) { return JNI_ERR;}
	pyCellClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCellClassLocal);
	(*env)->DeleteLocalRef(env, pyCellClassLocal);
	pyCellConstructor = (*env)->GetMethodID(env, pyCellClass, "<init>", "()V");
	pyCell_ob_ref = (*env)->GetFieldID(env, pyCellClass, "ob_ref", "Lorg/python/core/PyObject;");

	jclass pyClassClassLocal = (*env)->FindClass(env, "org/python/core/PyClass");
	if (pyClassClassLocal == NULL) { return JNI_ERR;}
	pyClassClass = (jclass) (*env)->NewWeakGlobalRef(env, pyClassClassLocal);
	(*env)->DeleteLocalRef(env, pyClassClassLocal);
	pyClassClassobj___new__ = (*env)->GetStaticMethodID(env, pyClassClass, "classobj___new__",
			"(Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyClass__bases__ = (*env)->GetFieldID(env, pyClassClass, "__bases__", "Lorg/python/core/PyTuple;");
	pyClass__dict__ = (*env)->GetFieldID(env, pyClassClass, "__dict__", "Lorg/python/core/PyObject;");
	pyClass__name__ = (*env)->GetFieldID(env, pyClassClass, "__name__", "Ljava/lang/String;");

	jclass pyMethodClassLocal = (*env)->FindClass(env, "org/python/core/PyMethod");
	if (pyMethodClassLocal == NULL) { return JNI_ERR;}
	pyMethodClass = (jclass) (*env)->NewWeakGlobalRef(env, pyMethodClassLocal);
	(*env)->DeleteLocalRef(env, pyMethodClassLocal);
	pyMethodConstructor = (*env)->GetMethodID(env, pyMethodClass, "<init>",
			"(Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyObject;)V");
	pyMethod__func__ = (*env)->GetFieldID(env, pyMethodClass, "__func__", "Lorg/python/core/PyObject;");
	pyMethod__self__ = (*env)->GetFieldID(env, pyMethodClass, "__self__", "Lorg/python/core/PyObject;");
	pyMethodImClass = (*env)->GetFieldID(env, pyMethodClass, "im_class", "Lorg/python/core/PyObject;");

	jclass pyFunctionClassLocal = (*env)->FindClass(env, "org/python/core/PyFunction");
	if (pyFunctionClassLocal == NULL) { return JNI_ERR;}
	pyFunctionClass = (jclass) (*env)->NewWeakGlobalRef(env, pyFunctionClassLocal);
	(*env)->DeleteLocalRef(env, pyFunctionClassLocal);
	pyFunctionConstructor = (*env)->GetMethodID(env, pyFunctionClass, "<init>",
			"(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;Lorg/python/core/PyCode;)V");
	pyFunction__code__ = (*env)->GetFieldID(env, pyFunctionClass, "__code__", "Lorg/python/core/PyCode;");
	pyFunctionSetCode = (*env)->GetMethodID(env, pyFunctionClass, "setCode", "(Lorg/python/core/PyCode;)V");
	pyFunctionGetFuncGlobals = (*env)->GetMethodID(env, pyFunctionClass, "getFuncGlobals", "()Lorg/python/core/PyObject;");
	pyFunction__module__ = (*env)->GetFieldID(env, pyFunctionClass, "__module__", "Lorg/python/core/PyObject;");
	pyFunctionGetFuncDefaults = (*env)->GetMethodID(env, pyFunctionClass, "getFuncDefaults", "()Lorg/python/core/PyObject;");
	pyFunctionSetFuncDefaults = (*env)->GetMethodID(env, pyFunctionClass, "setFuncDefaults", "(Lorg/python/core/PyObject;)V");
	pyFunctionFuncClosure = (*env)->GetFieldID(env, pyFunctionClass, "__closure__", "Lorg/python/core/PyObject;");
	pyFunction__doc__ = (*env)->GetFieldID(env, pyFunctionClass, "__doc__", "Lorg/python/core/PyObject;");
	pyFunction__name__ = (*env)->GetFieldID(env, pyFunctionClass, "__name__", "Ljava/lang/String;");

	jclass pyClassMethodClassLocal = (*env)->FindClass(env, "org/python/core/PyClassMethod");
	if (pyClassMethodClassLocal == NULL) { return JNI_ERR;}
	pyClassMethodClass = (jclass) (*env)->NewWeakGlobalRef(env, pyClassMethodClassLocal);
	(*env)->DeleteLocalRef(env, pyClassMethodClassLocal);
	pyClassMethodConstructor = (*env)->GetMethodID(env, pyClassMethodClass, "<init>", "(Lorg/python/core/PyObject;)V");
	pyClassMethod_callable = (*env)->GetFieldID(env, pyClassMethodClass, "callable", "Lorg/python/core/PyObject;");

	jclass pyStaticMethodClassLocal = (*env)->FindClass(env, "org/python/core/PyStaticMethod");
	if (pyStaticMethodClassLocal == NULL) { return JNI_ERR;}
	pyStaticMethodClass = (jclass) (*env)->NewWeakGlobalRef(env, pyStaticMethodClassLocal);
	(*env)->DeleteLocalRef(env, pyStaticMethodClassLocal);
	pyStaticMethodConstructor = (*env)->GetMethodID(env, pyStaticMethodClass, "<init>", "(Lorg/python/core/PyObject;)V");
	pyStaticMethod_callable = (*env)->GetFieldID(env, pyStaticMethodClass, "callable", "Lorg/python/core/PyObject;");

//	jclass pyMethodDescrClassLocal = (*env)->FindClass(env, "org/python/core/PyMethodDescr");
//	if (pyMethodDescrClassLocal == NULL) { return JNI_ERR;}
//	pyMethodDescrClass = (jclass) (*env)->NewWeakGlobalRef(env, pyMethodDescrClassLocal);
//	(*env)->DeleteLocalRef(env, pyMethodDescrClassLocal);
//
//	jclass pyClassMethodDescrClassLocal = (*env)->FindClass(env, "org/python/core/PyClassMethodDescr");
//	if (pyClassMethodDescrClassLocal == NULL) { return JNI_ERR;}
//	pyClassMethodDescrClass = (jclass) (*env)->NewWeakGlobalRef(env, pyClassMethodDescrClassLocal);
//	(*env)->DeleteLocalRef(env, pyClassMethodDescrClassLocal);

	jclass pyDictProxyClassLocal = (*env)->FindClass(env, "org/python/core/PyDictProxy");
	if (pyDictProxyClassLocal == NULL) { return JNI_ERR;}
	pyDictProxyClass = (jclass) (*env)->NewWeakGlobalRef(env, pyDictProxyClassLocal);
	(*env)->DeleteLocalRef(env, pyDictProxyClassLocal);
	pyDictProxyConstructor = (*env)->GetMethodID(env, pyDictProxyClass, "<init>", "(Lorg/python/core/PyObject;)V");
	pyDictProxy_dict = (*env)->GetFieldID(env, pyDictProxyClass, "dict", "Lorg/python/core/PyObject;");

	jclass pyPropertyClassLocal = (*env)->FindClass(env, "org/python/core/PyProperty");
	if (pyPropertyClassLocal == NULL) { return JNI_ERR;}
	pyPropertyClass = (jclass) (*env)->NewWeakGlobalRef(env, pyPropertyClassLocal);
	(*env)->DeleteLocalRef(env, pyPropertyClassLocal);
	pyPropertyConstructor = (*env)->GetMethodID(env, pyPropertyClass, "<init>", "()V");
	pyProperty_fget = (*env)->GetFieldID(env, pyPropertyClass, "fget", "Lorg/python/core/PyObject;");
	pyProperty_fset = (*env)->GetFieldID(env, pyPropertyClass, "fset", "Lorg/python/core/PyObject;");
	pyProperty_fdel = (*env)->GetFieldID(env, pyPropertyClass, "fdel", "Lorg/python/core/PyObject;");
	pyProperty_doc = (*env)->GetFieldID(env, pyPropertyClass, "doc", "Lorg/python/core/PyObject;");
	pyProperty_docFromGetter = (*env)->GetFieldID(env, pyPropertyClass, "docFromGetter", "Z");

	jclass pyBaseStringClassLocal = (*env)->FindClass(env, "org/python/core/PyBaseString");
	if (pyBaseStringClassLocal == NULL) { return JNI_ERR;}
	pyBaseStringClass = (jclass) (*env)->NewWeakGlobalRef(env, pyBaseStringClassLocal);
	(*env)->DeleteLocalRef(env, pyBaseStringClassLocal);

	jclass pyXRangeClassLocal = (*env)->FindClass(env, "org/python/core/PyXRange");
	if (pyXRangeClassLocal == NULL) { return JNI_ERR;}
	pyXRangeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyXRangeClassLocal);
	(*env)->DeleteLocalRef(env, pyXRangeClassLocal);

	jclass pySequenceIterClassLocal = (*env)->FindClass(env, "org/python/core/PySequenceIter");
	if (pySequenceIterClassLocal == NULL) { return JNI_ERR;}
	pySequenceIterClass = (jclass) (*env)->NewWeakGlobalRef(env, pySequenceIterClassLocal);
	(*env)->DeleteLocalRef(env, pySequenceIterClassLocal);

	jclass pyFastSequenceIterClassLocal = (*env)->FindClass(env, "org/python/core/PyFastSequenceIter");
	if (pyFastSequenceIterClassLocal == NULL) { return JNI_ERR;}
	pyFastSequenceIterClass = (jclass) (*env)->NewWeakGlobalRef(env, pyFastSequenceIterClassLocal);
	(*env)->DeleteLocalRef(env, pyFastSequenceIterClassLocal);

	jclass pyReversedIteratorClassLocal = (*env)->FindClass(env, "org/python/core/PyReversedIterator");
	if (pyReversedIteratorClassLocal == NULL) { return JNI_ERR;}
	pyReversedIteratorClass = (jclass) (*env)->NewWeakGlobalRef(env, pyReversedIteratorClassLocal);
	(*env)->DeleteLocalRef(env, pyReversedIteratorClassLocal);

	jclass pyBaseSetClassLocal = (*env)->FindClass(env, "org/python/core/BaseSet");
	if (pyBaseSetClassLocal == NULL) { return JNI_ERR;}
	pyBaseSetClass = (jclass) (*env)->NewWeakGlobalRef(env, pyBaseSetClassLocal);
	(*env)->DeleteLocalRef(env, pyBaseSetClassLocal);
	pyBaseSet_set = (*env)->GetFieldID(env, pyBaseSetClass, "_set", "Ljava/util/Set;");
	pyBaseSetSize = (*env)->GetMethodID(env, pyBaseSetClass, "size", "()I");
//	pyBaseSetClear = (*env)->GetMethodID(env, pyBaseSetClass, "clear", "()V");
//	pyBaseSetContains = (*env)->GetMethodID(env, pyBaseSetClass, "contains", "(Ljava/lang/Object;)Z");
//	pyBaseSetRemove = (*env)->GetMethodID(env, pyBaseSetClass, "remove", "(Ljava/lang/Object;)Z");
//	pyBaseSetAdd = (*env)->GetMethodID(env, pyBaseSetClass, "add", "(Ljava/lang/Object;)Z");
	pyBaseSet_update = (*env)->GetMethodID(env, pyBaseSetClass, "_update", "(Lorg/python/core/PyObject;)V");
	//pyBaseSetbaseset_union = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_union",
//	        "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseSetbaseset_issubset = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_issubset",
			"(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseSetbaseset_issuperset = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_issuperset",
			"(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseSetbaseset_isdisjoint = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_isdisjoint",
			"(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseSetbaseset_difference = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_difference",
			"(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseSetbaseset_differenceMulti = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_difference",
			"([Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseSetbaseset_symmetric_difference = (*env)->GetMethodID(env, pyBaseSetClass,
			"baseset_symmetric_difference", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseSetbaseset_intersection = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_intersection",
			"(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	//pyBaseSetbaseset_copy = (*env)->GetMethodID(env, pyBaseSetClass, "baseset_copy", "()Lorg/python/core/PyObject;");
	pyBaseSetbaseset___contains__ = (*env)->GetMethodID(env, pyBaseSetClass, "baseset___contains__",
			"(Lorg/python/core/PyObject;)Z");

	jclass pySetClassLocal = (*env)->FindClass(env, "org/python/core/PySet");
	if (pySetClassLocal == NULL) { return JNI_ERR;}
	pySetClass = (jclass) (*env)->NewWeakGlobalRef(env, pySetClassLocal);
	(*env)->DeleteLocalRef(env, pySetClassLocal);
	pySetFromIterableConstructor = (*env)->GetMethodID(env, pySetClass, "<init>", "(Lorg/python/core/PyObject;)V");
	pySetset_pop = (*env)->GetMethodID(env, pySetClass, "set_pop", "()Lorg/python/core/PyObject;");
	pySetset_clear = (*env)->GetMethodID(env, pySetClass, "set_clear", "()V");
	pySetset_discard = (*env)->GetMethodID(env, pySetClass, "set_discard", "(Lorg/python/core/PyObject;)V");
	pySetset_add = (*env)->GetMethodID(env, pySetClass, "set_add", "(Lorg/python/core/PyObject;)V");
	pySetset_difference_update = (*env)->GetMethodID(env, pySetClass, "set_difference_update",
			"([Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	pySetset_intersection_update = (*env)->GetMethodID(env, pySetClass, "set_intersection_update",
			"([Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	pySetset_symmetric_difference_update = (*env)->GetMethodID(env, pySetClass, "set_symmetric_difference_update",
			"(Lorg/python/core/PyObject;)V");

	jclass pyFrozenSetClassLocal = (*env)->FindClass(env, "org/python/core/PyFrozenSet");
	if (pyFrozenSetClassLocal == NULL) { return JNI_ERR;}
	pyFrozenSetClass = (jclass) (*env)->NewWeakGlobalRef(env, pyFrozenSetClassLocal);
	(*env)->DeleteLocalRef(env, pyFrozenSetClassLocal);
	pyFrozenSetFromIterableConstructor = (*env)->GetMethodID(env, pyFrozenSetClass, "<init>", "(Lorg/python/core/PyObject;)V");
	//pyFrozenSetSize = (*env)->GetMethodID(env, pyFrozenSetClass, "size", "()I");

	jclass pyEnumerationClassLocal = (*env)->FindClass(env, "org/python/core/PyEnumerate");
	if (pyEnumerationClassLocal == NULL) { return JNI_ERR;}
	pyEnumerationClass = (jclass) (*env)->NewWeakGlobalRef(env, pyEnumerationClassLocal);
	(*env)->DeleteLocalRef(env, pyEnumerationClassLocal);

	jclass pySliceClassLocal = (*env)->FindClass(env, "org/python/core/PySlice");
	if (pySliceClassLocal == NULL) { return JNI_ERR;}
	pySliceClass = (jclass) (*env)->NewWeakGlobalRef(env, pySliceClassLocal);
	(*env)->DeleteLocalRef(env, pySliceClassLocal);
	pySliceFromStartStopStepConstructor = (*env)->GetMethodID(env, pySliceClass, "<init>",
			"(Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyObject;)V");
	pySliceToString = (*env)->GetMethodID(env, pySliceClass, "toString", "()Ljava/lang/String;");
	pySliceGetStart = (*env)->GetMethodID(env, pySliceClass, "getStart", "()Lorg/python/core/PyObject;");
	pySliceGetStop = (*env)->GetMethodID(env, pySliceClass, "getStop", "()Lorg/python/core/PyObject;");
	pySliceGetStep = (*env)->GetMethodID(env, pySliceClass, "getStep", "()Lorg/python/core/PyObject;");
	pySliceIndicesEx = (*env)->GetMethodID(env, pySliceClass, "indicesEx", "(I)[I");

	jclass pyEllipsisClassLocal = (*env)->FindClass(env, "org/python/core/PyEllipsis");
	if (pyEllipsisClassLocal == NULL) { return JNI_ERR;}
	pyEllipsisClass = (jclass) (*env)->NewWeakGlobalRef(env, pyEllipsisClassLocal);
	(*env)->DeleteLocalRef(env, pyEllipsisClassLocal);

	jclass pyGeneratorClassLocal = (*env)->FindClass(env, "org/python/core/PyGenerator");
	if (pyGeneratorClassLocal == NULL) { return JNI_ERR;}
	pyGeneratorClass = (jclass) (*env)->NewWeakGlobalRef(env, pyGeneratorClassLocal);
	(*env)->DeleteLocalRef(env, pyGeneratorClassLocal);

	jclass pyWeakReferenceClassLocal = (*env)->FindClass(env, "org/python/modules/_weakref/ReferenceType");
	if (pyWeakReferenceClassLocal == NULL) { return JNI_ERR;}
	pyWeakReferenceClass = (jclass) (*env)->NewWeakGlobalRef(env, pyWeakReferenceClassLocal);
	(*env)->DeleteLocalRef(env, pyWeakReferenceClassLocal);

	jclass pyWeakProxyClassLocal = (*env)->FindClass(env, "org/python/modules/_weakref/ProxyType");
	if (pyWeakProxyClassLocal == NULL) { return JNI_ERR;}
	pyWeakProxyClass = (jclass) (*env)->NewWeakGlobalRef(env, pyWeakProxyClassLocal);
	(*env)->DeleteLocalRef(env, pyWeakProxyClassLocal);

	jclass pyWeakCallableProxyClassLocal = (*env)->FindClass(env, "org/python/modules/_weakref/CallableProxyType");
	if (pyWeakCallableProxyClassLocal == NULL) { return JNI_ERR;}
	pyWeakCallableProxyClass = (jclass) (*env)->NewWeakGlobalRef(env, pyWeakCallableProxyClassLocal);
	(*env)->DeleteLocalRef(env, pyWeakCallableProxyClassLocal);

	jclass pyCodeClassLocal = (*env)->FindClass(env, "org/python/core/PyCode");
	if (pyCodeClassLocal == NULL) { return JNI_ERR;}
	//pyCodeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCodeClassLocal);
	pyCode_co_name = (*env)->GetFieldID(env, pyCodeClassLocal, "co_name", "Ljava/lang/String;");
	(*env)->DeleteLocalRef(env, pyCodeClassLocal);

	jclass pyBaseCodeClassLocal = (*env)->FindClass(env, "org/python/core/PyBaseCode");
	if (pyBaseCodeClassLocal == NULL) { return JNI_ERR;}
	pyBaseCode_co_argcount = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_argcount", "I");
	pyBaseCode_co_cellvars = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_cellvars", "[Ljava/lang/String;");
	pyBaseCode_co_filename = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_filename", "Ljava/lang/String;");
	pyBaseCode_co_firstlineno = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_firstlineno", "I");
	//pyBaseCode_co_flags = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_flags", "I");
	pyBaseCode_co_freevars = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_freevars", "[Ljava/lang/String;");
	pyBaseCode_co_nlocals = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_nlocals", "I");
	pyBaseCode_co_varnames = (*env)->GetFieldID(env, pyBaseCodeClassLocal, "co_varnames", "[Ljava/lang/String;");
	(*env)->DeleteLocalRef(env, pyBaseCodeClassLocal);

	jclass pyBytecodeClassLocal = (*env)->FindClass(env, "org/python/core/PyBytecode");
	if (pyBytecodeClassLocal == NULL) { return JNI_ERR;}
	pyBytecodeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyBytecodeClassLocal);
	(*env)->DeleteLocalRef(env, pyBytecodeClassLocal);
	pyBytecodeConstructor = (*env)->GetMethodID(env, pyBytecodeClass, "<init>",
			"(IIIILjava/lang/String;[Lorg/python/core/PyObject;[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V");
//	int argcount, int nlocals, int stacksize, int flags, java.lang.String codestring,
//	PyObject[] constants, java.lang.String[] names, java.lang.String[] varnames,
//	java.lang.String filename, java.lang.String name, int firstlineno,
//	java.lang.String lnotab, java.lang.String[] cellvars, java.lang.String[] freevars
//	pyBytecode_co_code = (*env)->GetFieldID(env, pyBytecodeClass, "co_code", "[B");
	pyBytecode_co_consts = (*env)->GetFieldID(env, pyBytecodeClass, "co_consts", "[Lorg/python/core/PyObject;");
//	pyBytecode_co_lnotab = (*env)->GetFieldID(env, pyBytecodeClass, "co_lnotab", "[B");
	pyBytecode_co_names = (*env)->GetFieldID(env, pyBytecodeClass, "co_names", "[Ljava/lang/String;");
	pyBytecode_co_stacksize = (*env)->GetFieldID(env, pyBytecodeClass, "co_stacksize", "I");

	jclass pyTableCodeClassLocal = (*env)->FindClass(env, "org/python/core/PyTableCode");
	if (pyTableCodeClassLocal == NULL) { return JNI_ERR;}
	pyTableCodeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyTableCodeClassLocal);
	(*env)->DeleteLocalRef(env, pyTableCodeClassLocal);
	//pyTableCode_co_code;


	jclass pyCallIterClassLocal = (*env)->FindClass(env, "org/python/core/PyCallIter");
	if (pyCallIterClassLocal == NULL) { return JNI_ERR;}
	pyCallIterClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCallIterClassLocal);
	(*env)->DeleteLocalRef(env, pyCallIterClassLocal);

	jclass pySuperClassLocal = (*env)->FindClass(env, "org/python/core/PySuper");
	if (pySuperClassLocal == NULL) { return JNI_ERR;}
	pySuperClass = (jclass) (*env)->NewWeakGlobalRef(env, pySuperClassLocal);
	(*env)->DeleteLocalRef(env, pySuperClassLocal);

	jclass GlobalRefClassLocal = (*env)->FindClass(env, "org/python/modules/_weakref/GlobalRef");
	if (GlobalRefClassLocal == NULL) { return JNI_ERR;}
	GlobalRefClass = (jclass) (*env)->NewWeakGlobalRef(env, GlobalRefClassLocal);
	(*env)->DeleteLocalRef(env, GlobalRefClassLocal);
	GlobalRef_retryFactory = (*env)->GetMethodID(env, GlobalRefClass, "retryFactory", "()Lorg/python/modules/_weakref/ReferenceBackend;");

	jclass AbstractReferenceClassLocal = (*env)->FindClass(env, "org/python/modules/_weakref/AbstractReference");
	if (AbstractReferenceClassLocal == NULL) { return JNI_ERR;}
	AbstractReferenceClass = (jclass) (*env)->NewWeakGlobalRef(env, AbstractReferenceClassLocal);
	(*env)->DeleteLocalRef(env, AbstractReferenceClassLocal);
	AbstractReference_get = (*env)->GetMethodID(env, AbstractReferenceClass, "get", "()Lorg/python/core/PyObject;");

	jclass JyNIGlobalRefClassLocal = (*env)->FindClass(env, "JyNI/JyNIGlobalRef");
	if (JyNIGlobalRefClassLocal == NULL) { return JNI_ERR;}
	JyNIGlobalRefClass = (jclass) (*env)->NewWeakGlobalRef(env, JyNIGlobalRefClassLocal);
	(*env)->DeleteLocalRef(env, JyNIGlobalRefClassLocal);
	JyNIGlobalRef_initNativeHandle = (*env)->GetMethodID(env, JyNIGlobalRefClass, "initNativeHandle", "(J)V");

	jclass pyBaseExceptionClassLocal = (*env)->FindClass(env, "org/python/core/PyBaseException");
	if (pyBaseExceptionClassLocal == NULL) { return JNI_ERR;}
	pyBaseExceptionClass = (jclass) (*env)->NewWeakGlobalRef(env, pyBaseExceptionClassLocal);
	(*env)->DeleteLocalRef(env, pyBaseExceptionClassLocal);
	pyBaseExceptionEmptyConstructor = (*env)->GetMethodID(env, pyBaseExceptionClass, "<init>", "()V");
	pyBaseExceptionSubTypeConstructor = (*env)->GetMethodID(env, pyBaseExceptionClass, "<init>", "(Lorg/python/core/PyType;)V");
	pyBaseException__init__ = (*env)->GetMethodID(env, pyBaseExceptionClass, "__init__", "([Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	pyBaseException__setstate__ = (*env)->GetMethodID(env, pyBaseExceptionClass, "__setstate__", "(Lorg/python/core/PyObject;)Lorg/python/core/PyObject;");
	pyBaseException__unicode__ = (*env)->GetMethodID(env, pyBaseExceptionClass, "__unicode__", "()Lorg/python/core/PyUnicode;");
	pyBaseExceptionArgs = (*env)->GetFieldID(env, pyBaseExceptionClass, "args", "Lorg/python/core/PyObject;");
	pyBaseExceptionSetArgs = (*env)->GetMethodID(env, pyBaseExceptionClass, "setArgs", "(Lorg/python/core/PyObject;)V");
	pyBaseExceptionGetMessage = (*env)->GetMethodID(env, pyBaseExceptionClass, "getMessage", "()Lorg/python/core/PyObject;");
	pyBaseExceptionSetMessage = (*env)->GetMethodID(env, pyBaseExceptionClass, "setMessage", "(Lorg/python/core/PyObject;)V");

	jclass pyByteArrayClassLocal = (*env)->FindClass(env, "org/python/core/PyByteArray");
	if (pyByteArrayClassLocal == NULL) { return JNI_ERR;}
	pyByteArrayClass = (jclass) (*env)->NewWeakGlobalRef(env, pyByteArrayClassLocal);
	(*env)->DeleteLocalRef(env, pyByteArrayClassLocal);

	jclass pyBufferClassLocal = (*env)->FindClass(env, "org/python/core/PyBuffer");
	if (pyBufferClassLocal == NULL) { return JNI_ERR;}
	pyBufferClass = (jclass) (*env)->NewWeakGlobalRef(env, pyBufferClassLocal);
	(*env)->DeleteLocalRef(env, pyBufferClassLocal);

	jclass pyMemoryViewClassLocal = (*env)->FindClass(env, "org/python/core/PyMemoryView");
	if (pyMemoryViewClassLocal == NULL) { return JNI_ERR;}
	pyMemoryViewClass = (jclass) (*env)->NewWeakGlobalRef(env, pyMemoryViewClassLocal);
	(*env)->DeleteLocalRef(env, pyMemoryViewClassLocal);

	jclass __builtin__ClassLocal = (*env)->FindClass(env, "org/python/core/__builtin__");
	if (__builtin__ClassLocal == NULL) { return JNI_ERR;}
	__builtin__Class = (jclass) (*env)->NewWeakGlobalRef(env, __builtin__ClassLocal);
	(*env)->DeleteLocalRef(env, __builtin__ClassLocal);
	//__builtin__Import = (*env)->GetStaticMethodID(env, __builtin__Class, "__import__", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	__builtin__ImportLevel = (*env)->GetStaticMethodID(env, __builtin__Class, "__import__", "(Ljava/lang/String;Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyObject;I)Lorg/python/core/PyObject;");

	jclass impClassLocal = (*env)->FindClass(env, "org/python/core/imp");
	if (!impClassLocal) return JNI_ERR;
	impClass = (jclass) (*env)->NewWeakGlobalRef(env, impClassLocal);
	(*env)->DeleteLocalRef(env, impClassLocal);
	imp_importName = (*env)->GetStaticMethodID(env, impClass, "importName", "(Ljava/lang/String;Z)Lorg/python/core/PyObject;");
	imp_reload = (*env)->GetStaticMethodID(env, impClass, "reload", "(Lorg/python/core/PyModule;)Lorg/python/core/PyObject;");

	jclass exceptionsClassLocal = (*env)->FindClass(env, "org/python/core/exceptions");
	if (exceptionsClassLocal == NULL) { return JNI_ERR;}
	exceptionsClass = (jclass) (*env)->NewWeakGlobalRef(env, exceptionsClassLocal);
	(*env)->DeleteLocalRef(env, exceptionsClassLocal);

	exceptionsKeyError = (*env)->GetStaticMethodID(env, exceptionsClass, "KeyError", "()Lorg/python/core/PyObject;");
	exceptionsKeyError__str__ = (*env)->GetStaticMethodID(env, exceptionsClass, "KeyError__str__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyObject;");
	exceptionsEnvironmentError = (*env)->GetStaticMethodID(env, exceptionsClass, "EnvironmentError", "()Lorg/python/core/PyObject;");
	exceptionsEnvironmentError__init__ = (*env)->GetStaticMethodID(env, exceptionsClass, "EnvironmentError__init__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	exceptionsEnvironmentError__str__ = (*env)->GetStaticMethodID(env, exceptionsClass, "EnvironmentError__str__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyObject;");
	exceptionsEnvironmentError__reduce__ = (*env)->GetStaticMethodID(env, exceptionsClass, "EnvironmentError__reduce__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyObject;");
	exceptionsSyntaxError = (*env)->GetStaticMethodID(env, exceptionsClass, "SyntaxError", "()Lorg/python/core/PyObject;");
	exceptionsSyntaxError__init__ = (*env)->GetStaticMethodID(env, exceptionsClass, "SyntaxError__init__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	exceptionsSyntaxError__str__ = (*env)->GetStaticMethodID(env, exceptionsClass, "SyntaxError__str__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyString;");
	exceptionsSystemExit = (*env)->GetStaticMethodID(env, exceptionsClass, "SystemExit", "()Lorg/python/core/PyObject;");
	exceptionsSystemExit__init__ = (*env)->GetStaticMethodID(env, exceptionsClass, "SystemExit__init__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)V");

	exceptionsUnicodeError = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeError", "()Lorg/python/core/PyObject;");
	#ifdef Py_USING_UNICODE
	exceptionsGetStart = (*env)->GetStaticMethodID(env, exceptionsClass, "getStart", "(Lorg/python/core/PyObject;Z)I");
	exceptionsGetEnd = (*env)->GetStaticMethodID(env, exceptionsClass, "getEnd", "(Lorg/python/core/PyObject;Z)I");
	exceptionsGetString = (*env)->GetStaticMethodID(env, exceptionsClass, "getString", "(Lorg/python/core/PyObject;Ljava/lang/String;)Lorg/python/core/PyString;");
	exceptionsGetUnicode = (*env)->GetStaticMethodID(env, exceptionsClass, "getUnicode", "(Lorg/python/core/PyObject;Ljava/lang/String;)Lorg/python/core/PyUnicode;");
	exceptionsUnicodeError__init__ = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeError__init__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;Lorg/python/core/PyType;)V");
	exceptionsUnicodeEncodeError = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeDecodeError", "()Lorg/python/core/PyObject;");
	exceptionsUnicodeEncodeError__init__ = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeEncodeError__init__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	exceptionsUnicodeEncodeError__str__ = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeEncodeError__str__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyString;");
	exceptionsUnicodeDecodeError = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeEncodeError", "()Lorg/python/core/PyObject;");
	exceptionsUnicodeDecodeError__init__ = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeDecodeError__init__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	exceptionsUnicodeDecodeError__str__ = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeDecodeError__str__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyString;");
	exceptionsUnicodeTranslateError = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeTranslateError", "()Lorg/python/core/PyObject;");
	exceptionsUnicodeTranslateError__init__ = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeTranslateError__init__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)V");
	exceptionsUnicodeTranslateError__str__ = (*env)->GetStaticMethodID(env, exceptionsClass, "UnicodeTranslateError__str__", "(Lorg/python/core/PyObject;[Lorg/python/core/PyObject;[Ljava/lang/String;)Lorg/python/core/PyString;");
	#endif

	return JNI_VERSION_1_2;
}

inline jint initSingletons(JNIEnv *env)
{
	/*jmethodID pyUnicodeEmptyConstructor = (*env)->GetMethodID(env, pyUnicodeClass, "<init>", "()V");
	jobject unicode_emptyLocal = (*env)->NewObject(env, pyUnicodeClass, pyUnicodeEmptyConstructor);
	unicode_empty = JyNI_PyObject_FromJythonPyObject((*env)->NewWeakGlobalRef(env, unicode_emptyLocal));
	(*env)->DeleteLocalRef(env, unicode_emptyLocal);
	*/
	//jmethodID pyNoneConstructor = (*env)->GetMethodID(env, pyNoneClass, "<init>", "()V");
	//if (pyNoneConstructor == NULL) { return JNI_ERR;}
	//JyNone = JyNI_PyObject_FromJythonPyObject((*env)->NewWeakGlobalRef(env, (*env)->NewObject(env, pyNoneClass, pyNoneConstructor)));
	jfieldID jyNone = (*env)->GetStaticFieldID(env, pyPyClass, "None", "Lorg/python/core/PyObject;");
	JyNone = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyNone));
	jfieldID jyNotImplemented = (*env)->GetStaticFieldID(env, pyPyClass, "NotImplemented", "Lorg/python/core/PyObject;");
	JyNotImplemented = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyNotImplemented));
	jfieldID jyEllipsis = (*env)->GetStaticFieldID(env, pyPyClass, "Ellipsis", "Lorg/python/core/PyObject;");
	JyEllipsis = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyEllipsis));
	jfieldID jyEmptyFrozenSet = (*env)->GetStaticFieldID(env, pyPyClass, "EmptyFrozenSet", "Lorg/python/core/PyFrozenSet;");
	JyEmptyFrozenSet = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyEmptyFrozenSet));
	jfieldID jyEmptyString = (*env)->GetStaticFieldID(env, pyPyClass, "EmptyString", "Lorg/python/core/PyString;");
	JyEmptyString = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyEmptyString));
	jfieldID jyEmptyUnicode = (*env)->GetStaticFieldID(env, pyPyClass, "EmptyUnicode", "Lorg/python/core/PyUnicode;");
	JyEmptyUnicode = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyEmptyUnicode));
	jfieldID jyEmptyTuple = (*env)->GetStaticFieldID(env, pyPyClass, "EmptyTuple", "Lorg/python/core/PyTuple;");
	JyEmptyTuple = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyEmptyTuple));
	jfieldID jyEmptyStringArray = (*env)->GetStaticFieldID(env, pyPyClass, "NoKeywords", "[Ljava/lang/String;");
	length0StringArray = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyEmptyStringArray));
	jfieldID jyEmptyPyObjArray = (*env)->GetStaticFieldID(env, pyPyClass, "EmptyObjects", "[Lorg/python/core/PyObject;");
	length0PyObjectArray = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyEmptyPyObjArray));
	jfieldID jyTrue = (*env)->GetStaticFieldID(env, pyPyClass, "True", "Lorg/python/core/PyBoolean;");
	JyTrue = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyTrue));
	jfieldID jyFalse = (*env)->GetStaticFieldID(env, pyPyClass, "False", "Lorg/python/core/PyBoolean;");
	JyFalse = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, pyPyClass, jyFalse));


//	length0StringArray = (*env)->NewGlobalRef(env, (*env)->NewObjectArray(env, 0, stringClass, NULL));
//	length0PyObjectArray = (*env)->NewGlobalRef(env, (*env)->NewObjectArray(env, 0, pyObjectClass, NULL));

//	jfieldID jyMemoryErrorInst = (*env)->GetStaticFieldID(env, pyPyClass, );
//	PyExc_MemoryErrorInst = JyNI_PyObject_FromJythonPyObject();
	return JNI_VERSION_1_2;
}

#define native_table_index_long 10
#define native_table_index_ulong 11
static inline void patchJythonStructModule(JNIEnv *env)
{
	if (sizeof(long) != 4)
	{
		jclass structModuleClass = (*env)->FindClass(env, "org/python/modules/struct");
		jfieldID structModule_native_table = (*env)->GetStaticFieldID(env, structModuleClass,
				"native_table", "[Lorg/python/modules/struct$FormatDef;");
		jclass structModule_FormatDefClass = (*env)->FindClass(env, "org/python/modules/struct$FormatDef");
		jfieldID structModule_FormatDef_size = (*env)->GetFieldID(env, structModule_FormatDefClass, "size", "I");
		jfieldID structModule_FormatDef_align = (*env)->GetFieldID(env, structModule_FormatDefClass, "alignment", "I");
		jarray struct_native_table = (*env)->GetStaticObjectField(env, structModuleClass, structModule_native_table);
		jobject long_FormatDef = (*env)->GetObjectArrayElement(env, struct_native_table, native_table_index_long);
		jobject ulong_FormatDef = (*env)->GetObjectArrayElement(env, struct_native_table, native_table_index_ulong);
		(*env)->SetIntField(env, long_FormatDef, structModule_FormatDef_size, sizeof(long));
		(*env)->SetIntField(env, long_FormatDef, structModule_FormatDef_align, sizeof(long));
		(*env)->SetIntField(env, ulong_FormatDef, structModule_FormatDef_size, sizeof(long));
		(*env)->SetIntField(env, ulong_FormatDef, structModule_FormatDef_align, sizeof(long));
	}
}

int Py_DebugFlag; /* Needed by parser.c */
int Py_VerboseFlag; /* Needed by import.c */
int Py_InteractiveFlag; /* Needed by Py_FdIsInteractive() below */
int Py_InspectFlag; /* Needed to determine whether to exit at SystemExit */
int Py_NoSiteFlag; /* Suppress 'import site' */
int Py_BytesWarningFlag; /* Warn on str(bytes) and str(buffer) */
int Py_DontWriteBytecodeFlag; /* Suppress writing bytecode files (*.py[co]) */
int Py_UseClassExceptionsFlag = 1; /* Needed by bltinmodule.c: deprecated */
int Py_FrozenFlag; /* Needed by getpath.c */
int Py_UnicodeFlag = 0; /* Needed by compile.c */
int Py_OptimizeFlag = 0; /* Originally hosted in compile.c */
int Py_IgnoreEnvironmentFlag = 0; /* e.g. PYTHONPATH, PYTHONHOME */
/* _XXX Py_QnewFlag should go away in 2.3.  It's true iff -Qnew is passed,
  on the command line, and is used in 2.2 by ceval.c to make all "/" divisions
  true divisions (which they will be in 2.3). */
int _Py_QnewFlag = 0;
int Py_NoUserSiteDirectory = 0; /* for -s and site.py */
int Py_HashRandomizationFlag = 0; /* for -R and PYTHONHASHSEED */

//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
jint JyNI_init(JavaVM *jvm)
{
	//puts("JyNI_init");

	//cout << "Hier OnLoad7!" << endl;
	java = jvm; // cache the JavaVM pointer
	JNIEnv *env;
//	c++
//	if (jvm->GetEnv(java, (void **)&env, JNI_VERSION_1_2)) {
//		return JNI_ERR; // JNI version not supported
//	}

	if ((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_2)) {
		return JNI_ERR; // JNI version not supported
	}
	//Py_Py3kWarningFlag
	if (initJNI(env) == JNI_ERR) return JNI_ERR;
	//puts("initJNI done");
	if (initJyNI(env) == JNI_ERR) return JNI_ERR;
	//puts("initJyNI done");
	if (initJythonSite(env) == JNI_ERR) return JNI_ERR;
	//puts("initJythonSite done");
	if (initJythonObjects(env) == JNI_ERR) return JNI_ERR;
	//puts("initJythonObjects done");
	initBuiltinTypes();
	//puts("initBuiltinTypes done");
	initBuiltinExceptions();
	//puts("initBuiltinExceptions done");
	if (initSingletons(env) == JNI_ERR) return JNI_ERR;
	//puts("initSingletons done");

	patchJythonStructModule(env);

	//puts("characters-info:");
	//characters[UCHAR_MAX + 1]
	//characters2[0] = NULL;
	//printf("%u\n", characters2[0]);
	//dlopen("/home/stefan/eclipseWorkspace/JyNI/JyNI-C/Debug/libJyNI.so", RTLD_NOLOAD | RTLD_GLOBAL);
	//initErrorsFromJython(env);

	//init native objects where necessary:
	PyEval_InitThreads();
	_PyInt_Init();
	PyType_Ready(&PyBool_Type);

	return JNI_VERSION_1_2;
}

void JyNI_unload(JavaVM *jvm)
{
	jputs("JyNI_unload");
	PyString_Fini();
	PyInt_Fini();
	PyTuple_Fini();
	PyDict_Fini();
	PyCFunction_Fini();
	PySet_Fini();

	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].sync != NULL) free(builtinTypes[i].sync);
		if (builtinTypes[i].type_name != NULL) free(builtinTypes[i].type_name);
	}

	//free(specialPyInstance.sync);

	env();
	(*env)->DeleteWeakGlobalRef(env, JyNone);
	(*env)->DeleteWeakGlobalRef(env, JyNotImplemented);
	(*env)->DeleteWeakGlobalRef(env, JyEllipsis);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyFrozenSet);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyString);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyUnicode);
	(*env)->DeleteWeakGlobalRef(env, JyTrue);
	(*env)->DeleteWeakGlobalRef(env, JyFalse);

	(*env)->DeleteWeakGlobalRef(env, length0StringArray);
	(*env)->DeleteWeakGlobalRef(env, length0PyObjectArray);
}

//inline void JyNI_printJ(jobject obj)
//{
//	if (obj)
//	{
//		env();
//		jstring msg = (*env)->CallObjectMethod(env, obj, objectToString);
//		cstr_from_jstring(cmsg, msg);
//		puts(cmsg);
//	} else puts("NULL");
//}

inline void JyNI_jprintHash(jobject obj) {
	env();
	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_jPrintHash, obj);
}

inline void JyNI_jprintJ(jobject obj)
{
	//jputs(__FUNCTION__);
	if (obj)
	{
		//jputsLong(__LINE__);
		env();
		if ((*env)->GetObjectRefType(env, obj)) {
			if (!(*env)->IsSameObject(env, obj, NULL)) {
				//jputsLong(__LINE__);
				jstring msg = (*env)->CallObjectMethod(env, obj, objectToString);
				//jputsLong(__LINE__);
				(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_jPrint, msg);
				//jputsLong(__LINE__);
			} else jputs("NULL (equal)");
		} else jputs("Invalid reference");
	} else jputs("NULL");
}

inline void JyNI_printJInfo(jobject obj)
{
	if (obj)
	{
		env();
		//jclass cls = (*env)->GetObjectClass(env, obj);
		jobject cls = (*env)->CallObjectMethod(env, obj, objectGetClass);
		//jputs("got cls");
		JyNI_jprintJ(cls);
		//jstring msg = (*env)->CallObjectMethod(env, obj, objectToString);
		//cstr_from_jstring(cmsg, msg);
		//puts(cmsg);
	} else jputs("object is NULL");
}

inline void _jputs(const char* msg)
{
	env();
	jstring str = (*env)->NewStringUTF(env, msg);
	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_jPrint, str);
	(*env)->DeleteLocalRef(env, str);
}

inline void jputsLong(jlong val)
{
	env();
	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_jPrintLong, val);
}

inline void jputsPy(PyObject* o)
{
	jputs(o ? PyString_AS_STRING(PyObject_Str(o)) : "NULL-PyObject");
}

/*
 * This method currently only exists to prevent symbol lookup errors,
 * but must not be actually called. It will be implemented with
 * BufferProtocol support in JyNI 2.7-alpha.5
 */
PyObject *
PyMemoryView_FromObject(PyObject *base)
{
	//puts(__FUNCTION__);
	jputs("JyNI-Warning: PyMemoryView_FromObject is not implemented!");
	return NULL;
//    PyMemoryViewObject *mview;
//    Py_buffer view;
//
//    if (!PyObject_CheckBuffer(base)) {
//        PyErr_SetString(PyExc_TypeError,
//            "cannot make memory view because object does "
//            "not have the buffer interface");
//        return NULL;
//    }
//
//    if (PyObject_GetBuffer(base, &view, PyBUF_FULL_RO) < 0)
//        return NULL;
//
//    mview = (PyMemoryViewObject *)PyMemoryView_FromBuffer(&view);
//    if (mview == NULL) {
//        PyBuffer_Release(&view);
//        return NULL;
//    }
//
//    mview->base = base;
//    Py_INCREF(base);
//    return (PyObject *)mview;
}
