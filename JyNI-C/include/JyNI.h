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
 * JyNI.h
 *
 *  Created on: 13.03.2013, 23:15:00
 *      Author: Stefan Richthofer
 */

#ifndef JYNI_H_
#define JYNI_H_

//#define WITHOUT_COMPLEX
//PyExc_UnicodeDecodeError
#include <jni.h>
#include <Python_JyNI.h>
//#include <JyNI_JyNI.h>
#include <JyList.h>
#include <JyTState.h>
#include <JyNI-Debug.h>
#include <JyNI_JNI.h>

// We could alternatively include JyNI-Java/include/JyNI_JyNI.h,
// but for now it feels more lightweight to simply redefine the
// value we actually need here:
#define JyNI_JyNI_NATIVE_INT_METHOD_NOT_IMPLEMENTED -2L

/* JNI-Shortcuts: */

/* Shortcut to get JNI-environment, including check for whether
 * JNI version is supported. Note that we don't put a semicolon
 * to the end, so the macro-call must be succeeded by a semicolon,
 * giving it a more natural look...
 */
#define env(errRet) \
	JNIEnv *env; \
	if ((*java)->GetEnv(java, (void **)&env, JNI_VERSION_1_6)) \
		return errRet

/* For now we assume, nobody would cache _PyThreadState_Current for
 * use after the current method returns. So we need not acquire a
 * global reference.
 */
#define ENTER_JyNI \
	PyEval_AcquireLock(); \
	if (_PyThreadState_Current != NULL) Py_FatalError("ENTER_JyNI: overwriting non-NULL tstate"); \
	_PyThreadState_Current = (PyThreadState*) tstate;

#define LEAVE_JyNI0 \
	JyNI_GC_Explore(); \
	_PyThreadState_Current = NULL; \
	PyEval_ReleaseLock();

#define LEAVE_JyNI \
	if (PyErr_Occurred()) JyErr_InsertCurExc(); \
	LEAVE_JyNI0

/*
 * For methods that allow re-entering after a callback into JVM
 * given the same thread state is used.
 * Use with care!
 */
#define RE_ENTER_JyNI \
	int reenter = _PyThreadState_Current == (PyThreadState*) tstate; \
	if (!reenter) { ENTER_JyNI }

#define RE_LEAVE_JyNI \
	if (!reenter) { LEAVE_JyNI } \
	else {JyNI_GC_Explore();} //maybe also JyErr_InsertCurExc()...?

/* Cleanly convert a jstring to a cstring with minimal JVM lock-time.
 * Use only once per Function. For further conversions use
 * cstr_from_jstring2. Note that at least one call of "env()" must
 * have happened before in the same block or in some parent block.
 * ("+1" in 3rd line is for 0-termination)
 */
#define cstr_from_jstring(cstrName, jstr) \
	const char* utf_string = (*env)->GetStringUTFChars(env, jstr, NULL); \
	char cstrName[strlen(utf_string)+1]; \
	strcpy(cstrName, utf_string); \
	(*env)->ReleaseStringUTFChars(env, jstr, utf_string)

#define global_cstr_from_jstring(cstrName, jstr) \
	const char* utf_string = (*env)->GetStringUTFChars(env, jstr, NULL); \
	char* cstrName = malloc((strlen(utf_string)+1)*sizeof(char)); \
	strcpy(cstrName, utf_string); \
	(*env)->ReleaseStringUTFChars(env, jstr, utf_string)

/* Only use after one initial use of cstr_from_jstring in the same block.
 * ("+1" in 3rd line is for 0-termination)
 */
#define cstr_from_jstring2(cstrName, jstr) \
	utf_string = (*env)->GetStringUTFChars(env, jstr, NULL); \
	char cstrName[strlen(utf_string)+1]; \
	strcpy(cstrName, utf_string); \
	(*env)->ReleaseStringUTFChars(env, jstr, utf_string)

#define global_cstr_from_jstring2(cstrName, jstr) \
	utf_string = (*env)->GetStringUTFChars(env, jstr, NULL); \
	char* cstrName = malloc((strlen(utf_string)+1)*sizeof(char)); \
	strcpy(cstrName, utf_string); \
	(*env)->ReleaseStringUTFChars(env, jstr, utf_string)

#define pyTuple2jArray(pyTuple, javaElementType, javaDestName) \
	jobject javaDestName = NULL; \
	if (PyTuple_GET_SIZE(pyTuple)) \
	{ \
		javaDestName = (*env)->NewObjectArray(env, PyTuple_GET_SIZE(pyTuple), javaElementType, NULL); \
		for (i = 0; i < PyTuple_GET_SIZE(pyTuple); ++i) \
			(*env)->SetObjectArrayElement(env, javaDestName, i, \
				JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(pyTuple, i))); \
	}

#define pyStrTuple2jStrArray(pyTuple, javaDestName) \
	jobject javaDestName = NULL; \
	if (PyTuple_GET_SIZE(pyTuple)) \
	{ \
		javaDestName = (*env)->NewObjectArray(env, PyTuple_GET_SIZE(pyTuple), stringClass, NULL); \
		for (i = 0; i < PyTuple_GET_SIZE(pyTuple); ++i) \
			(*env)->SetObjectArrayElement(env, javaDestName, i, \
				(*env)->NewStringUTF(env, PyString_AS_STRING( \
					((PyStringObject*) PyTuple_GET_ITEM(pyTuple, i))))); \
	}

#define pyTuple2jStrArray(pyTuple, javaDestName) \
	jobject javaDestName = NULL; \
	if (PyTuple_GET_SIZE(pyTuple)) \
	{ \
		javaDestName = (*env)->NewObjectArray(env, PyTuple_GET_SIZE(pyTuple), stringClass, NULL); \
		for (i = 0; i < PyTuple_GET_SIZE(pyTuple); ++i) \
			(*env)->SetObjectArrayElement(env, javaDestName, i, \
				(*env)->NewStringUTF(env, PyString_AS_STRING(PyObject_Str(PyTuple_GET_ITEM(pyTuple, i))))); \
	}

#define jStringArray2pyTuple(jArray, resultName) \
	Py_ssize_t jArraySize = 0; \
	if (jArray) jArraySize = (*env)->GetArrayLength(env, jArray); \
	PyTupleObject* resultName = PyTuple_New(jArraySize); \
	if (jArraySize) \
	{ \
		Py_ssize_t i; \
		char* utf_string; \
		jobject jstr; \
		for (i = 0; i < jArraySize; ++i) \
		{ \
			jstr = (*env)->GetObjectArrayElement(env, jArray, i); \
			utf_string = (*env)->GetStringUTFChars(env, jstr, NULL); \
			char cstr[strlen(utf_string)+1]; \
			strcpy(cstr, utf_string); \
			(*env)->ReleaseStringUTFChars(env, jstr, utf_string); \
			PyTuple_SET_ITEM(resultName, i, PyString_FromString(cstr)); \
		} \
	}

