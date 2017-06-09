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
 * JySync.c
 *
 *  Created on: 23.04.2013, 04:13:04
 *      Author: Stefan Richthofer
 */

#include <JyNI.h>
#include <code_JyNI.h>
#include <frameobject_JyNI.h>

/*
typedef void (*jy2pySync)(jobject, PyObject*);
typedef void (*py2jySync)(PyObject*, jobject);
typedef jobject (*jyInitSync)(PyObject*, jclass);
typedef PyObject* (*pyInitSync)(jobject, PyTypeObject*);
typedef jlong (*pyChecksum)(PyObject*);
typedef jlong (*jyChecksum)(jobject);

typedef struct {jy2pySync jy2py; py2jySync py2jy; jyInitSync jyInit; pyInitSync pyInit; pyChecksum pyCheck; jyChecksum jyCheck;} SyncFunctions;
*/

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 * For every item in the tuple also a new reference is created. However the tuple
 * deallocator will take care of these.
 */
PyObject* JySync_Init_PyTuple_From_JyTuple(jobject src, PyTypeObject* nonNativeSubtype)
{
	//jputs("JySync_Init_PyTuple_From_JyTuple");
	env(NULL);
	//jarray back = (*env)->CallObjectMethod(env, src, pyTupleGetArray);
	jint srcSize = (*env)->CallIntMethod(env, src, collection_size);
	//jputs("### creating tuple...");
	PyObject* er = PyTuple_New(srcSize);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_setNativeHandle, src, (jlong) er);//, JNI_FALSE);
	AS_JY_WITH_GC(er)->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	//jputs("### created tuple...");
	//Py_XINCREF(er);
	//if (srcSize != PyTuple_GET_SIZE(dest)) //...throw exception since tuple is immutable
	int i;
	for (i = 0; i < srcSize; ++i)
	{
		//jputsLong(i);
		PyObject* item = JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pySequence_pyget, i));
		//Py_XINCREF(item);
		//PyTuple_SetItem(er, i, item);
		PyTuple_SET_ITEM(er, i, item);
//		if (!item) {
//			jputs("Add null-item to tuple:");
//			jputsLong(er);
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

jobject JySync_Init_JyTuple_From_PyTuple(PyObject* src, jclass subtype)
{
	if (!subtype && !PyTuple_GET_SIZE(src))
	{
		return JyEmptyTuple;
	} else {
		env(NULL);
		jarray back;
		if (PyTuple_GET_SIZE(src))
		{
			back = (*env)->NewObjectArray(env, PyTuple_GET_SIZE(src), pyObjectClass, NULL);
			//if (srcSize != PyTuple_GET_SIZE(dest)) //...throw exception since tuple is immutable
			int i;
			for (i = 0; i < PyTuple_GET_SIZE(src); ++i)
			{
				(*env)->SetObjectArrayElement(env, back, i, JyNI_JythonPyObject_FromPyObject(
						PyTuple_GET_ITEM(src, i)));
				//Py_XINCREF(PyTuple_GET_ITEM(src, i));
			}
		} else
			back = JyEmptyPyObjectArray;
		if (!subtype)
			return (*env)->NewObject(env, pyTupleClass, pyTuple_byPyObjectArrayBooleanConstructor,
					back, JNI_FALSE);
		else
		{
			jobject jsrcType = JyNI_JythonPyTypeObject_FromPyTypeObject(Py_TYPE(src));
			return (*env)->NewObject(env, subtype, pyTupleCPeer_Constructor,
					(jlong) src, jsrcType, back);
		}
	}
}

jboolean isPreAllocatedJythonString(jobject obj, char value)//jchar value)
{
	if (value >= LETTERCHAR_MAXJYTHON)
		return JNI_FALSE;
	env(JNI_FALSE);
	jarray scache = (*env)->GetStaticObjectField(env, pyPyClass, pyPy_lettersField);
	jobject cachedString = (*env)->GetObjectArrayElement(env, scache, (jint) value);
	return (*env)->IsSameObject(env, cachedString, obj);
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyString_From_JyString(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	jstring jstr = (*env)->CallObjectMethod(env, src, pyObject_asString);
	//cstr_from_jstring(cstr, jstr);

	//cstr_from_jstring(cstrName, jstr)
	char* utf_string = (*env)->GetStringUTFChars(env, jstr, NULL);
	size_t len = strlen(utf_string);
	char cstr[len+1];
	strcpy(cstr, utf_string);
	(*env)->ReleaseStringUTFChars(env, jstr, utf_string);
	if (len == 1 && !nonNativeSubtype) {
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
		if (nonNativeSubtype) ((PyStringObject *) result)->ob_sstate = SSTATE_NOT_INTERNED;
		return result;
	}
}

jobject JySync_Init_JyString_From_PyString(PyObject* src, jclass subtype)
{
	//todo: check interned-regulations on jython-side
	env(NULL);
	jstring jstr = (*env)->NewStringUTF(env, PyString_AS_STRING(src));
	if (JyNI_HasJyAttribute(AS_JY_NO_GC(src), JyAttributeStringInterned))
		jstr = (*env)->CallObjectMethod(env, jstr, string_intern);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPy_newString, jstr);
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	jstring jstr = (*env)->CallObjectMethod(env, src, pyObject_asString);
	jstring charsetName = (*env)->NewStringUTF(env, "UTF-16BE");
	jobject stringByteArray = (*env)->CallObjectMethod(env, jstr,
		string_getBytes, charsetName);
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
//PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src, PyTypeObject* nonNativeSubtype)
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

//PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src, PyTypeObject* nonNativeSubtype)
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

jobject JySync_Init_JyUnicode_From_PyUnicode(PyObject* src, jclass subtype)
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
		string_fromBytesAndCharsetNameConstructor, strByteArray, charsetName);
