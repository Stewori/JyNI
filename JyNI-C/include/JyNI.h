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
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
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
//#include <Python_JyNI.h>
#include <Python_JyNI.h>
//#include <JyNI_JyNI.h>
#include "JythonSite.h"
#include "JyList.h"
#include "JyTState.h"


/* JNI-Shortcuts: */

/* Shortcut to get JNI-environment, including check for whether
 * JNI version is supported. Note that we don't put a semicolon
 * to the end, so the macro-call must be succeeded by a semicolon,
 * giving it a more natural look...
 */
#define env(errRet) \
	JNIEnv *env;\
	if ((*java)->GetEnv(java, (void **)&env, JNI_VERSION_1_2))\
		return errRet

/* For now we assume, nobody would cache _PyThreadState_Current for
 * use after the current method returns. So we need not acquire a
 * global reference.
 */
#define ENTER_JyNI \
	PyEval_AcquireLock(); \
	if (_PyThreadState_Current != NULL) Py_FatalError("ENTER_JyNI: overwriting non-NULL tstate"); \
	_PyThreadState_Current = tstate;

#define LEAVE_JyNI0 \
	JyNI_GC_Explore(); \
	_PyThreadState_Current = NULL; \
	PyEval_ReleaseLock();

#define LEAVE_JyNI \
	if (PyErr_Occurred()) JyErr_InsertCurExc(); \
	LEAVE_JyNI0

/* Cleanly convert a jstring to a cstring with minimal JVM lock-time.
 * Use only once per Function. For further conversions use
 * cstr_from_jstring2. Note that at least one call of "env()" must
 * have happened before in the same block or in some parent block.
 * ("+1" in 3rd line is for 0-termination)
 */
#define cstr_from_jstring(cstrName, jstr) \
	char* utf_string = (*env)->GetStringUTFChars(env, jstr, NULL); \
	char cstrName[strlen(utf_string)+1]; \
	strcpy(cstrName, utf_string); \
	(*env)->ReleaseStringUTFChars(env, jstr, utf_string)

#define global_cstr_from_jstring(cstrName, jstr) \
	char* utf_string = (*env)->GetStringUTFChars(env, jstr, NULL); \
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
 * manage Types as truncated, if there are no macros in the official
 * Python-headers that directly modify the structure.
 * Additionally flags tell us whether a PyGC_Head is present (because
 * otherwise, it would be very tedious to write a macro like FROM_JY(o)).
 * They also tell whether jy refers to a PyCPeer, saving some
 * frequent type checking JNI-calls.
 *
 * The INITIALIZED-flag indicates whether a java-side object is already
 * prepared. Note that jy == NULL is no indication for this, because
 * jy may be pre-initialized with the corresponding TypeMapEntry,
 * saving a lookup at allocation.
 */

/* General Flags: */
#define JY_INITIALIZED_FLAG_MASK    1
#define JY_HAS_JHANDLE_FLAG_MASK    2
#define JY_GC_FLAG_MASK             4
#define JY_TRUNCATE_FLAG_MASK       8
//#define JY_PARTLY_TRUNCATE_MASK   8 (deprecated; indicated by JY_TRUNCATE_FLAG_MASK + non-zero truncate_trailing)
#define JY_CPEER_FLAG_MASK         16
#define JY_CACHE_GC_FLAG_MASK      32
#define JY_CACHE_ETERNAL_FLAG_MASK 64
//#define JY_GC_SINGLE_LINK        64
#define JY_GC_VAR_SIZE            128 /* This distinguishes array-like vs list-like links. */
#define JY_CACHE                   96 /* JY_CACHE_GC_FLAG_MASK | JY_CACHE_ETERNAL_FLAG_MASK */
//#define JY_GC_SPECIAL_CASE        192 /* JY_GC_SINGLE_LINK | JY_GC_FIXED_SIZE */
//#define JY_GC_VAR_SIZE              0 /* Default if JY_GC_FLAG_MASK is active. Just intended as a marker. */

#define Is_StaticSingleton(pyObject) \
	(pyObject == Py_None || pyObject == Py_Ellipsis || pyObject == Py_NotImplemented)

#define Is_StaticTypeObject(pyObject) \
	(PyType_Check(pyObject) && !PyType_HasFeature(Py_TYPE(pyObject), Py_TPFLAGS_HEAPTYPE))

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