/*
 * We use so-called JyObjects to reflect Jython-objects located inside the JVM.
 * The JyObject is stored in memory BEFORE every PyObject, so it never
 * interferes with the c-style struct-subclassing CPython is based on.
 *
 * One glitch to consider:
 * PyGC_Head uses the same trick (in fact I learned it from there).
 * So we put our JyObject data even before the
 * PyGC_Head if present. However, this involves a check via
 * PyObject_IS_GC(o) every time, we access a JyObject.
 * This is handled automatically by the macros AS_JY(o) and FROM_JY(o).
 *
 * Flags tell us, whether the subsequent PyObject is truncated to a bare
 * PyObject-Head (and thus only java-side needs to be taken care of).
 * If it is not truncated, flags can include some information on how
 * data synchronization should take place. As a rule of thumb, we
 * manage types as truncated, if there are no macros in the official
 * Python-headers that directly modify the structure.
 * Additionally flags tell us whether a PyGC_Head is present (because
 * otherwise, it would be very tedious or impossible to write a macro
 * like FROM_JY(o)). They also tell whether jy refers to a PyCPeer,
 * saving some frequent type checking JNI-calls.
 *
 * The INITIALIZED-flag indicates whether a java-side object is already
 * prepared. Note that jy == NULL is no indication for this, because
 * jy may be pre-initialized with the corresponding TypeMapEntry,
 * saving a lookup at allocation.
 *
 * The HAS_JHANDLE-flag indicates whether Java-side JyNI is already
 * tracking the native object, i.e. that JyNISetNativeHandle was called.
 * It is mainly an indicator for clean-up methods to call
 * JyNIClearNativeHandle.
 */

/* General Flags: */
#define JY_INITIALIZED_FLAG_MASK    1
#define JY_HAS_JHANDLE_FLAG_MASK    2
#define JY_GC_FLAG_MASK             4
#define JY_TRUNCATE_FLAG_MASK       8
//#define JY_PARTLY_TRUNCATE_MASK   8 (deprecated; indicated by JY_TRUNCATE_FLAG_MASK + non-zero truncate_trailing)
#define JY_CPEER_FLAG_MASK         16
//#define JY_CACHE_GC_FLAG_MASK    32 /* (currently not used) */
#define JY_SUBTYPE_FLAG_MASK       32 /* used to prevent an endless call-delegation loop for subtype PyCPeers */
#define JY_CACHE_ETERNAL_FLAG_MASK 64
//#define JY_GC_SINGLE_LINK        64
#define JY_GC_VAR_SIZE            128 /* This distinguishes array-like vs list-like links. */

// Derived flags:
#define JY_CACHE                   64 // Maybe 96 /* JY_CACHE_GC_FLAG_MASK | JY_CACHE_ETERNAL_FLAG_MASK */
#define JY_NATIVE_SUBTYPE          48 // JY_SUBTYPE_FLAG_MASK | JY_CPEER_FLAG_MASK
#define JY_POTENTIAL_DELEGATE      40 // JY_TRUNCATE_FLAG_MASK | JY_SUBTYPE_FLAG_MASK
//#define JY_GC_SPECIAL_CASE        192 /* JY_GC_SINGLE_LINK | JY_GC_FIXED_SIZE */
//#define JY_GC_VAR_SIZE              0 /* Default if JY_GC_FLAG_MASK is active. Just intended as a marker. */

/*
 * 1L<<11 is a flag currently unused by CPython, so we can use it here.
 * We use it to indicate that objects of this type are not likely to be
 * statically defined singletons.
 */
#define Jy_TPFLAGS_DYN_OBJECTS (1L<<11)

/*
 * This augments flags in methodobject.h:
 */
#define METH_JYTHON       0x0080
#define METH_JYTHON_CDEF  0x0100

// Temporarily add pointer-counting to check for leaks.
extern jlong ptrCount;

//#define Is_DynPtr(p) (minDynPtr && (((jlong) p) >= ((jlong) minDynPtr)) && (((jlong) p) <= ((jlong) maxDynPtr)))
#define Is_DynPtr(ptr) JyHash_contains(ptr)

#define Is_DynPtrPy(p) Is_DynPtr(_AS_JY_NO_GC(p))

//#define Update_DynPtr(p) \
//	if (!minDynPtr) minDynPtr = maxDynPtr = p; \
//	else { \
//		if (p < minDynPtr) minDynPtr = p; \
//		if (p > maxDynPtr) maxDynPtr = p; \
//	}

#define notifyAlloc(ptr) JyHash_insert(ptr);
#define notifyFree(ptr) JyHash_delete(ptr);

#define Is_StaticSingleton_NotBuiltin(pyObject) \
	(!(PyType_HasFeature(Py_TYPE(pyObject), Jy_TPFLAGS_DYN_OBJECTS) || \
	PyType_IS_GC(Py_TYPE(pyObject)) || Is_DynPtrPy(pyObject) || \
	PyType_HasFeature(Py_TYPE(pyObject), Py_TPFLAGS_HEAPTYPE)))
// e.g. <type '_ctypes.CThunkObject'> needs PyType_IS_GC-check
// e.g. <class 'ctypes.CDLL'> needs heaptype-check

#define Is_StaticSingleton(pyObject) \
	(pyObject == Py_None || pyObject == Py_Ellipsis || pyObject == Py_NotImplemented || \
	pyObject == Py_True || pyObject == Py_False || \
	Is_StaticSingleton_NotBuiltin(pyObject))

//#define Is_StaticTypeObject(pyObject) \
//	(PyType_Check(pyObject) && !PyType_HasFeature(Py_TYPE(pyObject), Py_TPFLAGS_HEAPTYPE))

#define Is_StaticTypeObject(pyObject) \
	(PyType_Check(pyObject) && !PyType_HasFeature((PyTypeObject*) pyObject, Py_TPFLAGS_HEAPTYPE))


#define Is_JyNICriticalType(tp) \
	(tp == &PyList_Type || tp == &PyCell_Type || !JyNI_IsBuiltinPyType(tp))

#define Has_Dealloc(pyObject) \
	(Py_TYPE(pyObject)->tp_dealloc)

/* Todo Invent a flag or something to explicitly mark heap-allocated PyObjects.
 * The current solution (i.e. the macro below) fails if an extension defines its
 * own static singletons or whatever.
 * Checking for tp_dealloc == NULL is a good indicator for static objects, but
 * is not entirely safe - the extension might use some equivalent of none_dealloc.
 */
#define Is_Static_PyObject(pyObject) \
	(Is_StaticSingleton(pyObject) || Is_StaticTypeObject(pyObject) || !Has_Dealloc(pyObject))

#define PyType_IsReady(pyObject) \
	(((PyTypeObject*) pyObject)->tp_flags & Py_TPFLAGS_READY)

#define IsReadyType(pyObject) \
	(PyType_CheckExact(pyObject) && PyType_IsReady(pyObject))

#define JyNIClearRef0(jobjectRef, flags, env0) \
	if (!(flags & JY_CACHE)) \
		(*env0)->DeleteLocalRef(env0, jobjectRef)

#define JyNIClearRef(jobjectRef, pyObject, env0) \
	if (!Is_Static_PyObject(pyObject)) { \
		jyRef0 = AS_JY(jobjectRef); \
		JyNIClearRef0(jobjectRef, jyRef0->flags, env0) \
	} else if (Is_StaticTypeObject(pyObject)) { \
		(*env0)->DeleteLocalRef(env0, jobjectRef); \
	}

