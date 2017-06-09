/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
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
#include "frameobject_JyNI.h"
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

/*
 * Class:     JyNI_JyNI
 * Method:    loadModule
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)Lorg/python/core/PyModule;
 */
jobject JyNI_loadModule(JNIEnv *env, jclass class, jstring moduleName, jstring modulePath, jlong tstate)
{
//	jputs("JyNI_loadModule...");
//	JyNI_jprintJ(moduleName);
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
//	puts(__FUNCTION__);
	//note: here should be done sync
	//(maybe sync-idea is obsolete anyway)
	PyObject* peer = (PyObject*) peerHandle;
//	jputsLong(peerHandle);
	//jputs(Py_TYPE(peer)->tp_name);
//	jputs(((PyTypeObject*) peer)->tp_name);
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
//		if (Py_TYPE(peer) == &PyCFunction_Type) {
//			jputsLong(((PyCFunctionObject*) peer)->m_ml);
//			jputsLong(((PyCFunctionObject*) peer)->m_self);
////			jputs(((PyCFunctionObject*) peer)->m_ml->ml_name);
//		}
		PyObject* jkw = JyNI_PyObject_FromJythonPyObject(kw);
//		jputs("before call");
//		jputsPy(peer);
		PyObject* jres = peer->ob_type->tp_call(peer, jargs, jkw);
		er = JyNI_JythonPyObject_FromPyObject(jres);
		Py_XDECREF(jargs);
		Py_XDECREF(jkw);
//		puts("decref result....");
//		printf("%lld\n", jres);
		Py_XDECREF(jres);
	} else {
//		PyErr_Format(PyExc_TypeError, "'%.200s' object is not callable",
//				peer->ob_type->tp_name);
		er = NULL;
	}
	LEAVE_JyNI
//	jputs("JyNI_callPyCPeer done");
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
//	jboolean jdbg = strcmp(cName, "__array_interface__") == 0;
//	if (jdbg) jputs(__FUNCTION__);
//	if (jdbg) jputs(Py_TYPE((PyObject*) handle)->tp_name);
	RE_ENTER_JyNI
	//jint ensresult = (*env)->EnsureLocalCapacity(env, 100);
	//jputs("ensresult:");
	//jputsLong(ensresult);
	//jputs("JyNI_getAttrString-Handle:");
	//jputsLong(handle);
	PyObject* jres = PyObject_GetAttrString((PyObject*) handle, cName);
//	jputs("JyNI_getAttrString-Result:");
	//jputsLong(jres);
	jobject er = JyNI_JythonPyObject_FromPyObject(jres);//PyObject_GetAttrString((PyObject*) handle, cName));
	Py_XDECREF(jres);
	RE_LEAVE_JyNI
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
//	jputs("JyNI_PyObjectAsString");
	if (!((PyObject*) handle)->ob_refcnt) return NULL;
	RE_ENTER_JyNI
	PyStringObject* ps = (PyStringObject*) PyObject_Str((PyObject*) handle);
//	jputsPy(ps);
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
//	jputs("JyNI_PyObjectAsPyString");
	ENTER_JyNI
	PyObject* res = PyObject_Str((PyObject*) handle);
	jobject er = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(res);
	LEAVE_JyNI
//	jputs("JyNI_PyObjectAsPyString done");
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