//	if (JyNI_HasJyAttribute(AS_JY_NO_GC(src), JyAttributeStringInterned))
//		jstr = (*env)->CallObjectMethod(env, jstr, stringIntern);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPy_newUnicode, jstr);
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
	jarray icache = (*env)->GetStaticObjectField(env, pyPyClass, pyPy_integerCacheField);
	jobject cachedInt = (*env)->GetObjectArrayElement(env, icache, value+NSMALLNEGINTSJYTHON);
	return (*env)->IsSameObject(env, cachedInt, obj);
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyInt_From_JyInt(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	//return PyInt_FromLong((long) (*env)->CallLongMethod(env, src, pyIntAsLong));
	jint value = (*env)->CallIntMethod(env, src, pyInt_getValue);
	PyObject* result = PyInt_FromLong((long) value);
	if (!nonNativeSubtype && isPreAllocatedJythonInt(src, value))
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
jobject JySync_Init_JyInt_From_PyInt(PyObject* src, jclass subtype)
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPy_newIntFromLong, (jlong) PyInt_AS_LONG(src));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyFloat_From_JyFloat(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	return PyFloat_FromDouble((double) (*env)->CallDoubleMethod(env, src, pyFloat_asDouble));
}

jobject JySync_Init_JyFloat_From_PyFloat(PyObject* src, jclass subtype)
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPy_newFloatFromDouble, (jdouble) PyFloat_AS_DOUBLE(src));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyComplex_From_JyComplex(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	return PyComplex_FromDoubles(
			(double) (*env)->GetDoubleField(env, src, pyComplexRealField),
			(double) (*env)->GetDoubleField(env, src, pyComplexImagField));
}

jobject JySync_Init_JyComplex_From_PyComplex(PyObject* src, jclass subtype)
{
	env(NULL);
	return (*env)->NewObject(env, pyComplexClass, pyComplex_by2DoubleConstructor,
			PyComplex_RealAsDouble(src),
			PyComplex_ImagAsDouble(src));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyLong_From_JyLong(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	jobject bival = (*env)->CallObjectMethod(env, src, pyLong_getValue);
	jarray jbytes = (*env)->CallObjectMethod(env, bival, bigInt_toByteArray);
	jsize n = (*env)->GetArrayLength(env, jbytes);
	jbyte* bbytes = (*env)->GetByteArrayElements(env, jbytes, NULL);
	//memcpy(bytes+n-cpl, bbytes+lb-cpl, cpl);
	PyObject *er = _PyLong_FromByteArray(bbytes, (size_t) n, JNI_FALSE, JNI_TRUE);
	(*env)->ReleaseByteArrayElements(env, jbytes, bbytes, JNI_ABORT);
	return er;
}

jobject JySync_Init_JyLong_From_PyLong(PyObject* src, jclass subtype)
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
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPy_newLongFromBigInt,
			(*env)->NewObject(env, bigIntClass, bigInt_fromByteArrayConstructor, jbytes));
}

jobject JySync_Init_JyList_From_PyList(PyObject* src, jclass subtype)
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
		jyList = (*env)->NewObject(env, JyListClass, JyList_fromBackendHandleConstructor, (jlong) src);
		JyObject_AddOrSetJyGCHead(src, AS_JY_WITH_GC(src), (*env)->NewWeakGlobalRef(env, jyList));
		JyNI_GC_ExploreObject(src);
	}
//	jobject jylist = (*env)->NewObject(env, JyListClass, JyListFromBackendHandleConstructor, (jlong) src);
//	if ((*env)->ExceptionCheck(env)) {
//		jputs("Exception happened converting list:");
//		(*env)->ExceptionDescribe(env);
//	}
	return (*env)->CallStaticObjectMethod(env, pyListClass, pyList_fromList, jyList);
}


