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
 * The sole purpose of this loader is to enable JyNI to use the
 * RTLD_GLOBAL flag properly on loading extensions dynamically.
 * JNI does not use this flag and we can't change that behavior
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
jobject (*JyNI_getItem)(JNIEnv*, jclass, jlong, jobject, jlong);
jint (*JyNI_setItem)(JNIEnv*, jclass, jlong, jobject, jobject, jlong);
jint (*JyNI_delItem)(JNIEnv*, jclass, jlong, jobject, jlong);
jint (*JyNI_PyObjectLength)(JNIEnv*, jclass, jlong, jlong);
jobject (*JyNI_descr_get)(jlong, jobject, jobject, jlong);
jint (*JyNI_descr_set)(jlong, jobject, jobject, jlong);
jstring (*JyNIPyObjectAsString)(JNIEnv*, jclass, jlong, jlong);
jobject (*JyNIPyObjectAsPyString)(JNIEnv*, jclass, jlong, jlong);
jint (*JyNISetAttrString)(JNIEnv*, jclass, jlong, jstring, jobject, jlong);
jobject (*JyNIlookupFromHandle)(JNIEnv*, jclass, jlong);
jint (*JyNIcurrentNativeRefCount)(JNIEnv*, jclass, jlong);
void (*JyNI_nativeIncref)(jlong, jlong);
void (*JyNI_nativeDecref)(jlong, jlong);
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

jobject (*JyNI_PyCFunction_getSelf)(jlong, jlong);
jobject (*JyNI_PyCFunction_getModule)(jlong, jlong);
jobject (*JyNI_CMethodDef_bind)(jlong, jobject, jlong);
jint (*JyNI_PyObject_Compare)(jlong, jobject, jlong);
jobject (*JyNI_PyObject_RichCompare)(jlong, jobject, jint, jlong);
jobject (*JyNI_PyObject_GetIter)(jlong, jlong);
jobject (*JyNI_PyIter_Next)(jlong, jlong);

void (*JyRefMonitor_setMemDebugFlags)(JNIEnv*, jclass, jint);
jboolean (*JyGC_clearNativeReferences)(JNIEnv*, jclass, jlongArray, jlong);
void (*JyGC_restoreCStubBackend)(JNIEnv*, jclass, jlong, jobject, jobject);
//jlongArray (*JyGC_validateGCHead)(JNIEnv*, jclass, jlong, jlongArray);
jboolean (*JyGC_validateGCHead)(JNIEnv*, jclass, jlong, jlongArray);
jlongArray (*JyGC_nativeTraverse)(JNIEnv*, jclass, jlong);
void (*JyNI_releaseWeakReferent)(JNIEnv*, jclass, jlong, jlong);

//void JyNI_unload(JavaVM *jvm);


//PyNumber-methods:
jobject  (*JyNI_PyNumber_Add)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Subtract)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Multiply)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Divide)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Remainder)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Divmod)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Power)(jlong, jobject, jobject, jlong);
jobject  (*JyNI_PyNumber_Negative)(jlong, jlong);
jobject  (*JyNI_PyNumber_Positive)(jlong, jlong);
jobject  (*JyNI_PyNumber_Absolute)(jlong, jlong);
jboolean (*JyNI_PyNumber_NonZero)(jlong, jlong);
jobject  (*JyNI_PyNumber_Invert)(jlong, jlong);
jobject  (*JyNI_PyNumber_Lshift)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Rshift)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_And)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Xor)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Or)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Coerce)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Int)(jlong, jlong);
jobject  (*JyNI_PyNumber_Long)(jlong, jlong);
jobject  (*JyNI_PyNumber_Float)(jlong, jlong);
jobject  (*JyNI_PyNumber_Oct)(jlong, jlong);
jobject  (*JyNI_PyNumber_Hex)(jlong, jlong);
jobject  (*JyNI_PyNumber_InPlaceAdd)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceSubtract)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceMultiply)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceDivide)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceRemainder)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlacePower)(jlong, jobject, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceLshift)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceRshift)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceAnd)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceXor)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceOr)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_FloorDivide)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_TrueDivide)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceFloorDivide)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_InPlaceTrueDivide)(jlong, jobject, jlong);
jobject  (*JyNI_PyNumber_Index)(jlong, jlong);

