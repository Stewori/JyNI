/*
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013 Stefan Richthofer.  All rights reserved.
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
//PyFPE_jbuf
/* JNI-Shortcuts: */


//Shortcut to get JNI-environment,
//including check for whether JNI version is supported.
//note that we don't put a semicolon to the end, so the
//macro-call must be succeeded by a semicolon, giving it
//a more natural look...
#define env(errRet) \
	JNIEnv *env;\
	if ((*java)->GetEnv(java, (void **)&env, JNI_VERSION_1_2))\
		return errRet

//Cleanly convert a jstring to a cstring
//with minimal JVM lock-time.
//Use only once per Function.
//For further conversions use
//cstr_from_jstring2.
//Note that at least one call of "env()"
//must have happened before in the
//same block or in some parent block.
//("+1" in 4th line is for 0-termination)
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

//Only use after one initial use of
//cstr_from_jstring in each function.
//("+1" in 3rd line is for 0-termination)
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
#define JY_INITIALIZED_FLAG_MASK 1
#define JY_GC_FLAG_MASK			 2
#define JY_TRUNCATE_FLAG_MASK	 4
#define JY_PARTLY_TRUNCATE_MASK  8
#define JY_CPEER_FLAG_MASK		16
//#define JY_TYPE_FLAG_MASK		32 //Types can't have JyObject-data
//32, 64, 128 reserved for future use...

/* define some method signatures for sync purposes: */

//jobject is src, PyObject* is dest. Src must not be modified.
typedef void (*jy2pySync)(jobject, PyObject*);

//PyObject* is src, jobject is dest. Src must not be modified.
typedef void (*py2jySync)(PyObject*, jobject);

typedef jobject (*jyInitSync)(PyObject*);
typedef PyObject* (*pyInitSync)(jobject);
typedef jobject (*jyFactoryMethod)();
//typedef void (*jy2pyItemSync)(jobject, PyObject*, int index);
//typedef void (*py2jyItemSync)(PyObject*, jobject, int index);
typedef jlong (*pyChecksum)(PyObject*);
typedef jlong (*jyChecksum)(jobject);

/* Sync-behavior Flags: */
#define SYNC_ON_PY_INIT_FLAG_MASK				  256
#define SYNC_ON_JY_INIT_FLAG_MASK				  512
#define SYNC_ON_PY_TO_JY_FLAG_MASK				 1024
#define SYNC_ON_JY_TO_PY_FLAG_MASK				 2048
#define SYNC_ALWAYS_COMPLETELY_FLAG_MASK		 4096
#define SYNC_ON_PY_CHECKSUM_CHANGE_FLAG_MASK	 8192
#define SYNC_ON_JY_CHECKSUM_CHANGE_FLAG_MASK	16384
#define SYNC_NEEDED_MASK	3072
	//(SYNC_ON_PY_TO_JY_FLAG_MASK | SYNC_ON_JY_TO_PY_FLAG_MASK)

/*
 * py2jy copies everything sanely from a PyObject to a jython jobject.
 * jy2py copies everything sanely from a jython jobject to a PyObject.
 * jyInit sanely calls a valid constructor on a jython jobject using
 * data from a corresponding PyObject.
 * pyInit sanely creates a valid PyObject initialized with the data
 * from the given jython jobject.
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
 * various purpose stuff. It's a dict-like idea implemented
 * as a simple linked list. This feature is intended for
 * rare use, so this naive implementation is sufficient.
 * Every JyAttribute must be uniquely identifyed by an
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
extern const char* JyAttributeStringInterned;
extern const char* JyAttributeSetEntry;
//extern const char* JyAttributeTruncateSize;
#define JY_ATTR_OWNS_VALUE_FLAG_MASK 1
#define JY_ATTR_VAR_SIZE_FLAG_MASK 2
typedef struct JyAttribute JyAttribute; //Forward declaration
struct JyAttribute { const char* name; void* value; char flags; JyAttribute* next;};
typedef struct JyAttributeElement JyAttributeElement; //Forward declaration
struct JyAttributeElement {void* value; JyAttributeElement* next;};
typedef struct { jobject jy; unsigned short flags; JyAttribute* attr;} JyObject;
typedef struct { PyTypeObject* py_type; jclass jy_class; unsigned short flags; SyncFunctions* sync; size_t truncate_trailing;} TypeMapEntry;
typedef struct { PyTypeObject* exc_type; jyFactoryMethod exc_factory;} ExceptionMapEntry;

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


/* Replacement for Py_EnterRecursiveCall.
 * Original usage: Py_EnterRecursiveCall(where) : fail (?)
 * Example:
 * if (Py_EnterRecursiveCall(" while doing blah"))
 *  	return NULL;
 *
 * New version:
 * Usage: Jy_EnterRecursiveCall(where) : void
 * Result is placed in Jy_EnterRecursiveCallResult.
 * Example:
 * Jy_EnterRecursiveCall(" while doing blah");
 * if (Jy_EnterRecursiveCallResult) return NULL;
 *
 * Alternative:
 * Usage: Jy_EnterRecursiveCall2(where, doOnFail) : void
 * Example:
 * Jy_EnterRecursiveCall2(" while doing blah", return NULL);
 *
 * Warning: Can only be used once per block. For subsequent usage one
 * needs to adopt this macro by hand and simply leave out the first line.
 *
 * Note that these macros are only usable after a call to the env-macro in the same block.
 */