/*
 * Class:     JyNI_JyNI
 * Method:    PyCFunction_getSelf
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
jobject JyNI_PyCFunction_getSelf(jlong handle, jlong tstate)
{
//	jputs(__FUNCTION__);
	env(NULL);
	RE_ENTER_JyNI
	jobject jres = JyNI_JythonPyObject_FromPyObject(
			((PyCFunctionObject*) handle)->m_self);
	RE_LEAVE_JyNI
	return jres;
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyCFunction_getModule
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
jobject JyNI_PyCFunction_getModule(jlong handle, jlong tstate)
{
	RE_ENTER_JyNI
	jobject jres = JyNI_JythonPyObject_FromPyObject(
			((PyCFunctionObject*) handle)->m_module);
	RE_LEAVE_JyNI
	return jres;
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_CMethodDef_bind
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
jobject JyNI_CMethodDef_bind(jlong handle, jobject bindTo, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* self = JyNI_PyObject_FromJythonPyObject(bindTo);
	PyObject* res = PyCFunction_New((PyMethodDef*) handle, self);
	jobject jres = JyNI_JythonPyObject_FromPyObject(res);
	Py_DECREF(res);
	RE_LEAVE_JyNI
	return jres;
}

//PySys_GetObject creates new object
/*Builtin types not relevant for Jython (?):
EncodingMapType,
wrappertype,
cmpwrapper_type,
sortwrapper_type,
Long_InfoType,
FloatInfoType,
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

	builtinTypes[TME_INDEX_CFunction].py_type = &PyCFunction_Type;
	builtinTypes[TME_INDEX_CFunction].jy_class = pyBuiltinCallableClass;//pyCFunctionClass;
	builtinTypes[TME_INDEX_CFunction].flags = JySYNC_ON_INIT_FLAGS;// | JY_CPEER_FLAG_MASK;
	builtinTypes[TME_INDEX_CFunction].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_CFunction].sync->jyInit = (jyInitSync) JySync_Init_JyCFunction_From_PyCFunction;
	builtinTypes[TME_INDEX_CFunction].sync->pyInit = (pyInitSync) JySync_Init_PyCFunction_From_JyBuiltinCallable;
	PyStaticMethod_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

	builtinTypes[TME_INDEX_MethodDescr].py_type = &PyMethodDescr_Type;
	builtinTypes[TME_INDEX_MethodDescr].jy_class = pyMethodDescrClass;
	builtinTypes[TME_INDEX_MethodDescr].flags = JySYNC_ON_INIT_FLAGS;
	builtinTypes[TME_INDEX_MethodDescr].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_MethodDescr].sync->jyInit = (jyInitSync) JySync_Init_JyMethodDescr_From_PyMethodDescr;
	builtinTypes[TME_INDEX_MethodDescr].sync->pyInit = (pyInitSync) JySync_Init_PyMethodDescr_From_JyMethodDescr;
	PyMethodDescr_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_ClassMethodDescr].py_type = &PyClassMethodDescr_Type;
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
	PyBaseStringType.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	builtinTypes[TME_INDEX_SeqIter].py_type = &PySeqIter_Type;
	builtinTypes[TME_INDEX_SeqIter].jy_class = pySequenceIterClass;
	builtinTypes[TME_INDEX_SeqIter].flags = JY_TRUNCATE_FLAG_MASK;
	PySeqIter_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_Range].py_type = &PyRange_Type;
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
	// Todo: What about Py.JavaCode?

	builtinTypes[TME_INDEX_Frame].py_type = &PyFrame_Type;
	builtinTypes[TME_INDEX_Frame].jy_class = pyFrameClass;
	builtinTypes[TME_INDEX_Frame].flags = JY_TRUNCATE_FLAG_MASK;
	//numpy.random.mtrand accesses f_lineno directly, which is rather at the end of the struct.
	//However for simplicity we just alloc the whole PyFrameObject struct:
	builtinTypes[TME_INDEX_Frame].truncate_trailing = sizeof(PyFrameObject);
	builtinTypes[TME_INDEX_Frame].sync = malloc(sizeof(SyncFunctions));
	builtinTypes[TME_INDEX_Frame].sync->jy2py = (jy2pySync) JySync_PyFrame_From_JyFrame;
	builtinTypes[TME_INDEX_Frame].sync->py2jy = (py2jySync) JySync_JyFrame_From_PyFrame;
	void JySync_PyFrame_From_JyFrame(jobject src, PyObject* dest);
	PyFrame_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

/*	builtinTypes[TME_INDEX_Super].py_type = &PySuper_Type;
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
	PyMemoryView_Type.tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;*/

	builtinTypes[TME_INDEX_BaseObject].py_type = &PyBaseObject_Type;
	builtinTypes[TME_INDEX_BaseObject].jy_class = pyObjectClass;
	builtinTypes[TME_INDEX_BaseObject].flags = 0;

	/*
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
	// PyCFunction_Type is a special case, where JyNI provides the Jython-equivalent.
	// We want to force the type to be a PyCPeerType so we exclude it from class-lookup here.
	if (type == &PyCFunction_Type) return NULL;
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
			for (i = 0; i != TME_INDEX_BaseObject; ++i) // TME_INDEX_BaseObject is last and shouldn't be considered
			{
				if (PyType_IsSubtype(type, builtinTypes[i].py_type))
				{
					if (builtinTypes[i].flags & JY_CPEER_FLAG_MASK)
						return NULL;
					//printf("%s is subtype of %s", type->tp_name, builtinTypes[i].py_type->tp_name);
					return &(builtinTypes[i]);
				}
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
		if (builtinTypes[i].jy_class != NULL && (*env)->CallBooleanMethod(env, jythonPyClass, class_equals, builtinTypes[i].jy_class))
			return &(builtinTypes[i]);
	}
	return NULL;
}

inline TypeMapEntry* JyNI_JythonTypeEntry_FromName(char* name)
{
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
	return JyNI_JythonTypeEntry_FromJStringName((*env)->CallObjectMethod(env, jythonPyType, pyType_getName));
}

/* Does not work for Heap-Type exceptions. */
inline jobject JyNI_JythonExceptionType_FromPyExceptionType(PyObject* exc)
{
	env(NULL);
	//return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNIExceptionByName, (*env)->NewStringUTF(env, ((PyTypeObject*) exc)->tp_name));
	return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_exceptionByName, (*env)->NewStringUTF(env, PyExceptionClass_Name(exc)));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * Does not work for Heap-Type exceptions.
 */
