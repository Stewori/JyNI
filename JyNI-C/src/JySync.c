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
 * JySync.c
 *
 *  Created on: 23.04.2013, 04:13:04
 *      Author: Stefan Richthofer
 */

#include <JyNI.h>
#include <code_JyNI.h>

/*
typedef void (*jy2pySync)(jobject, PyObject*);
typedef void (*py2jySync)(PyObject*, jobject);
typedef jobject (*jyInitSync)(PyObject*);
typedef PyObject* (*pyInitSync)(jobject);
//typedef void (*jy2pyItemSync)(jobject, PyObject*, int index);
//typedef void (*py2jyItemSync)(PyObject*, jobject, int index);
typedef jlong (*pyChecksum)(PyObject*);
typedef jlong (*jyChecksum)(jobject);

typedef struct {jy2pySync jy2py; py2jySync py2jy; jyInitSync jyInit; pyInitSync pyInit; pyChecksum pyCheck; jyChecksum jyCheck;} SyncFunctions;
//typedef struct {SyncInfo sync; jy2pyItemSync jy2pyItem; py2jyItemSync py2jyItem;} SyncVarInfo;
*/

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * For every item in the tuple also a new reference is created. However the tuple
 * deallocator will take care of these.
 */
PyObject* JySync_Init_PyTuple_From_JyTuple(jobject src)
{
	//jputs("JySync_Init_PyTuple_From_JyTuple");
	env(NULL);
	//jarray back = (*env)->CallObjectMethod(env, src, pyTupleGetArray);
	jint srcSize = (*env)->CallIntMethod(env, src, pyTupleSize);
	//jputs("### creating tuple...");
	PyObject* er = PyTuple_New(srcSize);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) er, JNI_FALSE);
	AS_JY_WITH_GC(er)->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	//jputs("### created tuple...");
	//Py_XINCREF(er);
	//if (srcSize != PyTuple_GET_SIZE(dest)) //...throw exception since tuple is immutable
	int i;
	for (i = 0; i < srcSize; ++i)
	{
		PyObject* item = JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pyTuplePyGet, i));
		//Py_XINCREF(item);
		//PyTuple_SetItem(er, i, item);
		PyTuple_SET_ITEM(er, i, item);
//		if (!item) {
//			jputs("Add null-item to tuple:");
//			jputsLong(er);
//			jputsLong(i);
//			jobject jNull = (*env)->CallObjectMethod(env, src, pyTuplePyGet, i);
//			if (!jNull) jputs("j also null");
//			else if ((*env)->IsSameObject(env, jNull, NULL)) jputs("j equal null");
//			else {
//				jputs("j not null:");
//				JyNI_printJInfo(jNull);
//			}
//		}
	}
	//JYNI_GC_TUPLE_EXPLORE(er);
	return er;
}

jobject JySync_Init_JyTuple_From_PyTuple(PyObject* src)
{
	if (PyTuple_GET_SIZE(src) == 0)
	{
		return JyEmptyTuple;
	} else {
		env(NULL);
		jarray back = (*env)->NewObjectArray(env, PyTuple_GET_SIZE(src), pyObjectClass, NULL);
		//if (srcSize != PyTuple_GET_SIZE(dest)) //...throw exception since tuple is immutable
		int i;
		for (i = 0; i < PyTuple_GET_SIZE(src); ++i)
		{
			(*env)->SetObjectArrayElement(env, back, i, JyNI_JythonPyObject_FromPyObject(
					PyTuple_GET_ITEM(src, i)));
			//Py_XINCREF(PyTuple_GET_ITEM(src, i));
		}
		return (*env)->NewObject(env, pyTupleClass, pyTupleByPyObjectArrayBooleanConstructor,
				back, JNI_FALSE);
	}
}