//#define JyNIToGlobalRef0(jobjectRef, flags, env0) \
//	if (!(flags & JY_CACHE)) { \
//		jobjectTmp0 = jobjectRef; \
//		jobjectRef = (*env0)->NewGlobalRef(env0, jobjectRef); \
//		((*env0)->DeleteLocalRef(env0, jobjectTmp0)); \
//	}
//
//#define JyNIToGlobalRef(jobjectRef, pyObject, env0) \
//	if (!Is_Static_PyObject(pyObject)) { \
//		JyNIToGlobalRef0(jobjectRef, AS_JY(pyObject)->flags, env0) \
//	} else if (Is_StaticTypeObject(pyObject)) { \
//		jobjectTmp0 = jobjectRef; \
//		jobjectRef = (*env0)->NewGlobalRef(env0, jobjectRef); \
//		((*env0)->DeleteLocalRef(env0, jobjectTmp0)); \
//	}

#define JyNIToWeakGlobalRef0(jobjectRef, flags, env0) \
	if (!(flags & JY_CACHE)) { \
		jobjectTmp0 = jobjectRef; \
		jobjectRef = (*env0)->NewWeakGlobalRef(env0, jobjectRef); \
		((*env0)->DeleteLocalRef(env0, jobjectTmp0)); \
	}

#define JyNIToWeakGlobalRef(jobjectRef, pyObject, env0) \
	if (!Is_Static_PyObject(pyObject)) { \
		JyNIToWeakGlobalRef0(jobjectRef, AS_JY(pyObject)->flags, env0) \
	} else if (Is_StaticTypeObject(pyObject)) { \
		jobjectTmp0 = jobjectRef; \
		jobjectRef = (*env0)->NewWeakGlobalRef(env0, jobjectRef); \
		((*env0)->DeleteLocalRef(env0, jobjectTmp0)); \
	}


/* Subtype Loop-safe macros help to prevent delegations jump endless
 * between native side and Java subtype PyCPeer.
 */

/* Subtype-delegte recursion flags: */
//#define sdrflag__call__          1
//#define sdrflag__findattr_ex__   2
//#define sdrflag__setattr__       4
//#define sdrflag__str__           8
//#define sdrflag__repr__         16
//#define sdrflag__finditem__     32
//#define sdrflag__setitem__      64
//#define sdrflag__delitem__     128
//#define sdrflag__len__         256
//#define sdrflag_toString       512

/* Generates the JNI jmethodID name for PyObject methods from bare method name. */
//#define JMID(method) pyObject ## method
#define JMID(method) jmid ## _ ## method

/* Generates a name for a tmp helper-var. */
//#define jytmp(jObject) jytmp_ ## jObject
//#define jytmpPy(jObject) jytmpPy_ ## jObject


// if (JyNI_HasJyAttribute(JyObject* obj, JyAttributeSubDelegFlags))
// short sdrFlags_tmp ## method = (short) JyNI_GetJyAttribute(JyObject* obj, JyAttributeSubDelegFlags);
// sdrFlags_tmp ## method |= sdrflag ## method;
// JyNI_AddOrSetJyAttribute(jytmp(jObject), JyAttributeSubDelegFlags, (void*) sdrFlags_tmp ## method);

//#define JyNICheckSubtypeInt(pyObject) \
//	if (JyNI_HasJyAttribute(AS_JY(pyObject), JyAttributeSubDelegFlags)) \
//			return JyNI_JyNI_NATIVE_INT_METHOD_NOT_IMPLEMENTED
//
//#define JyNICheckSubtype(pyObject) \
//	if (JyNI_HasJyAttribute(AS_JY(pyObject), JyAttributeSubDelegFlags)) \
//			return NULL

#define JyNICheckSubtypeInt(pyObject) \
	if (AS_JY(pyObject)->flags & JY_SUBTYPE_FLAG_MASK) \
			return JyNI_JyNI_NATIVE_INT_METHOD_NOT_IMPLEMENTED

#define JyNICheckSubtype(pyObject) \
	if (AS_JY(pyObject)->flags & JY_SUBTYPE_FLAG_MASK) \
			return NULL

#define ENTER_SubtypeLoop_Safe_Mode(jObject, method) \
	jmethodID jmid ## _ ## method = pyObject ## _ ## method; \
	if ((*env)->IsInstanceOf(env, jObject, cPeerNativeDelegateInterface)) \
		jmid ## _ ## method = super ## method;

#define ENTER_SubtypeLoop_Safe_ModePy(jObject, pyObj, method) \
	ENTER_SubtypeLoop_Safe_Mode(jObject, method)

#define LEAVE_SubtypeLoop_Safe_Mode(jObject, method)

#define LEAVE_SubtypeLoop_Safe_ModePy(jObject, method)

//#define ENTER_SubtypeLoop_Safe_Mode0(jObject, method) \
//	jputs(__FUNCTION__); \
//	jputsLong(__LINE__); \
//	PyThreadState * jobject ## method ## _savets = NULL; \
//	JyObject* jytmp(jObject) = NULL; \
//	jmethodID jmid ## method = pyObject ## method; \
//	short sdrFlags_tmp ## jObject; \
//	if ((*env)->IsInstanceOf(env, jObject, cPeerNativeDelegateInterface)) { \
//		jytmp(jObject) = AS_JY(JyNI_PyObject_FromJythonPyObject(jObject)); \
//		sdrFlags_tmp ## jObject = (short) \
//				JyNI_GetJyAttribute(jytmp(jObject), JyAttributeSubDelegFlags); \
//		if (sdrFlags_tmp ## jObject & sdrflag ## method) { \
//			jmid ## method = super ## method; \
//			Py_DECREF(FROM_JY(jytmp(jObject))); \
//			jytmp(jObject) = NULL; \
//		} else { \
//			jobject ## method ## _savets = PyEval_SaveThread(); \
//			sdrFlags_tmp ## jObject |= sdrflag ## method; \
//			JyNI_AddOrSetJyAttribute(jytmp(jObject), JyAttributeSubDelegFlags, \
//					(void*) sdrFlags_tmp ## jObject); \
//		} \
//	}
//
//#define ENTER_SubtypeLoop_Safe_ModePy0(jObject, pyObj, method) \
//	jputs(__FUNCTION__); \
//	jputsLong(__LINE__); \
//	PyThreadState * jobject ## method ## _savets = NULL; \
//	JyObject* jytmpPy(jObject) = NULL; \
//	jmethodID jmid ## method = pyObject ## method; \
//	short sdrFlags_tmp ## jObject; \
//	if ((*env)->IsInstanceOf(env, jObject, cPeerNativeDelegateInterface)) { \
//		jytmpPy(jObject) = AS_JY(pyObj); \
//		sdrFlags_tmp ## jObject = (short) \
//				JyNI_GetJyAttribute(jytmpPy(jObject), JyAttributeSubDelegFlags); \
//		if (sdrFlags_tmp ## jObject & sdrflag ## method)  { \
//			jmid ## method = super ## method; \
//			jytmpPy(jObject) = NULL; \
//		} else { \
//			jobject ## method ## _savets = PyEval_SaveThread(); \
//			sdrFlags_tmp ## jObject |= sdrflag ## method; \
//			JyNI_AddOrSetJyAttribute(jytmpPy(jObject), JyAttributeSubDelegFlags, \
//					(void*) sdrFlags_tmp ## jObject); \
//		} \
//	}
//
//#define LEAVE_SubtypeLoop_Safe_Mode0(jObject, method) \
//	jputsLong(__LINE__); \
//	if (jytmp(jObject)) { \
//		sdrFlags_tmp ## jObject = (short) \
//				JyNI_GetJyAttribute(jytmp(jObject), JyAttributeSubDelegFlags); \
//		sdrFlags_tmp ## jObject &= ~sdrflag ## method; \
//		JyNI_AddOrSetJyAttribute(jytmp(jObject), JyAttributeSubDelegFlags, \
//				(void*) sdrFlags_tmp ## jObject); \
//		Py_DECREF(FROM_JY(jytmp(jObject))); \
//	} \
//	if (jobject ## method ## _savets) \
//		PyEval_RestoreThread(jobject ## method ## _savets);
//
//#define LEAVE_SubtypeLoop_Safe_ModePy0(jObject, method) \
//	jputsLong(__LINE__); \
//	if (jytmpPy(jObject)) { \
//		sdrFlags_tmp ## jObject = (short) \
//				JyNI_GetJyAttribute(jytmpPy(jObject), JyAttributeSubDelegFlags); \
//		sdrFlags_tmp ## jObject &= ~sdrflag ## method; \
//		JyNI_AddOrSetJyAttribute(jytmpPy(jObject), JyAttributeSubDelegFlags, \
//				(void*) sdrFlags_tmp ## jObject); \
//	} \
//	if (jobject ## method ## _savets) \
//		PyEval_RestoreThread(jobject ## method ## _savets);