inline PyTypeObject* JyNI_PyExceptionType_FromJythonExceptionType(jobject exc)
{
	env(NULL);
	jboolean isCopy;
	jobject name = (*env)->CallObjectMethod(env, exc, pyType_getName);
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
		if ((*env)->GetBooleanField(env, pyBooleanClass, pyBoolean_valueField)) Py_RETURN_TRUE;
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
			PyObject* er = (PyObject*) (*env)->GetLongField(env, jythonPyObject, pyCPeer_objectHandleField);
			Py_INCREF(er);
			return er;
		} else {
			PyObject* er = (PyObject*) (*env)->CallLongMethod(env, jythonPyObject, jyGCHead_getHandle);
			Py_INCREF(er);
			return er;
		}
	}
	if (lookupNative)
	{
		PyObject* handle = (PyObject*) (*env)->CallStaticLongMethod(env, JyNIClass, JyNI_lookupNativeHandle, jythonPyObject);
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
//	jboolean dbg = strcmp(cName, "builtin_function_or_method") == 0;
//	jputs(cName);
	TypeMapEntry* tme = JyNI_JythonTypeEntry_FromName(cName);
//	if (tme && (tme->flags & JY_CPEER_FLAG_MASK))
//		tme = NULL; //todo: Fix this (check class, subtype etc)
	if (tme)
	{
//		if (dbg) printf("%i\n", __LINE__);
//		if (dbg) putsPy(tme->py_type);
//		JyNI_printHash(jythonPyObject);
		//if (tme && tme->py_type) putsPy(tme->py_type);
		//No need to incref here, since JyNI_InitPyObject returns NEW ref.
		return JyNI_InitPyObject(tme, jythonPyObject);
	} else
	{
		//if (dbg) jputs(__LINE__);
		//JyNI_printHash(jythonPyObject);
		ExceptionMapEntry* eme = JyNI_PyExceptionMapEntry_FromPyExceptionType(
			JyNI_PyExceptionType_FromJythonExceptionType(
			(*env)->CallObjectMethod(env, jythonPyObject, pyObject_getType)));
		if (eme)
		{
			// No need to incref here, since JyNI_InitPyException returns NEW ref.
			PyObject* er = JyNI_InitPyException(eme, jythonPyObject);
			return er;
		} else
		{
			jobject tpe = (*env)->CallObjectMethod(env, jythonPyObject, pyObject_getType);
			PyTypeObject* pytpe;
			PyObject* result;
			if ((*env)->IsSameObject(env, jythonPyObject, tpe))
			{
				pytpe = JyNI_InitPyObjectNativeTypePeer(tpe);
				result = pytpe;
			} else
			{
				pytpe = (PyTypeObject*) JyNI_PyObject_FromJythonPyObject(tpe);
				result = JyNI_InitPyObjectSubtype(jythonPyObject, pytpe);
			}
//			if (dbg) {
//				jputsLong(result);
//				JyNI_printJInfo(jythonPyObject);
//			}
			if (!result)
			{
				jputs("JyNI-warning: Unable to handle object:");
				if (pytpe) jputs(pytpe->tp_name);
				else jputs("failed to convert type");
			}
			return result;
		}
	}
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
		jobject type = JyNI_JythonExceptionType_FromPyExceptionType((PyObject*) Py_TYPE(src));
		if (type)
			dest = (*env)->NewObject(env, pyBaseExceptionClass, pyBaseException_subTypeConstructor, type);
		else
			dest = (*env)->NewObject(env, pyBaseExceptionClass, pyBaseException_emptyConstructor);
	}
	if (!dest) return NULL;

	srcJy->jy = (*env)->NewWeakGlobalRef(env, dest);
	if (!(srcJy->flags & JY_HAS_JHANDLE_FLAG_MASK)) {  //some exc_factories might already init this
		(*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_setNativeHandle, dest, (jlong) src);//, srcJy->flags & JY_TRUNCATE_FLAG_MASK);
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
	//if (strcmp("numpy.dtype", Py_TYPE(op)->tp_name) == 0 || Is_StaticSingleton_NotBuiltin(op) && !PyType_Check(op))
		return JyNI_InitStaticJythonPyObject(op);
	else if (strcmp("numpy.dtype", Py_TYPE(op)->tp_name) == 0 || strcmp("numpy.bool_", Py_TYPE(op)->tp_name) == 0)
	{
//		jputsLong(ptrCount);
		jputs("JyNI-Warning: numpy.dtype occurred as type of a non-static object!");
		//puts("JyNI-Warning: numpy.dtype occurred as type of a non-static object!");
		//return JyNI_InitStaticJythonPyObject(op);
//		jputs("JyNI is currently not able to handle this, segfault expected.");
//		if (Is_DynPtrPy(op)) jputs("Is_DynPtrPy");
//		if (PyType_IS_GC(Py_TYPE(op))) jputs("Is_GC");
//		if(PyType_Check(op)) jputs(((PyTypeObject*) op)->tp_name);
	}
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
//		if ((*env)->IsSameObject(env, jy->jy, NULL)) {
//			jputs("Alert!!! Initialized jy->jy became null!!");
//			jputsLong((jlong) jy->jy);
//			if (!op->ob_type) jputs("type is NULL");
//			if (!op->ob_type->tp_name) jputs("type name is NULL");
//			jputs(op->ob_type->tp_name);
//		}
	}
	TypeMapEntry* tme;
	if (jy->jy)
	{
		tme = (TypeMapEntry*) jy->jy;
	} else {
		tme = JyNI_JythonTypeEntry_FromPyType(Py_TYPE(op));
		if (!tme) tme = JyNI_JythonTypeEntry_FromSubTypeWithPeer(Py_TYPE(op));
	}
	if (tme)
		return JyNI_InitJythonPyObject(tme, op, jy);
	else
	{
		ExceptionMapEntry* eme = JyNI_PyExceptionMapEntry_FromPyExceptionType(Py_TYPE(op));
		if (eme) {
			return JyNI_InitJythonPyException(eme, op, jy);
		} else
		{
			//setup and return PyCPeer in this case...
			env(NULL);
			//The following lookup is not necessary, because if there already was a PyCPeer,
			//JyObject_IS_INITIALIZED would have evaluated true.
			//jobject er = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupCPeerFromHandle, (jlong) op);
			//if (er != NULL) return er;

			//PyCPeer has to be created...
//			puts("create CPeer:");
//			puts(Py_TYPE(op)->tp_name);
			Py_INCREF(op);
			//first obtain type:
			//Earlier we used this line, but it would not support HeapTypes
			//or natively defined metatypes:
			//jobject opType = JyNI_JythonPyTypeObject_FromPyTypeObject(Py_TYPE(op));
			//However, the general conversion method should also work and has this support:
			if (PyType_Check(op)) {
				return _JyNI_JythonPyTypeObject_FromPyTypeObject((PyTypeObject*) op, NULL);
			}

			jobject opType = JyNI_JythonPyObject_FromPyObject((PyObject*) Py_TYPE(op));
			jobject er = PyObject_IS_GC(op) ?
					(*env)->NewObject(env, pyCPeerGCClass, pyCPeerGC_Constructor, (jlong) op, opType) :
					(*env)->NewObject(env, pyCPeerClass, pyCPeer_Constructor, (jlong) op, opType);

			jy->flags |= JY_INITIALIZED_FLAG_MASK;
			/* JyNI_GC_EnsureHeadObject not needed here because the head is the
			 * CPeer itself and this has surely been properly initialized.
			 */
			jy->flags |= JY_CPEER_FLAG_MASK;
			jy->jy = (*env)->NewWeakGlobalRef(env, er);
			return er;
		}
	}
}