jboolean isPreAllocatedJythonString(jobject obj, char value)//jchar value)
{
	if (value >= LETTERCHAR_MAXJYTHON)
		return JNI_FALSE;
	env(JNI_FALSE);
	jarray scache = (*env)->GetStaticObjectField(env, pyPyClass, pyPyLetters);
	jobject cachedString = (*env)->GetObjectArrayElement(env, scache, (jint) value);
	return (*env)->IsSameObject(env, cachedString, obj);
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyString_From_JyString(jobject src)
{
	env(NULL);
	jstring jstr = (*env)->CallObjectMethod(env, src, pyStringAsString);
	//cstr_from_jstring(cstr, jstr);

	//cstr_from_jstring(cstrName, jstr)
	char* utf_string = (*env)->GetStringUTFChars(env, jstr, NULL);
	size_t len = strlen(utf_string);
	char cstr[len+1];
	strcpy(cstr, utf_string);
	(*env)->ReleaseStringUTFChars(env, jstr, utf_string);
	if (len == 1) {
		PyObject* result = PyString_FromString(cstr);
		if (isPreAllocatedJythonString(src, cstr[0]))
			/* The JY_CACHE_ETERNAL-flag tells JyNI permanently that accessing
			 * AS_JY_NO_GC(blah)->jy is safe, i.e. the reference cannot be
			 * garbage-collected on Java-side. Usually methods in JySync should
			 * not set flags or perform JyObject initialization. However setting
			 * this flag is only feasible in JySync.c as the decision is very
			 * type-specific. So we set it here to tell the caller.
			 */
			AS_JY_NO_GC(result)->flags |= JY_CACHE_ETERNAL_FLAG_MASK;
		return result;
	} else {
		PyObject* result = PyString_FromString(cstr);
		return result;
	}
}

jobject JySync_Init_JyString_From_PyString(PyObject* src)
{
	//todo: check interned-regulations on jython-side
	env(NULL);
	jstring jstr = (*env)->NewStringUTF(env, PyString_AS_STRING(src));
	if (JyNI_HasJyAttribute(AS_JY_NO_GC(src), JyAttributeStringInterned))
		jstr = (*env)->CallObjectMethod(env, jstr, stringIntern);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewString, jstr);
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src)
{
	env(NULL);
	jstring jstr = (*env)->CallObjectMethod(env, src, pyUnicodeAsString);
	jstring charsetName = (*env)->NewStringUTF(env, "UTF-16BE");
	jobject stringByteArray = (*env)->CallObjectMethod(env, jstr,
		stringGetBytesUsingCharset, charsetName);
	jbyte* stringBytes = (*env)->GetByteArrayElements(env, stringByteArray, NULL);
	jsize len = (*env)->GetArrayLength(env, stringByteArray);
	int byteOrder = 1; //indicates BE
	PyObject* unicode = PyUnicode_DecodeUTF16(
			(char*) stringBytes,  /* UTF-16 encoded string */
			len,                  /* size of string */
			NULL,                 /* error handling */
			&byteOrder            /* pointer to byteorder to use
			                         0=native;-1=LE,1=BE; */
			);
	(*env)->ReleaseByteArrayElements(env, stringByteArray, stringBytes, JNI_ABORT);
	return unicode;
}

//UTF-8 variant
//PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src)
//{
////	jputs("JySync_Init_PyUnicode_From_JyUnicode");
//	env(NULL);
//	jstring jstr = (*env)->CallObjectMethod(env, src, pyUnicodeAsString);
//	//jchar* uc = (*env)->GetStringChars(env, jstr, NULL);
//	jstring charsetName = (*env)->NewStringUTF(env, "UTF-8");
//	jobject stringByteArray = (*env)->CallObjectMethod(env, jstr,
//		stringGetBytesUsingCharset, charsetName);
//	jbyte* stringBytes = (*env)->GetByteArrayElements(env, stringByteArray, NULL);
//	//jsize len = (*env)->GetStringLength(env, jstr);
//	jsize len = (*env)->GetArrayLength(env, stringByteArray);
//	//jputsLong(len);
//	PyObject* unicode = PyUnicode_DecodeUTF8((char*) stringBytes, len, NULL);
//	(*env)->ReleaseByteArrayElements(env, stringByteArray, stringBytes, JNI_ABORT);
//	return unicode;
//}

//PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src)
//{
//	jputs("JySync_Init_PyUnicode_From_JyUnicode");
//	env(NULL);
//	jstring jstr = (*env)->CallObjectMethod(env, src, pyUnicodeAsString);
//	jchar* uc = (*env)->GetStringChars(env, jstr, NULL);
//	//PyObject* unicode = PyUnicode_EncodeUTF16(
//	int byteOrder = 1;
//	PyObject* unicode = PyUnicode_DecodeUTF16(
//		    (char*) uc, //const char *string,         /* UTF-16 encoded string */
//		    (*env)->GetStringLength(env, jstr), //Py_ssize_t length,          /* size of string */
//		    NULL, //const char *errors,         /* error handling */
//		    &byteOrder //int *byteorder             pointer to byteorder to use
////		                                   0=native;-1=LE,1=BE;
//		    );
//	(*env)->ReleaseStringChars(env, jstr, uc);
//	jputs("result-type:");
//	jputs(unicode->ob_type->tp_name);
//	return unicode;
//	//cstr_from_jstring(cstr, jstr);
//	//return PyString_FromString(cstr);
//}