/* GC-macro-replacements */
#define _JyNI_GC_TRACK(o) PyObject_GC_Track(o) //_PyObject_GC_TRACK(o)
#define _JyNI_GC_TRACK_NoExplore(o) PyObject_GC_Track_NoExplore(o)
#define _JyNI_GC_UNTRACK(o) PyObject_GC_UnTrack(o) //_PyObject_GC_UNTRACK(o)
#define _JyNI_GC_IS_TRACKED(o) _PyObject_GC_IS_TRACKED(o)
#define _JyNI_GC_MAY_BE_TRACKED(obj) _PyObject_GC_MAY_BE_TRACKED(obj)

#define JYNI_GC_TUPLE_EXPLORE(tp) JyNI_GC_ExploreObject(tp)
#define JYNI_GC_LIST_EXPLORE(tp) JyNI_GC_ExploreObject(tp)

/* Additional values for _PyGC_REFS. For consistency we also list the original
 * values here:
 * #define _PyGC_REFS_UNTRACKED                    (-2)
 * #define _PyGC_REFS_REACHABLE                    (-3)
 * #define _PyGC_REFS_TENTATIVELY_UNREACHABLE      (-4)
 */
#define _PyGC_REFS_UNEXPLORED                      (-5)
#define _PyGC_REFS_EXPLORING                       (-6)
#define _PyGC_REFS_EXPLORED                        (-7)

/* Macro to get the tp_richcompare field of a type if defined */
#define RICHCOMPARE(t) (PyType_HasFeature((t), Py_TPFLAGS_HAVE_RICHCOMPARE) \
				? (t)->tp_richcompare : NULL)

/* define some method-signatures for sync purposes: */

/* jobject is src, PyObject* is dest. Src must not be modified. */
typedef void (*jy2pySync)(jobject, PyObject*);

/* PyObject* is src, jobject is dest. Src must not be modified. */
typedef void (*py2jySync)(PyObject*, jobject);

typedef jobject (*jyInitSync)(PyObject*, jclass);
typedef PyObject* (*pyInitSync)(jobject, PyTypeObject*);
typedef jobject (*jyFactoryMethod)();
//typedef void (*jy2pyItemSync)(jobject, PyObject*, int index);
//typedef void (*py2jyItemSync)(PyObject*, jobject, int index);
typedef jlong (*pyChecksum)(PyObject*);
typedef jlong (*jyChecksum)(jobject);

/* Sync-behavior flags: */
#define SYNC_ON_PY_INIT_FLAG_MASK                 256
#define SYNC_ON_JY_INIT_FLAG_MASK                 512
#define SYNC_ON_PY_TO_JY_FLAG_MASK               1024
#define SYNC_ON_JY_TO_PY_FLAG_MASK               2048
#define SYNC_ON_PY_CHECKSUM_CHANGE_FLAG_MASK     4096
#define SYNC_ON_JY_CHECKSUM_CHANGE_FLAG_MASK     8192
//#define SYNC_ALWAYS_COMPLETELY_FLAG_MASK      16384
#define SYNC_NEEDED_MASK	3072
	//(SYNC_ON_PY_TO_JY_FLAG_MASK | SYNC_ON_JY_TO_PY_FLAG_MASK)

/* GC-exploration-behavior flags: */
//#define GC_NO_INITIAL_EXPLORE                   16384
#define GC_CRITICAL                             32768


/*
 * py2jy copies everything sanely from a PyObject to a Jython jobject.
 * jy2py copies everything sanely from a Jython jobject to a PyObject.
 * jyInit sanely calls a valid constructor on a Jython jobject using
 * data from a corresponding PyObject.
 * pyInit sanely creates a valid PyObject initialized with the data
 * from the given Jython jobject.
 * pyInit may be NULL - in that case PyObject_New(), PyObject_NewVar()
 * PyObject_GC_New() or PyObject_GC_NewVar() is called directly,
 * depending on whether the GC-flag is set in flags and whether
 * itemsize > 0 (then var, otherwise normal).
 * If jyInit is NULL, either a PyCPeer is created (if the PyCPeer-flag
 * is active) or an argless constructor is searched.
 */
typedef struct {
	jy2pySync     jy2py;
	py2jySync     py2jy;
	jyInitSync    jyInit;
	pyInitSync    pyInit;
	pyChecksum    pyCheck;
	jyChecksum    jyCheck;
} SyncFunctions;
//typedef struct {SyncInfo sync; jy2pyItemSync jy2pyItem; py2jyItemSync py2jyItem;} SyncVarInfo;

//typedef Py_ssize_t (*lenfunc)(PyObject *);

/* JyAttributes are intended to track allocated memory for
 * various-purpose stuff. It's a dict-like idea implemented
 * as a simple linked list. This feature is intended for
 * rare use, so this is a sufficient implementation with minimal
 * overhead.
 * Every JyAttribute must be uniquely identified by an
 * "interned" cstring. Comparison is done via == (pointer-equal)
 * rather than strcmp. So for lookup one must pass the very same
 * char* as on declaration.
 * JyAttributeElement can be used to chain var-length data into
 * JyAttribute value pointer. Specify the JY_ATTR_VAR_SIZE-flag,
 * which tells JyNI to tidy up everything in the end (i.e. call free
 * for every element in the value-chain).
 * Note that maintaining JGCHead and JyGCRefTmp as JyAttributes
 * is a temporary solution and might be improved in future.
 * Currently the only possible attributes are the following:
 */