inline void JyNI_SyncPyCPeerTypeMRO(PyTypeObject* type, jobject jtype)
{
	if (type->tp_mro)
	{
		env();
		if (!jtype)
			jtype = (*env)->CallStaticObjectMethod(env, JyNIClass,
					JyNI_lookupCPeerFromHandle, (jlong) type);
		if (jtype && !(*env)->IsSameObject(env, jtype, NULL))
		{
			if (PyTuple_Check(type->tp_mro)) {
				jobjectArray jmro = (*env)->NewObjectArray(env,
						PyTuple_GET_SIZE(type->tp_mro), pyObjectClass, NULL);
				jint pos;
				for (pos = 0; pos < PyTuple_GET_SIZE(type->tp_mro); ++pos)
					(*env)->SetObjectArrayElement(env, jmro, pos,
							JyNI_JythonPyObject_FromPyObject(
									PyTuple_GET_ITEM(type->tp_mro, pos)));
				(*env)->SetObjectField(env, jtype, pyType_mroField, jmro);
			} else {
				jputs("JyNI-warning: Encountered non-NULL MRO that is not a tuple:");
				jputs(type->tp_name);
			}
		}
	} //else
	//{
	// Happens frequently, so this warning is too annoying.
//				jputs("JyNI-warning: Encountered NULL-MRO:");
//				jputs(type->tp_name);
			// Should we set mro now to empty tuple?
			// For now we try to get away without it.
	//}
}