jobject JySync_Init_JyUnicode_From_PyUnicode(PyObject* src)
{
	PyObject* utf16 = PyUnicode_EncodeUTF16(PyUnicode_AS_UNICODE(src),
			 PyUnicode_GET_SIZE(src),
			 NULL,
			 1); //BE
	Py_ssize_t len = PyString_GET_SIZE(utf16);
	//Py_ssize_t len = PyUnicode_GET_DATA_SIZE(utf16);
//	jputsLong(len);
	env(NULL);
	//jstring jstr = (*env)->NewStringUTF(env, PyString_AS_STRING(src));
	jstring charsetName = (*env)->NewStringUTF(env, "UTF-16BE");
	jobject strByteArray = (*env)->NewByteArray(env, len);
	jbyte* strBytes = (*env)->GetByteArrayElements(env, strByteArray, NULL);
	memcpy(strBytes, PyString_AS_STRING(utf16), len);
	//memcpy(strBytes, PyUnicode_AS_DATA(utf16), len);
	(*env)->ReleaseByteArrayElements(env, strByteArray, strBytes, 0); //copy back and free buffer
	jobject jstr = (*env)->NewObject(env, stringClass,
		stringFromBytesAndCharsetNameConstructor, strByteArray, charsetName);
//	if (JyNI_HasJyAttribute(AS_JY_NO_GC(src), JyAttributeStringInterned))
//		jstr = (*env)->CallObjectMethod(env, jstr, stringIntern);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewUnicode, jstr);
}

//jobject JySync_Init_JyUnicode_From_PyUnicode(PyObject* src)
//{
////	jputs("JySync_Init_JyUnicode_From_PyUnicode");
//	PyObject* utf8 = PyUnicode_AsUTF8String(src);
//	//PyUnicode_GET_SIZE(src)
//	Py_ssize_t len = PyString_GET_SIZE(utf8);
////	jputsLong(len);
//	env(NULL);
//	//jstring jstr = (*env)->NewStringUTF(env, PyString_AS_STRING(src));
//	jstring charsetName = (*env)->NewStringUTF(env, "UTF-8");
//	jobject strByteArray = (*env)->NewByteArray(env, len);
//	jbyte* strBytes = (*env)->GetByteArrayElements(env, strByteArray, NULL);
//	memcpy(strBytes, PyString_AS_STRING(utf8), len);
//	(*env)->ReleaseByteArrayElements(env, strByteArray, strBytes, 0); //copy back and free buffer
//	jobject jstr = (*env)->NewObject(env, stringClass,
//		stringFromBytesAndCharsetNameConstructor, strByteArray, charsetName);
////	if (JyNI_HasJyAttribute(AS_JY_NO_GC(src), JyAttributeStringInterned))
////		jstr = (*env)->CallObjectMethod(env, jstr, stringIntern);
//	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewUnicode, jstr);
//}

static inline jboolean isPreAllocatedJythonInt(jobject obj, jint value)
{
	if (value < -NSMALLNEGINTSJYTHON || value >= NSMALLPOSINTSJYTHON)
		return JNI_FALSE;
	env(JNI_FALSE);
	jarray icache = (*env)->GetStaticObjectField(env, pyPyClass, pyPyIntegerCache);
	jobject cachedInt = (*env)->GetObjectArrayElement(env, icache, value+NSMALLNEGINTSJYTHON);
	return (*env)->IsSameObject(env, cachedInt, obj);
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyInt_From_JyInt(jobject src)
{
	env(NULL);
	//return PyInt_FromLong((long) (*env)->CallLongMethod(env, src, pyIntAsLong));
	jint value = (*env)->CallIntMethod(env, src, pyIntGetValue);
	PyObject* result = PyInt_FromLong((long) value);
	if (isPreAllocatedJythonInt(src, value))
		/* The JY_CACHE_ETERNAL-flag tells JyNI permanently that accessing
		 * AS_JY_NO_GC(blah)->jy is safe, i.e. the reference cannot be
		 * garbage-collected on Java-side. Usually methods in JySync should
		 * not set flags or perform JyObject initialization. However setting
		 * this flag is only feasible in JySync.c as the decision is very
		 * type-specific. So we set it here to tell the caller.
		 */
		AS_JY_NO_GC(result)->flags |= JY_CACHE_ETERNAL_FLAG_MASK;
	return result;
}

/*
 * This method can in fact return a Jython-PyLong. This can be the case, if
 * on 64-bit systems, native PyInteger has 64 bit backend while Jython-PyInteger
 * is always guaranteed to be 32 bit.
 * This might cause confusion, if the type is looked up again for some reason.
 * As of this writing, we consider this as less confusing than changing a number
 * value during conversion.
 */
jobject JySync_Init_JyInt_From_PyInt(PyObject* src)
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewIntFromLong, (jlong) PyInt_AS_LONG(src));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyFloat_From_JyFloat(jobject src)
{
	env(NULL);
	return PyFloat_FromDouble((double) (*env)->CallDoubleMethod(env, src, pyFloatAsDouble));
}