extern const char* JyAttributePyChecksum;
extern const char* JyAttributeJyChecksum;
extern const char* JyAttributeSyncFunctions;
extern const char* JyAttributeModuleFile;
extern const char* JyAttributeModuleName;
extern const char* JyAttributeTypeName;
extern const char* JyAttributeMethodName;
extern const char* JyAttributeMethodDoc;
extern const char* JyAttributeMethodDef;
extern const char* JyAttributeStringInterned;
extern const char* JyAttributeSetEntry;
extern const char* JyAttributeJyGCHead;
extern const char* JyAttributeJyGCRefTmp;
extern const char* JyAttributeWeakRefCount;
//extern const char* JyAttributeSubDelegFlags;
//extern const char* JyAttributeTruncateSize;

#define JY_ATTR_OWNS_VALUE_FLAG_MASK 1
#define JY_ATTR_VAR_SIZE_FLAG_MASK 2
#define JY_ATTR_JWEAK_VALUE_FLAG_MASK 4
typedef struct JyAttribute JyAttribute; /* Forward declaration */
struct JyAttribute { void* value; JyAttribute* next; const char* name;  char flags;};
typedef struct JyAttributeElement JyAttributeElement; /* Forward declaration */
struct JyAttributeElement {void* value; JyAttributeElement* next;};

typedef struct {
	jweak             jy;
	JyAttribute*      attr;
	unsigned short    flags;
} JyObject;

typedef struct { JyObject jy; PyIntObject pyInt;} JyIntObject;
/* only used for pre-allocated blocks */

typedef struct { JyObject jy; PyFloatObject pyFloat;} JyFloatObject;
/* only used for pre-allocated blocks */

/* type_name is optional and defaults to py_type->tp_name */
typedef struct {
	PyTypeObject*     py_type;
	jclass            jy_class;
	jclass            jy_subclass;
	SyncFunctions*    sync;
	size_t            truncate_trailing;
	unsigned short    flags;
	char*             type_name;
} TypeMapEntry;

typedef struct {
	PyTypeObject*     exc_type;
	jyFactoryMethod   exc_factory;
} ExceptionMapEntry;

#define JyObject_HasJyGCHead(pyObject, jyObject) \
	JyNI_HasJyAttribute(jyObject, JyAttributeJyGCHead)

#define JyObject_AddOrSetJyGCHead(pyObject, jyObject, head) \
	JyNI_AddOrSetJyAttributeWithFlags(jyObject, JyAttributeJyGCHead, head, \
			JY_ATTR_OWNS_VALUE_FLAG_MASK | JY_ATTR_JWEAK_VALUE_FLAG_MASK)

#define JyObject_AddJyGCHead(pyObject, jyObject, head) \
	JyNI_AddJyAttributeWithFlags(jyObject, JyAttributeJyGCHead, head, \
			JY_ATTR_OWNS_VALUE_FLAG_MASK | JY_ATTR_JWEAK_VALUE_FLAG_MASK)

#define JyObject_GetJyGCHead(pyObject, jyObject) \
	JyNI_GetJyAttribute(jyObject, JyAttributeJyGCHead)

#define JyObject_DelJyGCHead(pyObject, jyObject) \
	JyNI_ClearJyAttribute(jyObject, JyAttributeJyGCHead)

#define GC_UNEXPLORED _PyGC_REFS_UNEXPLORED
#define GC_EXPLORING _PyGC_REFS_EXPLORING
#define GC_EXPLORED _PyGC_REFS_EXPLORED

// Get an object's GC head
#define AS_GC(o) ((PyGC_Head *)(o)-1)

// Get the object given the GC head
#define FROM_GC(g) ((PyObject *)(((PyGC_Head *)g)+1))

#define IS_UNEXPLORED(op) \
	(!IsReadyType(op) && (!PyObject_IS_GC(op) || (AS_GC(op)->gc.gc_refs < 0 && AS_GC(op)->gc.gc_refs > GC_EXPLORING)))

#define UNKNOWN_FIXED_GC_SIZE -2

#include "JyRefMonitor.h"

#define JyObject_IS_GC(o) (((JyObject *) o)->flags & JY_GC_FLAG_MASK)
#define JyObject_IS_INITIALIZED(o) (((JyObject *) o)->flags & JY_INITIALIZED_FLAG_MASK)
#define JyObject_IS_TRUNCATED(o) ((((JyObject *) o)->flags & JY_TRUNCATE_FLAG_MASK) || PyFunction_Check(FROM_JY(o)))
#define PyObject_IS_TRUNCATED(o) ((AS_JY(o)->flags & JY_TRUNCATE_FLAG_MASK) || PyFunction_Check(o))
//#define JyObject_IS_TYPE(o) ((JyObject *) o)->flags & JY_TYPE_FLAG_MASK
//#define JyObject_IS_CPEER(o) (JyObject_IS_TYPE(o) == 0) && (((JyObject *) o)->flags & JY_CPEER_FLAG_MASK)
//#define JyObject_IS_CPEERTYPE(o) (((JyObject *) o)->flags & JY_CPEER_FLAG_MASK) && (JyObject_IS_TYPE(o))
#define JyObject_IS_CPEER(o) (((JyObject *) o)->flags & JY_CPEER_FLAG_MASK)
#define JyNI_FROM_GC(o) ((PyObject *)(((PyGC_Head *)o)+1))

#define _AS_JY(o) ((  (PyObject_IS_GC(o)) ? (JyObject *) _Py_AS_GC(o) : (JyObject *) (o)  )-1)
#define FROM_JY(o) ((JyObject_IS_GC(o)) ? JyNI_FROM_GC((((JyObject *)(o))+1)) : ((PyObject *)(((JyObject *)(o))+1)))
#define GC_FROM_JY(o) (PyGC_Head*) (((JyObject *)(o))+1)
#define FROM_JY_WITH_GC(o) (JyNI_FROM_GC((((JyObject *)(o))+1)))
#define FROM_JY_NO_GC(o) ((PyObject *)(((JyObject *)(o))+1))
#define _AS_JY_WITH_GC(o) ((JyObject *)(_Py_AS_GC(o))-1)
#define _AS_JY_NO_GC(o) (((JyObject *)(o))-1)

//Is_DynPtrPy(p)
//#define AS_JY(o) (Is_DynPtr(_AS_JY(o)) ? _AS_JY(o) : (puts("AS_JY-Warning:"), puts(__FUNCTION__), _AS_JY(o)))