inline jobject _JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type, jclass cls)
{
//	jputs(__FUNCTION__);
//	jputs(type->tp_name);
	//if (type == NULL) return NULL;
//	jboolean jdbg = strcmp(type->tp_name, "sqlite3.Cursor") == 0;
//	if (jdbg) jputs(__FUNCTION__);
//	if (jdbg) jputs(type->tp_name);
	env(NULL);
	if (cls != NULL)
	{
		return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_getPyType, cls);
	} else {
		//setup and return PyCPeerType in this case...
		jobject er = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_lookupCPeerFromHandle, (jlong) type);
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
			jlong methFlags = 0, onel = 1;
			int pos;
			if (type->tp_as_number || type->tp_as_sequence || type->tp_as_mapping || type->tp_as_buffer)
			{
				pos = 0;
				int posOff;
				if (type->tp_as_number)
				{
					binaryfunc* nb = (binaryfunc*) type->tp_as_number;
					for (posOff = 0; posOff < sizeof(PyNumberMethods)/sizeof(binaryfunc); ++posOff)
					{
						if (*(nb++))
							methFlags += onel << pos++;
						else pos++;
					}
				} else pos += sizeof(PyNumberMethods)/sizeof(binaryfunc);
				if (type->tp_as_sequence)
				{
					binaryfunc* nb = (binaryfunc*) type->tp_as_sequence;
					for (posOff = 0; posOff < sizeof(PySequenceMethods)/sizeof(binaryfunc); ++posOff)
					{
						if (*(nb++)) methFlags += onel << pos++;
						else pos++;
					}
				} else pos += sizeof(PySequenceMethods)/sizeof(binaryfunc);
				if (type->tp_as_mapping)
				{
					binaryfunc* nb = (binaryfunc*) type->tp_as_mapping;
					for (posOff = 0; posOff < sizeof(PyMappingMethods)/sizeof(binaryfunc); ++posOff)
					{
						if (*(nb++)) methFlags += onel << pos++;
						else pos++;
					}
				} else pos += sizeof(PyMappingMethods)/sizeof(binaryfunc);
				if (type->tp_as_buffer)
				{
					binaryfunc* nb = (binaryfunc*) type->tp_as_buffer;
					for (posOff = 0; posOff < sizeof(PyBufferProcs)/sizeof(binaryfunc); ++posOff)
					{
						if (*(nb++)) methFlags += onel << pos++;
						else pos++;
					}
				} else pos += sizeof(PyBufferProcs)/sizeof(binaryfunc);
			} else
				pos = sizeof(PyNumberMethods)/sizeof(binaryfunc) +
						sizeof(PySequenceMethods)/sizeof(binaryfunc) +
						sizeof(PyMappingMethods)/sizeof(binaryfunc) +
						sizeof(PyBufferProcs)/sizeof(binaryfunc);
			if (type->tp_compare) methFlags += onel << pos++;
			else pos++;
			if (RICHCOMPARE(type)) methFlags += onel << pos++;
			else pos++;
			if (type->tp_iter) methFlags += onel << pos++;
			else pos++;


			if (!PyObject_IS_GC((PyObject*) type))
			{
				if (Py_TYPE(type) == NULL || Py_TYPE(type) == &PyType_Type)
				{
					er = (*env)->NewObject(env, pyCPeerTypeClass, pyCPeerType_withNameAndDictConstructor,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict), methFlags);
				} else
				{
					er = (*env)->NewObject(env, pyCPeerTypeClass, pyCPeerType_withNameDictTypeConstructor,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict), methFlags,
							JyNI_JythonPyObject_FromPyObject((PyObject*) Py_TYPE(type)));
				}
			} else
			{
				if (Py_TYPE(type) == NULL || Py_TYPE(type) == &PyType_Type)
				{
					er = (*env)->NewObject(env, pyCPeerTypeGCClass, pyCPeerTypeGC_Constructor,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict), methFlags);
				} else
				{
					er = (*env)->NewObject(env, pyCPeerTypeGCClass, pyCPeerTypeGC_ConstructorSubtype,
							(jlong) type, (*env)->NewStringUTF(env, type->tp_name),
							JyNI_JythonPyObject_FromPyObject(type->tp_dict), methFlags,
							JyNI_JythonPyObject_FromPyObject((PyObject*) Py_TYPE(type)));
				}
			}
			jweak ref = (*env)->NewWeakGlobalRef(env, er);
			//todo: Check whether we clean this jweak up properly.
			(*env)->SetLongField(env, er, pyCPeerType_refHandleField, (jlong) ref);
			/* Okay, here we insert the method resolution order to Jython-side, because
			 * there are situations where Jython-PyType accesses its (or another PyType's)
			 * mro-field directly, e.g. in computeMro. This can cause a subsequent
			 * nullpointer exception, so we initialize it here.
			 */
			JyNI_SyncPyCPeerTypeMRO(type, er);
			(*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_setNativeHandle, er, (jlong) type);
			return er;
		}
	}
}

