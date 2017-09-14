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
 * JyNILoader.c
 *
 *  Created on: 13.04.2013, 07:55:05
 *      Author: Stefan Richthofer
 *
 *
 * Windows-stub of JyNILoader.c.
 *
 * Original description of JyNILoader.c:
 *
 * The sole purpose of this loader is to enable JyNI to use the
 * RTLD_GLOBAL flag properly on loading extensions dynamically.
 * JNI does not use this flag and we can't change that behavior
 * directly. So we need this intermediate step to load JyNI with
 * RTLD_GLOBAL set.
 */

#include <JyNI_JyNI.h>
#include <JyNI.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <stdio.h>
//#include <string.h>

JavaVM* java;
void* JyNIHandle;

/*JNIEXPORT*/ jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	//puts("JNI_OnLoad");
	java = jvm; // cache the JavaVM pointer
	return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
	puts("JNI_OnUnload");
	JyNI_unload(jvm); //JyNIUnload(jvm);
}

/*
 * Class:     JyNI_JyNI
 * Method:    initJyNI
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_initJyNI
  (JNIEnv * env, jclass class, jstring JyNILibPath)
{
	//puts("initJyNI...");

//	char* utf_string = (*env)->GetStringUTFChars(env, JyNILibPath, NULL);//&isCopy);
//	//"+1" for 0-termination:
//	char mPath[strlen(utf_string)+1];
//	strcpy(mPath, utf_string);
//	(*env)->ReleaseStringUTFChars(env, JyNILibPath, utf_string);

	jint result = JyNI_init(java);
	if (result != JNI_VERSION_1_2) puts("Init-result indicates error!");
}

/*
 * Class:     JyNI_JyNI
 * Method:    loadModule
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)Lorg/python/core/PyModule;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_loadModule
	(JNIEnv *env, jclass class, jstring moduleName, jstring modulePath, jlong tstate)
{
	//puts("JyNI-Loader: Java_JyNI_JyNI_loadModule...");
	return JyNI_loadModule(env, class, moduleName, modulePath, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    clearPyCPeer
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_clearPyCPeer
	(JNIEnv *env, jclass class, jlong objectHandle, jlong refHandle)
{
	JyNI_clearPyCPeer(env, class, objectHandle, refHandle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNIDebugMessage
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyNIDebugMessage
	(JNIEnv *env, jclass class, jlong mode, jlong value, jstring msg)
{
	JyNI_JyNIDebugMessage(env, class, mode, value, msg);
}

/*
 * Class:     JyNI_JyNI
 * Method:    callPyCPeer
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_callPyCPeer
	(JNIEnv *env, jclass class, jlong peerHandle, jobject args, jobject kw, jlong tstate)
{
	return JyNI_callPyCPeer(env, class, peerHandle, args, kw, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    getAttrString
 * Signature: (JLjava/lang/String;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_getAttrString
	(JNIEnv *env, jclass class, jlong handle, jstring name, jlong tstate)
{
	return JyNI_getAttrString(env, class, handle, name, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setAttrString
 * Signature: (JLjava/lang/String;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_setAttrString
	(JNIEnv *env, jclass class, jlong handle, jstring name, jobject value, jlong tstate)
{
	return JyNI_setAttrString(env, class, handle, name, value, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    repr
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_repr
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_repr(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsString
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_JyNI_JyNI_PyObjectAsString
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_PyObjectAsString(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsPyString
 * Signature: (JJ)Lorg/python/core/PyString;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyObjectAsPyString
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_PyObjectAsPyString(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeRecursionLimit
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_setNativeRecursionLimit
	(JNIEnv *env, jclass class, jint recursionLimit)
{
	JyTState_setNativeRecursionLimit(env, class, recursionLimit);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeCallDepth
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_setNativeCallDepth
	(JNIEnv *env, jclass class, jlong nativeThreadState, jint callDepth)
{
	JyTState_setNativeCallDepth(env, class, nativeThreadState, callDepth);
}

/*
 * Class:     JyNI_JyNI
 * Method:    initNativeThreadState
 * Signature: (LJyNI/JyTState;Lorg/python/core/ThreadState;)J
 */