jobject JySync_Init_JyFloat_From_PyFloat(PyObject* src)
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewFloatFromDouble, (jdouble) PyFloat_AS_DOUBLE(src));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyComplex_From_JyComplex(jobject src)
{
	env(NULL);
	return PyComplex_FromDoubles(
			(double) (*env)->GetDoubleField(env, src, pyComplexRealField),
			(double) (*env)->GetDoubleField(env, src, pyComplexImagField));
}

jobject JySync_Init_JyComplex_From_PyComplex(PyObject* src)
{
	env(NULL);
	return (*env)->NewObject(env, pyComplexClass, pyComplexBy2DoubleConstructor,
			PyComplex_RealAsDouble(src),
			PyComplex_ImagAsDouble(src));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyLong_From_JyLong(jobject src)
{
	env(NULL);
	jobject bival = (*env)->CallObjectMethod(env, src, pyLongGetValue);
	jarray jbytes = (*env)->CallObjectMethod(env, bival, bigIntToByteArray);
	jsize n = (*env)->GetArrayLength(env, jbytes);
	jbyte* bbytes = (*env)->GetByteArrayElements(env, jbytes, NULL);
	//memcpy(bytes+n-cpl, bbytes+lb-cpl, cpl);
	PyObject *er = _PyLong_FromByteArray(bbytes, (size_t) n, JNI_FALSE, JNI_TRUE);
	(*env)->ReleaseByteArrayElements(env, jbytes, bbytes, JNI_ABORT);
	return er;
}

jobject JySync_Init_JyLong_From_PyLong(PyObject* src)
{
	env(NULL);
	size_t numBits = _PyLong_NumBits(src);

	size_t n = 1+numBits/8;
	if (numBits%8 != 0) ++n;
	jarray jbytes = (*env)->NewByteArray(env, (jsize) n);
	jbyte* bbytes = (*env)->GetByteArrayElements(env, jbytes, NULL);
	//memcpy(bbytes, bytes, n);
	_PyLong_AsByteArray(src, bbytes, n, JNI_FALSE, JNI_TRUE);
	(*env)->ReleaseByteArrayElements(env, jbytes, bbytes, 0);
	//jobject bival;
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewLongFromBigInt,
			(*env)->NewObject(env, bigIntClass, bigIntegerFromByteArrayConstructor, jbytes));
}

jobject JySync_Init_JyList_From_PyList(PyObject* src)
{
	//jputs(__FUNCTION__);
	//Since it is likely that src was created by an extension, we
	//explicitly explore it here:
	JyNI_GC_ExploreObject(src);
	//JyNI_GC_ExploreObject should have initialized JyGCHead to a JyList-object.
	env(NULL);
	jobject jyList = (*env)->NewLocalRef(env, (jobject) JyObject_GetJyGCHead(src, AS_JY_WITH_GC(src)));
	if (!jyList || (*env)->IsSameObject(env, jyList, NULL)) {
		jputs("This should not happen! JyList gc-head was deleted after explore.");
		jputs(__FUNCTION__);
		if (!jyList) jputs("truely NULL");
		else (*env)->DeleteWeakGlobalRef(env, jyList);
		jputs("Restore it...");
		jyList = (*env)->NewObject(env, JyListClass, JyListFromBackendHandleConstructor, (jlong) src);
		JyObject_AddOrSetJyGCHead(src, AS_JY_WITH_GC(src), (*env)->NewWeakGlobalRef(env, jyList));
		JyNI_GC_ExploreObject(src);
	}
//	jobject jylist = (*env)->NewObject(env, JyListClass, JyListFromBackendHandleConstructor, (jlong) src);
//	if ((*env)->ExceptionCheck(env)) {
//		jputs("Exception happened converting list:");
//		(*env)->ExceptionDescribe(env);
//	}
	return (*env)->CallStaticObjectMethod(env, pyListClass, pyListFromList, jyList);
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyList_From_JyList(jobject src)
{
	env(NULL);
	//maybe optimize a bit more here:
	//if most objects in the list have not yet been converted,
	//much lookup is done twice here.
	jarray handles = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupNativeHandles, src);
	jsize size = (*env)->GetArrayLength(env, handles);
	PyObject* op = PyList_New((Py_ssize_t) size);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNISetNativeHandle, src, (jlong) op, JNI_FALSE);
	AS_JY_WITH_GC(op)->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	jsize i;
	PyObject* v;
	jlong* arr = (*env)->GetLongArrayElements(env, handles, NULL);
	for (i = 0; i < size; ++i)
	{
		v = (PyObject*) arr[i];
		if (!v) v = JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pyListPyGet, i));
		else Py_INCREF(v);