inline jobject JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type)
{
	if (type == NULL) return NULL;
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
				jythonPyTypeObject, pyCPeerType_objectHandleField);
		Py_INCREF(er);
		return er;
	}
	jstring name = (*env)->CallObjectMethod(env, jythonPyTypeObject, pyType_getName);
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

inline PyObject* JyNI_PyObject_Call(jobject func, PyObject *arg, PyObject *kw)
{
	env(NULL);

	jobject jdict;
	jint dictSize;
	if (kw)
	{
		jdict = JyNI_JythonPyObject_FromPyObject(kw);
		ENTER_SubtypeLoop_Safe_ModePy(jdict, kw, __len__)
		dictSize = (*env)->CallIntMethod(env, jdict, JMID(__len__));
		LEAVE_SubtypeLoop_Safe_ModePy(jdict, __len__)
	} else dictSize = 0;
	jobject args = (*env)->NewObjectArray(env,
			PyTuple_GET_SIZE(arg)
			+dictSize,
			pyObjectClass, NULL);
	//printf("arg-count: %i  kw: %i", PyTuple_GET_SIZE(arg), dictSize);
//	jputs(__FUNCTION__);
//	jputsLong(PyTuple_GET_SIZE(arg));
//	jputsLong(dictSize);
	int i;
	jobject argi;
	for (i = 0; i < PyTuple_GET_SIZE(arg); ++i)
	{
		argi = JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(arg, i));
		(*env)->SetObjectArrayElement(env, args, i, argi);
	}
	jobject jkw = dictSize > 0 ?
			(*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, args, jdict) :
			JyEmptyStringArray;
	jobject er;
	Py_BEGIN_ALLOW_THREADS
	er = (*env)->CallObjectMethod(env, func,
			pyObject___call__, args, jkw);
	Py_END_ALLOW_THREADS
	if ((*env)->ExceptionCheck(env))
	{
		jobject exc = (*env)->ExceptionOccurred(env);
		jputs("Java-exception during object call");
		jPrintCStackTrace();
		(*env)->ExceptionDescribe(env);
		//JyNI_jprintJ(exc);

	}
	return JyNI_PyObject_FromJythonPyObject(er);
}