/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyList_From_JyList(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	//maybe optimize a bit more here:
	//if most objects in the list have not yet been converted,
	//much lookup is done twice here.
	jarray handles = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_lookupNativeHandles, src);
	jsize size = (*env)->GetArrayLength(env, handles);
	PyObject* op = PyList_New((Py_ssize_t) size);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_setNativeHandle, src, (jlong) op);//, JNI_FALSE);
	AS_JY_WITH_GC(op)->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	jsize i;
	PyObject* v;
	jlong* arr = (*env)->GetLongArrayElements(env, handles, NULL);
	for (i = 0; i < size; ++i)
	{
		v = (PyObject*) arr[i];
		if (!v) v = JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pySequence_pyget, i));
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
		jyList = (*env)->NewObject(env, JyListClass, JyList_fromBackendHandleConstructor, (jlong) src);
		JyObject_AddOrSetJyGCHead(src, AS_JY_WITH_GC(src), (*env)->NewWeakGlobalRef(env, jyList));
		JyNI_GC_ExploreObject(src);
	}
	//(*env)->CallVoidMethod(env, jyList, JyListInstallToPyList, src);
	(*env)->SetObjectField(env, src, pyList_listField, jyList);
//	jputsLong(__LINE__);
	(*env)->CallObjectMethod(env, jyList, pyObjectGCHead_setPyObject, src);
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
PyObject* JySync_Init_PySet_From_JySet(jobject src, PyTypeObject* nonNativeSubtype) //needed because truncation is only partial
{
	PySetObject* so = (PySetObject *) PySet_Type.tp_alloc(&PySet_Type, 0);
	JyObject* jy = AS_JY(so);
//	jy->jy = src;
//	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	env(NULL);
	so->used = (*env)->CallIntMethod(env, src, collection_size);
	jobject _set = (*env)->GetObjectField(env, src, pyBaseSet__setField);
	jobject jySet = (*env)->NewObject(env, JySetClass, JySet_fromBackendHandleConstructor, _set, (jlong) so);
//	(*env)->CallVoidMethod(env, jySet, JySetInstallToPySet, src);
	(*env)->SetObjectField(env, src, pyBaseSet__setField, jySet);
	return (PyObject*) so;
}

//jobject JySync_Init_JyFrozenSet_From_PyFrozenSet(PyObject* src) not needed because of truncation

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyFrozenSet_From_JyFrozenSet(jobject src, PyTypeObject* nonNativeSubtype) //needed because truncation is only partial
{
	PySetObject* so = (PySetObject *) PyFrozenSet_Type.tp_alloc(&PyFrozenSet_Type, 0);
	JyObject* jy = AS_JY(so);
//	jy->jy = src;
//	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	env(NULL);
	so->used = (*env)->CallIntMethod(env, src, collection_size);
	jobject _set = (*env)->GetObjectField(env, src, pyBaseSet__setField);
	jobject jySet = (*env)->NewObject(env, JySetClass, JySet_fromBackendHandleConstructor, _set, (jlong) so);
//	(*env)->CallVoidMethod(env, jySet, JySetInstallToPySet, src);
	(*env)->SetObjectField(env, src, pyBaseSet__setField, jySet);
	return (PyObject*) so;
}


jobject JySync_Init_JyClass_From_PyClass(PyObject* src, jclass subtype)
{
	PyClassObject* cls = (PyClassObject*) src;
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyClassClass, pyClass_classobj___new__,
		JyNI_JythonPyObject_FromPyObject(cls->cl_name),
		JyNI_JythonPyObject_FromPyObject(cls->cl_bases),
		JyNI_JythonPyObject_FromPyObject(cls->cl_dict));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyClass_From_JyClass(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	jstring nm = (*env)->GetObjectField(env, src, pyClass___name__Field);
	cstr_from_jstring(cnm, nm);
//	jputs("Sync-Class:");
//	jputs(cnm);
	return PyClass_New(
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyClass___bases__Field)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyClass___dict__Field)),
		PyString_FromString(cnm));
}


jobject JySync_Init_JyInstance_From_PyInstance(PyObject* src, jclass subtype)
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
	return (*env)->NewObject(env, pyInstanceClass, pyInstance_Constructor,
		JyNI_JythonPyObject_FromPyObject((PyObject*) inst->in_class),
		JyNI_JythonPyObject_FromPyObject(inst->in_dict));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyInstance_From_JyInstance(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_suspendPyInstanceFinalizer, src);
	PyObject* er = PyInstance_NewRaw(
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyInstance_instclassField)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyInstance___dict__Field)));
	return er;
}

///*
// * This function returns a NEW reference, i.e. caller must decref it in the end.
// */
//PyObject* JySync_Init_Special_PyInstance(jobject src, PyTypeObject* nonNativeSubtype)
//{
//	env(NULL);
//	//todo: Care for finalizer also in this case.
//	//(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_suspendPyInstanceFinalizer, src);
//	jobject old_cls = (*env)->CallStaticObjectMethod(env, JyNIClass,
//						JyNI_getTypeOldStyleParent, src);
//
//	PyObject* er = PyInstance_NewRaw(
//		JyNI_PyObject_FromJythonPyObject(old_cls),
//		JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pyObjectGetDict)));
//	return er;
//}




