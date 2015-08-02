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
 * JyNILoader.c
 *
 *  Created on: 13.04.2013, 07:55:05
 *      Author: Stefan Richthofer
 *
 * The sole purpose of this loader is to enable JyNI to use the
 * RTLD_GLOBAL flag properly on loading extensions dynamically.
 * JNI does not use this flag and we can't change that behaviour
 * directly. So we need this intermediate step to load JyNI with
 * RTLD_GLOBAL set.
 */

#include <JyNI_JyNI.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <stdio.h>
#include <string.h>

//#if defined(__NetBSD__)
//#include <sys/param.h>
//#if (NetBSD < 199712)
//#include <nlist.h>
//#include <link.h>
//#define dlerror() "error in dynamic linking"
//#endif
//#endif /* NetBSD */

#include <dlfcn.h>

JavaVM* java;
void* JyNIHandle;

jobject (*JyNILoadModule)(JNIEnv*, jclass, jstring, jstring, jlong);
void (*JyNIClearPyCPeer)(JNIEnv*, jclass, jlong, jlong);
void (*JyNI_JyNIDebugMessage)(JNIEnv*, jclass, jlong, jlong, jstring);
jobject (*JyNICallPyCPeer)(JNIEnv*, jclass, jlong, jobject, jobject, jlong);
jobject (*JyNIGetAttrString)(JNIEnv*, jclass, jlong, jstring, jlong);
jobject (*JyNIrepr)(JNIEnv*, jclass, jlong, jlong);
jstring (*JyNIPyObjectAsString)(JNIEnv*, jclass, jlong, jlong);
jobject (*JyNIPyObjectAsPyString)(JNIEnv*, jclass, jlong, jlong);
jint (*JyNISetAttrString)(JNIEnv*, jclass, jlong, jstring, jobject, jlong);
jobject (*JyNIlookupFromHandle)(JNIEnv*, jclass, jlong);
jint (*JyNIcurrentNativeRefCount)(JNIEnv*, jclass, jlong);
jstring (*JyNIgetNativeTypeName)(JNIEnv*, jclass, jlong);
void (*JyNIUnload)(JavaVM*);

void (*JyTStateSetNativeRecursionLimit)(JNIEnv*, jclass, jint);
void (*JyTStateSetNativeCallDepth)(JNIEnv*, jclass, jlong, jint);
jlong (*JyTStateInitNativeThreadState)(JNIEnv*, jclass, jobject, jobject);
void (*JyTStateClearNativeThreadState)(JNIEnv*, jclass, jlong);

jobject (*JyList_get)(JNIEnv*, jclass, jlong, jint);
jint (*JyList_size)(JNIEnv*, jclass, jlong);
jobject (*JyList_set)(JNIEnv*, jclass, jlong, jint, jobject, jlong);
void (*JyList_add)(JNIEnv*, jclass, jlong, jint, jobject, jlong);
jobject (*JyList_remove)(JNIEnv*, jclass, jlong, jint);

void (*JySet_putSize)(JNIEnv*, jclass, jlong, jint);

