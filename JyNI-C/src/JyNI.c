/*
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
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
//PyObject_MALLOC
#include <JyNI.h>
#include <JySync.h>
#include <code_JyNI.h>
//#include <JyNI_JyNI.h>
//#include "frameobject.h"
#include "importdl.h"
//#include <dlfcn.h>
//#include "stringlib/string_format.h"
//_Py_CheckInterval
const char* excPrefix = "exceptions.";
//jlong JyNIDebugMode = 0;

TypeMapEntry builtinTypes[builtinTypeCount];
static TypeMapEntry specialPyInstance;

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
	ENTER_JyNI
	if (PyErr_Occurred()) jputs("PyErrOccured01 (beginning of JyNI_loadModule)");//this should never happen!
	const char* utf_string;
	utf_string = (*env)->GetStringUTFChars(env, moduleName, NULL);
	//"+1" for 0-termination:
	char mName[strlen(utf_string)+1];
	strcpy(mName, utf_string);
	(*env)->ReleaseStringUTFChars(env, moduleName, utf_string);
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

	jobject er = _PyImport_LoadDynamicModuleJy(mName, mPath, fp);
	if (fclose(fp))
		jputs("Some error occurred on file close");
	LEAVE_JyNI
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
	//jputs("JyNI_callPyCPeer called");
	//note: here should be done sync
	//(maybe sync-idea is obsolete anyway)
	PyObject* peer = (PyObject*) peerHandle;
//	if (!peer->ob_type) jputs("ob_type of peer is NULL");
	ENTER_JyNI
	PyObject* jargs = JyNI_PyObject_FromJythonPyObject(args);
	//(*env)->DeleteLocalRef(env, args);
	PyObject* jkw = JyNI_PyObject_FromJythonPyObject(kw);
	//(*env)->DeleteLocalRef(env, kw);
	jobject er;
	//offsetof(PyTypeObject, tp_call);
	if (peer->ob_type->tp_call) {
//		jputsLong(__LINE__);
//		jputs(peer->ob_type->tp_name);
		PyObject* jres = peer->ob_type->tp_call(peer, jargs, jkw);
		er = JyNI_JythonPyObject_FromPyObject(jres);
		Py_XDECREF(jres);
	} else {
//		PyErr_Format(PyExc_TypeError, "'%.200s' object is not callable",
//				peer->ob_type->tp_name);
		er = NULL;
	}
	Py_XDECREF(jargs);
	Py_XDECREF(jkw);

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
	//jputs("JyNI_getAttrString");
	//jputsLong(tstate);
	if (handle == 0) return NULL;
	cstr_from_jstring(cName, name);
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
	ENTER_JyNI
	PyObject* res = PyObject_Repr((PyObject*) handle);
	jobject er = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(res);
	LEAVE_JyNI
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
	ENTER_JyNI
	PyObject* pkey = JyNI_PyObject_FromJythonPyObject(key);
	PyObject* res = PyObject_GetItem((PyObject*) handle, pkey);
	jobject er = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(pkey);
	Py_XDECREF(res);
	LEAVE_JyNI
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
 * Method:    PyObjectAsString
 * Signature: (JJ)Ljava/lang/String;
 */