//		if (!v) v = _JyNI_PyObject_FromJythonPyObject(
//				(*env)->CallObjectMethod(env, src, pyListPyGet, i), JNI_FALSE, JNI_FALSE, JNI_TRUE);
//JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject, jboolean lookupNative, jboolean checkCPeer, jboolean checkForType)
		PyList_SET_ITEM(op, i, v);
	}
	//jputsLong(__LINE__);
	(*env)->ReleaseLongArrayElements(env, handles, arr, JNI_ABORT);
	JyNI_GC_ExploreObject(op);
	//JyNI_GC_ExploreObject should have initialized JyGCHead to a JyList-object.
	jobject jyList = (*env)->NewLocalRef(env, (jobject) JyObject_GetJyGCHead(op, AS_JY_WITH_GC(op)));
	if (!jyList || (*env)->IsSameObject(env, jyList, NULL)) {
		jputs("This should not happen! JyList gc-head was deleted after explore.");
		jputs(__FUNCTION__);
		if (!jyList) jputs("truely NULL");
		else (*env)->DeleteWeakGlobalRef(env, jyList);
		jputs("Restore it...");
		jyList = (*env)->NewObject(env, JyListClass, JyListFromBackendHandleConstructor, (jlong) src);
		JyObject_AddOrSetJyGCHead(src, AS_JY_WITH_GC(src), (*env)->NewWeakGlobalRef(env, jyList));
		JyNI_GC_ExploreObject(src);
	}
	//(*env)->CallVoidMethod(env, jyList, JyListInstallToPyList, src);
	(*env)->SetObjectField(env, src, pyListBackend, jyList);
	(*env)->CallObjectMethod(env, jyList, pyObjectGCHeadSetObject, src);
	(*env)->DeleteLocalRef(env, jyList);

	//Py_INCREF(op); //For now we make the list immortal here. Later GC will take care of it.
	//(should now work without this)
	//jputsLong(__LINE__);
	return op;
}


//jobject JySync_Init_JySet_From_PySet(PyObject* src) not needed because of truncation

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PySet_From_JySet(jobject src) //needed because truncation is only partial
{
	PySetObject* so = (PySetObject *) PySet_Type.tp_alloc(&PySet_Type, 0);
	JyObject* jy = AS_JY(so);
	jy->jy = src;
	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	env(NULL);
	so->used = (*env)->CallIntMethod(env, src, pyBaseSetSize);
	jobject jySet = (*env)->NewObject(env, JySetClass, JySetFromBackendHandleConstructor, (jlong) so);
	(*env)->CallVoidMethod(env, jySet, JySetInstallToPySet, src); //maybe do this direct to have lesser security manager issues
	return (PyObject*) so;
}

//jobject JySync_Init_JyFrozenSet_From_PyFrozenSet(PyObject* src) not needed because of truncation

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyFrozenSet_From_JyFrozenSet(jobject src) //needed because truncation is only partial
{
	PySetObject* so = (PySetObject *) PyFrozenSet_Type.tp_alloc(&PyFrozenSet_Type, 0);
	JyObject* jy = AS_JY(so);
	jy->jy = src;
	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	env(NULL);
	so->used = (*env)->CallIntMethod(env, src, pyBaseSetSize);
	jobject jySet = (*env)->NewObject(env, JySetClass, JySetFromBackendHandleConstructor, (jlong) so);
	(*env)->CallVoidMethod(env, jySet, JySetInstallToPySet, src); //maybe do this direct to have lesser security manager issues
	return (PyObject*) so;
}