jobject JySync_Init_JyMethod_From_PyMethod(PyObject* src, jclass subtype)
{
	PyMethodObject* meth = (PyMethodObject*) src;
	env(NULL);
	return (*env)->NewObject(env, pyMethodClass, pyMethod_Constructor,
		JyNI_JythonPyObject_FromPyObject(meth->im_func),
		JyNI_JythonPyObject_FromPyObject(meth->im_self),
		JyNI_JythonPyObject_FromPyObject(meth->im_class));
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyMethod_From_JyMethod(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	return PyMethod_New(
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyMethod___func__Field)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyMethod___self__Field)),
		JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env, src, pyMethod_im_classField)));
}

jobject JySync_Init_JyClassMethod_From_PyClassMethod(PyObject* src, jclass subtype)
{
	env(NULL);
	return (*env)->NewObject(env, pyClassMethodClass, pyClassMethod_Constructor,
		JyNI_JythonPyObject_FromPyObject(((classmethod*) src)->cm_callable));
}

PyObject* JySync_Init_PyClassMethod_From_JyClassMethod(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	return PyClassMethod_New(JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env,
		src, pyClassMethod_callableField)));
}

jobject JySync_Init_JyStaticMethod_From_PyStaticMethod(PyObject* src, jclass subtype)
{
	env(NULL);
	return (*env)->NewObject(env, pyStaticMethodClass, pyStaticMethod_Constructor,
		JyNI_JythonPyObject_FromPyObject(((staticmethod*) src)->sm_callable));
}

PyObject* JySync_Init_PyStaticMethod_From_JyStaticMethod(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	return PyStaticMethod_New(JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env,
		src, pyStaticMethod_callableField)));
}

jobject JySync_Init_JyDictProxy_From_PyDictProxy(PyObject* src, jclass subtype)
{
	env(NULL);
	return (*env)->NewObject(env, pyDictProxyClass, pyDictProxy_Constructor,
		JyNI_JythonPyObject_FromPyObject(((proxyobject*) src)->dict));
}

PyObject* JySync_Init_PyDictProxy_From_JyDictProxy(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	return PyDictProxy_New(JyNI_PyObject_FromJythonPyObject((*env)->GetObjectField(env,
		src, pyDictProxy_dictField)));
}

jobject JySync_Init_JyProperty_From_PyProperty(PyObject* src, jclass subtype)
{
	env(NULL);
	jobject result = (*env)->NewObject(env, pyPropertyClass, pyProperty_Constructor);
	(*env)->SetObjectField(env, result, pyProperty_fgetField,
		JyNI_JythonPyObject_FromPyObject(((propertyobject*) src)->prop_get));
	(*env)->SetObjectField(env, result, pyProperty_fsetField,
			JyNI_JythonPyObject_FromPyObject(((propertyobject*) src)->prop_set));
	(*env)->SetObjectField(env, result, pyProperty_fdelField,
			JyNI_JythonPyObject_FromPyObject(((propertyobject*) src)->prop_del));
	(*env)->SetObjectField(env, result, pyProperty_docField,
			JyNI_JythonPyObject_FromPyObject(((propertyobject*) src)->prop_doc));
	(*env)->SetBooleanField(env, result, pyProperty_docFromGetterField,
			JyNI_JythonPyObject_FromPyObject(((propertyobject*) src)->getter_doc));
	return result;
}

PyObject* JySync_Init_PyProperty_From_JyProperty(jobject src, PyTypeObject* nonNativeSubtype)
{
	env(NULL);
	propertyobject* result = PyObject_GC_New(propertyobject, &PyProperty_Type);
	if (result != NULL) {
		result->prop_get = JyNI_PyObject_FromJythonPyObject(
				(*env)->GetObjectField(env, src, pyProperty_fgetField));
		result->prop_set = JyNI_PyObject_FromJythonPyObject(
				(*env)->GetObjectField(env, src, pyProperty_fsetField));
		result->prop_del = JyNI_PyObject_FromJythonPyObject(
				(*env)->GetObjectField(env, src, pyProperty_fdelField));
		result->prop_doc = JyNI_PyObject_FromJythonPyObject(
				(*env)->GetObjectField(env, src, pyProperty_docField));
		result->getter_doc = (*env)->GetBooleanField(env, src, pyProperty_docFromGetterField);
		_JyNI_GC_TRACK(result);
	}
	return (PyObject *) result;
}

#define PyWeakref_GET_OBJECT0(ref)                          \
    (Py_REFCNT(((PyWeakReference *)(ref))->wr_object) > 0   \
     ? ((PyWeakReference *)(ref))->wr_object                \
     : NULL)

jobject JySync_Init_JyWeakReference_From_PyWeakReference(PyObject* src, jclass subtype)
{
	PyObject* referent = PyWeakref_GET_OBJECT0(src);
//	jputs("Sync PyWeakRef* -> jython");
//	jputsLong((jlong) referent);
	jobject jReferent = NULL;
	if (referent) jReferent = JyNI_JythonPyObject_FromPyObject(referent);
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_createWeakReferenceFromNative,
			jReferent, (jlong) referent, NULL); //todo: Support native callbacks.
}