//#define AS_JY(o) (Is_DynPtr(_AS_JY(o)) ? _AS_JY(o) : (printf("AS_JY-Warning: %s %i (%lld)\n", __FUNCTION__, __LINE__, o), _AS_JY(o)))
//#define AS_JY_WITH_GC(o) (Is_DynPtr(_AS_JY_WITH_GC(o)) ? _AS_JY_WITH_GC(o) : (printf("AS_JY-Warning: %s %i (%lld)\n", __FUNCTION__, __LINE__, o), _AS_JY_WITH_GC(o)))
//#define AS_JY_NO_GC(o) (Is_DynPtr(_AS_JY_NO_GC(o)) ? _AS_JY_NO_GC(o) : (printf("AS_JY-Warning: %s %i (%lld)\n", __FUNCTION__, __LINE__, o), _AS_JY_NO_GC(o)))

#define AS_JY(o) _AS_JY(o)
#define AS_JY_WITH_GC(o) _AS_JY_WITH_GC(o)
#define AS_JY_NO_GC(o) _AS_JY_NO_GC(o)

//#define AS_GC(o) ((PyGC_Head *)(o)-1)
//#define FROM_GC(g) ((PyObject *)(((PyGC_Head *)g)+1))
//#define GC_UNEXPLORED _PyGC_REFS_UNEXPLORED
//#define GC_EXPLORING _PyGC_REFS_EXPLORING
//#define GC_EXPLORED _PyGC_REFS_EXPLORED
//#define IS_UNEXPLORED(op) \
//	(!IsReadyType(op) && (!PyObject_IS_GC(op) || (AS_GC(op)->gc.gc_refs < 0 && AS_GC(op)->gc.gc_refs > GC_EXPLORING)))

#define JySYNC_ON_INIT_FLAGS (SYNC_ON_PY_INIT_FLAG_MASK | SYNC_ON_JY_INIT_FLAG_MASK)
#define Jy_InitImmutable(jyObj) \
	(jyObj)->flags = JySYNC_ON_INIT_FLAGS; \
	(jyObj)->attr = NULL; \
	(jyObj)->jy = NULL

#define JyNI_InitSingleton(cSingle, jSingle) \
	AS_JY_NO_GC(cSingle)->flags |= JY_CACHE_ETERNAL_FLAG_MASK | JY_INITIALIZED_FLAG_MASK; \
	AS_JY_NO_GC(cSingle)->jy = jSingle

#define JyNI_InitSingletonGC(cSingle, jSingle) \
	AS_JY_WITH_GC(cSingle)->flags |= JY_GC_FLAG_MASK | JY_CACHE_ETERNAL_FLAG_MASK | JY_INITIALIZED_FLAG_MASK; \
	AS_JY_WITH_GC(cSingle)->jy = jSingle


/* Call-ins: */
jobject JyNI_loadModule(JNIEnv *env, jclass class, jstring moduleName, jstring modulePath, jlong tstate);
jint JyNI_init(JavaVM *jvm);
void JyNI_unload(JavaVM *jvm);
void JyNI_clearPyCPeer(JNIEnv *env, jclass class, jlong objectHandle, jlong refHandle);
void JyNI_JyNIDebugMessage(JNIEnv *env, jclass class, jlong mode, jlong value, jstring msg);
jobject JyNI_callPyCPeer(JNIEnv *env, jclass class, jlong peerHandle, jobject args, jobject kw, jlong tstate);
jobject JyNI_getAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jlong tstate);
jint JyNI_setAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jobject value, jlong tstate);
jobject JyNI_repr(JNIEnv *env, jclass class, jlong handle, jlong tstate);
jstring JyNI_PyObjectAsString(JNIEnv *env, jclass class, jlong handle, jlong tstate);
jobject JyNI_PyObjectAsPyString(JNIEnv *env, jclass class, jlong handle, jlong tstate);
jobject JyNIlookupFromHandle(JNIEnv *env, jclass class, jlong handle);
jint JyNIcurrentNativeRefCount(JNIEnv *env, jclass class, jlong handle);
void JyNI_nativeIncref(jlong handle, jlong tstate);
void JyNI_nativeDecref(jlong handle, jlong tstate);
jstring JyNIgetNativeTypeName(JNIEnv *env, jclass class, jlong handle);
//In gcmodule (declared here to preserve original gcmodule.h):
jboolean JyGC_clearNativeReferences(JNIEnv *env, jclass class, jlongArray references, jlong tstate);
void JyGC_restoreCStubBackend(JNIEnv *env, jclass class, jlong handle, jobject backend, jobject newHead);
//jlongArray JyGC_validateGCHead(JNIEnv *env, jclass class, jlong handle, jlongArray oldLinks);
jboolean JyGC_validateGCHead(JNIEnv *env, jclass class, jlong handle, jlongArray oldLinks);
jlongArray JyGC_nativeTraverse(JNIEnv *env, jclass class, jlong handle);
void JyNI_releaseWeakReferent(JNIEnv *env, jclass class, jlong handle, jlong tstate);
jobject JyNI_getItem(JNIEnv *env, jclass class, jlong handle, jobject key, jlong tstate);
jint JyNI_setItem(JNIEnv *env, jclass class, jlong handle, jobject key, jobject value, jlong tstate);
jint JyNI_delItem(JNIEnv *env, jclass class, jlong handle, jobject key, jlong tstate);
jint JyNI_PyObjectLength(JNIEnv *env, jclass class, jlong handle, jlong tstate);
jobject JyNI_descr_get(jlong self, jobject obj, jobject type, jlong tstate);
jint JyNI_descr_set(jlong self, jobject obj, jobject value, jlong tstate);
jint JyNI_PyObject_Compare(jlong handle, jobject o, jlong tstate);
jobject JyNI_PyObject_RichCompare(jlong handle, jobject o, jint op, jlong tstate);
jobject JyNI_PyObject_GetIter(jlong handle, jlong tstate);
jobject JyNI_PyIter_Next(jlong handle, jlong tstate);

// PyCFunction call-ins:
jobject JyNI_PyCFunction_getSelf(jlong handle, jlong tstate);
jobject JyNI_PyCFunction_getModule(jlong handle, jlong tstate);
jobject JyNI_CMethodDef_bind(jlong handle, jobject bindTo, jlong tstate);