jobject JySync_Init_JyClass_From_PyClass(PyObject* src)
{
	PyClassObject* cls = (PyClassObject*) src;
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyClassClass, pyClassClassobj___new__,
		JyNI_JythonPyObject_FromPyObject(cls->cl_name),
		JyNI_JythonPyObject_FromPyObject(cls->cl_bases),
		JyNI_JythonPyObject_FromPyObject(cls->cl_dict));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyClass_From_JyClass(jobject src)
{
	env(NULL);
	jstring nm = (*env)->GetObjectField(env, src, pyClass__name__);
	cstr_from_jstring(cnm, nm);
	return PyClass_New(
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyClass__bases__)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyClass__dict__)),
		PyString_FromString(cnm));
}


jobject JySync_Init_JyInstance_From_PyInstance(PyObject* src)
{
	PyInstanceObject* inst = (PyInstanceObject*) src;
	env(NULL);
	/*
	 * We would have to suspend the result's FinalizeTrigger, but actually
	 * PyInstance doesn't get one anyway, if it is constructed using its
	 * constructor (rather than by PyClass.__call__). JyNIFinalizeTriggerFactory
	 * takes care to keep the instance's Finalizetrigger suspended as long
	 * as we desire.
	 */
	return (*env)->NewObject(env, pyInstanceClass, pyInstanceConstructor,
		JyNI_JythonPyObject_FromPyObject((PyObject*) inst->in_class),
		JyNI_JythonPyObject_FromPyObject(inst->in_dict));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyInstance_From_JyInstance(jobject src)
{
	env(NULL);
	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_suspendPyInstanceFinalizer, src);
	PyObject* er = PyInstance_NewRaw(
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyInstanceInstclassField)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyInstance__dict__)));
	return er;
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_Special_PyInstance(jobject src)
{
	env(NULL);
	//todo: Care for finalizer also in this case.
	//(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_suspendPyInstanceFinalizer, src);
	jobject old_cls = (*env)->CallStaticObjectMethod(env, JyNIClass,
						JyNI_getTypeOldStyleParent, src);

	PyObject* er = PyInstance_NewRaw(
		JyNI_PyObject_FromJythonPyObject(old_cls),
		JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pyObjectGetDict)));
	return er;
}


jobject JySync_Init_JyMethod_From_PyMethod(PyObject* src)
{
	PyMethodObject* meth = (PyMethodObject*) src;
	env(NULL);
	return (*env)->NewObject(env, pyMethodClass, pyMethodConstructor,
		JyNI_JythonPyObject_FromPyObject(meth->im_func),
		JyNI_JythonPyObject_FromPyObject(meth->im_self),
		JyNI_JythonPyObject_FromPyObject(meth->im_class));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyMethod_From_JyMethod(jobject src)
{
	env(NULL);
	return PyMethod_New(
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyMethod__func__)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyMethod__self__)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyMethodImClass)));
}

#define PyWeakref_GET_OBJECT0(ref)                          \
    (Py_REFCNT(((PyWeakReference *)(ref))->wr_object) > 0   \
     ? ((PyWeakReference *)(ref))->wr_object                \
     : NULL)

jobject JySync_Init_JyWeakReference_From_PyWeakReference(PyObject* src)
{
	PyObject* referent = PyWeakref_GET_OBJECT0(src);
	jputs("Sync PyWeakRef* -> jython");
	jputsLong((jlong) referent);
	jobject jReferent = NULL;
	if (referent) jReferent = JyNI_JythonPyObject_FromPyObject(referent);
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_createWeakReferenceFromNative,
			jReferent, (jlong) referent, NULL); //todo: Support native callbacks.
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyWeakReference_From_JyWeakReference(jobject src)
{
	// Todo: Handle case that the native counterpart of the Java-referent is
	//       not weakly referencable in CPython-terms.
	//       See PyType_SUPPORTS_WEAKREFS(Py_TYPE(ob))
	env(NULL);
	jobject jReferent = (*env)->CallObjectMethod(env, src, AbstractReference_get);
	PyObject* referent = JyNI_PyObject_FromJythonPyObject(jReferent);
	jputs("Sync jython -> PyWeakRef*");
	jputsLong((jlong) referent);
	/* Note that an extra Py_INCREF is not necessary since the conversion method
	 * returns a new reference.
	 * JyNI-note: It is okay to hold a refcount for the weakly referenced object.
	 * This will be decreffed when the Java-side GlobalRef is released.
	 */
	PyObject* result = PyWeakref_NewRef(referent, NULL);
	/* Todo: Consider how this shall work for non-heap objects.
	 */
	incWeakRefCount(AS_JY(result));
	// Todo: Support callback.
	return result;
}