/* GC-macro-replacements */
#define _JyNI_GC_TRACK(o) PyObject_GC_Track(o) //_PyObject_GC_TRACK(o)
#define _JyNI_GC_TRACK_NoExplore(o) PyObject_GC_Track_NoExplore(o)
#define _JyNI_GC_UNTRACK(o) PyObject_GC_UnTrack(o) //_PyObject_GC_UNTRACK(o)
#define _JyNI_GC_IS_TRACKED(o) _PyObject_GC_IS_TRACKED(o)
#define _JyNI_GC_MAY_BE_TRACKED(obj) _PyObject_GC_MAY_BE_TRACKED(obj)

/* Additional values for _PyGC_REFS. For consistency we also list the orginal
 * values here:
 * #define _PyGC_REFS_UNTRACKED                    (-2)
 * #define _PyGC_REFS_REACHABLE                    (-3)
 * #define _PyGC_REFS_TENTATIVELY_UNREACHABLE      (-4)
 */
#define _PyGC_REFS_UNEXPLORED                      (-5)
#define _PyGC_REFS_EXPLORING                       (-6)
#define _PyGC_REFS_EXPLORED                        (-7)

/* define some method-signatures for sync purposes: */

/* jobject is src, PyObject* is dest. Src must not be modified. */
typedef void (*jy2pySync)(jobject, PyObject*);

/* PyObject* is src, jobject is dest. Src must not be modified. */
typedef void (*py2jySync)(PyObject*, jobject);

typedef jobject (*jyInitSync)(PyObject*);
typedef PyObject* (*pyInitSync)(jobject);
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
typedef struct {jy2pySync jy2py; py2jySync py2jy; jyInitSync jyInit; pyInitSync pyInit; pyChecksum pyCheck; jyChecksum jyCheck;} SyncFunctions;
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
 * Currently the only possible attributes are the following:
 */
extern const char* JyAttributePyChecksum;
extern const char* JyAttributeJyChecksum;
extern const char* JyAttributeSyncFunctions;
extern const char* JyAttributeModuleFile;
extern const char* JyAttributeModuleName;
extern const char* JyAttributeTypeName;
extern const char* JyAttributeStringInterned;
extern const char* JyAttributeSetEntry;
extern const char* JyAttributeJyGCHead;
//extern const char* JyAttributeTruncateSize;
#define JY_ATTR_OWNS_VALUE_FLAG_MASK 1
#define JY_ATTR_VAR_SIZE_FLAG_MASK 2
#define JY_ATTR_JWEAK_VALUE_FLAG_MASK 4
typedef struct JyAttribute JyAttribute; /* Forward declaration */
struct JyAttribute { const char* name; void* value; char flags; JyAttribute* next;};
typedef struct JyAttributeElement JyAttributeElement; /* Forward declaration */
struct JyAttributeElement {void* value; JyAttributeElement* next;};
typedef struct { jweak jy; unsigned short flags; JyAttribute* attr;} JyObject;
typedef struct { JyObject jy; PyIntObject pyInt;} JyIntObject; /* only used for pre-allocated blocks */
typedef struct { JyObject jy; PyFloatObject pyFloat;} JyFloatObject;  /* only used for pre-allocated blocks */
/* type_name is optional and defaults to py_type->tp_name */
typedef struct { PyTypeObject* py_type; jclass jy_class; unsigned short flags; SyncFunctions* sync; size_t truncate_trailing; char* type_name;} TypeMapEntry;
typedef struct { PyTypeObject* exc_type; jyFactoryMethod exc_factory;} ExceptionMapEntry;

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

#define UNKNOWN_FIXED_GC_SIZE -2

#include "JyRefMonitor.h"

#define JyObject_IS_GC(o) (((JyObject *) o)->flags & JY_GC_FLAG_MASK)
#define JyObject_IS_INITIALIZED(o) (((JyObject *) o)->flags & JY_INITIALIZED_FLAG_MASK)
#define JyObject_IS_TRUNCATED(o) (((JyObject *) o)->flags & JY_TRUNCATE_FLAG_MASK)
//#define JyObject_IS_TYPE(o) ((JyObject *) o)->flags & JY_TYPE_FLAG_MASK
//#define JyObject_IS_CPEER(o) (JyObject_IS_TYPE(o) == 0) && (((JyObject *) o)->flags & JY_CPEER_FLAG_MASK)
//#define JyObject_IS_CPEERTYPE(o) (((JyObject *) o)->flags & JY_CPEER_FLAG_MASK) && (JyObject_IS_TYPE(o))
#define JyObject_IS_CPEER(o) (((JyObject *) o)->flags & JY_CPEER_FLAG_MASK)
#define JyNI_FROM_GC(o) ((PyObject *)(((PyGC_Head *)o)+1))