JNIEXPORT jlong JNICALL Java_JyNI_JyNI_initNativeThreadState
	(JNIEnv *env, jclass class, jobject jyTState, jobject threadState)
{
	return JyTState_initNativeThreadState(env, class, jyTState, threadState);
}

/*
 * Class:     JyNI_JyNI
 * Method:    clearNativeThreadState
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_clearNativeThreadState
	(JNIEnv *env, jclass class, jlong threadState)
{
	JyTState_clearNativeThreadState(env, class, threadState);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_get
 * Signature: (JI)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1get
	(JNIEnv *env, jclass class, jlong handle, jint index)
{
	return JyList_get(env, class, handle, index);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_size
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyList_1size
	(JNIEnv *env, jclass class, jlong handle)
{
	return JyList_size(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_set
 * Signature: (JILorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1set
	(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
	return JyList_set(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_add
 * Signature: (JILorg/python/core/PyObject;J)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyList_1add
	(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
	JyList_add(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_remove
 * Signature: (JI)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1remove
	(JNIEnv *env, jclass class, jlong handle, jint index)
{
	return JyList_remove(env, class, handle, index);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JySet_putSize
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JySet_1putSize
	(JNIEnv *env, jclass class, jlong handle, jint size)
{
	JySet_putSize(env, class, handle, size);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyRefMonitor_setMemDebugFlags
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyRefMonitor_1setMemDebugFlags
	(JNIEnv *env, jclass class, jint flags)
{
	JyRefMonitor_setMemDebugFlags(env, class, flags);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_clearNativeReferences
 * Signature: ([JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_JyNI_JyNI_JyGC_1clearNativeReferences
	(JNIEnv *env, jclass class, jlongArray references, jlong tstate)
{
	return JyGC_clearNativeReferences(env, class, references, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    lookupFromHandle
 * Signature: (J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_lookupFromHandle
	(JNIEnv *env, jclass class, jlong handle)
{
	return JyNIlookupFromHandle(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    currentNativeRefCount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_currentNativeRefCount
	(JNIEnv *env, jclass class, jlong handle)
{
	return JyNIcurrentNativeRefCount(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    nativeIncref
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_nativeIncref
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	JyNI_nativeIncref(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    nativeDecref
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_nativeDecref
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	JyNI_nativeDecref(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    getNativeTypeName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_JyNI_JyNI_getNativeTypeName
	(JNIEnv *env, jclass class, jlong handle)
{
	return JyNIgetNativeTypeName(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_restoreCStubBackend
 * Signature: (JLorg/python/core/PyObject;LJyNI/gc/JyGCHead;)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyGC_1restoreCStubBackend
	(JNIEnv *env, jclass class, jlong handle, jobject backend, jobject newHead)
{
	JyGC_restoreCStubBackend(env, class, handle, backend, newHead);
}

///*
// * Class:     JyNI_JyNI
// * Method:    JyGC_validateGCHead
// * Signature: (J[J)[J
// */
//JNIEXPORT jlongArray JNICALL Java_JyNI_JyNI_JyGC_1validateGCHead(JNIEnv *env, jclass class,
//		jlong handle, jlongArray oldLinks)
/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_validateGCHead
 * Signature: (J[J)Z
 */
JNIEXPORT jboolean JNICALL Java_JyNI_JyNI_JyGC_1validateGCHead
	(JNIEnv *env, jclass class, jlong handle, jlongArray oldLinks)
{
	return JyGC_validateGCHead(env, class, handle, oldLinks);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_nativeTraverse
 * Signature: (J)[J
 */
JNIEXPORT jlongArray JNICALL Java_JyNI_JyNI_JyGC_1nativeTraverse
	(JNIEnv *env, jclass class, jlong handle)
{
	return JyGC_nativeTraverse(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    releaseWeakReferent
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_releaseWeakReferent
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	JyNI_releaseWeakReferent(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    getItem
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_getItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jlong tstate)
{
	return JyNI_getItem(env, class, handle, key, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setItem
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_setItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jobject value, jlong tstate)
{
	return JyNI_setItem(env, class, handle, key, value, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    delItem
 * Signature: (JLorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_delItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jlong tstate)
{
	return JyNI_delItem(env, class, handle, key, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectLength
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_PyObjectLength
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_PyObjectLength(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    descr_get
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_descr_1get
  (JNIEnv *env, jclass class, jlong self, jobject obj, jobject type, jlong tstate)
{
	return JyNI_descr_get(self, obj, type, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    descr_set
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_descr_1set
  (JNIEnv *env, jclass class, jlong self, jobject obj, jobject value, jlong tstate)
{
	return JyNI_descr_set(self, obj, value, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyCFunction_getSelf
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyCFunction_1getSelf
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_PyCFunction_getSelf(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyCFunction_getModule
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyCFunction_1getModule
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_PyCFunction_getModule(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_CMethodDef_bind
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1CMethodDef_1bind
  (JNIEnv *env, jclass class, jlong handle, jobject bindTo, jlong tstate)
{
	return JyNI_CMethodDef_bind(handle, bindTo, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyObject_Compare
 * Signature: (JLorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PyObject_1Compare
  (JNIEnv *env, jclass class, jlong handle, jobject o, jlong tstate)
{
	return JyNI_PyObject_Compare(handle, o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyObject_RichCompare
 * Signature: (JLorg/python/core/PyObject;IJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyObject_1RichCompare
  (JNIEnv *env, jclass class, jlong handle, jobject o, jint op, jlong tstate)
{
	return JyNI_PyObject_RichCompare(handle, o, op, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyObject_GetIter
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyObject_1GetIter
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_PyObject_GetIter(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyIter_Next
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyIter_1Next
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return JyNI_PyIter_Next(handle, tstate);
}




// PyNumber-methods:

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Add
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Add
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Add(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Subtract
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Subtract
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Subtract(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Multiply
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Multiply
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Multiply(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Divide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Divide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Divide(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Remainder
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Remainder
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Remainder(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Divmod
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Divmod
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Divmod(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Power
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Power
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jobject o3, jlong tstate)
{
	return JyNI_PyNumber_Power(o1, o2, o3, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Negative
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Negative
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Negative(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Positive
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Positive
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Positive(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Absolute
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Absolute
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Absolute(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_NonZero
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1NonZero
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_NonZero(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Invert
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Invert
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Invert(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Lshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Lshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Lshift(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Rshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Rshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Rshift(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_And
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1And
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_And(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Xor
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Xor
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Xor(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Or
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Or
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Or(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Coerce
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Coerce
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_Coerce(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Int
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Int
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Int(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Long
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Long
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Long(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Float
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Float
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Float(o, tstate);
}

///*
// * Class:     JyNI_JyNI
// * Method:    JyNI_PyNumber_Oct
// * Signature: (JJ)Lorg/python/core/PyObject;
// */
//JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Oct
//  (JNIEnv *env, jclass class, jlong o, jlong tstate)
//{
//	return JyNI_PyNumber_Oct(o, tstate);
//}

///*
// * Class:     JyNI_JyNI
// * Method:    JyNI_PyNumber_Hex
// * Signature: (JJ)Lorg/python/core/PyObject;
// */
//JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Hex
//  (JNIEnv *env, jclass class, jlong o, jlong tstate)
//{
//	return JyNI_PyNumber_Hex(o, tstate);
//}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceAdd
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceAdd
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceAdd(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceSubtract
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceSubtract
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceSubtract(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceMultiply
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceMultiply
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceMultiply(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceDivide(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceRemainder
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceRemainder
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceRemainder(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlacePower
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlacePower
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jobject o3, jlong tstate)
{
	return JyNI_PyNumber_InPlacePower(o1, o2, o3, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceLshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceLshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceLshift(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceRshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceRshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceRshift(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceAnd
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceAnd
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceAnd(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceXor
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceXor
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceXor(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceOr
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceOr
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceOr(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_FloorDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1FloorDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_FloorDivide(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_TrueDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1TrueDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_TrueDivide(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceFloorDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceFloorDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceFloorDivide(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceTrueDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceTrueDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PyNumber_InPlaceTrueDivide(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Index
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Index
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyNumber_Index(o, tstate);
}




// PySequence-methods:

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_Length
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1Length
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PySequence_Length(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_Concat
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1Concat
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PySequence_Concat(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_Repeat
 * Signature: (JIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1Repeat
  (JNIEnv *env, jclass class, jlong o, jint l, jlong tstate)
{
	return JyNI_PySequence_Repeat(o, l, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_GetItem
 * Signature: (JIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1GetItem
  (JNIEnv *env, jclass class, jlong o, jint l, jlong tstate)
{
	return JyNI_PySequence_GetItem(o, l, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_GetSlice
 * Signature: (JIIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1GetSlice
  (JNIEnv *env, jclass class, jlong o, jint l1, jint l2, jlong tstate)
{
	return JyNI_PySequence_GetSlice(o, l1, l2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_SetItem
 * Signature: (JILorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1SetItem
  (JNIEnv *env, jclass class, jlong o1, jint l, jobject o2, jlong tstate)
{
	return JyNI_PySequence_SetItem(o1, l, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_SetSlice
 * Signature: (JIILorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1SetSlice
  (JNIEnv *env, jclass class, jlong o1, jint l1, jint l2, jobject o2, jlong tstate)
{
	return JyNI_PySequence_SetSlice(o1, l1, l2, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_Contains
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1Contains
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PySequence_Contains(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_InPlaceConcat
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1InPlaceConcat
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return JyNI_PySequence_InPlaceConcat(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_InPlaceRepeat
 * Signature: (JIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1InPlaceRepeat
  (JNIEnv *env, jclass class, jlong o, jint l, jlong tstate)
{
	return JyNI_PySequence_InPlaceRepeat(o, l, tstate);
}




// PyMapping-methods:

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyMapping_Length
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PyMapping_1Length
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return JyNI_PyMapping_Length(o, tstate);
}

///*
// * Class:     JyNI_JyNI
// * Method:    JyNI_PyMapping_Subscript
// * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
// */
//JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyMapping_1Subscript
//  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
//{
//	return JyNI_PyMapping_Subscript(o1, o2, tstate);
//}
//
///*
// * Class:     JyNI_JyNI
// * Method:    JyNI_PyMapping_AssSubscript
// * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)I
// */
//JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PyMapping_1AssSubscript
//  (JNIEnv *env, jclass class, jlong o1, jobject o2, jobject o3, jlong tstate)
//{
//	return JyNI_PyMapping_AssSubscript(o1, o2, o3, tstate);
//}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_putenv
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1putenv
  (JNIEnv *env, jclass cls, jstring value)
{
	return JyNI_putenv(env, value);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_mbcs_encode
 * Signature: (Lorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1mbcs_1encode
  (JNIEnv *env, jclass cls, jobject input, jobject errors, jlong tstate)
{
	return JyNI_mbcs_encode(input, errors, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_mbcs_decode
 * Signature: (Lorg/python/core/PyObject;Lorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1mbcs_1decode
  (JNIEnv *env, jclass cls, jobject input, jobject errors, jobject final, jlong tstate)
{
	return JyNI_mbcs_decode(input, errors, final, tstate);
}