#define Jy_EnterRecursiveCall(where) \
	jobject tstate = (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyGetThreadState); \
	(*env)->CallVoidMethod(env, tstate, pyThreadStateEnterRecursiveCall, (*env)->NewStringUTF(env, where)); \
	jboolean Jy_EnterRecursiveCallResult = JNI_FALSE; \
	if ((*env)->ExceptionCheck(env)) \
	{ \
		(*env)->ExceptionClear(env); \
		Jy_EnterRecursiveCallResult = JNI_TRUE; \
	}

#define Jy_EnterRecursiveCall2(where, doOnFail) \
	jobject tstate = (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyGetThreadState); \
	(*env)->CallVoidMethod(env, tstate, pyThreadStateEnterRecursiveCall, (*env)->NewStringUTF(env, where)); \
	if ((*env)->ExceptionCheck(env)) \
	{ \
		(*env)->ExceptionClear(env); \
		doOnFail; \
	}

/* Replacement for Py_LeaveRecursiveCall.
 * Original usage: Py_LeaveRecursiveCall()
 * Example:
 * Py_LeaveRecursiveCall();
 *
 * New version:
 * Usage: Jy_LeaveRecursiveCall()
 * i.e. usage did not change from original
 *
 * Note that this macro is only usable after a call to
 * Jy_EnterRecursiveCall or Jy_EnterRecursiveCall2 in the same block.
 */
#define Jy_LeaveRecursiveCall() \
	(*env)->CallVoidMethod(env, tstate, pyThreadStateLeaveRecursiveCall)

/* Call-ins: */
jobject JyNI_loadModule(JNIEnv *env, jclass class, jstring moduleName, jstring modulePath);
jint JyNI_init(JavaVM *jvm);
void JyNI_unload(JavaVM *jvm);
void JyNI_clearPyCPeer(JNIEnv *env, jclass class, jlong objectHandle, jlong refHandle);
jobject JyNI_callPyCPeer(JNIEnv *env, jclass class, jlong peerHandle, jobject args, jobject kw);
jobject JyNI_getAttrString(JNIEnv *env, jclass class, jlong handle, jstring name);
jint JyNI_setAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jobject value);
jobject JyNI_repr(JNIEnv *env, jclass class, jlong handle);
jstring JyNI_PyObjectAsString(JNIEnv *env, jclass class, jlong handle);
jobject JyNI_PyObjectAsPyString(JNIEnv *env, jclass class, jlong handle);

#define builtinTypeCount 50
extern TypeMapEntry builtinTypes[builtinTypeCount];

/* "Hidden" PyTypes: */
extern PyTypeObject PyNone_Type;
extern PyTypeObject PyNotImplemented_Type;
extern PyTypeObject Pyrangeiter_Type; //jython uses PySequenceIter.
extern PyTypeObject PyTupleIter_Type; //jython uses PyFastSequenceIter.
extern PyTypeObject PyListIter_Type; //jython uses PyFastSequenceIter.
extern PyTypeObject PyListRevIter_Type; //jython uses PyReversedSequenceIter.
extern PyTypeObject PySetIter_Type; //jython uses inline subclass of PyIterator.
//extern PyTypeObject PyMethodDescr_Type;
//extern PyTypeObject PyClassMethodDescr_Type;
extern PyTypeObject PyTraceBack_Type;