jstring JyNI_PyObjectAsString(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	//jputs("JyNI_PyObjectAsString");
	if (!((PyObject*) handle)->ob_refcnt) return NULL;
	ENTER_JyNI
	PyStringObject* ps = (PyStringObject*) PyObject_Str((PyObject*) handle);
	jstring er = JyNI_jstring_FromPyStringObject(env, ps);
	Py_XDECREF(ps);
	LEAVE_JyNI
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

	builtinTypes[0].py_type = &PyType_Type;
	builtinTypes[0].jy_class = pyTypeClass;
	builtinTypes[0].flags = JY_TRUNCATE_FLAG_MASK;
	//builtinTypes[0].truncate_trailing = sizeof(PyTypeObject)-sizeof(PyVarObject);
	builtinTypes[0].truncate_trailing = sizeof(PyTypeObject)-sizeof(PyObject);
	//In fact, the trailing is so large that actually nothing is truncated.
	//This currently serves testing purposes. We are still checking out,
	//which fields are crucial and which are not. Although the only macros that
	//access PyType directly just need tp_flags, there are several direct accesses to
	//tp_-variables throughout CPython source.
	builtinTypes[0].sync = malloc(sizeof(SyncFunctions));
	//builtinTypes[0].sync->py2jy = (py2jySync) JySync_JyCell_From_PyCell;
	builtinTypes[0].sync->jy2py = (jy2pySync) JySync_PyType_From_JyType;

	builtinTypes[1].py_type = &PyNotImplemented_Type; //(_Py_NotImplementedStruct.ob_type);
	builtinTypes[1].jy_class = pyNotImplementedClass;
	//builtinTypes[1].flags = 0;

	builtinTypes[2].py_type = &PyNone_Type;//(_Py_NoneStruct.ob_type);
	builtinTypes[2].jy_class = pyNoneClass;
	//builtinTypes[2].flags = 0;

	builtinTypes[3].py_type = &PyFile_Type;
	builtinTypes[3].jy_class = pyFileClass;
	builtinTypes[3].flags = JY_TRUNCATE_FLAG_MASK;

	//In JyNI no GC-type since it is fully truncated.
	builtinTypes[4].py_type = &PyModule_Type;
	builtinTypes[4].jy_class = pyModuleClass;
	builtinTypes[4].flags = JY_TRUNCATE_FLAG_MASK;

	builtinTypes[5].py_type = &PyCell_Type;
	builtinTypes[5].jy_class = pyCellClass;
	//builtinTypes[5].flags = JY_GC_FIXED_SIZE; //JY_GC_SINGLE_LINK | JySYNC_ON_INIT_FLAGS;
	builtinTypes[5].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[5].sync->py2jy = (py2jySync) JySync_JyCell_From_PyCell;
	builtinTypes[5].sync->jy2py = (jy2pySync) JySync_PyCell_From_JyCell;

	builtinTypes[6].py_type = &PyClass_Type;
	builtinTypes[6].jy_class = pyClassClass;
	builtinTypes[6].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 6 links
	builtinTypes[6].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[6].sync->jyInit = (jyInitSync) JySync_Init_JyClass_From_PyClass;
	builtinTypes[6].sync->pyInit = (pyInitSync) JySync_Init_PyClass_From_JyClass;

	builtinTypes[7].py_type = &PyInstance_Type;
	builtinTypes[7].jy_class = pyInstanceClass;
	builtinTypes[7].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 2 links
	builtinTypes[7].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[7].sync->jyInit = (jyInitSync) JySync_Init_JyInstance_From_PyInstance;
	builtinTypes[7].sync->pyInit = (pyInitSync) JySync_Init_PyInstance_From_JyInstance;

	specialPyInstance.py_type = &PyInstance_Type;
	specialPyInstance.jy_class = pyInstanceClass;
	specialPyInstance.flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 2 links
	specialPyInstance.sync = malloc(sizeof(SyncFunctions));
	specialPyInstance.sync->pyInit = (pyInitSync) JySync_Init_Special_PyInstance;

	builtinTypes[8].py_type = &PyMethod_Type;
	builtinTypes[8].jy_class = pyMethodClass;
	builtinTypes[8].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE; // 3 links
	builtinTypes[8].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[8].sync->jyInit = (jyInitSync) JySync_Init_JyMethod_From_PyMethod;
	builtinTypes[8].sync->pyInit = (pyInitSync) JySync_Init_PyMethod_From_JyMethod;

	//Note: PyFunction is actually truncated in the sense that JyNI does not populate
	//all fields. However no memory can be saved because e.g. the last field func_module
	//is mirrored. This is needed e.g. for the PyFunction_GET_MODULE-macro.
	//In order to let the gcmodule get this right as a partly CStub, we still mark it
	//truncated and supply full length as truncate_trailing.
	builtinTypes[9].py_type = &PyFunction_Type;
	builtinTypes[9].jy_class = pyFunctionClass;
	//builtinTypes[9].flags = JY_GC_FIXED_SIZE; // 5 (8) links
	//This variant currently fails with delegation. Todo: Fix this.
	//builtinTypes[9].flags = JY_TRUNCATE_FLAG_MASK;
	//builtinTypes[9].truncate_trailing = 9*sizeof(PyObject*);
	builtinTypes[9].sync = malloc(sizeof(SyncFunctions));
	//builtinTypes[9].sync->jyInit = NULL;//(jyInitSync) JySync_Init_JyFunction_From_PyFunction;
	//builtinTypes[9].sync->pyInit = (pyInitSync) JySync_Init_PyFunction_From_JyFunction;
	builtinTypes[9].sync->jy2py = (jy2pySync) JySync_PyFunction_From_JyFunction;

/*	builtinTypes[10].py_type = &PyClassMethod_Type;
	builtinTypes[10].jy_class = pyClassMethodClass;
	builtinTypes[10].flags = 0;

	builtinTypes[11].py_type = &PyStaticMethod_Type;
	builtinTypes[11].jy_class = pyStaticMethodClass;
	builtinTypes[11].flags = 0;

	builtinTypes[12].py_type = &PyMethodDescr_Type;
	builtinTypes[12].jy_class = pyMethodDescrClass;
	builtinTypes[12].flags = 0;

	builtinTypes[13].py_type = &PyClassMethodDescr_Type;
	builtinTypes[13].jy_class = pyClassMethodDescrClass;
	builtinTypes[13].flags = 0;

	builtinTypes[14].py_type = &PyDictProxy_Type;
	builtinTypes[14].jy_class = pyDictProxyClass;
	builtinTypes[14].flags = 0;

	builtinTypes[15].py_type = &PyProperty_Type;
	builtinTypes[15].jy_class = pyPropertyClass;
	builtinTypes[15].flags = 0;*/

	builtinTypes[16].py_type = &PyBool_Type;
	builtinTypes[16].jy_class = pyBooleanClass;
	//builtinTypes[16].flags = 0;

	builtinTypes[17].py_type = &PyFloat_Type;
	builtinTypes[17].jy_class = pyFloatClass;
	builtinTypes[17].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[17].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[17].sync->jyInit = (jyInitSync) JySync_Init_JyFloat_From_PyFloat;
	builtinTypes[17].sync->pyInit = (pyInitSync) JySync_Init_PyFloat_From_JyFloat;

	builtinTypes[18].py_type = &PyInt_Type;
	builtinTypes[18].jy_class = pyIntClass;
	builtinTypes[18].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[18].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[18].sync->jyInit = (jyInitSync) JySync_Init_JyInt_From_PyInt;
	builtinTypes[18].sync->pyInit = (pyInitSync) JySync_Init_PyInt_From_JyInt;

	//for computational efficiency we mirror PyLong, although it could also be wrapped
	//later we are going to offer both options by a configuration-parameter in JyNI.
	builtinTypes[19].py_type = &PyLong_Type;
	builtinTypes[19].jy_class = pyLongClass;
	builtinTypes[19].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[19].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[19].sync->jyInit = (jyInitSync) JySync_Init_JyLong_From_PyLong;
	builtinTypes[19].sync->pyInit = (pyInitSync) JySync_Init_PyLong_From_JyLong;

	builtinTypes[20].py_type = &PyComplex_Type;
	builtinTypes[20].jy_class = pyComplexClass;
	builtinTypes[20].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[20].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[20].sync->jyInit = (jyInitSync) JySync_Init_JyComplex_From_PyComplex;
	builtinTypes[20].sync->pyInit = (pyInitSync) JySync_Init_PyComplex_From_JyComplex;

	builtinTypes[21].py_type = &PyUnicode_Type;
	builtinTypes[21].jy_class = pyUnicodeClass;
	builtinTypes[21].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[21].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[21].sync->jyInit = (jyInitSync) JySync_Init_JyUnicode_From_PyUnicode;
	builtinTypes[21].sync->pyInit = (pyInitSync) JySync_Init_PyUnicode_From_JyUnicode;

	builtinTypes[22].py_type = &PyString_Type;
	builtinTypes[22].jy_class = pyStringClass;
	builtinTypes[22].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[22].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[22].sync->jyInit = (jyInitSync) JySync_Init_JyString_From_PyString;
	builtinTypes[22].sync->pyInit = (pyInitSync) JySync_Init_PyString_From_JyString;

	builtinTypes[23].py_type = &_PyWeakref_RefType;
	builtinTypes[23].jy_class = pyWeakReferenceClass;
	builtinTypes[23].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[23].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[23].sync->jyInit = (jyInitSync) JySync_Init_JyWeakReference_From_PyWeakReference;
	builtinTypes[23].sync->pyInit = (pyInitSync) JySync_Init_PyWeakReference_From_JyWeakReference;

	builtinTypes[24].py_type = &_PyWeakref_ProxyType;
	builtinTypes[24].jy_class = pyWeakProxyClass;
	builtinTypes[24].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[24].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[24].sync->jyInit = (jyInitSync) JySync_Init_JyWeakProxy_From_PyWeakProxy;
	builtinTypes[24].sync->pyInit = (pyInitSync) JySync_Init_PyWeakProxy_From_JyWeakProxy;

	builtinTypes[25].py_type = &_PyWeakref_CallableProxyType;
	builtinTypes[25].jy_class = pyWeakCallableProxyClass;
	builtinTypes[25].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[25].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[25].sync->jyInit =
			(jyInitSync) JySync_Init_JyWeakCallableProxy_From_PyWeakCallableProxy;
	builtinTypes[25].sync->pyInit =
			(pyInitSync) JySync_Init_PyWeakCallableProxy_From_JyWeakCallableProxy;

/*	builtinTypes[23].py_type = &PyBaseString_Type;
	builtinTypes[23].jy_class = pyBaseStringClass;
	builtinTypes[23].flags = 0;

	builtinTypes[24].py_type = &PySeqIter_Type;
	builtinTypes[24].jy_class = pySequenceIterClass;
	builtinTypes[24].flags = 0;

	builtinTypes[25].py_type = &PyRange_Type;
	builtinTypes[25].jy_class = pyXRangeClass;
	builtinTypes[25].flags = 0;

	builtinTypes[26].py_type = &Pyrangeiter_Type; //jython uses PySequenceIter. Map this to PySeqIter_Typ
	builtinTypes[26].jy_class = pySequenceIterClass;
	builtinTypes[26].flags = 0;*/

	builtinTypes[27].py_type = &PyTuple_Type;
	builtinTypes[27].jy_class = pyTupleClass;
	builtinTypes[27].flags = JySYNC_ON_INIT_FLAGS;// | JY_GC_FIXED_SIZE;// | GC_NO_INITIAL_EXPLORE; // Py_SIZE(o) links
	builtinTypes[27].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[27].sync->jyInit = (jyInitSync) JySync_Init_JyTuple_From_PyTuple;
	builtinTypes[27].sync->pyInit = (pyInitSync) JySync_Init_PyTuple_From_JyTuple;

/*	builtinTypes[28].py_type = &PyTupleIter_Type; //jython uses PyFastSequenceIter. Map this to PySeqIter_Type
	builtinTypes[28].jy_class = pyFastSequenceIterClass;
	builtinTypes[28].flags = 0;*/

	builtinTypes[29].py_type = &PyList_Type;
	builtinTypes[29].jy_class = pyListClass;
	builtinTypes[29].flags = JySYNC_ON_INIT_FLAGS | GC_CRITICAL | JY_GC_VAR_SIZE;
			// | JY_GC_SPECIAL_CASE | GC_NO_INITIAL_EXPLORE;
	builtinTypes[29].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[29].sync->jyInit = (jyInitSync) JySync_Init_JyList_From_PyList;
	builtinTypes[29].sync->pyInit = (pyInitSync) JySync_Init_PyList_From_JyList;

/*	builtinTypes[30].py_type = &PyListIter_Type; //jython uses PyFastSequenceIter. Map this to PySeqIter_Type
	builtinTypes[30].jy_class = pyFastSequenceIterClass;
	builtinTypes[30].flags = 0;

	builtinTypes[31].py_type = &PyListRevIter_Type; //jython uses PyReversedIterator.
	builtinTypes[31].jy_class = pyReversedIteratorClass;
	builtinTypes[31].flags = 0;*/

	/* In JyNI no GC-type since it is fully truncated. */
	// Todo: What about subtype/subclass?
	builtinTypes[32].py_type = &PyDict_Type;
	builtinTypes[32].jy_class = pyDictClass;
	builtinTypes[32].jy_subclass = pyDictCPeerClass;
	builtinTypes[32].flags = JY_TRUNCATE_FLAG_MASK;

	/* In the CPython->Java lookup direction, this is
	   overwritten by the previous entry with pyDictClass.
	   However, it ensures that pyStringMapClass has a valid
	   entry if the other lookup direction is done.
	   Native PyDict methods are implemented in a way that is
	   agnostic about whether PyDictionary or PyStringMap is used
	   on Java/Jython-side.
	   Would be GC-relevant, but is fully truncated. */
	builtinTypes[33].py_type = &PyDict_Type;
	builtinTypes[33].jy_class = pyStringMapClass;
	builtinTypes[33].flags = JY_TRUNCATE_FLAG_MASK;
	char* tp_name33 = "stringmap";
	builtinTypes[33].type_name = malloc(strlen(tp_name33)+1);
	strcpy(builtinTypes[33].type_name, tp_name33);

	//In JyNI no GC-type since it is almost fully truncated.
	builtinTypes[34].py_type = &PySet_Type;
	builtinTypes[34].jy_class = pySetClass;
	builtinTypes[34].flags = JY_TRUNCATE_FLAG_MASK | JySYNC_ON_INIT_FLAGS;
	builtinTypes[34].truncate_trailing = sizeof(Py_ssize_t); //setObject.fill is covered by PyVarObject.ob_size. We add another sizeof(Py_ssize_t) to allocate space for setObject.used
	builtinTypes[34].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[34].sync->jyInit = NULL;//(jyInitSync) JySync_Init_JySet_From_PySet;
	builtinTypes[34].sync->pyInit = (pyInitSync) JySync_Init_PySet_From_JySet;

/*	builtinTypes[34].py_type = &PySetIter_Type; //jython uses inline subclass of PyIterator. Map this to PySeqIter_Type for now
	builtinTypes[34].jy_class = pySequenceIterClass;
	builtinTypes[34].flags = 0;*/

	//In JyNI no GC-type since it is almost fully truncated.
	builtinTypes[35].py_type = &PyFrozenSet_Type;
	builtinTypes[35].jy_class = pyFrozenSetClass;
	builtinTypes[35].flags = JY_TRUNCATE_FLAG_MASK | JySYNC_ON_INIT_FLAGS;
	builtinTypes[35].truncate_trailing = sizeof(Py_ssize_t); //setObject.fill is covered by PyVarObject.ob_size. We add another sizeof(Py_ssize_t) to allocate space for setObject.used
	builtinTypes[35].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[35].sync->jyInit = NULL;//(jyInitSync) JySync_Init_JyFrozenSet_From_PyFrozenSet;
	builtinTypes[35].sync->pyInit = (pyInitSync) JySync_Init_PyFrozenSet_From_JyFrozenSet;

/*	builtinTypes[36].py_type = &PyEnum_Type;
	builtinTypes[36].jy_class = pyEnumerationClass;
	builtinTypes[36].flags = 0;*/

	builtinTypes[37].py_type = &PySlice_Type;
	builtinTypes[37].jy_class = pySliceClass;
	builtinTypes[37].flags = JY_TRUNCATE_FLAG_MASK;

	builtinTypes[38].py_type = &PyEllipsis_Type;
	builtinTypes[38].jy_class = pyEllipsisClass;
	//builtinTypes[38].flags = 0;

/*	builtinTypes[39].py_type = &PyGen_Type;
	builtinTypes[39].jy_class = pyGeneratorClass;
	builtinTypes[39].flags = 0;*/

	/* Code objects are not subject to GC in CPython although they contain some links.
	 * However the contained links are always strings or string-tuples and nothing that
	 * could lead to reference-cycles.
	 * Todo: - Check whether this simplification of GC-traversal can be applied in Jython too.
	 *       - This might need to be improved for GIL-free mode. Check this.
	 */
	builtinTypes[40].py_type = &PyCode_Type;
	builtinTypes[40].jy_class = pyBytecodeClass;
	builtinTypes[40].flags = JY_TRUNCATE_FLAG_MASK;// | JySYNC_ON_INIT_FLAGS;
//	allocates trailing space for the macro PyCode_GetNumFree(op).
//	This macro accesses the memory-pos of co_freevars, so memory till co_freevars must
//	be allocated via truncate_trailing-mode. The size-formula 4*sizeof(int) + 5*sizeof(PyObject*)
//	arises from the first 9 members of PyCodeObject:
//	int co_argcount;	int co_nlocals;	int co_stacksize; int co_flags;
//	PyObject *co_code; PyObject *co_consts; PyObject *co_names; PyObject *co_varnames;
//	PyObject *co_freevars;
	builtinTypes[40].truncate_trailing = 4*sizeof(int) + 5*sizeof(PyObject*);
	builtinTypes[40].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[40].sync->jy2py = (jy2pySync) JySync_PyCode_From_JyCode;
	char* tp_name40 = "bytecode";
	builtinTypes[40].type_name = malloc(strlen(tp_name40)+1);
	strcpy(builtinTypes[40].type_name, tp_name40);

	builtinTypes[41].py_type = &PyCode_Type;
	builtinTypes[41].jy_class = pyTableCodeClass;
	builtinTypes[41].flags = JY_TRUNCATE_FLAG_MASK;// | JySYNC_ON_INIT_FLAGS;
	builtinTypes[41].truncate_trailing = builtinTypes[40].truncate_trailing;
	builtinTypes[41].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[41].sync->jy2py = (jy2pySync) JySync_PyCode_From_JyCode;
	char* tp_name41 = "tablecode";
	builtinTypes[41].type_name = malloc(strlen(tp_name41)+1);
	strcpy(builtinTypes[41].type_name, tp_name41);

/*	builtinTypes[42].py_type = &PyFrame_Type;
	builtinTypes[42].jy_class = pyFrameClass;
	builtinTypes[42].flags = 0;

	builtinTypes[43].py_type = &PySuper_Type;
	builtinTypes[43].jy_class = pySuperClass;
	builtinTypes[43].flags = 0;*/

	//In JyNI no GC-type since it is fully truncated.
	builtinTypes[44].py_type = (PyTypeObject*) PyExc_BaseException;
	builtinTypes[44].jy_class = pyBaseExceptionClass;
	builtinTypes[44].flags = JY_TRUNCATE_FLAG_MASK;

	//todo: Improve PyTraceBack-implementation
	builtinTypes[45].py_type = &PyTraceBack_Type;
	builtinTypes[45].jy_class = pyTracebackClass;
	builtinTypes[45].flags = JY_TRUNCATE_FLAG_MASK;

/*	builtinTypes[46].py_type = &PyByteArray_Type;
	builtinTypes[46].jy_class = pyByteArrayClass;
	builtinTypes[46].flags = 0;

	builtinTypes[47].py_type = &PyBuffer_Type;
	builtinTypes[47].jy_class = pyBufferClass;
	builtinTypes[47].flags = 0;

	builtinTypes[48].py_type = &PyMemoryView_Type;
	builtinTypes[48].jy_class = pyMemoryViewClass;
	builtinTypes[48].flags = 0;

	builtinTypes[49].py_type = &PyBaseObject_Type;
	builtinTypes[49].jy_class = pyObjectClass;
	builtinTypes[49].flags = 0;

	builtinTypes[50].py_type = &PyCallIter_Type;
	builtinTypes[50].jy_class = pyCallIterClass;
	builtinTypes[50].flags = 0;
	*/

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
	if (&PyDictProxy_Type == type) return JNI_TRUE;
	if (&PyProperty_Type == type) return JNI_TRUE;
	if (&EncodingMapType == type) return JNI_TRUE;
	if (&wrappertype == type) return JNI_TRUE;
	if (&cmpwrapper_type == type) return JNI_TRUE;
	if (&sortwrapper_type == type) return JNI_TRUE;
	if (&Long_InfoType == type) return JNI_TRUE;
	if (&FloatInfoType == type) return JNI_TRUE;
	if (&_PyWeakref_RefType == type) return JNI_TRUE;
	if (&_PyWeakref_ProxyType == type) return JNI_TRUE;
	if (&_PyWeakref_CallableProxyType == type) return JNI_TRUE;
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

/* Does not work for Heap-Types. */
inline TypeMapEntry* JyNI_JythonTypeEntry_FromSubType(PyTypeObject* type)
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

/* Does not work for Heap-Types. */
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
	char* utf_string = (*env)->GetStringUTFChars(env, name, &isCopy);
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
 * This method returns a NEW reference. Also note that the object is not yet
 * tracked by GC.
 */
/*
inline PyObject* JyNI_GenericAlloc(PyTypeObject* type, Py_ssize_t nitems)
{
	TypeMapEntry* tme = JyNI_JythonTypeEntry_FromPyType(type);
	if (tme)
	{
		if (tme->py_type->tp_itemsize == 0) return JyNI_Alloc(tme);
		else return JyNI_AllocVar(tme, nitems);
	} else
	{
		//We don't have to take care of exception allocation at other places (hopefully)
		//because all exception types use this method for allocation.
		if (PyExceptionClass_Check(type))
		{
			ExceptionMapEntry* eme = JyNI_PyExceptionMapEntry_FromPyExceptionType(type);
			if (eme != NULL) return JyNI_ExceptionAlloc(eme);
		}
		if (tme->py_type->tp_itemsize == 0) return JyNI_AllocNative(type);
		else return JyNI_AllocNativeVar(type, nitems);
	}
}
*/

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
		//jputs("alloc gc");
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		AS_JY_WITH_GC(obj)->flags |= tme->flags;
	} else
	{
		//jputs("ordinary alloc");
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

	//In contrast to var variant of this method, no decision needed here:
	PyObject_INIT(obj, tme->py_type);

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

	// JyNI note: What if the allocated object is not var size?
	// At the end of this method happens a decision, whether to use
	// PyObject_INIT(obj, tme->py_type);
	// or
	// (void) PyObject_INIT_VAR((PyVarObject *)obj, tme->py_type, nitems);
	//
	// This shows that the original method PyType_GenericAlloc was also
	// intended for non var size objects. I suspect that nitems == 0 should
	// indicate that an object is not var size; _PyObject_VAR_SIZE seems to
	// behave like _PyObject_SIZE for nitems == 0. Unfortunately this sentinel
	// thing adds 1 to nitems, also if nitems == 0. So nitems == -1 would in
	// fact indicate a non var size object. I wonder whether this is intended
	// or whether there should be a decision like nitems == 0 ? 0 : nitems + 1
	// For now I keep it unchanged to stick to original behavior and implement
	// JyNI_Alloc for fixed size objects.

	if (PyType_IS_GC(tme->py_type))
	{
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		AS_JY_WITH_GC(obj)->flags |= tme->flags;
	} else
	{
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

	if (PyType_IS_GC(tme->py_type))
		_JyNI_GC_TRACK(obj);
	return obj;
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_AllocSubtypeVar(PyTypeObject* subtype, TypeMapEntry* tme, Py_ssize_t nitems)
{
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
	 * For now this memory is just wasted. Maybe we can return the unused section to the python memory
	 * manager one day. It would be -however- an effort then to get malloc and clean right. Especially
	 * if the surrounding blocks are freed while the middle is still in use. On the other hand it might
	 * be a rearely needed feature, so might not be worth any further optimization.
	 */

	size_t size = _PyObject_VAR_SIZE(subtype, nitems+1);
	/* note that we need to add one, for the sentinel */

	if (PyType_IS_GC(subtype))
	{
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		AS_JY_WITH_GC(obj)->flags |= tme->flags;
	} else
	{
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

	if (PyType_IS_GC(subtype))
		_JyNI_GC_TRACK(obj);
	return obj;
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * Note that the resulting object is tracked by gc, but not yet explored. Though
 * JyNI is robust against this, it is cleaner and more efficient if objects are
 * explicitly explored by gc after they were created and fully populated.
 * Use void JyNI_GC_Explore(PyObject* op) to let gc explore an object. Exploration
 * is not performed on GC_TRACK, because often the values of the object were not
 * yet populated at that time.
 */
inline PyObject* JyNI_AllocNative(PyTypeObject* type)
{
	PyObject *obj;
	size_t size = _PyObject_SIZE(type);

	if (PyType_IS_GC(type))
	{
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		JyObject* jy = AS_JY_WITH_GC(obj);
		jy->flags |= JY_CPEER_FLAG_MASK;
	} else
	{
		JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->flags = JY_CPEER_FLAG_MASK;
		jy->attr = NULL;
		jy->jy = NULL;
		obj = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, obj, jy, size+sizeof(JyObject), type->tp_name);
	}

	//if (obj == NULL) return PyErr_NoMemory();

	memset(obj, '\0', size);

	if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(type);

	PyObject_INIT(obj, type);

	if (PyType_IS_GC(type))
		_JyNI_GC_TRACK_NoExplore(obj);
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
//	jputs(__FUNCTION__);
//	jputs(type->tp_name);
//	jputsLong((jlong) size);
//	jputsLong((jlong) sizeof(PyTypeObject));
//	jputsLong((jlong) type->tp_basicsize);
//	jputsLong((jlong) type->tp_itemsize);
	//jputsLong((jlong) offsetof(PyTypeObject, tp_name));
	if (PyType_IS_GC(type))
	{
		obj = _PyObject_GC_Malloc(size);
		if (obj == NULL) return PyErr_NoMemory();
		JyObject* jy = AS_JY_WITH_GC(obj);
		jy->flags |= JY_CPEER_FLAG_MASK;
//		jputs("JyNI_AllocNativeVar GC-allocated jy:");
//		jputsLong((jlong) jy);
//		jputsLong(size+sizeof(JyObject));
	} else
	{
		JyObject* jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject));
//		jputs("JyNI_AllocNativeVar allocated jy:");
//		jputsLong((jlong) jy);
//		jputsLong(size+sizeof(JyObject));
		if (jy == NULL) return (PyObject *) PyErr_NoMemory();
		jy->flags = JY_CPEER_FLAG_MASK;
		jy->attr = NULL;
		jy->jy = NULL;
		obj = (PyObject*) FROM_JY_NO_GC(jy);
		JyNIDebug(JY_NATIVE_ALLOC, obj, jy, size+sizeof(JyObject), type->tp_name);
	}

	//if (obj == NULL) return PyErr_NoMemory();

	memset(obj, '\0', size);

	if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
		Py_INCREF(type);

	if (type->tp_itemsize == 0)
		PyObject_INIT(obj, type);
	else
		(void) PyObject_INIT_VAR((PyVarObject *)obj, type, nitems);

	if (PyType_IS_GC(type))
		_JyNI_GC_TRACK_NoExplore(obj);
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
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) obj, jy->flags & JY_TRUNCATE_FLAG_MASK);
	jy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	//}
	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	if (PyType_IS_GC(eme->exc_type))
		JyNI_GC_ExploreObject(obj);
	return obj;
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 *
 * Not intended for Heap-Types.
 * These don't have an associated TypeMapEntry anyway.
 */
