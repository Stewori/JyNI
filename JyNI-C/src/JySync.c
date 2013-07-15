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
 * JySync.c
 *
 *  Created on: 23.04.2013, 04:13:04
 *      Author: Stefan Richthofer
 */

#include <JyNI.h>

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


//Tuple-Area:
// Macro, trading safety for speed
//#define PyTuple_GET_ITEM(op, i) (((PyTupleObject *)(op))->ob_item[i])
//#define PyTuple_GET_SIZE(op)    Py_SIZE(op)

// Macro, *only* to be used to fill in brand new tuples
//#define PyTuple_SET_ITEM(op, i, v) (((PyTupleObject *)(op))->ob_item[i] = v)

PyObject* JySync_Init_PyTuple_From_JyTuple(jobject src)
{
	env(NULL);
	//jarray back = (*env)->CallObjectMethod(env, src, pyTupleGetArray);
	jint srcSize = (*env)->CallIntMethod(env, src, pyTupleSize);
	PyObject* er = PyTuple_New(srcSize);
	//Py_XINCREF(er);
	//if (srcSize != PyTuple_GET_SIZE(dest)) //...throw exception since tuple is immutable
	int i;
	for (i = 0; i < srcSize; ++i)
	{
		PyObject* item = JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, src, pyTuplePyGet, i));
		//Py_XINCREF(item);
		//PyTuple_SetItem(er, i, item);
		PyTuple_SET_ITEM(er, i, item);
	}
	return er;
}

jobject JySync_Init_JyTuple_From_PyTuple(PyObject* src)
{
	env(NULL);
	jarray back = (*env)->NewObjectArray(env, PyTuple_GET_SIZE(src), pyObjectClass, NULL);
	//if (srcSize != PyTuple_GET_SIZE(dest)) //...throw exception since tuple is immutable
	int i;
	for (i = 0; i < PyTuple_GET_SIZE(src); ++i)
	{
		(*env)->SetObjectArrayElement(env, back, i, JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(src, i)));
		//Py_XINCREF(PyTuple_GET_ITEM(src, i));
	}
	//return (*env)->NewGlobalRef(env, (*env)->NewObject(env, pyTupleClass, pyTupleByPyObjectArrayBooleanConstructor, back, JNI_FALSE));
	return (*env)->NewObject(env, pyTupleClass, pyTupleByPyObjectArrayBooleanConstructor, back, JNI_FALSE);
}


//String-Area:
/*
// Use only if you know it's a string
#define PyString_CHECK_INTERNED(op) (((PyStringObject *)(op))->ob_sstate)

// Macro, trading safety for speed
#define PyString_AS_STRING(op) (((PyStringObject *)(op))->ob_sval)
#define PyString_GET_SIZE(op)  Py_SIZE(op)
*/
PyObject* JySync_Init_PyString_From_JyString(jobject src)
{
	env(NULL);
	jstring jstr = (*env)->CallObjectMethod(env, src, pyStringAsString);
	cstr_from_jstring(cstr, jstr);
	return PyString_FromString(cstr);
}

jobject JySync_Init_JyString_From_PyString(PyObject* src)
{
	//todo: check interned-regulations on jython-side
	env(NULL);
//	return	(*env)->NewGlobalRef(env,
//					(*env)->NewObject(env, pyStringClass, pyStringByJStringConstructor,
//						(*env)->NewStringUTF(env, PyString_AS_STRING(src))
//					)
//				);
	jstring jstr = (*env)->NewStringUTF(env, PyString_AS_STRING(src));
	if (JyNI_HasJyAttribute(AS_JY_NO_GC(src), JyAttributeStringInterned))
		jstr = (*env)->CallObjectMethod(env, jstr, stringIntern);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewString, jstr);
}

PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src)
{
	env(NULL);
	jstring jstr = (*env)->CallObjectMethod(env, src, pyUnicodeAsString);
	jchar* uc = (*env)->GetStringChars(env, jstr, NULL);
	PyObject* unicode = PyUnicode_EncodeUTF16(
		    (char*) uc, //const char *string,         /* UTF-16 encoded string */
		    (*env)->GetStringLength(env, jstr), //Py_ssize_t length,          /* size of string */
		    NULL, //const char *errors,         /* error handling */
		    1 //int *byteorder             pointer to byteorder to use
//		                                   0=native;-1=LE,1=BE; updated on
//		                                   exit
		    );
	(*env)->ReleaseStringChars(env, jstr, uc);
	return unicode;
	//cstr_from_jstring(cstr, jstr);
	//return PyString_FromString(cstr);
}

jobject JySync_Init_JyUnicode_From_PyUnicode(PyObject* src)
{
	env(NULL);
	//jstring jstr = (*env)->NewStringUTF(env, PyString_AS_STRING(src));
	jstring jstr = (*env)->NewString(env, PyUnicode_AsUTF16String(src), PyUnicode_GET_SIZE(src));
//	if (JyNI_HasJyAttribute(AS_JY_NO_GC(src), JyAttributeStringInterned))
//		jstr = (*env)->CallObjectMethod(env, jstr, stringIntern);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewUnicode, jstr);
}

PyObject* JySync_Init_PyInt_From_JyInt(jobject src)
{
	env(NULL);
	return PyInt_FromLong((long) (*env)->CallLongMethod(env, src, pyIntAsLong));
}

/*
 * This method can in fact return a jython PyLong. This can be the case, if
 * on 64-bit systems, native PyInteger has 64 bit backend while jython PyInteger
 * is always guaranteed to be 32 bit.
 * This might cause confusion, if the type is looked up again for some reason.
 * When writing this, we consider this as less confusing than changing a number
 * value during conversion.
 */
jobject JySync_Init_JyInt_From_PyInt(PyObject* src)
{
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewIntFromLong, (jlong) PyInt_AS_LONG(src));
}

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
	env(NULL);
	return (*env)->CallStaticObjectMethod(env, pyListClass, pyListFromList,
			(*env)->NewObject(env, JyListClass, JyListFromBackendHandleConstructor, (jlong) src));
}

PyObject* JySync_Init_PyList_From_JyList(jobject src)
{
	env(NULL);
	//maybe optimize a bit more here:
	//if most objects in the list have not yet been converted,
	//much lookup is done twice here.
	jarray handles = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNILookupNativeHandles, src);
	jsize size = (*env)->GetArrayLength(env, handles);
	PyObject* op = PyList_New((Py_ssize_t) size);
	jlong* arr = (*env)->GetLongArrayElements(env, handles, NULL);
	jsize i;
	PyObject* v;
	for (i = 0; i < size; ++i)
	{
		v = (PyObject*) arr[i];
		if (v == NULL) v = _JyNI_PyObject_FromJythonPyObject(
				(*env)->CallObjectMethod(env, src, pyListPyGet, i), JNI_FALSE, JNI_FALSE, JNI_TRUE);
//JyNI_PyObject_FromJythonPyObject(jobject jythonPyObject, jboolean lookupNative, jboolean checkCPeer, jboolean checkForType)
		PyList_SET_ITEM(op, i, v);
	}
	(*env)->ReleaseLongArrayElements(env, handles, arr, JNI_ABORT);
	jobject jyList = (*env)->NewObject(env, JyListClass, JyListFromBackendHandleConstructor, (jlong) op);
	(*env)->CallVoidMethod(env, jyList, JyListInstallToPyList, src);
	return op;
}