#define AS_JY(o) ((JyObject *)(  (PyObject_IS_GC(o)) ? _Py_AS_GC(o) : (o)  )-1)
#define FROM_JY(o) ((JyObject_IS_GC(o)) ? JyNI_FROM_GC((((JyObject *)(o))+1)) : ((PyObject *)(((JyObject *)(o))+1)))
#define GC_FROM_JY(o) (PyGC_Head*) (((JyObject *)(o))+1)
#define FROM_JY_WITH_GC(o) (JyNI_FROM_GC((((JyObject *)(o))+1)))
#define FROM_JY_NO_GC(o) ((PyObject *)(((JyObject *)(o))+1))
#define AS_JY_WITH_GC(o) ((JyObject *)(_Py_AS_GC(o))-1)
#define AS_JY_NO_GC(o) ((JyObject *)(o)-1)

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
//In gcmodule (declared here to preserve original gcmodule.h):
void JyGC_clearNativeReferences(JNIEnv *env, jclass class, jlongArray references, jlong tstate);

#define builtinTypeCount 46
extern TypeMapEntry builtinTypes[builtinTypeCount];

/* "Hidden" PyTypes: */
extern PyTypeObject PyNone_Type;
extern PyTypeObject PyNotImplemented_Type;
extern PyTypeObject Pyrangeiter_Type; /* jython uses PySequenceIter. */
extern PyTypeObject PyTupleIter_Type; /* jython uses PyFastSequenceIter. */
extern PyTypeObject PyListIter_Type; /* jython uses PyFastSequenceIter. */
extern PyTypeObject PyListRevIter_Type; /* jython uses PyReversedSequenceIter. */
extern PyTypeObject PySetIter_Type; /* jython uses inline subclass of PyIterator. */
//extern PyTypeObject PyMethodDescr_Type;
//extern PyTypeObject PyClassMethodDescr_Type;
extern PyTypeObject PyTraceBack_Type;

/* Type-Lookup: */
inline jboolean JyNI_IsBuiltinPyType(PyTypeObject* type);
inline jclass JyNI_JythonClassFromPyType(PyTypeObject* type);
inline TypeMapEntry* JyNI_JythonTypeEntry_FromPyType(PyTypeObject* type);
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

inline jobject JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type);
inline jobject _JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type, jclass cls);

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
inline PyTypeObject* JyNI_PyTypeObject_FromJythonPyTypeObject(jobject jythonPyTypeObject);

inline jstring JyNI_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op);
inline jstring JyNI_interned_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op);

//Use PyErr_* methods with converted PyObject*-Exception instead!
/* Exception-Stuff: */
//inline void JyNI_JyErr_SetString(jobject exception, const char *string);
//inline void JyNI_JyErr_SetObject(jobject exception, PyObject *value);
//inline void JyNI_JyErr_SetExceptionObject(jobject exception);
//inline int JyNI_JyErr_ExceptionMatches(jobject exc);
//inline PyObject* JyNI_JyErr_Format(jobject exception, const char *format, ...);

/* JyNI-Stuff: */
//inline PyObject* JyNI_GenericAlloc(PyTypeObject* type, Py_ssize_t nitems);
inline PyObject* JyNI_Alloc(TypeMapEntry* tme);
inline PyObject* JyNI_AllocVar(TypeMapEntry* tme, Py_ssize_t nitems);
inline PyObject* JyNI_AllocNative(PyTypeObject* type);
inline PyObject* JyNI_AllocNativeVar(PyTypeObject* type, Py_ssize_t nitems);
inline PyObject* JyNI_ExceptionAlloc(ExceptionMapEntry* eme);
inline jint JyNI_GetDLOpenFlags();
inline void JyNI_CleanUp_JyObject(JyObject* obj);
inline jobject JyNI_GetJythonDelegate(PyObject* v);
//Should be used as replacement for PyObject_Del, taking care of JyObjects:
//void JyNI_Del(void * obj);
//inline void JyNI_Py_CLEAR(jobject obj);
//inline char* PyLongToJavaSideString(PyObject* pl);
//inline void JyNI_printJ(jobject obj);
inline void JyNI_printJInfo(jobject obj);
inline void JyNI_jprintHash(jobject obj);
inline void JyNI_jprintJ(jobject obj);
inline void jputs(const char* msg);
inline void jputsLong(jlong val);