void (*JyRefMonitor_setMemDebugFlags)(JNIEnv*, jclass, jint);
jboolean (*JyGC_clearNativeReferences)(JNIEnv*, jclass, jlongArray, jlong);
void (*JyGC_restoreCStubBackend)(JNIEnv*, jclass, jlong, jobject, jobject);
//jlongArray (*JyGC_validateGCHead)(JNIEnv*, jclass, jlong, jlongArray);
jboolean (*JyGC_validateGCHead)(JNIEnv*, jclass, jlong, jlongArray);
jlongArray (*JyGC_nativeTraverse)(JNIEnv*, jclass, jlong);
//void JyNI_unload(JavaVM *jvm);


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	//puts("JNI_OnLoad");
	java = jvm; // cache the JavaVM pointer
	return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
	puts("JNI_OnUnload");
	(*JyNIUnload)(jvm);
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
	//puts("path:");
	//jboolean isCopy;

	char* utf_string = (*env)->GetStringUTFChars(env, JyNILibPath, NULL);//&isCopy);
	//"+1" for 0-termination:
	char mPath[strlen(utf_string)+1];
	strcpy(mPath, utf_string);
	(*env)->ReleaseStringUTFChars(env, JyNILibPath, utf_string);
	//puts(mPath);

	//JyNIHandle = dlopen(mPath, JyNI_JyNI_RTLD_LAZY | JyNI_JyNI_RTLD_GLOBAL);
	JyNIHandle = dlopen(mPath, RTLD_LAZY | RTLD_GLOBAL);
	//printf("%i\n", (int) (RTLD_LAZY | RTLD_GLOBAL));
	//printf("%i\n", (int) JyNI_JyNI_RTLD_LAZY);

	if (JyNIHandle == NULL) {
		const char *error = dlerror();
		if (error == NULL)
			error = "unknown dlopen() error in JyNILoader";
		puts("dlopen-error in JyNILoader:");
		puts(error);
		//PyErr_SetString(PyExc_ImportError, error);
		//JyNI_JyErr_SetString((*env)->GetStaticObjectField(env, pyPyClass, pyPyImportError), error);
		//return NULL;
	}
	//jint JyNI_init(JavaVM *jvm);
	jint (*JyNIInit)(JavaVM*);
	//puts("call dlsym...");
	*(void **) (&JyNIInit) = dlsym(JyNIHandle, "JyNI_init");
	*(void **) (&JyNILoadModule) = dlsym(JyNIHandle, "JyNI_loadModule");
	*(void **) (&JyNIClearPyCPeer) = dlsym(JyNIHandle, "JyNI_clearPyCPeer");
	*(void **) (&JyNI_JyNIDebugMessage) = dlsym(JyNIHandle, "JyNI_JyNIDebugMessage");
	*(void **) (&JyNICallPyCPeer) = dlsym(JyNIHandle, "JyNI_callPyCPeer");
	*(void **) (&JyNIGetAttrString) = dlsym(JyNIHandle, "JyNI_getAttrString");
	*(void **) (&JyNIrepr) = dlsym(JyNIHandle, "JyNI_repr");
	*(void **) (&JyNIPyObjectAsString) = dlsym(JyNIHandle, "JyNI_PyObjectAsString");
	*(void **) (&JyNIPyObjectAsPyString) = dlsym(JyNIHandle, "JyNI_PyObjectAsPyString");
	*(void **) (&JyNISetAttrString) = dlsym(JyNIHandle, "JyNI_setAttrString");
	*(void **) (&JyNIlookupFromHandle) = dlsym(JyNIHandle, "JyNIlookupFromHandle");
	*(void **) (&JyNIcurrentNativeRefCount) = dlsym(JyNIHandle, "JyNIcurrentNativeRefCount");
	*(void **) (&JyNIgetNativeTypeName) = dlsym(JyNIHandle, "JyNIgetNativeTypeName");
	*(void **) (&JyNIUnload) = dlsym(JyNIHandle, "JyNI_unload");

	*(void **) (&JyTStateSetNativeRecursionLimit) = dlsym(JyNIHandle, "JyTState_setNativeRecursionLimit");
	*(void **) (&JyTStateSetNativeCallDepth) = dlsym(JyNIHandle, "JyTState_setNativeCallDepth");
	*(void **) (&JyTStateInitNativeThreadState) = dlsym(JyNIHandle, "JyTState_initNativeThreadState");
	*(void **) (&JyTStateClearNativeThreadState) = dlsym(JyNIHandle, "JyTState_clearNativeThreadState");

	*(void **) (&JyList_get) = dlsym(JyNIHandle, "JyList_get");
	*(void **) (&JyList_size) = dlsym(JyNIHandle, "JyList_size");
	*(void **) (&JyList_set) = dlsym(JyNIHandle, "JyList_set");
	*(void **) (&JyList_add) = dlsym(JyNIHandle, "JyList_add");
	*(void **) (&JyList_remove) = dlsym(JyNIHandle, "JyList_remove");

	*(void **) (&JySet_putSize) = dlsym(JyNIHandle, "JySet_putSize");

	*(void **) (&JyRefMonitor_setMemDebugFlags) = dlsym(JyNIHandle, "JyRefMonitor_setMemDebugFlags");
	*(void **) (&JyGC_clearNativeReferences) = dlsym(JyNIHandle, "JyGC_clearNativeReferences");
	*(void **) (&JyGC_restoreCStubBackend) = dlsym(JyNIHandle, "JyGC_restoreCStubBackend");
	*(void **) (&JyGC_validateGCHead) = dlsym(JyNIHandle, "JyGC_validateGCHead");
	*(void **) (&JyGC_nativeTraverse) = dlsym(JyNIHandle, "JyGC_nativeTraverse");

	jint result = (*JyNIInit)(java);
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
	return (*JyNILoadModule)(env, class, moduleName, modulePath, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    clearPyCPeer
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_clearPyCPeer
  (JNIEnv *env, jclass class, jlong objectHandle, jlong refHandle)
{
	(*JyNIClearPyCPeer)(env, class, objectHandle, refHandle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNIDebugMessage
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyNIDebugMessage
  (JNIEnv *env, jclass class, jlong mode, jlong value, jstring msg)
{
	(*JyNI_JyNIDebugMessage)(env, class, mode, value, msg);
}

/*
 * Class:     JyNI_JyNI
 * Method:    callPyCPeer
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_callPyCPeer
  (JNIEnv *env, jclass class, jlong peerHandle, jobject args, jobject kw, jlong tstate)
{
	return (*JyNICallPyCPeer)(env, class, peerHandle, args, kw, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    getAttrString
 * Signature: (JLjava/lang/String;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_getAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jlong tstate)
{
	return (*JyNIGetAttrString)(env, class, handle, name, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setAttrString
 * Signature: (JLjava/lang/String;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_setAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jobject value, jlong tstate)
{
	return (*JyNISetAttrString)(env, class, handle, name, value, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    repr
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_repr(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNIrepr)(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsString
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_JyNI_JyNI_PyObjectAsString(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNIPyObjectAsString)(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsPyString
 * Signature: (JJ)Lorg/python/core/PyString;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyObjectAsPyString(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNIPyObjectAsPyString)(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeRecursionLimit
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_setNativeRecursionLimit
  (JNIEnv *env, jclass class, jint recursionLimit)
{
	(*JyTStateSetNativeRecursionLimit)(env, class, recursionLimit);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeCallDepth
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_setNativeCallDepth
  (JNIEnv *env, jclass class, jlong nativeThreadState, jint callDepth)
{
	(*JyTStateSetNativeCallDepth)(env, class, nativeThreadState, callDepth);
}

/*
 * Class:     JyNI_JyNI
 * Method:    initNativeThreadState
 * Signature: (LJyNI/JyTState;Lorg/python/core/ThreadState;)J
 */
JNIEXPORT jlong JNICALL Java_JyNI_JyNI_initNativeThreadState
  (JNIEnv *env, jclass class, jobject jyTState, jobject threadState)
{
	return (*JyTStateInitNativeThreadState)(env, class, jyTState, threadState);
}

/*
 * Class:     JyNI_JyNI
 * Method:    clearNativeThreadState
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_clearNativeThreadState
  (JNIEnv *env, jclass class, jlong threadState)
{
	(*JyTStateClearNativeThreadState)(env, class, threadState);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_get
 * Signature: (JI)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1get(JNIEnv *env, jclass class,
		jlong handle, jint index)
{
	return (*JyList_get)(env, class, handle, index);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_size
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyList_1size(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyList_size)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_set
 * Signature: (JILorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1set(JNIEnv *env, jclass class,
		jlong handle, jint index, jobject obj, jlong pyObj)
{
	return (*JyList_set)(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_add
 * Signature: (JILorg/python/core/PyObject;J)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyList_1add(JNIEnv *env, jclass class,
		jlong handle, jint index, jobject obj, jlong pyObj)
{
	(*JyList_add)(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_remove
 * Signature: (JI)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1remove(JNIEnv *env, jclass class,
		jlong handle, jint index)
{
	return (*JyList_remove)(env, class, handle, index);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JySet_putSize
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JySet_1putSize(JNIEnv *env, jclass class,
		jlong handle, jint size)
{
	(*JySet_putSize)(env, class, handle, size);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyRefMonitor_setMemDebugFlags
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyRefMonitor_1setMemDebugFlags(JNIEnv *env,
		jclass class, jint flags)
{
	(*JyRefMonitor_setMemDebugFlags)(env, class, flags);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_clearNativeReferences
 * Signature: ([JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_JyNI_JyNI_JyGC_1clearNativeReferences(JNIEnv *env,
		jclass class, jlongArray references, jlong tstate)
{
	return (*JyGC_clearNativeReferences)(env, class, references, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    lookupFromHandle
 * Signature: (J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_lookupFromHandle(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyNIlookupFromHandle)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    currentNativeRefCount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_currentNativeRefCount(JNIEnv *env, jclass class, jlong handle)
{
	return JyNIcurrentNativeRefCount(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    getNativeTypeName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_JyNI_JyNI_getNativeTypeName(JNIEnv *env, jclass class, jlong handle)
{
	return JyNIgetNativeTypeName(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_restoreCStubBackend
 * Signature: (JLorg/python/core/PyObject;LJyNI/gc/JyGCHead;)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyGC_1restoreCStubBackend(JNIEnv *env, jclass class,
		jlong handle, jobject backend, jobject newHead)
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
JNIEXPORT jboolean JNICALL Java_JyNI_JyNI_JyGC_1validateGCHead(JNIEnv *env, jclass class,
		jlong handle, jlongArray oldLinks)
{
	return JyGC_validateGCHead(env, class, handle, oldLinks);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_nativeTraverse
 * Signature: (J)[J
 */
JNIEXPORT jlongArray JNICALL Java_JyNI_JyNI_JyGC_1nativeTraverse(JNIEnv *env, jclass class,
		jlong handle)
{
	return JyGC_nativeTraverse(env, class, handle);
}