// PySequence-methods:
jint    (*JyNI_PySequence_Length)(jlong, jlong);
jobject (*JyNI_PySequence_Concat)(jlong, jobject, jlong);
jobject (*JyNI_PySequence_Repeat)(jlong, jint, jlong);
jobject (*JyNI_PySequence_GetItem)(jlong, jint, jlong);
jobject (*JyNI_PySequence_GetSlice)(jlong, jint, jint, jlong);
jint    (*JyNI_PySequence_SetItem)(jlong, jint, jobject, jlong);
jint    (*JyNI_PySequence_SetSlice)(jlong, jint, jint, jobject, jlong);
jint    (*JyNI_PySequence_Contains)(jlong, jobject, jlong);
jobject (*JyNI_PySequence_InPlaceConcat)(jlong, jobject, jlong);
jobject (*JyNI_PySequence_InPlaceRepeat)(jlong, jint, jlong);

// PyMapping-methods:
jint    (*JyNI_PyMapping_Length)(jlong, jlong);
//jobject (*JyNI_PyMapping_Subscript)(jlong, jobject, jlong);
//jint    (*JyNI_PyMapping_AssSubscript)(jlong, jobject, jobject, jlong);


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
	*(void **) (&JyNI_nativeIncref) = dlsym(JyNIHandle, "JyNI_nativeIncref");
	*(void **) (&JyNI_nativeDecref) = dlsym(JyNIHandle, "JyNI_nativeDecref");
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

	*(void **) (&JyNI_PyCFunction_getSelf) = dlsym(JyNIHandle, "JyNI_PyCFunction_getSelf");
	*(void **) (&JyNI_PyCFunction_getModule) = dlsym(JyNIHandle, "JyNI_PyCFunction_getModule");
	*(void **) (&JyNI_CMethodDef_bind) = dlsym(JyNIHandle, "JyNI_CMethodDef_bind");

	*(void **) (&JyRefMonitor_setMemDebugFlags) = dlsym(JyNIHandle, "JyRefMonitor_setMemDebugFlags");
	*(void **) (&JyGC_clearNativeReferences) = dlsym(JyNIHandle, "JyGC_clearNativeReferences");
	*(void **) (&JyGC_restoreCStubBackend) = dlsym(JyNIHandle, "JyGC_restoreCStubBackend");
	*(void **) (&JyGC_validateGCHead) = dlsym(JyNIHandle, "JyGC_validateGCHead");
	*(void **) (&JyGC_nativeTraverse) = dlsym(JyNIHandle, "JyGC_nativeTraverse");
	*(void **) (&JyNI_releaseWeakReferent) = dlsym(JyNIHandle, "JyNI_releaseWeakReferent");

	*(void **) (&JyNI_getItem) = dlsym(JyNIHandle, "JyNI_getItem");
	*(void **) (&JyNI_setItem) = dlsym(JyNIHandle, "JyNI_setItem");
	*(void **) (&JyNI_delItem) = dlsym(JyNIHandle, "JyNI_delItem");
	*(void **) (&JyNI_PyObjectLength) = dlsym(JyNIHandle, "JyNI_PyObjectLength");
	*(void **) (&JyNI_descr_get) = dlsym(JyNIHandle, "JyNI_descr_get");
	*(void **) (&JyNI_descr_set) = dlsym(JyNIHandle, "JyNI_descr_set");
	*(void **) (&JyNI_PyObject_Compare) = dlsym(JyNIHandle, "JyNI_PyObject_Compare");
	*(void **) (&JyNI_PyObject_RichCompare) = dlsym(JyNIHandle, "JyNI_PyObject_RichCompare");
	*(void **) (&JyNI_PyObject_GetIter) = dlsym(JyNIHandle, "JyNI_PyObject_GetIter");
	*(void **) (&JyNI_PyIter_Next) = dlsym(JyNIHandle, "JyNI_PyIter_Next");


	// PyNumber-methods:
	*(void **) (&JyNI_PyNumber_Add) = dlsym(JyNIHandle, "JyNI_PyNumber_Add");
	*(void **) (&JyNI_PyNumber_Subtract) = dlsym(JyNIHandle, "JyNI_PyNumber_Subtract");
	*(void **) (&JyNI_PyNumber_Multiply) = dlsym(JyNIHandle, "JyNI_PyNumber_Multiply");
	*(void **) (&JyNI_PyNumber_Divide) = dlsym(JyNIHandle, "JyNI_PyNumber_Divide");
	*(void **) (&JyNI_PyNumber_Remainder) = dlsym(JyNIHandle, "JyNI_PyNumber_Remainder");
	*(void **) (&JyNI_PyNumber_Divmod) = dlsym(JyNIHandle, "JyNI_PyNumber_Divmod");
	*(void **) (&JyNI_PyNumber_Power) = dlsym(JyNIHandle, "JyNI_PyNumber_Power");
	*(void **) (&JyNI_PyNumber_Negative) = dlsym(JyNIHandle, "JyNI_PyNumber_Negative");
	*(void **) (&JyNI_PyNumber_Positive) = dlsym(JyNIHandle, "JyNI_PyNumber_Positive");
	*(void **) (&JyNI_PyNumber_Absolute) = dlsym(JyNIHandle, "JyNI_PyNumber_Absolute");
	*(void **) (&JyNI_PyNumber_NonZero) = dlsym(JyNIHandle, "JyNI_PyNumber_NonZero");
	*(void **) (&JyNI_PyNumber_Invert) = dlsym(JyNIHandle, "JyNI_PyNumber_Invert");
	*(void **) (&JyNI_PyNumber_Lshift) = dlsym(JyNIHandle, "JyNI_PyNumber_Lshift");
	*(void **) (&JyNI_PyNumber_Rshift) = dlsym(JyNIHandle, "JyNI_PyNumber_Rshift");
	*(void **) (&JyNI_PyNumber_And) = dlsym(JyNIHandle, "JyNI_PyNumber_And");
	*(void **) (&JyNI_PyNumber_Xor) = dlsym(JyNIHandle, "JyNI_PyNumber_Xor");
	*(void **) (&JyNI_PyNumber_Or) = dlsym(JyNIHandle, "JyNI_PyNumber_Or");
	*(void **) (&JyNI_PyNumber_Coerce) = dlsym(JyNIHandle, "JyNI_PyNumber_Coerce");
	*(void **) (&JyNI_PyNumber_Int) = dlsym(JyNIHandle, "JyNI_PyNumber_Int");
	*(void **) (&JyNI_PyNumber_Long) = dlsym(JyNIHandle, "JyNI_PyNumber_Long");
	*(void **) (&JyNI_PyNumber_Float) = dlsym(JyNIHandle, "JyNI_PyNumber_Float");
	*(void **) (&JyNI_PyNumber_Oct) = dlsym(JyNIHandle, "JyNI_PyNumber_Oct");
	*(void **) (&JyNI_PyNumber_Hex) = dlsym(JyNIHandle, "JyNI_PyNumber_Hex");
	*(void **) (&JyNI_PyNumber_InPlaceAdd) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceAdd");
	*(void **) (&JyNI_PyNumber_InPlaceSubtract) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceSubtract");
	*(void **) (&JyNI_PyNumber_InPlaceMultiply) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceMultiply");
	*(void **) (&JyNI_PyNumber_InPlaceDivide) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceDivide");
	*(void **) (&JyNI_PyNumber_InPlaceRemainder) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceRemainder");
	*(void **) (&JyNI_PyNumber_InPlacePower) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlacePower");
	*(void **) (&JyNI_PyNumber_InPlaceLshift) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceLshift");
	*(void **) (&JyNI_PyNumber_InPlaceRshift) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceRshift");
	*(void **) (&JyNI_PyNumber_InPlaceAnd) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceAnd");
	*(void **) (&JyNI_PyNumber_InPlaceXor) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceXor");
	*(void **) (&JyNI_PyNumber_InPlaceOr) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceOr");
	*(void **) (&JyNI_PyNumber_FloorDivide) = dlsym(JyNIHandle, "JyNI_PyNumber_FloorDivide");
	*(void **) (&JyNI_PyNumber_TrueDivide) = dlsym(JyNIHandle, "JyNI_PyNumber_TrueDivide");
	*(void **) (&JyNI_PyNumber_InPlaceFloorDivide) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceFloorDivide");
	*(void **) (&JyNI_PyNumber_InPlaceTrueDivide) = dlsym(JyNIHandle, "JyNI_PyNumber_InPlaceTrueDivide");
	*(void **) (&JyNI_PyNumber_Index) = dlsym(JyNIHandle, "JyNI_PyNumber_Index");

	// PySequence-methods:
	*(void **) (&JyNI_PySequence_Length) = dlsym(JyNIHandle, "JyNI_PySequence_Length");
	*(void **) (&JyNI_PySequence_Concat) = dlsym(JyNIHandle, "JyNI_PySequence_Concat");
	*(void **) (&JyNI_PySequence_Repeat) = dlsym(JyNIHandle, "JyNI_PySequence_Repeat");
	*(void **) (&JyNI_PySequence_GetItem) = dlsym(JyNIHandle, "JyNI_PySequence_GetItem");
	*(void **) (&JyNI_PySequence_GetSlice) = dlsym(JyNIHandle, "JyNI_PySequence_GetSlice");
	*(void **) (&JyNI_PySequence_SetItem) = dlsym(JyNIHandle, "JyNI_PySequence_SetItem");
	*(void **) (&JyNI_PySequence_SetSlice) = dlsym(JyNIHandle, "JyNI_PySequence_SetSlice");
	*(void **) (&JyNI_PySequence_Contains) = dlsym(JyNIHandle, "JyNI_PySequence_Contains");
	*(void **) (&JyNI_PySequence_InPlaceConcat) = dlsym(JyNIHandle, "JyNI_PySequence_InPlaceConcat");
	*(void **) (&JyNI_PySequence_InPlaceRepeat) = dlsym(JyNIHandle, "JyNI_PySequence_InPlaceRepeat");

	// PyMapping-methods:
	*(void **) (&JyNI_PyMapping_Length) = dlsym(JyNIHandle, "JyNI_PyMapping_Length");