/*
 * This function returns a NEW reference, i.e. caller must decref it in the end.
 */
PyObject* JySync_Init_PyWeakReference_From_JyWeakReference(jobject src, PyTypeObject* nonNativeSubtype)
{
	// Todo: Handle case that the native counterpart of the Java-referent is
	//       not weakly referenceable in CPython-terms.
	//       See PyType_SUPPORTS_WEAKREFS(Py_TYPE(ob))
	env(NULL);
	jobject jReferent = (*env)->CallObjectMethod(env, src, AbstractReference_get);
	PyObject* referent = JyNI_PyObject_FromJythonPyObject(jReferent);
//	jputs("Sync jython -> PyWeakRef*");
//	jputsLong((jlong) referent);
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

jobject JySync_Init_JyWeakProxy_From_PyWeakProxy(PyObject* src, jclass subtype)
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
PyObject* JySync_Init_PyWeakProxy_From_JyWeakProxy(jobject src, PyTypeObject* nonNativeSubtype)
{
	// Todo: Handle case that the native counterpart of the Java-referent is
	//       not weakly referenceable in CPython-terms.
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

jobject JySync_Init_JyWeakCallableProxy_From_PyWeakCallableProxy(PyObject* src, jclass subtype)
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
PyObject* JySync_Init_PyWeakCallableProxy_From_JyWeakCallableProxy(jobject src, PyTypeObject* nonNativeSubtype)
{
	// Todo: Handle case that the native counterpart of the Java-referent is
	//       not weakly referenceable in CPython-terms.
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


//jobject JySync_Init_JyMethodDescr_From_PyMethodDescr(PyObject* src, jclass subtype)
//{
//	PyMethodDescrObject* inst = (PyMethodDescrObject*) src;
//	env(NULL);
//	return NULL;
//}
//
///*
// * This function returns a NEW reference, i.e. caller must decref it in the end.
// */
//PyObject* JySync_Init_PyMethodDescr_From_JyMethodDescr(jobject src, PyTypeObject* nonNativeSubtype)
//{
//	env(NULL);
//	return NULL;
//}


jobject JySync_Init_JyCFunction_From_PyCFunction(PyObject* src, jclass subtype)
{
//	jputs(__FUNCTION__);
	if (JyNI_HasJyAttribute(AS_JY(src), JyAttributeMethodDef))
	{
		jputs("JyNI-warning: JySync_Init_JyCFunction_From_PyCFunction shouldn't be called with non-native PyCFunction.");
	}
	JyObject* srcJy = AS_JY_WITH_GC(src);
	// Note that JyNI.PyCFunction implements CPeerInterface, so we must set the CPeer-flag:
	srcJy->flags |= JY_CPEER_FLAG_MASK;
	Py_INCREF(src);

	//JyNI_GC_ExploreObject(src);
//	jboolean dbg = strcmp(((PyCFunctionObject*) src)->m_ml->ml_name, "in_dll") == 0;
//	jputs(Py_TYPE(src)->tp_name);
	//if (_PyThreadState_Current) jputsPy(PyObject_GetAttrString(src, "__class__"));
	PyCFunctionObject* func = (PyCFunctionObject*) src;
//	jputs(func->m_ml->ml_name);
//	jputsLong(func->m_self);
	//putsPy(func->m_self);
//	jputsLong(func->m_module);
	env(NULL);
	jstring name = (*env)->NewStringUTF(env, func->m_ml->ml_name);
	jstring doc = (*env)->NewStringUTF(env, func->m_ml->ml_doc);
	jobject jtype = JyNI_JythonPyObject_FromPyObject(Py_TYPE(src));
	//JyNI_jprintJ(jtype); //builtinTypes[TME_INDEX_CFunction].jy_class
	//jobject test = _JyNI_JythonPyTypeObject_FromPyTypeObject(Py_TYPE(src), NULL);

	jobject result = (*env)->NewObject(env, pyCFunctionClass, pyCFunction_Constructor,
			(jlong) src, jtype,
			name, func->m_ml->ml_flags & METH_NOARGS, doc);
//	if (dbg) {
//		jputs(__FUNCTION__);
//		jputs(((PyCFunctionObject*) src)->m_ml->ml_name);
//		jputsPy(((PyCFunctionObject*) src)->m_self);
//		jputsLong(((PyCFunctionObject*) src));
//		jputsLong(((PyCFunctionObject*) src)->m_ml);
//		jputsLong(((PyCFunctionObject*) src)->m_self);
//		jPrintCStackTrace();
//	}
//	jputs("result:");
//	JyNI_jprintJ(result);
//	if ((*env)->IsSameObject(env, result, NULL)) jputs("result is null");
//	JyNI_printJInfo(result);
//	JyNI_jprintHash(result);
	return result;
}


//static PyObject* jyBuiltinCall(PyObject* self, PyObject* args)
//{
//	jobject jself = JyNI_JythonPyObject_FromPyObject(self);
//	return JyNI_PyObject_Call(jself, args, NULL);
//}

static PyObject* jyBuiltinCallWithKeywords(PyObject* self, PyObject* args, PyObject* kw)
{
	//jputs(__FUNCTION__);
	jputs("JyNI-warning: Native caller ignored Jython-flag.");
//	jobject jself = self ? JyNI_JythonPyObject_FromPyObject(self) : JyNone;
	jobject jself = JyNI_JythonPyObject_FromPyObject(self);
	return JyNI_PyObject_Call(jself, args, kw);
}

//static PyObject* jyBuiltinCallNoArgs(PyObject* self)
//{
//	jobject jself = JyNI_JythonPyObject_FromPyObject(self);
//	return JyNI_PyObject_Call(jself, PyTuple_New(0), NULL);
//}

PyObject* JySync_Init_PyCFunction_From_JyBuiltinCallable(jobject src, PyTypeObject* nonNativeSubtype)
{
//	jputs(__FUNCTION__);
	env(NULL);
	if ((*env)->IsInstanceOf(env, src, pyCFunctionClass))
	{
		jputs("JyNI-warning: JySync_Init_PyCFunction_From_JyBuiltinCallable shouldn't be called with PyCFunction.");
	}
	PyMethodDef* mdef = malloc(sizeof(PyMethodDef));
	jobject info = (*env)->GetObjectField(env, src, pyBuiltinCallable_infoField);
	jint max = (*env)->CallIntMethod(env, info, pyBuiltinCallableInfo_getMaxargs);
	mdef->ml_flags = (max ? (METH_KEYWORDS | METH_VARARGS) : METH_NOARGS) | METH_JYTHON;

	jstring jtmp = (*env)->CallObjectMethod(env, info, pyBuiltinCallableInfo_getName);
	global_cstr_from_jstring(cName, jtmp);
	mdef->ml_name = cName;
//	puts(cName);

	jtmp = (*env)->CallObjectMethod(env, src, pyBuiltinCallable_getDoc);
	if (jtmp)
	{
		global_cstr_from_jstring2(cDoc, jtmp);
		mdef->ml_doc = cDoc;
	} else mdef->ml_doc = NULL;

	mdef->ml_meth = (PyCFunctionWithKeywords) jyBuiltinCallWithKeywords;

	jobject jmodule = (*env)->CallObjectMethod(env, src, pyBuiltinCallable_getModule);
	jobject jself = (*env)->CallObjectMethod(env, src, pyBuiltinCallable_getSelf);
	// Account for incompatible behavior:
	// In Jython an unbound method has self = None while in CPython self = NULL
	PyObject* mself = (*env)->IsSameObject(env, jself, JyNone) ? NULL :
			JyNI_PyObject_FromJythonPyObject(jself);
//	putsPy(mself);
	PyCFunctionObject* res = PyCFunction_NewEx(mdef, mself,
			JyNI_PyObject_FromJythonPyObject(jmodule));
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY(res), JyAttributeMethodName,
			cName, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	if (mdef->ml_doc)
		JyNI_AddOrSetJyAttributeWithFlags(AS_JY(res), JyAttributeMethodDoc,
				mdef->ml_doc, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY(res), JyAttributeMethodDef,
			mdef, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	return (PyObject*) res;
}

jobject JySync_Init_JyMethodDescr_From_PyMethodDescr(PyObject* src, jclass subtype)
{
//	jboolean jdbg = strcmp(((PyMethodDescrObject*) src)->d_method->ml_name, "__array__") == 0 &&
//			strcmp(((PyMethodDescrObject*) src)->d_type->tp_name, "numpy.ndarray") == 0;
//	if (jdbg) jputs(__FUNCTION__);
//	if (jdbg) jputs(((PyMethodDescrObject*) src)->d_method->ml_name);
//	if (jdbg) jputs(((PyMethodDescrObject*) src)->d_type->tp_name);
//	if (jdbg) jputsLong(((PyMethodDescrObject*) src)->d_type->tp_as_number->nb_add);
	env(NULL);
	jobject mdef = (*env)->NewObject(env, pyCMethodDefClass, pyCMethodDef_Constructor,
			(jlong) ((PyMethodDescrObject*) src)->d_method,
			(*env)->NewStringUTF(env, ((PyMethodDescrObject*) src)->d_method->ml_name),
			((PyMethodDescrObject*) src)->d_method->ml_flags & METH_NOARGS,
			(*env)->NewStringUTF(env, ((PyMethodDescrObject*) src)->d_method->ml_doc));
	jobject res = (*env)->NewObject(env, pyMethodDescrClass, pyMethodDescr_Constructor,
			JyNI_JythonPyObject_FromPyObject((PyObject*) ((PyMethodDescrObject*) src)->d_type),
			mdef);
	return res;
}

PyObject* JySync_Init_PyMethodDescr_From_JyMethodDescr(jobject src, PyTypeObject* nonNativeSubtype)
{
//	jputs(__FUNCTION__);
	env(NULL);
	jint max = (*env)->CallIntMethod(env, src, pyBuiltinCallableInfo_getMaxargs);
	//mdef->ml_flags = (max ? (METH_KEYWORDS | METH_VARARGS) : METH_NOARGS) | METH_JYTHON;

	jstring jtmp = (*env)->CallObjectMethod(env, src, pyBuiltinCallableInfo_getName);
	global_cstr_from_jstring(cName, jtmp);
	//mdef->ml_name = cName;

	jobject dtype = (*env)->GetObjectField(env, src, pyDescr_dtypeField);

	PyMethodDef* mdef = malloc(sizeof(PyMethodDef));
	mdef->ml_flags = (max ? (METH_KEYWORDS | METH_VARARGS) : METH_NOARGS)
			| METH_JYTHON | METH_JYTHON_CDEF;
	mdef->ml_name = cName;

	jtmp = (*env)->CallObjectMethod(env, src, pyMethodDescr_getDoc);
	if (jtmp)
	{
		global_cstr_from_jstring2(cDoc, jtmp);
		mdef->ml_doc = cDoc;
	} else mdef->ml_doc = NULL;
	mdef->ml_meth = (PyCFunctionWithKeywords) jyBuiltinCallWithKeywords;

	PyObject* result = PyDescr_NewMethod(JyNI_PyObject_FromJythonPyObject(dtype), mdef);
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY_WITH_GC(result), JyAttributeMethodName,
			cName, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	if (mdef->ml_doc)
		JyNI_AddOrSetJyAttributeWithFlags(AS_JY_WITH_GC(result), JyAttributeMethodDoc,
				mdef->ml_doc, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY_WITH_GC(result), JyAttributeMethodDef,
			mdef, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	return result;
}


//jobject JySync_Init_JyCode_From_PyCode(PyObject* src) not needed because of truncation
//PyObject* JySync_Init_PyCode_From_JyCode(jobject src)
void JySync_PyCode_From_JyCode(jobject src, PyObject* dest)
{
	env();
	//puts("JySync_PyCode_From_JyCode");
	//jobject jCode = JyNI_JythonPyObject_FromPyObject(src);
	jobject jArray = (*env)->GetObjectField(env, src, pyBaseCode_co_freevarsField);
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
	jobject jCode = (*env)->GetObjectField(env, src, pyFunction___code__Field);
	PyObject* pt = JyNI_PyObject_FromJythonPyObject(jCode);
	((PyFunctionObject*) dest)->func_code = pt;
	jobject jGlobals = (*env)->CallObjectMethod(env, src, pyFunction_getFuncGlobals);
	((PyFunctionObject*) dest)->func_globals = JyNI_PyObject_FromJythonPyObject(jGlobals);
	jobject jModule = (*env)->GetObjectField(env, src, pyFunction___module__Field);
	((PyFunctionObject*) dest)->func_module = JyNI_PyObject_FromJythonPyObject(jModule);
	jobject jDefaults = (*env)->CallObjectMethod(env, src, pyFunction_getFuncDefaults);
	((PyFunctionObject*) dest)->func_defaults = JyNI_PyObject_FromJythonPyObject(jDefaults);
	jobject jClosure = (*env)->GetObjectField(env, src, pyFunction___closure__Field);
	((PyFunctionObject*) dest)->func_closure = JyNI_PyObject_FromJythonPyObject(jClosure);
	jstring jName = (*env)->GetObjectField(env, src, pyFunction___name__Field);
	cstr_from_jstring(cName, jName);
	((PyFunctionObject*) dest)->func_name = PyString_FromString(cName);
}

void JySync_JyCell_From_PyCell(PyObject* src, jobject dest)
{
	env();
	(*env)->SetObjectField(env, dest, pyCell_ob_refField,
		JyNI_JythonPyObject_FromPyObject(PyCell_GET(src)));

}

void JySync_PyCell_From_JyCell(jobject src, PyObject* dest)
{
	env();
	PyCell_SET(dest, JyNI_PyObject_FromJythonPyObject(
		(*env)->GetObjectField(env, src, pyCell_ob_refField)));
}

void JySync_JyFrame_From_PyFrame(PyObject* src, jobject dest)
{
	// we only care for lineno so far
	env();
	(*env)->SetIntField(env, dest, pyFrame_f_linenoField, ((PyFrameObject*) src)->f_lineno);
}

void JySync_PyFrame_From_JyFrame(jobject src, PyObject* dest)
{
	// we only care for lineno so far
	env();
	((PyFrameObject*) dest)->f_lineno = (*env)->GetIntField(env, src, pyFrame_f_linenoField);
}

jobject JyExc_KeyErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_KeyError);
}

jobject JyExc_SystemExitFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_SystemExit);
}

jobject JyExc_EnvironmentErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_EnvironmentError);
}