// PyNumber call-ins:
jobject  JyNI_PyNumber_Add(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Subtract(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Multiply(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Divide(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Remainder(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Divmod(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Power(jlong o1, jobject o2, jobject o3, jlong tstate);
jobject  JyNI_PyNumber_Negative(jlong o, jlong tstate);
jobject  JyNI_PyNumber_Positive(jlong o, jlong tstate);
jobject  JyNI_PyNumber_Absolute(jlong o, jlong tstate);
jboolean JyNI_PyNumber_NonZero(jlong o, jlong tstate);
jobject  JyNI_PyNumber_Invert(jlong o, jlong tstate);
jobject  JyNI_PyNumber_Lshift(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Rshift(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_And(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Xor(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Or(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Coerce(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Int(jlong o1, jlong tstate);
jobject  JyNI_PyNumber_Long(jlong o1, jlong tstate);
jobject  JyNI_PyNumber_Float(jlong o1, jlong tstate);
jobject  JyNI_PyNumber_Oct(jlong o1, jlong tstate);
jobject  JyNI_PyNumber_Hex(jlong o1, jlong tstate);
jobject  JyNI_PyNumber_InPlaceAdd(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceSubtract(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceMultiply(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceDivide(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceRemainder(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlacePower(jlong o1, jobject o2, jobject o3, jlong tstate);
jobject  JyNI_PyNumber_InPlaceLshift(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceRshift(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceAnd(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceXor(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceOr(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_FloorDivide(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_TrueDivide(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceFloorDivide(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_InPlaceTrueDivide(jlong o1, jobject o2, jlong tstate);
jobject  JyNI_PyNumber_Index(jlong o, jlong tstate);

// PySequence call-ins:
jint    JyNI_PySequence_Length(jlong o, jlong tstate);
jobject JyNI_PySequence_Concat(jlong o1, jobject o2, jlong tstate);
jobject JyNI_PySequence_Repeat(jlong o, jint l, jlong tstate);
jobject JyNI_PySequence_GetItem(jlong o, jint l, jlong tstate);
jobject JyNI_PySequence_GetSlice(jlong o, jint l1, jint l2, jlong tstate);
jint    JyNI_PySequence_SetItem(jlong o1, jint l, jobject o2, jlong tstate);
jint    JyNI_PySequence_SetSlice(jlong o1, jint l1, jint l2, jobject o2, jlong tstate);
jint    JyNI_PySequence_Contains(jlong o1, jobject o2, jlong tstate);
jobject JyNI_PySequence_InPlaceConcat(jlong o1, jobject o2, jlong tstate);
jobject JyNI_PySequence_InPlaceRepeat(jlong o, jint l, jlong tstate);

// PyMapping call-ins:
jint    JyNI_PyMapping_Length(jlong o, jlong tstate);
//jobject JyNI_PyMapping_Subscript(jlong o1, jobject o2, jlong tstate);
//jint    JyNI_PyMapping_AssSubscript(jlong o1, jobject o2, jobject o3, jlong tstate);


#define builtinTypeCount 42
extern TypeMapEntry builtinTypes[builtinTypeCount];
#define TME_INDEX_Type                   0
#define TME_INDEX_NotImplemented         1
#define TME_INDEX_None                   2
#define TME_INDEX_File                   3
#define TME_INDEX_Module                 4
#define TME_INDEX_Cell                   5
#define TME_INDEX_Class                  6
#define TME_INDEX_Instance               7
#define TME_INDEX_Method                 8
#define TME_INDEX_Function               9
#define TME_INDEX_ClassMethod           10
#define TME_INDEX_StaticMethod          11
#define TME_INDEX_CFunction             12
#define TME_INDEX_MethodDescr           13
#define TME_INDEX_ClassMethodDescr      14
#define TME_INDEX_DictProxy             15
#define TME_INDEX_Property              16
#define TME_INDEX_Bool                  17
#define TME_INDEX_Float                 18
#define TME_INDEX_Int                   19
#define TME_INDEX_Long                  20
#define TME_INDEX_Complex               21
#define TME_INDEX_Unicode               22
#define TME_INDEX_String                23
#define TME_INDEX_Weakref_Ref           24
#define TME_INDEX_Weakref_Proxy         25
#define TME_INDEX_Weakref_CallableProxy 26
//#define TME_INDEX_BaseString          26
#define TME_INDEX_SeqIter               27
//#define TME_INDEX_Range               28
//#define TME_INDEX_rangeiter           29
#define TME_INDEX_Tuple                 28
//#define TME_INDEX_TupleIter           28
#define TME_INDEX_List                  29
//#define TME_INDEX_ListIter            30
//#define TME_INDEX_ListRevIter         31
#define TME_INDEX_Dict                  30
#define TME_INDEX_StringMap             31
#define TME_INDEX_Set                   32
//#define TME_INDEX_SetIter             34
#define TME_INDEX_FrozenSet             33
//#define TME_INDEX_Enum                36
#define TME_INDEX_Slice                 34
#define TME_INDEX_Ellipsis              35
//#define TME_INDEX_Gen                 39
#define TME_INDEX_Code_Bytecode         36
#define TME_INDEX_Code_Tablecode        37
#define TME_INDEX_Frame                 38
//#define TME_INDEX_Super               43
#define TME_INDEX_Exc_BaseException     39
#define TME_INDEX_TraceBack             40
//#define TME_INDEX_ByteArray           46
//#define TME_INDEX_Buffer              47
//#define TME_INDEX_MemoryView          48
//#define TME_INDEX_CallIter            50
#define TME_INDEX_BaseObject            41 //must be last type in list

/* "Hidden" PyTypes: */
extern PyTypeObject PyNone_Type;
extern PyTypeObject PyNotImplemented_Type;
extern PyTypeObject Pyrangeiter_Type; /* jython uses PySequenceIter. */
extern PyTypeObject PyTupleIter_Type; /* jython uses PyFastSequenceIter. */
extern PyTypeObject PyListIter_Type; /* jython uses PyFastSequenceIter. */
extern PyTypeObject PyListRevIter_Type; /* jython uses PyReversedSequenceIter. */
extern PyTypeObject PySetIter_Type; /* jython uses inline subclass of PyIterator. */
extern PyTypeObject PyMethodDescr_Type;
extern PyTypeObject PyClassMethodDescr_Type;
extern PyTypeObject PyTraceBack_Type;
extern PyTypeObject Long_InfoType;
extern PyTypeObject FloatInfoType;
extern PyTypeObject EncodingMapType;
extern PyTypeObject wrappertype;
extern PyTypeObject cmpwrapper_type;
extern PyTypeObject sortwrapper_type;

/* Type-Lookup: */
inline jboolean JyNI_IsBuiltinPyType(PyTypeObject* type);
inline jclass JyNI_JythonClassFromPyType(PyTypeObject* type);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromPyType(PyTypeObject* type);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromSubType(PyTypeObject* type);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromSubTypeWithPeer(PyTypeObject* type);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromJythonPyClass(jclass jythonPyClass);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromName(char* name);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromJStringName(jstring name);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromJythonPyType(jobject jythonPyType);
inline ExceptionMapEntry* JyNI_PyExceptionMapEntry_FromPyExceptionType(PyTypeObject* excType);
inline jobject JyNI_JythonExceptionType_FromPyExceptionType(PyObject* exc);

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyTypeObject* JyNI_PyExceptionType_FromJythonExceptionType(jobject exc);

/* Conversion-Stuff: */

/*
 * Returns a jobject as a new JNI-local reference, unless op has a
 * cache-flag set. The caller should call deleteLocalRef on it after
 * work was done. In case the reference must be stored or cached for
 * future used, NewGlobalRef must be used.
 */
inline jobject JyNI_JythonPyObject_FromPyObject(PyObject* op);

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyObject* JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject);

//inline PyObject* JyNI_PyObject_FromJythonPyObject_verbose(jobject jythonPyObject);

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* _JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject, jboolean lookupNative, jboolean checkCPeer, jboolean checkForType);

inline void JyNI_SyncPyCPeerTypeMRO(PyTypeObject* type, jobject jtype);
inline jobject JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type);
inline jobject _JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type, jclass cls);

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyTypeObject* JyNI_PyTypeObject_FromJythonPyTypeObject(jobject jythonPyTypeObject);

inline jstring JyNI_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op);
inline jstring JyNI_interned_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op);

/* Exception-Stuff: */
void JyErr_SetFromJNIEnv();

/* JyNI-Stuff: */
inline PyObject* JyNI_PyObject_Call(jobject func, PyObject *arg, PyObject *kw);
inline jint JyNI_GetDLOpenFlags();
inline void JyNI_CleanUp_JyObject(JyObject* obj);
inline jobject JyNI_GetJythonDelegate(PyObject* v);
inline void JyNI_printJInfo(jobject obj);
inline void JyNI_jprintHash(jobject obj);
inline void JyNI_printHash(jobject obj);
inline void JyNI_jprintJ(jobject obj);

/* To save lookups: */
inline void _PyObject_GC_InitJy(PyObject *op, TypeMapEntry* tme);
PyObject* _JyObject_GC_New(PyTypeObject *tp, TypeMapEntry* tme);
PyVarObject* _JyObject_GC_NewVar(PyTypeObject *tp, Py_ssize_t nitems, TypeMapEntry* tme);
inline void _PyObject_InitJy(PyObject *op, TypeMapEntry* tme);
inline PyObject * _JyObject_New(PyTypeObject *tp, TypeMapEntry* tme);
jobject _PyImport_LoadDynamicModuleJy(char *name, char *pathname, FILE *fp);
inline int PyModule_AddStringConstantJy(jobject m, const char *name, const char *value);
inline int PyModule_AddObjectJy(jobject m, const char *name, jobject o);
inline int _PyObject_Compare(PyObject *v, PyObject *w);

/* Hashmap stuff */
inline void JyHash_init();
inline void JyHash_printTable();
inline int JyHash_insert(void* ptr);
inline int JyHash_contains(void* ptr);
inline int JyHash_delete(void* ptr);

/* JyNI specific:
 * Backdoor to reach original alloc-functions, which were renamed with "Raw"-prefix: */
PyAPI_FUNC(void *) PyObject_RawMalloc(size_t);
PyAPI_FUNC(void *) PyObject_RawRealloc(void *, size_t);
PyAPI_FUNC(void) PyObject_RawFree(void *);

/* JyNI-CG: */
//some error-int-return constants:
#define GC_OBJECT_UNEXPLORED  -4
#define GC_OBJECT_NOHEAD      -5
#define GC_OBJECT_INVALIDHEAD -6
#define GC_OBJECT_JNIFAIL     -7
void JyNI_GC_Explore();
void JyNI_GC_ExploreObject(PyObject* op);
jboolean JyNI_GC_EnsureHeadObject(JNIEnv* env, PyObject* op, JyObject* jy);
jobject JyNI_GC_ObtainJyGCHead(JNIEnv* env, PyObject* op, JyObject* jy);
void JyNI_GC_Track_CStub(PyObject* op);
void PyObject_GC_Track_NoExplore(void *op);
int updateJyGCHeadLink(PyObject* op, JyObject* jy, jsize index,
		PyObject* newItem, JyObject* newItemJy);
int updateClearJyGCHeadLinks(PyObject* op, JyObject* jy, jsize startIndex);
int updateInsertJyGCHeadLink(PyObject* op, JyObject* jy, jsize index,
		PyObject* newItem, JyObject* newItemJy);
int updateJyGCHeadLinks(PyObject* op, JyObject* jy);
inline int getWeakRefCount(JyObject* referent);
inline int incWeakRefCount(JyObject* referent);
inline int decWeakRefCount(JyObject* referent);

/* Provide header for nullstring from stringobject.c
 * This way, the nullstring can also be used from other
 * places.
 * Note that nullstring and unicode_empty are initialized
 * on demand. One has to check them to be non-NULL before use.
 */
extern PyStringObject *nullstring;
extern PyUnicodeObject *unicode_empty;

/* Other hidden stuff: */
typedef struct {
	PyObject_HEAD
	PyObject *cm_callable;
} classmethod;

typedef struct {
	PyObject_HEAD
	PyObject *sm_callable;
} staticmethod;

typedef struct {
	PyObject_HEAD
	PyObject *dict;
} proxyobject;

typedef struct {
	PyObject_HEAD
	PyObject *prop_get;
	PyObject *prop_set;
	PyObject *prop_del;
	PyObject *prop_doc;
	int getter_doc;
} propertyobject;

/* Some caches to be considered for eternal caching: */
/* (These macros overwrite those in intobject.c.) */
#define NSMALLPOSINTS           257
#define NSMALLNEGINTS           5
#define NSMALLPOSINTSJYTHON     900
#define NSMALLNEGINTSJYTHON     100
#define LETTERCHAR_MAXJYTHON    256
extern JyIntObject *small_ints[NSMALLNEGINTS + NSMALLPOSINTS];
extern PyStringObject *characters[UCHAR_MAX + 1];
extern PyUnicodeObject *unicode_latin1[256];
//extern PyFloatBlock *block_list;

/* Load- and unload-stuff from Python-files, missing in the headers currently used: */
int _PyInt_Init(void);
void PyString_Fini(void);
void PyInt_Fini(void);
void PyTuple_Fini(void);
void PyDict_Fini(void);
void PyCFunction_Fini(void);
void PySet_Fini(void);

/*
inline void JyNI_SetUpJyObject(JyObject* type);
//inline void JyNI_SetUpJyVarObject(JyVarObject* type);
inline void JyNI_CleanUpJyObject(JyObject* peer);
//inline void JyNI_CleanUpJyVarObject(JyVarObject* peer);

inline PyObject* JyNI_NewPyObject_FromJythonPyObject(jobject jythonPyObject);

//PyObject* PyObject_FromJythonJyObject(jobject jythonJyObject);
 */

/* JyAttribute management: */
#define JyNI_ClearJyAttributes(obj) \
		_JyNI_ClearJyAttributes(obj)
		//(jputs(__FUNCTION__), jputsLong(__LINE__), _JyNI_ClearJyAttributes(obj))
inline void _JyNI_ClearJyAttributes(JyObject* obj);
inline void JyNI_ClearJyAttribute(JyObject* obj, const char* name);
inline void JyNI_ClearJyAttributeValue(JyAttribute* att);
#define JyNI_GetJyAttribute(obj, name) \
		_JyNI_GetJyAttribute(obj, name)
		//(jputs(__FUNCTION__), jputsLong(__LINE__), _JyNI_GetJyAttribute(obj, name))
inline void* _JyNI_GetJyAttribute(JyObject* obj, const char* name);
inline void JyNI_AddJyAttribute(JyObject* obj, const char* name, void* value);
inline void JyNI_AddJyAttributeWithFlags(JyObject* obj, const char* name, void* value, char flags);
inline void JyNI_AddOrSetJyAttribute(JyObject* obj, const char* name, void* value);
inline void JyNI_AddOrSetJyAttributeWithFlags(JyObject* obj, const char* name, void* value, char flags);
inline jboolean JyNI_HasJyAttribute(JyObject* obj, const char* name);

//extern jlong JyNIDebugMode;

PyObject* PyMemoryView_FromObject(PyObject *base);

#endif /* JYNI_H_ */