/*returns local ref!*/
inline jstring JyNI_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op)
{
	JyObject* jy = AS_JY_NO_GC(op);
	if (jy->flags & JY_INITIALIZED_FLAG_MASK)
	{
		return (*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) op), pyObject_asString);
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
		jstring er = (*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) op), pyObject_asString);
		if (JyNI_HasJyAttribute(jy, JyAttributeStringInterned))
		{
			return er;
		} else
		{
			return (*env)->CallObjectMethod(env, er, string_intern);
		}
	} else
	{
		return (*env)->CallObjectMethod(env, (*env)->NewStringUTF(env, PyString_AS_STRING(op)), string_intern);
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
#define JY_MAYBE_DELEGATE_TYPE(v, flags) \
		( !(flags & JY_SUBTYPE_FLAG_MASK) && \
		  !(flags & JY_CPEER_FLAG_MASK) && \
		   (flags & JY_HAS_JHANDLE_FLAG_MASK))
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
//	jputs(__FUNCTION__);
	//if (!Is_DynPtrPy(v)) return NULL;
	if (Is_StaticSingleton_NotBuiltin(v)) return NULL;
	if (!PyType_Check(v)) // && !PyExc_Check(v)PyStructSequence_InitType
	{
		JyObject* jy = AS_JY(v);
//		jputsLong(jy->flags);
		if (JY_DELEGATE(v, jy->flags)) {
			jobject er = JyNI_JythonPyObject_FromPyObject(v);
			return er;
		}
	} else {
		if (PyExceptionClass_Check(v)) return NULL; //never delegate for ExceptionType
		if (PyType_HasFeature(((PyTypeObject*) v), Py_TPFLAGS_HEAPTYPE))
		{
			JyObject* jy = AS_JY(v);
			if (JY_DELEGATE(v, jy->flags))
				return JyNI_JythonPyObject_FromPyObject(v);
			else if (JY_MAYBE_DELEGATE_TYPE(v, jy->flags))
			{
				// Now it depends on the bases whether we must delegate.
				// If any of the bases cannot be delegated, we don't delegate.
				PyTypeObject* base = ((PyTypeObject*) v)->tp_base;
				while (base)
				{
					if (PyType_HasFeature(base, Py_TPFLAGS_HEAPTYPE))
					{
						//puts("base-heaptype");
						if (!JyNI_GetJythonDelegate(base)) {
							//printf("Wouldn't delegate: %s\n", ((PyTypeObject*) v)->tp_name);
							break;
						}
					} else
					{
						if (!JyNI_IsBuiltinPyType(base))
						{
							//printf("Wouldn't delegate: %s\n", ((PyTypeObject*) v)->tp_name);
							break;
						}
					}
					base = base->tp_base;
				}
				if (!base)
				{
					//printf("Would delegate: %s\n", ((PyTypeObject*) v)->tp_name);
					return JyNI_JythonPyObject_FromPyObject(v);
				}
			}
		}
		jclass cls = JyNI_JythonClassFromPyType((PyTypeObject*) v);
		if (cls != NULL) //delegate
			return _JyNI_JythonPyTypeObject_FromPyTypeObject((PyTypeObject*) v, cls);
	}
	return NULL;
}