jobject JySync_Init_JyWeakProxy_From_PyWeakProxy(PyObject* src)
{
	PyObject* referent = PyWeakref_GET_OBJECT0(src);
	jobject jReferent = NULL;
	if (referent) jReferent = JyNI_JythonPyObject_FromPyObject(referent);
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_createProxyFromNative,
			jReferent, (jlong) referent, NULL); //todo: Support native callbacks.
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyWeakProxy_From_JyWeakProxy(jobject src)
{
	// Todo: Handle case that the native counterpart of the Java-referent is
	//       not weakly referencable in CPython-terms.
	//       See PyType_SUPPORTS_WEAKREFS(Py_TYPE(ob))
	env(NULL);
	jobject jReferent = (*env)->CallObjectMethod(env, src, AbstractReference_get);
	PyObject* referent = JyNI_PyObject_FromJythonPyObject(jReferent);
	/* Note that an extra Py_INCREF is not necessary since the conversion method
	 * returns a new reference.
	 * JyNI-note: It is okay to hold a refcount for the weakly referenced object.
	 * This will be decreffed when the Java-side GlobalRef is released.
	 */
	PyObject* result = PyWeakref_NewProxy(referent, NULL);
	// Todo: Support callback.
	return result;
}

jobject JySync_Init_JyWeakCallableProxy_From_PyWeakCallableProxy(PyObject* src)
{
	PyObject* referent = PyWeakref_GET_OBJECT0(src);
	jobject jReferent = NULL;
	if (referent) jReferent = JyNI_JythonPyObject_FromPyObject(referent);
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_createCallableProxyFromNative,
			jReferent, (jlong) referent, NULL); //todo: Support native callbacks.
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyWeakCallableProxy_From_JyWeakCallableProxy(jobject src)
{
	// Todo: Handle case that the native counterpart of the Java-referent is
	//       not weakly referencable in CPython-terms.
	//       See PyType_SUPPORTS_WEAKREFS(Py_TYPE(ob))
	env(NULL);
	jobject jReferent = (*env)->CallObjectMethod(env, src, AbstractReference_get);
	PyObject* referent = JyNI_PyObject_FromJythonPyObject(jReferent);
	/* Note that an extra Py_INCREF is not necessary since the conversion method
	 * returns a new reference.
	 * JyNI-note: It is okay to hold a refcount for the weakly referenced object.
	 * This will be decreffed when the Java-side GlobalRef is released.
	 */
	PyObject* result = PyWeakref_NewProxy(referent, NULL);
	// Todo: Support callback.
	return result;
}

//jobject JySync_Init_JyCode_From_PyCode(PyObject* src) not needed because of truncation
//PyObject* JySync_Init_PyCode_From_JyCode(jobject src)
void JySync_PyCode_From_JyCode(jobject src, PyObject* dest)
{
	env();
	//puts("JySync_PyCode_From_JyCode");
	//jobject jCode = JyNI_JythonPyObject_FromPyObject(src);
	jobject jArray = (*env)->GetObjectField(env, src, pyBaseCode_co_freevars);
	//puts("get free vars...");
	//if (jArray == NULL) //puts("free vars NULL");
	Py_ssize_t size = 0;
	if (jArray) size = (*env)->GetArrayLength(env, jArray);
	//puts("get free vars size...");

	/* We create a fake-tuple here. It uses minimal memory, i.e. has actual length 0.
	 * It provides number of free vars in its size-field to be suitable for the
	 * macro PyCode_GetNumFree(op), which is defined as
	 * (PyTuple_GET_SIZE((op)->co_freevars))
	 * Since the dummy-tuple is only accessed by this macro, this should cause no
	 * problems as long as extensions stick to official API. All methods that access the
	 * field are redirected to Jython. The corresponding memberdef was replaced by a
	 * getsetdef that directs to Jython.
	 *
	 * To not mess up with tuple deallocation, the dummy-tuple's actual size is restored
	 * in code_dealloc before the tuple's refcount is decreased.
	 */
	PyObject* dummyTuple = (PyObject*) PyObject_GC_NewVar(PyTupleObject, &PyTuple_Type, 0);
	PyTuple_GET_SIZE(dummyTuple) = size;
	((PyCodeObject*) dest)->co_freevars = dummyTuple;
}