inline PyObject* JyNI_InitPyObject(TypeMapEntry* tme, jobject src)
{
//	jputs("JyNI_InitPyObject");
//	jputs(tme->py_type->tp_name);
	PyObject* dest = NULL;
	if (tme->flags & SYNC_ON_JY_INIT_FLAG_MASK)
	{
		if (tme->sync != NULL && tme->sync->pyInit != NULL)
			dest = tme->sync->pyInit(src);
		//todo: Check that JySync.c always returns new ref in sync on init methods.
	} else
	{
		//dest = PyObject_GC_New(tme->py_type);
		//puts("InitPyObject by GC New");

		//dest = tme->py_type->tp_itemsize ? JyNI_AllocVar(tme) : JyNI_Alloc(tme);
		dest = JyNI_Alloc(tme);

		//printf("PyObject-size: %u\n", (jlong) sizeof(PyObject));
		//PyObject_GC_Track(dest);
		if (dest && tme->sync && tme->sync->jy2py)
			tme->sync->jy2py(src, dest);
		//else jputs("no sync needed");
	}
	if (dest)
	{
		//if dest is a PyType, mark it as heap-type
		//(PyType_Ready won't be called, since JyNI
		//truncates heap-PyType objects, so we have
		//to set the flag here)
		//if (PyType_Check(dest))
		if (PyType_CheckExact(dest))
		{
			//puts("created heap type");
			//puts(((PyTypeObject*) dest)->tp_name);
			//PyType_HasFeature(dest->ob_type, Py_TPFLAGS_HEAPTYPE);
			((PyTypeObject*) dest)->tp_flags = Py_TPFLAGS_HEAPTYPE;
		}

		//printf("dest at %u\n", (jlong) dest);
		JyObject* jy = AS_JY(dest);
//		printf("jy at %u\n", (jlong) jy);
//		printf("JyObject-size: %u\n", (jlong) sizeof(JyObject));
//		printf("GC_Head-size: %u\n", (jlong) sizeof(PyGC_Head));
//		puts("try access jy:");
//		printf("jy-Flags: %u\n", (int) jy->flags);
//		puts("try access dest:");
//		printf("dest-type: %u\n", (int) dest->ob_type);
//		puts("type-name:");
//		puts(dest->ob_type->tp_name);
//		printf("dest-check for module: %u\n", (int) PyModule_Check(dest));
		//PyBaseObject_Type
		if (jy->flags & SYNC_NEEDED_MASK)
			JyNI_AddJyAttribute(jy, JyAttributeSyncFunctions, tme->sync);//, char flags)
		env(NULL);
		jy->jy = (*env)->NewWeakGlobalRef(env, src);
		if (!(jy->flags & JY_HAS_JHANDLE_FLAG_MASK)) { //some sync-on-init methods might already init this
			(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) dest, jy->flags & JY_TRUNCATE_FLAG_MASK);
			jy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
		}

		/* Take care for already existing Jython-weak references */
		jobject gref = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_getGlobalRef, src);
		if (gref && (*env)->IsInstanceOf(env, gref, GlobalRefClass)) {
			gref = (*env)->CallObjectMethod(env, gref, GlobalRef_retryFactory);
		}
		if (gref) {
			//jputs("native call to JyNIGlobalRef_initNativeHandle");
			//jputsLong((jlong) dest);
			Py_INCREF(dest);
			incWeakRefCount(jy);
			(*env)->CallVoidMethod(env, gref, JyNIGlobalRef_initNativeHandle, (jlong) dest);
		}

		jy->flags |= JY_INITIALIZED_FLAG_MASK;
		if (PyObject_IS_GC(dest)) {
//			if (jy->flags & JY_INITIALIZED_FLAG_MASK) {
//				jputs("start explore initialized");
//				jputsLong(dest);
//				jputsLong(jy);
//			} else {
//				jputs("start explore uninitialized");
//				jputsLong(dest);
//				jputsLong(jy);
//			}
			JyNI_GC_ExploreObject(dest);
		}
		//printf("dest-check for module2: %u\n", (int) PyModule_Check(dest));
	}
	//printf("dest-check for module3: %u\n", (int) PyModule_Check(dest));
	return dest;
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* _JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject, jboolean lookupNative, jboolean checkCPeer, jboolean checkForType)
{
	if (jythonPyObject == NULL) return NULL;
	//puts("Transform jython jobject to PyObject*...");
//	if (jythonPyObject == JyNone) return Py_None;
//	if (jythonPyObject == JyNotImplemented) return Py_NotImplemented;
//	if (jythonPyObject == JyEllipsis) return Py_Ellipsis;
	env(NULL);
	if ((*env)->IsSameObject(env, jythonPyObject, NULL)) return NULL;
	/* In principal, the caller is responsible to decide whether or not
	 * the conversion-result should be INCREFed. However, in singleton cases
	 * missing INCREFs do more harm than duplicate INCREFs, so for now, we
	 * INCREF singletons here.
	 */
	if ((*env)->IsSameObject(env, jythonPyObject, JyNone)) Py_RETURN_NONE;
	if ((*env)->IsSameObject(env, jythonPyObject, JyNotImplemented))
	{
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	//Todo: Maybe the check for nullstring and unicode_empty is not needed
	//here and should be processed via usual lookup. (Since these singletons
	//are on the heap)
	if ((*env)->IsSameObject(env, jythonPyObject, JyEmptyString) && nullstring)
	{
		Py_INCREF(nullstring);
		return nullstring;
	}
	if ((*env)->IsSameObject(env, jythonPyObject, JyEmptyUnicode) && unicode_empty)
	{
		Py_INCREF(unicode_empty);
		return unicode_empty;
	}
	if ((*env)->IsSameObject(env, jythonPyObject, JyEllipsis))
	{
		Py_INCREF(Py_Ellipsis);
		return Py_Ellipsis;
	}
	if (checkForType && (*env)->IsInstanceOf(env, jythonPyObject, pyTypeClass))
	{
		//No increfs here, since JyNI_PyTypeObject_FromJythonPyTypeObject returns NEW ref if any.
		PyObject* er = (PyObject*) JyNI_PyTypeObject_FromJythonPyTypeObject(jythonPyObject);
		if (er) return er;
		//No increfs here, since JyNI_PyExceptionType_FromJythonExceptionType returns NEW ref if any.
		er = (PyObject*) JyNI_PyExceptionType_FromJythonExceptionType(jythonPyObject);
		if (er) return er;
		//heap-type case: Proceed same way like for ordinary PyObjects.
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
	//jputs("no handle exists yet");
	//initialize PyObject*...
	//find tme:
//	jstring tpName = (*env)->CallObjectMethod(env,
//			(*env)->CallObjectMethod(env, jythonPyObject, pyObjectGetType),
//			pyTypeGetName);
	jstring tpName = (*env)->CallStaticObjectMethod(env, JyNIClass,
			JyNI_getTypeNameForNativeConversion, jythonPyObject);
	//todo find out what name occurs if a Jython Java-proxy is used.
	//     Can we make sense of it in default-instance case?
	//jputs("tp name obtained:");
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
		//jputsLong(__LINE__);
		//jputs("initialize handle:");
		//if (tme->type_name) jputs(tme->type_name);
		//if (tme->py_type) jputs(tme->py_type->tp_name);

		//No need to incref here, since JyNI_InitPyObject returns NEW ref.
		PyObject* er = JyNI_InitPyObject(tme, jythonPyObject);

		//jputs("handle initialized");
		return er;
	} else
	{
		//jputs("tme is NULL...");
		ExceptionMapEntry* eme = JyNI_PyExceptionMapEntry_FromPyExceptionType(
			JyNI_PyExceptionType_FromJythonExceptionType(
			(*env)->CallObjectMethod(env, jythonPyObject, pyObjectGetType)));
		//jputs("exception type...");
		if (eme)
		{
			//jputs("exception type2...");
			//No need to incref here, since JyNI_InitPyException returns NEW ref.
			PyObject* er = JyNI_InitPyException(eme, jythonPyObject);
			return er;
		} else {
			// We finally try a hack for some special new-style classes:
			jobject old_cls = (*env)->CallStaticObjectMethod(env, JyNIClass,
					JyNI_getTypeOldStyleParent, jythonPyObject);
			if (!(*env)->IsSameObject(env, old_cls, NULL)) {
				return JyNI_InitPyObject(&specialPyInstance, jythonPyObject);
			}
			//Todo: Add case for new-style classes or Jython-defined types.
			//jputs("returning NULL...");
			//JyNI_jprintJ(jythonPyObject);
			//JyNI_printJInfo(jythonPyObject);
			return NULL;
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
	SyncFunctions* sync = (SyncFunctions*) JyNI_GetJyAttribute(jy, JyAttributeSyncFunctions);
	if (sync != NULL && sync->py2jy != NULL) sync->py2jy(op, jy->jy);
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
		jobject type = JyNI_JythonExceptionType_FromPyExceptionType(Py_TYPE(src));
		if (type)
			dest = (*env)->NewObject(env, pyBaseExceptionClass, pyBaseExceptionSubTypeConstructor, type);
		else
			dest = (*env)->NewObject(env, pyBaseExceptionClass, pyBaseExceptionEmptyConstructor);
	}
	if (!dest) return NULL;

	srcJy->jy = (*env)->NewWeakGlobalRef(env, dest);
	if (!(srcJy->flags & JY_HAS_JHANDLE_FLAG_MASK)) {  //some exc_factories might already init this
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, dest, (jlong) src, srcJy->flags & JY_TRUNCATE_FLAG_MASK);
		srcJy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	}
	srcJy->flags |= JY_INITIALIZED_FLAG_MASK;
	return srcJy->jy;
}

/*
 * Returns a JNI LocalRef.
 */
inline jobject JyNI_InitJythonPyObject(TypeMapEntry* tme, PyObject* src, JyObject* srcJy)
{
	jobject dest = NULL;
	//jputs(__FUNCTION__);
	//jputsLong(__LINE__);
	env(NULL);
	if (Py_TYPE(src) != tme->py_type) {
		if (!tme->jy_subclass || !PyType_IsSubtype(Py_TYPE(src), tme->py_type))
		{
			jputs("JyNI_InitJythonPyObject called with inconsistent args!");
			jputs("Subtype:");
			jputs(Py_TYPE(src)->tp_name);
			jputs("Mapping-type:");
			jputsLong(tme);
			jputsLong(tme->py_type);
			jputs(tme->py_type->tp_name);
			PyErr_BadInternalCall();
		}
		jobject jsrcType = JyNI_JythonPyTypeObject_FromPyTypeObject(Py_TYPE(src));
		jmethodID subconst = (*env)->GetMethodID(env, tme->jy_subclass, "<init>",
				"(JLJyNI/PyCPeerType;)V");
		dest = (*env)->NewObject(env, tme->jy_subclass, subconst, (jlong) src, jsrcType);
		srcJy->flags |= JY_CPEER_FLAG_MASK; //maybe introduce a distinct flag for subtype case
	} else if (tme->flags & SYNC_ON_JY_INIT_FLAG_MASK)
	{
		//jputsLong(__LINE__);
		if (tme->sync && tme->sync->jyInit)
			dest = tme->sync->jyInit(src);
	} else
	{
		//jputsLong(__LINE__);
		jmethodID cm = (*env)->GetMethodID(env, tme->jy_class, "<init>", "()V");
		if (cm)
		{
			dest = (*env)->NewObject(env, tme->jy_class, cm);
			if (tme->sync && tme->sync->py2jy)
				tme->sync->py2jy(src, dest);
		}
	}
	//jputsLong(__LINE__);
	if (!dest) return NULL;
	if (dest && (srcJy->flags & SYNC_NEEDED_MASK))
		JyNI_AddJyAttribute(srcJy, JyAttributeSyncFunctions, tme->sync);
	srcJy->jy = (*env)->NewWeakGlobalRef(env, dest);
	if (!(srcJy->flags & JY_HAS_JHANDLE_FLAG_MASK)) {  //some sync-on-init methods might already init this
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, dest,
				(jlong) src, srcJy->flags & JY_TRUNCATE_FLAG_MASK);
		srcJy->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	}
	srcJy->flags |= JY_INITIALIZED_FLAG_MASK;
	return dest;
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
	/* nullstring might be uninitialized, which is no matter here.
	 * If it was uninitialized, the usual string conversion code
	 * will initialize it and return it in the end. Uninitialized
	 * nullstring cannot trigger "return JyEmptyString" here, since
	 * it would already have triggered "return NULL" some lines above.
	 */
	if (op == nullstring) return JyEmptyString;
	if (op == unicode_empty) return JyEmptyUnicode;
	//jputsLong(__LINE__);
	if (op->ob_type == NULL)
	{
		//jputs("type of op is NULL");
		//we assume that only type-objects (and as such also exception types) can have ob_type == NULL.
		//So we call PyType_Ready to init it. However this might fail brutally, if ob_type was
		//NULL for some other reason. However this would not go far without segfault then anyway.
		PyType_Ready(op); //this is the wrong place to do this... it's just a quick hack. Find better solution soon...
	}
	//jputsLong(__LINE__);
//	jputs("convert:");
//	if (!op->ob_type) jputs("type is NULL");
//	if (!op->ob_type->tp_name) jputs("type name is NULL");
//	jputs(op->ob_type->tp_name);
	//The following block cares for statically defined type objects.
	//Heap-types are treated like ordinary objects.
	if (PyType_Check(op) && !PyType_HasFeature(op->ob_type, Py_TPFLAGS_HEAPTYPE))
	{
		if (PyExceptionClass_Check(op))
		{
			//jputs("convert exception...");
			jobject er = JyNI_JythonExceptionType_FromPyExceptionType(op);
			if (er != NULL) return er;
		} else
		{
			//jputs("convert type...");
			jobject er = JyNI_JythonPyTypeObject_FromPyTypeObject((PyTypeObject*) op);
			if (er != NULL) return er;
		}
	}
	//jputsLong(__LINE__);
	//if (PyType_Check(op)) puts("appears to be a HeapType");
	JyObject* jy = AS_JY(op);
	//jputsLong(__LINE__);
	//if (JyNI_IsJyObject(op))
	if (JyObject_IS_INITIALIZED(jy))
	{
		//jputsLong(__LINE__);
		if (jy->flags & JY_CACHE_ETERNAL_FLAG_MASK) {
			//jputsLong(__LINE__);
			if (jy->flags & SYNC_ON_PY_TO_JY_FLAG_MASK)
				JyNI_SyncPy2Jy(op, jy);
			return jy->jy;
		} else {
			//jputsLong(__LINE__);
			env(NULL);
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
				//jputsLong(__LINE__);
				if (jy->flags & SYNC_ON_PY_TO_JY_FLAG_MASK)
					JyNI_SyncPy2Jy(op, jy);
				return jy->jy;
			} else {
				/* This is actually okay. It can happen if the Java-counterpart
				 * of a mirrored object was collected by Java-gc.
				 */
				//jputs("JyNI-Warning: Deleted object was not cleared!");
				JyNI_CleanUp_JyObject(jy);
			}
			//jputsLong(__LINE__);
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
	} //else
	//{
	//jputsLong(__LINE__);
	TypeMapEntry* tme;
	if (jy->jy != NULL)
	{
		//jputsLong(__LINE__);
		tme = (TypeMapEntry*) jy->jy;
	} else {
		//jputsLong(__LINE__);
		tme = JyNI_JythonTypeEntry_FromPyType(Py_TYPE(op));
		if (!tme) tme = JyNI_JythonTypeEntry_FromSubType(Py_TYPE(op));
	}
	//jputsLong(tme);
	//tme = JyNI_JythonTypeEntry_FromPyType(Py_TYPE(op));
	if (tme)// != NULL
	{
		//jputsLong(__LINE__);
		//jputs(Py_TYPE(op)->tp_name);
		//if (!tme->py_type) jputs("py_type is NULL");
		//if (!tme->py_type->tp_name) jputs("py_type name is NULL");
		//jputs(tme->py_type->tp_name);
		return JyNI_InitJythonPyObject(tme, op, jy);
	}
	else
	{
		//jputsLong(__LINE__);
		ExceptionMapEntry* eme = JyNI_PyExceptionMapEntry_FromPyExceptionType(Py_TYPE(op));
		if (eme)
			return JyNI_InitJythonPyException(eme, op, jy);
		else
		{
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
			jobject opType = JyNI_JythonPyTypeObject_FromPyTypeObject(Py_TYPE(op));
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
			jobject er = PyObject_IS_GC(op) ?
					(*env)->NewObject(env, pyCPeerGCClass, pyCPeerGCConstructor, (jlong) op, opType):
					(*env)->NewObject(env, pyCPeerClass, pyCPeerConstructor, (jlong) op, opType);
			//jobject er = (*env)->NewObject(env, pyCPeerClass, pyCPeerConstructor, (jlong) op, opType);
			jy->flags |= JY_INITIALIZED_FLAG_MASK;
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
	env(NULL);
	if (cls != NULL)
	{
		return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNIGetPyType, cls);
	} else {
		//setup and return PyCPeerType in this case...
		jobject er = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupCPeerFromHandle, (jlong) type);
		if (er != NULL) return er;
		//No JyObjects available for types!
		//JyObject* jy = AS_JY_NO_GC(type); //since we currently don't init types properly, GC-check would not work with PyTypeObjects
		//if (JyObject_IS_INITIALIZED(jy))
		//	return jy->jy; //no sync-attempt for types
		else {
//			jputs(__FUNCTION__);
//			jputs(type->tp_name);
//			jputsLong(type);
			Py_INCREF(type);
			//jobject er = (*env)->NewObject(env, pyCPeerTypeClass, pyCPeerTypeConstructor, (jlong) type);
			jobject er = (*env)->NewObject(env, pyCPeerTypeClass, pyCPeerTypeWithNameAndDictConstructor,
				(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
				JyNI_JythonPyObject_FromPyObject(type->tp_dict));
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
 * Only builtin types or native declared types work.
 */
inline PyTypeObject* JyNI_PyTypeObject_FromJythonPyTypeObject(jobject jythonPyTypeObject)
{
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
 */
//Only delegate if truncated:
#define JY_DELEGATE(v, flags) (flags & JY_TRUNCATE_FLAG_MASK)

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
		//jputsLong(jy->flags);
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

//Like PyErr_SetString but without detour for exception.
//inline void JyNI_JyErr_SetString(jobject exception, const char *string)
//{
//	env();
//	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNIPyErr_SetString, exception, (*env)->NewStringUTF(env, string));
//}
//
//inline void JyNI_JyErr_SetObject(jobject exception, PyObject *value)
//{
//	//Py_XINCREF(exception);
//	//Py_XINCREF(value);
//	//PyErr_Restore(exception, value, (PyObject *)NULL);
//	env();
//	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNIPyErr_SetObject, exception, JyNI_JythonPyObject_FromPyObject(value));
//}
//
//inline void JyNI_JyErr_SetExceptionObject(jobject exception)
//{
//	//Py_XINCREF(exception);
//	//Py_XINCREF(value);
//	//PyErr_Restore(exception, value, (PyObject *)NULL);
//	env();
//	jobject tstate = (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyGetThreadState);
//	(*env)->SetObjectField(env, tstate, pyThreadStateExceptionField, exception);
//}
//
//inline int JyNI_JyErr_ExceptionMatches(jobject exc)
//{
//	env(0);
//	return (*env)->CallStaticBooleanMethod(env, JyNIClass, JyNIPyErr_ExceptionMatches, exc);
//
//    //return PyErr_GivenExceptionMatches(PyErr_Occurred(), exc);
//}
//
//inline PyObject* JyNI_JyErr_Format(jobject exception, const char *format, ...)
//{
//	va_list vargs;
//	PyObject* string;
//
//#ifdef HAVE_STDARG_PROTOTYPES
//	va_start(vargs, format);
//#else
//	va_start(vargs);
//#endif
//
//	string = PyString_FromFormatV(format, vargs);
//	JyNI_JyErr_SetObject(exception, string);
//	Py_XDECREF(string);
//	va_end(vargs);
//	return NULL;
//}

inline void JyNI_CleanUp_JyObject(JyObject* obj)
{
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

//Now PyObject_Free does this job.
//void JyNI_Del(void * obj)
//{
//	//assume no gc, maybe change this later:
//	JyObject* jy = AS_JY_NO_GC(obj);
//	JyNI_CleanUp_JyObject(jy);
//	PyObject_RawFree(jy);
//}

//inline void JyNI_INCREF(jobject obj)
//{
//	env();
//	PyObject* handle = (PyObject*) (*env)->CallStaticLongMethod(env, JyNIClass, JyNILookupNativeHandle, obj);
//	if (handle == NULL) return;
//	Py_INCREF(handle);
//}
//
//inline void JyNI_DECREF(jobject obj)
//{
//	env();
//	PyObject* handle = (PyObject*) (*env)->CallStaticLongMethod(env, JyNIClass, JyNILookupNativeHandle, obj);
//	if (handle == NULL) return;
//	Py_DECREF(handle);
//}
//
//inline void JyNI_Py_CLEAR(jobject obj)
//{
//	env();
//	PyObject* handle = (PyObject*) (*env)->CallStaticLongMethod(env, JyNIClass, JyNILookupNativeHandle, obj);
//	if (handle == NULL) return;
//	Py_CLEAR(handle);
//}

//inline char* PyLongToJavaSideString(PyObject* pl)
//{
//	if (pl == NULL) return "plNULL";
//	JyObject* jy = AS_JY_NO_GC(pl);
//	if (jy->jy == NULL) return ("jyNULL");
//	env(NULL);
//	jstring er = (*env)->CallObjectMethod(env, jy->jy, pyObjectAsString);
//	//cstr_from_jstring(er2, er);
//	return "hier2";
//}

//singletons:
JavaVM* java;
jweak JyNone;
jweak JyNotImplemented;
jweak JyEllipsis;
jweak JyEmptyFrozenSet;
jweak JyEmptyString;
jweak JyEmptyUnicode;
jweak JyEmptyTuple;
//PyUnicodeObject* unicode_empty;
PyObject* PyTrue;
PyObject* PyFalse;
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
jmethodID JySetInstallToPySet;

jclass JyLockClass;
jmethodID JyLockConstructor;
jmethodID JyLockAcquire;
jmethodID JyLockRelease;

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

// Subclasses:
jclass pyDictCPeerClass;

jclass jyGCHeadClass;
jmethodID traversableGCHeadSetLinks;
jmethodID traversableGCHeadSetLink;
jmethodID traversableGCHeadInsertLink;
jmethodID traversableGCHeadClearLink;
jmethodID traversableGCHeadClearLinksFromIndex;
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
jmethodID pyCPeerTypeConstructor;
jmethodID pyCPeerTypeWithNameAndDictConstructor;
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
jmethodID pyBooleanConstructor;

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
//jmethodID pyTypeGetMro;
jmethodID pyTypeIsSubType;
jfieldID pyTypeMROField;

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
jclass pyStaticMethodClass;
jclass pyMethodDescrClass;
jclass pyClassMethodDescrClass;
jclass pyDictProxyClass;
jclass pyPropertyClass;
jclass pyBaseStringClass;
jclass pyXRangeClass;
jclass pySequenceIterClass;
jclass pyFastSequenceIterClass;
jclass pyReversedIteratorClass;

jclass pyBaseSetClass;
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
jmethodID __builtin__Import;
jmethodID __builtin__ImportLevel;

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
	//puts("   JyNISetNextResultClass created");
	JyNISetNextResultKeyField = (*env)->GetFieldID(env, JyNISetNextResultClass, "key", "Lorg/python/core/PyObject;");
	//puts("   key done");
	JyNISetNextResultNewIndexField = (*env)->GetFieldID(env, JyNISetNextResultClass, "newIndex", "I");
	//puts("   newIndex done");
	JyNISetNextResultKeyHandleField = (*env)->GetFieldID(env, JyNISetNextResultClass, "keyHandle", "J");
	//puts("   keyHandle done");
	//puts("  initJyNISet done");

	jclass JyListClassLocal = (*env)->FindClass(env, "JyNI/JyList");
	JyListClass = (jclass) (*env)->NewWeakGlobalRef(env, JyListClassLocal);
	(*env)->DeleteLocalRef(env, JyListClassLocal);
	JyListFromBackendHandleConstructor = (*env)->GetMethodID(env, JyListClass, "<init>", "(J)V");
	//JyListInstallToPyList = (*env)->GetMethodID(env, JyListClass, "installToPyList", "(Lorg/python/core/PyList;)V");
	//puts("  initJyList done");

	jclass JySetClassLocal = (*env)->FindClass(env, "JyNI/JySet");
	JySetClass = (jclass) (*env)->NewWeakGlobalRef(env, JySetClassLocal);
	(*env)->DeleteLocalRef(env, JySetClassLocal);
	JySetFromBackendHandleConstructor = (*env)->GetMethodID(env, JySetClass, "<init>", "(J)V");
	JySetInstallToPySet = (*env)->GetMethodID(env, JySetClass, "installToPySet", "(Lorg/python/core/BaseSet;)V");
	//puts("  initJyNISet done");

	jclass JyLockClassLocal = (*env)->FindClass(env, "JyNI/JyLock");
	JyLockClass = (jclass) (*env)->NewWeakGlobalRef(env, JyLockClassLocal);
	(*env)->DeleteLocalRef(env, JySetClassLocal);
	JyLockConstructor = (*env)->GetMethodID(env, JyLockClass, "<init>", "()V");
	JyLockAcquire = (*env)->GetMethodID(env, JyLockClass, "acquire", "(Z)Z");
	JyLockRelease = (*env)->GetMethodID(env, JyLockClass, "release", "()V");

	jclass JyReferenceMonitorClassLocal = (*env)->FindClass(env, "JyNI/JyReferenceMonitor");
	JyReferenceMonitorClass = (jclass) (*env)->NewWeakGlobalRef(env, JyReferenceMonitorClassLocal);
	(*env)->DeleteLocalRef(env, JyReferenceMonitorClassLocal);
	JyRefMonitorMakeDebugInfo = (*env)->GetStaticMethodID(env, JyReferenceMonitorClass, "makeDebugInfo",
			"(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)LJyNI/JyReferenceMonitor$ObjectLogDebugInfo;");
	JyRefMonitorAddAction = (*env)->GetStaticMethodID(env, JyReferenceMonitorClass, "addNativeAction",
			//"(SLorg/python/core/PyObject;JJLjava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)V");
			"(SLorg/python/core/PyObject;JJLJyNI/JyReferenceMonitor$ObjectLogDebugInfo;)V");

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

	jclass pyDictCPeerClassLocal = (*env)->FindClass(env, "JyNI/PyDictionaryCPeer");
	pyDictCPeerClass = (jclass) (*env)->NewWeakGlobalRef(env, pyDictCPeerClassLocal);
	(*env)->DeleteLocalRef(env, pyDictCPeerClassLocal);

	jclass pyCPeerTypeClassLocal = (*env)->FindClass(env, "JyNI/PyCPeerType");
	pyCPeerTypeClass = (jclass) (*env)->NewWeakGlobalRef(env, pyCPeerTypeClassLocal);
	(*env)->DeleteLocalRef(env, pyCPeerTypeClassLocal);
	pyCPeerTypeConstructor = (*env)->GetMethodID(env, pyCPeerTypeClass, "<init>", "(J)V");
	pyCPeerTypeWithNameAndDictConstructor = (*env)->GetMethodID(env, pyCPeerTypeClass, "<init>",
			"(JLjava/lang/String;Lorg/python/core/PyObject;)V");
	pyCPeerTypeObjectHandle = (*env)->GetFieldID(env, pyCPeerTypeClass, "objectHandle", "J");
	pyCPeerTypeRefHandle = (*env)->GetFieldID(env, pyCPeerTypeClass, "refHandle", "J");

	jclass traversableGCHeadInterface = (*env)->FindClass(env, "JyNI/gc/TraversableGCHead");
	traversableGCHeadSetLinks = (*env)->GetMethodID(env, traversableGCHeadInterface, "setLinks", "(Ljava/lang/Object;)V");
	traversableGCHeadSetLink = (*env)->GetMethodID(env, traversableGCHeadInterface, "setLink", "(ILJyNI/gc/JyGCHead;)I");
	traversableGCHeadInsertLink = (*env)->GetMethodID(env, traversableGCHeadInterface, "insertLink", "(ILJyNI/gc/JyGCHead;)I");
	traversableGCHeadClearLink = (*env)->GetMethodID(env, traversableGCHeadInterface, "clearLink", "(I)I");
	traversableGCHeadClearLinksFromIndex = (*env)->GetMethodID(env, traversableGCHeadInterface, "clearLinksFromIndex", "(I)I");
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
	pyBooleanConstructor = (*env)->GetMethodID(env, pyBooleanClass, "<init>", "(Z)V");

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
	//pyTypeGetMro = (*env)->GetMethodID(env, pyTypeClass, "getMro", "()Lorg/python/core/PyTuple;");
	pyTypeIsSubType = (*env)->GetMethodID(env, pyTypeClass, "isSubType", "(Lorg/python/core/PyType;)Z");
	pyTypeMROField = (*env)->GetFieldID(env, pyTypeClass, "mro", "[Lorg/python/core/PyObject;");

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

	jclass pyStaticMethodClassLocal = (*env)->FindClass(env, "org/python/core/PyStaticMethod");
	if (pyStaticMethodClassLocal == NULL) { return JNI_ERR;}
	pyStaticMethodClass = (jclass) (*env)->NewWeakGlobalRef(env, pyStaticMethodClassLocal);
	(*env)->DeleteLocalRef(env, pyStaticMethodClassLocal);

	jclass pyMethodDescrClassLocal = (*env)->FindClass(env, "org/python/core/PyMethodDescr");
	if (pyMethodDescrClassLocal == NULL) { return JNI_ERR;}
	pyMethodDescrClass = (jclass) (*env)->NewWeakGlobalRef(env, pyMethodDescrClassLocal);
	(*env)->DeleteLocalRef(env, pyMethodDescrClassLocal);

	jclass pyClassMethodDescrClassLocal = (*env)->FindClass(env, "org/python/core/PyClassMethodDescr");
	if (pyClassMethodDescrClassLocal == NULL) { return JNI_ERR;}
	pyClassMethodDescrClass = (jclass) (*env)->NewWeakGlobalRef(env, pyClassMethodDescrClassLocal);
	(*env)->DeleteLocalRef(env, pyClassMethodDescrClassLocal);

	jclass pyDictProxyClassLocal = (*env)->FindClass(env, "org/python/core/PyDictProxy");
	if (pyDictProxyClassLocal == NULL) { return JNI_ERR;}
	pyDictProxyClass = (jclass) (*env)->NewWeakGlobalRef(env, pyDictProxyClassLocal);
	(*env)->DeleteLocalRef(env, pyDictProxyClassLocal);

	jclass pyPropertyClassLocal = (*env)->FindClass(env, "org/python/core/PyProperty");
	if (pyPropertyClassLocal == NULL) { return JNI_ERR;}
	pyPropertyClass = (jclass) (*env)->NewWeakGlobalRef(env, pyPropertyClassLocal);
	(*env)->DeleteLocalRef(env, pyPropertyClassLocal);

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
	__builtin__Import = (*env)->GetStaticMethodID(env, __builtin__Class, "__import__", "(Ljava/lang/String;)Lorg/python/core/PyObject;");
	__builtin__ImportLevel = (*env)->GetStaticMethodID(env, __builtin__Class, "__import__", "(Ljava/lang/String;Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyObject;I)Lorg/python/core/PyObject;");

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
	//PyTrue = JyNI_PyObject_FromJythonPyObject((*env)->NewWeakGlobalRef(env, (*env)->NewObject(env, pyBooleanClass, pyBooleanConstructor, JNI_TRUE)));
	//PyFalse = JyNI_PyObject_FromJythonPyObject((*env)->NewWeakGlobalRef(env, (*env)->NewObject(env, pyBooleanClass, pyBooleanConstructor, JNI_FALSE)));

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


//	length0StringArray = (*env)->NewGlobalRef(env, (*env)->NewObjectArray(env, 0, stringClass, NULL));
//	length0PyObjectArray = (*env)->NewGlobalRef(env, (*env)->NewObjectArray(env, 0, pyObjectClass, NULL));

//	jfieldID jyMemoryErrorInst = (*env)->GetStaticFieldID(env, pyPyClass, );
//	PyExc_MemoryErrorInst = JyNI_PyObject_FromJythonPyObject();
	return JNI_VERSION_1_2;
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

	//puts("characters-info:");
	//characters[UCHAR_MAX + 1]
	//characters2[0] = NULL;
	//printf("%u\n", characters2[0]);
	//dlopen("/home/stefan/eclipseWorkspace/JyNI/JyNI-C/Debug/libJyNI.so", RTLD_NOLOAD | RTLD_GLOBAL);
	//initErrorsFromJython(env);

	//init native objects where necessary:
	PyEval_InitThreads();
	_PyInt_Init();

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

	free(specialPyInstance.sync);

	env();
	(*env)->DeleteWeakGlobalRef(env, JyNone);
	(*env)->DeleteWeakGlobalRef(env, JyNotImplemented);
	(*env)->DeleteWeakGlobalRef(env, JyEllipsis);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyFrozenSet);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyString);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyUnicode);

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

inline void jputs(const char* msg)
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