/* To save lookups: */
PyObject* _JyObject_GC_New(PyTypeObject *tp, TypeMapEntry* tme);
PyVarObject* _JyObject_GC_NewVar(PyTypeObject *tp, Py_ssize_t nitems, TypeMapEntry* tme);
inline PyObject * _JyObject_New(PyTypeObject *tp, TypeMapEntry* tme);
jobject _PyImport_LoadDynamicModuleJy(char *name, char *pathname, FILE *fp);
inline int PyModule_AddStringConstantJy(jobject m, const char *name, const char *value);
inline int PyModule_AddObjectJy(jobject m, const char *name, jobject o);

/* JyNI specific:
 * Backdoor to reach original alloc-functions, which were renamed with "Raw"-prefix: */
PyAPI_FUNC(void *) PyObject_RawMalloc(size_t);
PyAPI_FUNC(void *) PyObject_RawRealloc(void *, size_t);
PyAPI_FUNC(void) PyObject_RawFree(void *);

/* JyNI-CG: */
void JyNI_GC_Explore();
void JyNI_GC_ExploreObject(PyObject* op);
void PyObject_GC_Track_NoExplore(void *op);
//int updateJyGCHeadLink(JNIEnv* env, PyObject* op, JyObject* jy, jsize index,
//		PyObject* newItem, JyObject* newItemJy);
//void updateJyGCHeadLinks(JNIEnv* env, PyObject* op, JyObject* jy);

/* Provide header for nullstring from stringobject.c
 * This way, the nullstring can also be used from other
 * places.
 * Note that nullstring and unicode_empty are initialized
 * on demand. One has to check them to be non-NULL before use.
 */
extern PyStringObject *nullstring;
extern PyUnicodeObject *unicode_empty;

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
inline void JyNI_ClearJyAttributes(JyObject* obj);
inline void JyNI_ClearJyAttribute(JyObject* obj, const char* name);
inline void JyNI_ClearJyAttributeValue(JyAttribute* att);
inline void* JyNI_GetJyAttribute(JyObject* obj, const char* name);
inline void JyNI_AddJyAttribute(JyObject* obj, const char* name, void* value);
inline void JyNI_AddJyAttributeWithFlags(JyObject* obj, const char* name, void* value, char flags);
inline void JyNI_AddOrSetJyAttribute(JyObject* obj, const char* name, void* value);
inline void JyNI_AddOrSetJyAttributeWithFlags(JyObject* obj, const char* name, void* value, char flags);
inline jboolean JyNI_HasJyAttribute(JyObject* obj, const char* name);


/* JNI IDs for Jython-stuff: */

/* singletons: */
extern JavaVM* java;
extern jweak length0StringArray;
extern jweak length0PyObjectArray;

extern jclass objectClass;
extern jmethodID objectToString;
extern jmethodID objectGetClass;

extern jclass classClass;
extern jmethodID classEquals;

extern jclass arrayListClass;
extern jmethodID arrayListConstructor;
extern jmethodID listAdd;

//extern jclass systemClass;
//extern jmethodID arraycopy;

extern jclass stringClass;
extern jmethodID stringToUpperCase;
extern jmethodID stringFromBytesAndCharsetNameConstructor;
extern jmethodID stringGetBytesUsingCharset;
extern jmethodID stringIntern;

extern jclass bigIntClass;
extern jmethodID bigIntFromStringConstructor;
extern jmethodID bigIntFromStringRadixConstructor;
extern jmethodID bigIntegerFromByteArrayConstructor;
extern jmethodID bigIntegerFromSignByteArrayConstructor;
extern jmethodID bigIntToByteArray;
extern jmethodID bigIntSignum;
extern jmethodID bigIntToStringRadix;