jobject JyExc_SyntaxErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_SyntaxError);
}

jobject JyExc_UnicodeErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_UnicodeError);
}

#ifdef Py_USING_UNICODE
jobject JyExc_UnicodeEncodeErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_UnicodeEncodeError);
}

jobject JyExc_UnicodeDecodeErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_UnicodeDecodeError);
}

jobject JyExc_UnicodeTranslateErrorFactory()
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, exceptionsClass, exceptions_UnicodeTranslateError);
}
#endif

PyObject* JySync_Init_PyTuple_From_JyTupleForMRO(jobject src)
{
	env(NULL);
	jint srcSize = (*env)->CallIntMethod(env, src, collection_size);
	PyObject* er = PyTuple_New(srcSize);
	(*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_setNativeHandle, src, (jlong) er);
	AS_JY_WITH_GC(er)->flags |= JY_HAS_JHANDLE_FLAG_MASK;
	int i;
	for (i = 1; i < srcSize; ++i)
	{
		PyObject* item = JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pySequence_pyget, i));
		PyTuple_SET_ITEM(er, i, item);
	}
	return er;
}

void JySync_PyType_From_JyType(jobject src, PyObject* dest)
{
//	jputs(__FUNCTION__);
//	JyNI_jprintJ(src);
	PyTypeObject* tp = (PyTypeObject*) dest;
	env();
	//name:
	jobject jtmp = (*env)->CallObjectMethod(env, src, pyType_getName);
	//jobject jtmp = (*env)->GetObjectField(env, src, pyTypeNameField);
	//if (!jtmp) jputs("JySync_PyType_From_JyType: type with NULL-name!");
	//cstr_from_jstring(cname, jname);
	char* utf_string = (*env)->GetStringUTFChars(env, jtmp, NULL);
	char* cname = malloc(strlen(utf_string)+1);
	strcpy(cname, utf_string);
	(*env)->ReleaseStringUTFChars(env, jtmp, utf_string);
//	jboolean dbg = strcmp(cname, "Class") == 0;
//	if (dbg) {
//		jputs(__FUNCTION__);
//		jputs(cname);
//		jputsLong(dest);
//		jPrintCStackTrace();
//	}
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY(dest), JyAttributeTypeName, cname, JY_ATTR_OWNS_VALUE_FLAG_MASK);
	tp->tp_name = cname;

	//SyncFunctions* sync = (SyncFunctions*) JyNI_GetJyAttribute(AS_JY_WITH_GC(dest), JyAttributeSyncFunctions);

	//dict:
	jtmp = (*env)->CallObjectMethod(env, src, pyObject_fastGetDict);
	tp->tp_dict = JyNI_PyObject_FromJythonPyObject(jtmp);

	//base:
	jtmp = (*env)->CallObjectMethod(env, src, pyType_getBase);
	tp->tp_base = (PyTypeObject*) JyNI_PyObject_FromJythonPyObject(jtmp);

	// Note that basicsize must be set before bases, because some bases access basicsize
	// during convertion to native objects.
	//basicsize:
	tp->tp_basicsize = tp->tp_base == Py_None ? sizeof(PyObject) : tp->tp_base->tp_basicsize;

	//bases:
	jtmp = (*env)->CallObjectMethod(env, src, pyType_getBases);
	tp->tp_bases = JyNI_PyObject_FromJythonPyObject(jtmp);

	//We try to get away with just setting this to default for now:
	tp->tp_flags |= Py_TPFLAGS_DEFAULT;