inline jint JyNI_GetDLOpenFlags()
{
	env(0x00001 | 0x00100); //RTLD_LAZY | RTLD_GLOBAL
	return (*env)->CallStaticIntMethod(env, JyNIClass, JyNI_getDLOpenFlags);
}

inline void JyNI_CleanUp_JyObject(JyObject* obj)
{
//	jputs(__FUNCTION__);
//	jputs(Py_TYPE(FROM_JY(obj))->tp_name);
	//Todo: Maybe clean up gc-head here (?)
	if (obj->attr) JyNI_ClearJyAttributes(obj);
	if (obj->jy && (obj->flags & JY_INITIALIZED_FLAG_MASK))
	{
		env();
		if ((obj->flags & JY_HAS_JHANDLE_FLAG_MASK) && !(*env)->IsSameObject(env, obj->jy, NULL))
			(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_clearNativeHandle, obj->jy);
//		if (obj->flags & JY_CPEER_FLAG_MASK)
//			(*env)->DeleteWeakGlobalRef(env, obj->jy);
//		else
//			(*env)->DeleteGlobalRef(env, obj->jy);
		(*env)->DeleteWeakGlobalRef(env, obj->jy);
		obj->flags = 0;
		obj->jy = NULL;
	}
}

/*
 * Py_GetVersion is usually hosted in getversion.c, but in JyNI we host it
 * here for simplicity.
 */
const char *
Py_GetVersion(void)
{
	env(NULL);
	static char version[250];
	jstring ver = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_getPlatform);
	char* utf_string = (*env)->GetStringUTFChars(env, ver, NULL);
	strcpy(version, utf_string);
	(*env)->ReleaseStringUTFChars(env, ver, utf_string);
//	PyOS_snprintf(version, sizeof(version), "%.80s (%.80s) %.80s",
//		      PY_VERSION, Py_GetBuildInfo(), Py_GetCompiler());
	return version;
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
	jputsLong((*env)->CallStaticIntMethod(env, JyNIClass, JyNI_jGetHash, obj));
}

inline void JyNI_printHash(jobject obj) {
	env();
	printf("%li\n", ((*env)->CallStaticIntMethod(env, JyNIClass, JyNI_jGetHash, obj)));
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
				jstring msg = (*env)->CallObjectMethod(env, obj, object_toString);
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
		jobject cls = (*env)->CallObjectMethod(env, obj, object_getClass);
		JyNI_jprintJ(cls);
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

inline void putsPy(PyObject* o)
{
	puts(o ? PyString_AS_STRING(PyObject_Str(o)) : "NULL-PyObject");
}

void jPrintCStackTrace()
{
	jputs(__FUNCTION__);
	void* buf[300];
	int size = backtrace(buf, 300);
	jputsLong(size);
	char** funcs = backtrace_symbols(buf, size);
	int i;
	for (i = 0; i < size; ++i)
		jputs(funcs[i]);
	free(funcs);
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
	//jputs("JyNI-Warning: PyMemoryView_FromObject is not implemented!");
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