//PyObject* JySync_Init_PyFunction_From_JyFunction(jobject src)
void JySync_PyFunction_From_JyFunction(jobject src, PyObject* dest)
{
//Need to sync:
//func_code, func_globals, func_module, func_defaults, func_closure
	env();
	jobject jCode = (*env)->GetObjectField(env, src, pyFunction__code__);
	PyObject* pt = JyNI_PyObject_FromJythonPyObject(jCode);
	((PyFunctionObject*) dest)->func_code = pt;
	jobject jGlobals = (*env)->CallObjectMethod(env, src, pyFunctionGetFuncGlobals);
	((PyFunctionObject*) dest)->func_globals = JyNI_PyObject_FromJythonPyObject(jGlobals);
	jobject jModule = (*env)->GetObjectField(env, src, pyFunction__module__);
	((PyFunctionObject*) dest)->func_module = JyNI_PyObject_FromJythonPyObject(jModule);
	jobject jDefaults = (*env)->CallObjectMethod(env, src, pyFunctionGetFuncDefaults);
	((PyFunctionObject*) dest)->func_defaults = JyNI_PyObject_FromJythonPyObject(jDefaults);
	jobject jClosure = (*env)->GetObjectField(env, src, pyFunctionFuncClosure);
	((PyFunctionObject*) dest)->func_closure = JyNI_PyObject_FromJythonPyObject(jClosure);
}

void JySync_JyCell_From_PyCell(PyObject* src, jobject dest)
{
	env();
	(*env)->SetObjectField(env, dest, pyCell_ob_ref,
		JyNI_JythonPyObject_FromPyObject(PyCell_GET(src)));

}

void JySync_PyCell_From_JyCell(jobject src, PyObject* dest)
{
	env();
	PyCell_SET(dest, JyNI_PyObject_FromJythonPyObject(
		(*env)->GetObjectField(env, src, pyCell_ob_ref)));
}

jobject JyExc_KeyErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsKeyError);
}

jobject JyExc_SystemExitFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsSystemExit);
}

jobject JyExc_EnvironmentErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsEnvironmentError);
}

jobject JyExc_SyntaxErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsSyntaxError);
}

jobject JyExc_UnicodeErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsUnicodeError);
}

#ifdef Py_USING_UNICODE
jobject JyExc_UnicodeEncodeErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsUnicodeEncodeError);
}

jobject JyExc_UnicodeDecodeErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsUnicodeDecodeError);
}

jobject JyExc_UnicodeTranslateErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsUnicodeTranslateError);
}
#endif

void JySync_PyType_From_JyType(jobject src, PyObject* dest)
{
	PyTypeObject* tp = (PyTypeObject*) dest;
	env();

	//name:
	jobject jtmp = (*env)->CallObjectMethod(env, src, pyTypeGetName);
	//if (!jtmp) jputs("JySync_PyType_From_JyType: type with NULL-name!");
	//cstr_from_jstring(cname, jname);
	char* utf_string = (*env)->GetStringUTFChars(env, jtmp, NULL);
	char* cname = malloc(strlen(utf_string)+1);
	strcpy(cname, utf_string);
	(*env)->ReleaseStringUTFChars(env, jtmp, utf_string);
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY_WITH_GC(dest), JyAttributeTypeName, cname, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	tp->tp_name = cname;

	//dict:
	jtmp = (*env)->CallObjectMethod(env, src, pyObjectFastGetDict);
	tp->tp_dict = JyNI_PyObject_FromJythonPyObject(jtmp);

	//base:
	jtmp = (*env)->CallObjectMethod(env, src, pyTypeGetBase);
	tp->tp_base = (PyTypeObject*) JyNI_PyObject_FromJythonPyObject(jtmp);

	//bases:
	jtmp = (*env)->CallObjectMethod(env, src, pyTypeGetBases);
	tp->tp_bases = JyNI_PyObject_FromJythonPyObject(jtmp);

	//mro:
//	jtmp = (*env)->CallObjectMethod(env, src, pyTypeGetMro);
//	tp->tp_mro = JyNI_PyObject_FromJythonPyObject(jtmp);
}