//	jputsLong(tp->tp_flags);// & Py_TPFLAGS_HAVE_CLASS);
	//jputsLong(tp->tp_mro);
	//if (!tp->tp_alloc)
	if (!(tp->tp_flags & Py_TPFLAGS_READY))
	{
		if (tp->tp_base == Py_None)
			tp->tp_base = NULL;
		PyType_Ready(tp);
		//tp->tp_flags = (tp->tp_flags & ~Py_TPFLAGS_READYING) | Py_TPFLAGS_READY;
//		JyNI_GC_ExploreObject(tp);
	} //else puts("already ready");
//	jputs("sync mro...");
//	jputsLong(tp);
	//mro:
	if (!tp->tp_mro)
	{
		jtmp = (*env)->CallObjectMethod(env, src, pyType_getMro);
		PyObject* mro = JySync_Init_PyTuple_From_JyTupleForMRO(jtmp);
		PyTuple_SET_ITEM(mro, 0, tp);
		tp->tp_mro = mro;

		// Currently tp_traverse is out-commented.
		// Once we tested and stabilized heap-type exploration
		// the following section must be included here, because
		// we changed mro after initial exploration in PyType_Ready.
		/* tp_traverse visits type members in this order:
		 *	Py_VISIT(type->tp_dict);
		 *	Py_VISIT(type->tp_cache);
		 *	Py_VISIT(type->tp_mro);
		 *	Py_VISIT(type->tp_bases);
		 *	Py_VISIT(type->tp_base);
		 * so mro-index is 2.
		 */
		if (!IS_UNEXPLORED(tp))
			updateJyGCHeadLink(tp, AS_JY_WITH_GC(tp), 2 /* mro-index */,
					tp->tp_mro, AS_JY_WITH_GC(tp->tp_mro));
	}
//	jputs("type-sync done");
}