extern jclass JyNIClass;
extern jmethodID JyNISetNativeHandle;
//extern jmethodID JyNIRegisterNativeStaticTypeDict;
extern jmethodID JyNI_registerNativeStaticJyGCHead;
extern jmethodID JyNI_getNativeStaticJyGCHead;
extern jmethodID JyNILookupNativeHandle;
extern jmethodID JyNIClearNativeHandle;
extern jmethodID JyNILookupCPeerFromHandle;
extern jmethodID JyNIGetDLOpenFlags;
extern jmethodID JyNIGetDLVerbose;
extern jmethodID JyNIGetJyObjectByName;
extern jmethodID JyNIGetPyObjectByName;
extern jmethodID JyNI_PyImport_FindExtension;
extern jmethodID JyNIGetPyType;
extern jmethodID JyNI_getNativeAvailableKeysAndValues;
extern jmethodID JyNIGetPyDictionary_Next;
extern jmethodID JyNIGetPySet_Next;
extern jmethodID JyNIPyImport_GetModuleDict;
extern jmethodID JyNIPyImport_AddModule;
extern jmethodID JyNIJyNI_GetModule;
extern jmethodID JyNISlice_compare;
extern jmethodID JyNIPrintPyLong;
extern jmethodID JyNILookupNativeHandles;
extern jmethodID JyNI_prepareKeywordArgs;
extern jmethodID JyNI_getCurrentThreadID;
extern jmethodID JyNI_pyCode_co_code;
extern jmethodID JyNI_pyCode_co_flags;
extern jmethodID JyNI_pyCode_co_lnotab;
extern jmethodID JyNI_jPrint;
extern jmethodID JyNI_jPrintLong;
extern jmethodID JyNI_jPrintHash;
//extern jmethodID JyNIPySet_pop;
extern jmethodID JyNI_makeGCHead;
extern jmethodID JyNI_makeStaticGCHead;

extern jclass JyTStateClass;
extern jmethodID JyTState_setRecursionLimit;
extern jfieldID JyTState_nativeRecursionLimitField;

extern jclass JyNIDictNextResultClass;
extern jfieldID JyNIDictNextResultKeyField;
extern jfieldID JyNIDictNextResultValueField;
extern jfieldID JyNIDictNextResultNewIndexField;
extern jfieldID JyNIDictNextResultKeyHandleField;
extern jfieldID JyNIDictNextResultValueHandleField;

extern jclass JyNISetNextResultClass;
extern jfieldID JyNISetNextResultKeyField;
extern jfieldID JyNISetNextResultNewIndexField;
extern jfieldID JyNISetNextResultKeyHandleField;

extern jmethodID JyNIExceptionByName;
//extern jmethodID JyErr_SetCurExc;
//extern jmethodID JyErr_GetCurExc;
extern jmethodID JyNIJyErr_InsertCurExc;
//extern jmethodID JyNIPyErr_Restore;
//extern jmethodID JyNIPyErr_Clear;
//extern jmethodID JyNIPyErr_Occurred;
extern jmethodID JyNIPyErr_ExceptionMatches;
//extern jmethodID JyNIPyErr_SetObject;
//extern jmethodID JyNIPyErr_SetString;
//extern jmethodID JyNIPyErr_SetNone;
//extern jmethodID JyNIPyErr_NoMemory;
//extern jmethodID JyNIPyErr_Fetch;
extern jmethodID JyNIPyErr_WriteUnraisable;

extern jclass JyListClass;
extern jmethodID JyListFromBackendHandleConstructor;
//extern jmethodID JyListInstallToPyList;

extern jclass JySetClass;
extern jmethodID JySetFromBackendHandleConstructor;
extern jmethodID JySetInstallToPySet;

extern jclass JyLockClass;
extern jmethodID JyLockConstructor;
extern jmethodID JyLockAcquire;
extern jmethodID JyLockRelease;

extern jclass JyReferenceMonitorClass;
extern jmethodID JyRefMonitorAddAction;

extern jclass pyCPeerClass;
extern jmethodID pyCPeerConstructor;
extern jfieldID pyCPeerObjectHandle;
//extern jfieldID pyCPeerRefHandle;

extern jclass pyCPeerGCClass;
extern jmethodID pyCPeerGCConstructor;
//extern jfieldID pyCPeerLinksHandle;

extern jclass jyGCHeadClass;
extern jmethodID traversableGCHeadSetLinks;
extern jmethodID traversableGCHeadSetLink;
extern jmethodID traversableGCHeadClearLink;
extern jmethodID traversableGCHeadClearLinksFromIndex;
extern jmethodID pyObjectGCHeadSetObject;

extern jclass pyCPeerTypeClass;
extern jmethodID pyCPeerTypeConstructor;
extern jmethodID pyCPeerTypeWithNameAndDictConstructor;
extern jfieldID pyCPeerTypeObjectHandle;
extern jfieldID pyCPeerTypeRefHandle;

//extern jlong JyNIDebugMode;
#endif /* JYNI_H_ */