//	*(void **) (&JyNI_PyMapping_Subscript) = dlsym(JyNIHandle, "JyNI_PyMapping_Subscript");
//	*(void **) (&JyNI_PyMapping_AssSubscript) = dlsym(JyNIHandle, "JyNI_PyMapping_AssSubscript");


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
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_getAttrString
	(JNIEnv *env, jclass class, jlong handle, jstring name, jlong tstate)
{
	return (*JyNIGetAttrString)(env, class, handle, name, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setAttrString
 * Signature: (JLjava/lang/String;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_setAttrString
	(JNIEnv *env, jclass class, jlong handle, jstring name, jobject value, jlong tstate)
{
	return (*JyNISetAttrString)(env, class, handle, name, value, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    repr
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_repr
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNIrepr)(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsString
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_JyNI_JyNI_PyObjectAsString
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNIPyObjectAsString)(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsPyString
 * Signature: (JJ)Lorg/python/core/PyString;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyObjectAsPyString
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
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
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1get
	(JNIEnv *env, jclass class, jlong handle, jint index)
{
	return (*JyList_get)(env, class, handle, index);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_size
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyList_1size
	(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyList_size)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_set
 * Signature: (JILorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1set
	(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
	return (*JyList_set)(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_add
 * Signature: (JILorg/python/core/PyObject;J)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyList_1add
	(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
	(*JyList_add)(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_remove
 * Signature: (JI)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1remove
	(JNIEnv *env, jclass class, jlong handle, jint index)
{
	return (*JyList_remove)(env, class, handle, index);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JySet_putSize
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JySet_1putSize
	(JNIEnv *env, jclass class, jlong handle, jint size)
{
	(*JySet_putSize)(env, class, handle, size);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyRefMonitor_setMemDebugFlags
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyRefMonitor_1setMemDebugFlags
	(JNIEnv *env, jclass class, jint flags)
{
	(*JyRefMonitor_setMemDebugFlags)(env, class, flags);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_clearNativeReferences
 * Signature: ([JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_JyNI_JyNI_JyGC_1clearNativeReferences
	(JNIEnv *env, jclass class, jlongArray references, jlong tstate)
{
	return (*JyGC_clearNativeReferences)(env, class, references, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    lookupFromHandle
 * Signature: (J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_lookupFromHandle
	(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyNIlookupFromHandle)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    currentNativeRefCount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_currentNativeRefCount
	(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyNIcurrentNativeRefCount)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    nativeIncref
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_nativeIncref
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	(*JyNI_nativeIncref)(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    nativeDecref
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_nativeDecref
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	(*JyNI_nativeDecref)(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    getNativeTypeName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_JyNI_JyNI_getNativeTypeName
	(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyNIgetNativeTypeName)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_restoreCStubBackend
 * Signature: (JLorg/python/core/PyObject;LJyNI/gc/JyGCHead;)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyGC_1restoreCStubBackend
	(JNIEnv *env, jclass class, jlong handle, jobject backend, jobject newHead)
{
	(*JyGC_restoreCStubBackend)(env, class, handle, backend, newHead);
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
	return (*JyGC_validateGCHead)(env, class, handle, oldLinks);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_nativeTraverse
 * Signature: (J)[J
 */
JNIEXPORT jlongArray JNICALL Java_JyNI_JyNI_JyGC_1nativeTraverse
	(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyGC_nativeTraverse)(env, class, handle);
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
	return (*JyNI_getItem)(env, class, handle, key, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setItem
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_setItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jobject value, jlong tstate)
{
	return (*JyNI_setItem)(env, class, handle, key, value, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    delItem
 * Signature: (JLorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_delItem
	(JNIEnv *env, jclass class, jlong handle, jobject key, jlong tstate)
{
	return (*JyNI_delItem)(env, class, handle, key, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectLength
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_PyObjectLength
	(JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNI_PyObjectLength)(env, class, handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    descr_get
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_descr_1get
  (JNIEnv *env, jclass class, jlong self, jobject obj, jobject type, jlong tstate)
{
	return (*JyNI_descr_get)(self, obj, type, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    descr_set
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_descr_1set
  (JNIEnv *env, jclass class, jlong self, jobject obj, jobject value, jlong tstate)
{
	return (*JyNI_descr_set)(self, obj, value, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyCFunction_getSelf
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyCFunction_1getSelf
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNI_PyCFunction_getSelf)(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyCFunction_getModule
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyCFunction_1getModule
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNI_PyCFunction_getModule)(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_CMethodDef_bind
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1CMethodDef_1bind
  (JNIEnv *env, jclass class, jlong handle, jobject bindTo, jlong tstate)
{
	return (*JyNI_CMethodDef_bind)(handle, bindTo, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyObject_Compare
 * Signature: (JLorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PyObject_1Compare
  (JNIEnv *env, jclass class, jlong handle, jobject o, jlong tstate)
{
	return (*JyNI_PyObject_Compare)(handle, o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyObject_RichCompare
 * Signature: (JLorg/python/core/PyObject;IJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyObject_1RichCompare
  (JNIEnv *env, jclass class, jlong handle, jobject o, jint op, jlong tstate)
{
	return (*JyNI_PyObject_RichCompare)(handle, o, op, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyObject_GetIter
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyObject_1GetIter
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNI_PyObject_GetIter)(handle, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyIter_Next
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyIter_1Next
  (JNIEnv *env, jclass class, jlong handle, jlong tstate)
{
	return (*JyNI_PyIter_Next)(handle, tstate);
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
	return (*JyNI_PyNumber_Add)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Subtract
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Subtract
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Subtract)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Multiply
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Multiply
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Multiply)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Divide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Divide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Divide)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Remainder
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Remainder
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Remainder)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Divmod
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Divmod
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Divmod)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Power
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Power
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jobject o3, jlong tstate)
{
	return (*JyNI_PyNumber_Power)(o1, o2, o3, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Negative
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Negative
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Negative)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Positive
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Positive
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Positive)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Absolute
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Absolute
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Absolute)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_NonZero
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1NonZero
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_NonZero)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Invert
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Invert
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Invert)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Lshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Lshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Lshift)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Rshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Rshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Rshift)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_And
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1And
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_And)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Xor
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Xor
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Xor)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Or
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Or
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Or)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Coerce
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Coerce
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_Coerce)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Int
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Int
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Int)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Long
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Long
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Long)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Float
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Float
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Float)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Oct
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Oct
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Oct)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Hex
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Hex
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Hex)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceAdd
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceAdd
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceAdd)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceSubtract
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceSubtract
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceSubtract)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceMultiply
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceMultiply
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceMultiply)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceDivide)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceRemainder
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceRemainder
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceRemainder)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlacePower
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlacePower
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jobject o3, jlong tstate)
{
	return (*JyNI_PyNumber_InPlacePower)(o1, o2, o3, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceLshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceLshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceLshift)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceRshift
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceRshift
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceRshift)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceAnd
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceAnd
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceAnd)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceXor
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceXor
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceXor)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceOr
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceOr
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceOr)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_FloorDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1FloorDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_FloorDivide)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_TrueDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1TrueDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_TrueDivide)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceFloorDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceFloorDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceFloorDivide)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_InPlaceTrueDivide
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1InPlaceTrueDivide
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PyNumber_InPlaceTrueDivide)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PyNumber_Index
 * Signature: (JJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyNumber_1Index
  (JNIEnv *env, jclass class, jlong o, jlong tstate)
{
	return (*JyNI_PyNumber_Index)(o, tstate);
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
	return (*JyNI_PySequence_Length)(o, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_Concat
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1Concat
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PySequence_Concat)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_Repeat
 * Signature: (JIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1Repeat
  (JNIEnv *env, jclass class, jlong o, jint l, jlong tstate)
{
	return (*JyNI_PySequence_Repeat)(o, l, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_GetItem
 * Signature: (JIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1GetItem
  (JNIEnv *env, jclass class, jlong o, jint l, jlong tstate)
{
	return (*JyNI_PySequence_GetItem)(o, l, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_GetSlice
 * Signature: (JIIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1GetSlice
  (JNIEnv *env, jclass class, jlong o, jint l1, jint l2, jlong tstate)
{
	return (*JyNI_PySequence_GetSlice)(o, l1, l2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_SetItem
 * Signature: (JILorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1SetItem
  (JNIEnv *env, jclass class, jlong o1, jint l, jobject o2, jlong tstate)
{
	return (*JyNI_PySequence_SetItem)(o1, l, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_SetSlice
 * Signature: (JIILorg/python/core/PyObject;J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1SetSlice
  (JNIEnv *env, jclass class, jlong o1, jint l1, jint l2, jobject o2, jlong tstate)
{
	return (*JyNI_PySequence_SetSlice)(o1, l1, l2, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_Contains
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1Contains
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PySequence_Contains)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_InPlaceConcat
 * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1InPlaceConcat
  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
{
	return (*JyNI_PySequence_InPlaceConcat)(o1, o2, tstate);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyNI_PySequence_InPlaceRepeat
 * Signature: (JIJ)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PySequence_1InPlaceRepeat
  (JNIEnv *env, jclass class, jlong o, jint l, jlong tstate)
{
	return (*JyNI_PySequence_InPlaceRepeat)(o, l, tstate);
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
	return (*JyNI_PyMapping_Length)(o, tstate);
}

///*
// * Class:     JyNI_JyNI
// * Method:    JyNI_PyMapping_Subscript
// * Signature: (JLorg/python/core/PyObject;J)Lorg/python/core/PyObject;
// */
//JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyNI_1PyMapping_1Subscript
//  (JNIEnv *env, jclass class, jlong o1, jobject o2, jlong tstate)
//{
//	return (*JyNI_PyMapping_Subscript)(o1, o2, tstate);
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
//	return (*JyNI_PyMapping_AssSubscript)(o1, o2, o3, tstate);
//}