//extern PyStringObject* characters;

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
inline PyTypeObject* JyNI_PyExceptionType_FromJythonExceptionType(jobject exc);

/* Conversion-Stuff: */
inline jobject JyNI_JythonPyObject_FromPyObject(PyObject* op);
inline PyObject* JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject);
inline PyObject* _JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject, jboolean lookupNative, jboolean checkCPeer, jboolean checkForType);
inline jobject JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type);
inline jobject _JyNI_JythonPyTypeObject_FromPyTypeObject(PyTypeObject* type, jclass cls);
inline PyTypeObject* JyNI_PyTypeObject_FromJythonPyTypeObject(jobject jythonPyTypeObject);
inline jstring JyNI_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op);
inline jstring JyNI_interned_jstring_FromPyStringObject(JNIEnv *env, PyStringObject* op);

/* Exception-Stuff: */
inline void JyNI_JyErr_SetString(jobject exception, const char *string);
inline void JyNI_JyErr_SetObject(jobject exception, PyObject *value);
inline void JyNI_JyErr_SetExceptionObject(jobject exception);
inline int JyNI_JyErr_ExceptionMatches(jobject exc);
inline PyObject* JyNI_JyErr_Format(jobject exception, const char *format, ...);

/* JyNI-Stuff: */
inline PyObject* JyNI_GenericAlloc(PyTypeObject* type, Py_ssize_t nitems);
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
inline void JyNI_Py_CLEAR(jobject obj);
//inline char* PyLongToJavaSideString(PyObject* pl);

/* To save lookups: */
PyObject* _JyObject_GC_New(PyTypeObject *tp, TypeMapEntry* tme);
PyVarObject* _JyObject_GC_NewVar(PyTypeObject *tp, Py_ssize_t nitems, TypeMapEntry* tme);
inline PyObject * _JyObject_New(PyTypeObject *tp, TypeMapEntry* tme);
jobject _PyImport_LoadDynamicModuleJy(char *name, char *pathname, FILE *fp);
inline int PyModule_AddStringConstantJy(jobject m, const char *name, const char *value);
inline int PyModule_AddObjectJy(jobject m, const char *name, jobject o);

/* Backdoor to reach original alloc-functions, which were renamed with "Raw"-prefix: */
PyAPI_FUNC(void *) PyObject_RawMalloc(size_t);
PyAPI_FUNC(void *) PyObject_RawRealloc(void *, size_t);
PyAPI_FUNC(void) PyObject_RawFree(void *);

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

//JNI IDs for Jython-stuff:
//singletons:
extern JavaVM* java;
extern jobject length0StringArray;
extern jobject length0PyObjectArray;

extern jclass classClass;
extern jmethodID classEquals;

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
extern jmethodID JyNILookupNativeHandle;
extern jmethodID JyNIClearNativeHandle;
extern jmethodID JyNILookupCPeerHandle;
//extern jmethodID JyNIConstructDefaultObject;
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
//extern jmethodID JyNIPySet_pop;

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
extern jmethodID JyErr_SetCurExc;
extern jmethodID JyErr_GetCurExc;
extern jmethodID JyErr_InsertCurExc;
extern jmethodID JyNIPyErr_Restore;
extern jmethodID JyNIPyErr_Clear;
extern jmethodID JyNIPyErr_Occurred;
extern jmethodID JyNIPyErr_ExceptionMatches;
extern jmethodID JyNIPyErr_SetObject;
extern jmethodID JyNIPyErr_SetString;
extern jmethodID JyNIPyErr_SetNone;
extern jmethodID JyNIPyErr_NoMemory;
extern jmethodID JyNIPyErr_Fetch;
extern jmethodID JyNIPyErr_WriteUnraisable;

extern jclass JyListClass;
extern jmethodID JyListFromBackendHandleConstructor;
extern jmethodID JyListInstallToPyList;

extern jclass JySetClass;
extern jmethodID JySetFromBackendHandleConstructor;
extern jmethodID JySetInstallToPySet;

extern jclass pyCPeerClass;
extern jmethodID pyCPeerConstructor;
extern jfieldID pyCPeerObjectHandle;
//extern jfieldID pyCPeerRefHandle;

extern jclass pyCPeerTypeClass;
extern jmethodID pyCPeerTypeConstructor;
extern jfieldID pyCPeerTypeObjectHandle;
extern jfieldID pyCPeerTypeRefHandle;

#endif /* JYNI_H_ */
