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
 * JNI_util.h
 *
 *  Created on: 17.08.2016
 *      Author: Stefan Richthofer
 */

#ifndef INCLUDE_JNI_UTIL_H_
#define INCLUDE_JNI_UTIL_H_

// EXPAND is required by MSVC, see https://stackoverflow.com/a/5134656
// It is used frequently in this file to fix parsing of __VA_ARGS__.
#define EXPAND(x) x

#define obj_sig(cls) "L" pack_ ## cls ";"

#define __JNI_ , ,
#define __JNI_boolean , "Z",
#define __JNI_byte , "B",
#define __JNI_char , "C",
#define __JNI_short , "S",
#define __JNI_int , "I",
#define __JNI_long , "J",
#define __JNI_float , "F",
#define __JNI_double , "D",
#define __JNI_void , "V",

#define __JNI_jboolean , jboolean,
#define __JNI_jbyte , jbyte,
#define __JNI_jchar , jchar,
#define __JNI_jshort , jshort,
#define __JNI_jint , jint,
#define __JNI_jlong , jlong,
#define __JNI_jfloat , jfloat,
#define __JNI_jdouble , jdouble,
#define __JNI_jvoid , void,
#define __JNI_jstring , jstring,
#define __JNI_jarray , jarray,
#define __JNI_jclass , jclass,

#define __JNI_kboolean , jboolean,
#define __JNI_kbyte , jbyte,
#define __JNI_kchar , jchar,
#define __JNI_kshort , jshort,
#define __JNI_kint , jint,
#define __JNI_klong , jlong,
#define __JNI_kfloat , jfloat,
#define __JNI_kdouble , jdouble,

#define array__JNI_(x) , "[" JNI_sig0(x),
#define array__JNI_clear

#define array__JNI_j(x) , JNI_tp1(x)Array

#define CHECK_N(x, y, n, ...) n
#define CHECK(x, ...) EXPAND(CHECK_N(x, __VA_ARGS__, x))

#define JNI_sig0(x) CHECK(obj_sig(x), __JNI_ ## x)

#define JNI_arr(x) x ## _NO_JNI_ARRAY
#define JNI_sig(x) CHECK(JNI_sig0(x), x ## __JNI_(x ## __JNI_clear))
#define JNI_tp1(x) CHECK(jobject, __JNI_k ## x)
#define JNI_tp0(x) CHECK(jobject, __JNI_j ## x)
#define JNI_tp(x) CHECK(JNI_tp0(x), x ## __JNI_j(x))

// Yes there are hacks to achieve true recursion, but here we simply limit
// to a handful of args - trade completeness for maintainability
#define JNI_sigN(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN2(__VA_ARGS__))
#define JNI_sigN2(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN3(__VA_ARGS__))
#define JNI_sigN3(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN4(__VA_ARGS__))
#define JNI_sigN4(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN5(__VA_ARGS__))
#define JNI_sigN5(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN6(__VA_ARGS__))
#define JNI_sigN6(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN7(__VA_ARGS__))
#define JNI_sigN7(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN8(__VA_ARGS__))
#define JNI_sigN8(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN9(__VA_ARGS__))
#define JNI_sigN9(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN10(__VA_ARGS__))
#define JNI_sigN10(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN11(__VA_ARGS__))
#define JNI_sigN11(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN12(__VA_ARGS__))
#define JNI_sigN12(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN13(__VA_ARGS__))
#define JNI_sigN13(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN14(__VA_ARGS__))
#define JNI_sigN14(arg, ...) JNI_sig(arg) EXPAND(JNI_sigN15(__VA_ARGS__))
#define JNI_sigN15(arg, ...) JNI_sig(arg)

#define JNI_sigM(ret, ...) "(" EXPAND(JNI_sigN(__VA_ARGS__)) ")" JNI_sig(ret)

#define JNI_CLS(cPrefix, cls, jPath) \
	{ \
		jclass cPrefix ## cls ## Local = (*env)->FindClass(env, jPath); \
		if (cPrefix ## cls ## Local == NULL) { puts(jPath); return JNI_ERR;} \
		cPrefix ## cls = (jclass) (*env)->NewWeakGlobalRef(env, cPrefix ## cls ## Local); \
		(*env)->DeleteLocalRef(env, cPrefix ## cls ## Local); \
	}

#define JNI_CLASS(cPrefix) JNI_CLS(cPrefix, Class, pack_ ## cPrefix)
#define JNI_INTERFACE(cPrefix, jPath) JNI_CLS(cPrefix, Interface, jPath)
#define JNI_INTERFACE1(cPrefix) JNI_CLS(cPrefix, Interface, pack_ ## cPrefix)

#define JNI_METH_EXT(classID, name) extern jmethodID classID ## _ ## name;

#define JNI_METH_CLS(classID, name, cls, sign) \
	classID ## _ ## name = (*env)->GetMethodID(env, classID ## cls, #name, sign);

#define JNI_METH_CLS2(classID, name, cName, cls, sign) \
	classID ## _ ## cName = (*env)->GetMethodID(env, classID ## cls, #name, sign);

#define JNI_METH_CLS_NOPREFIX(classID, name, cls, ret, ...) \
	name = (*env)->GetMethodID(env, classID ## cls, #name, JNI_sigM(ret, __VA_ARGS__));

#define JNI_METH_CLASS(classID, name, ret, ...) JNI_METH_CLS(classID, name, Class, JNI_sigM(ret, __VA_ARGS__))
#define JNI_METH_INTERFACE(classID, name, ret, ...) JNI_METH_CLS(classID, name, Interface, JNI_sigM(ret, __VA_ARGS__))
#define JNI_METH_CLASS2(classID, name, cName, ret, ...) JNI_METH_CLS2(classID, name, cName, Class, JNI_sigM(ret, __VA_ARGS__))

#define JNI_METH_STATIC(classID, name, ret, ...) \
	classID ## _ ## name = (*env)->GetStaticMethodID(env, classID ## Class, #name, JNI_sigM(ret, __VA_ARGS__));
#define JNI_METH_STATIC2(classID, name, cName, ret, ...) \
	classID ## _ ## cName = (*env)->GetStaticMethodID(env, classID ## Class, #name, JNI_sigM(ret, __VA_ARGS__));
#define JNI_FIELD(classID, name, tp) \
	classID ## _ ## name ## Field = (*env)->GetFieldID(env, classID ## Class, #name, JNI_sig(tp));
#define JNI_FIELD_STATIC(classID, name, tp) \
	classID ## _ ## name ## Field = (*env)->GetStaticFieldID(env, classID ## Class, #name, JNI_sig(tp));

#define JNI_CONSTRUCTOR(classID, cName, ...) \
	classID ## _ ## cName = (*env)->GetMethodID(env, classID ## Class, "<init>", JNI_sigM(void, __VA_ARGS__));

#define JNI_SINGLETON(classID, name, tp) \
	{ \
		jfieldID jy ## name = (*env)->GetStaticFieldID(env, classID ## Class, #name, JNI_sig(tp)); \
		Jy ## name = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, classID ## Class, jy ## name)); \
	}

#define JNI_SINGLETON2(classID, name, cName, tp) \
	{ \
		jfieldID jy ## name = (*env)->GetStaticFieldID(env, classID ## Class, #name, JNI_sig(tp)); \
		Jy ## cName = (*env)->NewWeakGlobalRef(env, (*env)->GetStaticObjectField(env, classID ## Class, jy ## name)); \
	}

#define CallobjectMethod CallObjectMethod
#define jObject jobject

#define JNI_METH(classID, name, ret) \
	jmethodID classID ## _ ## name; \
	inline j ## ret JNI_ ## classID ## _ ## name (JNIEnv* env, jobject obj) { \
		return (*env)->Call ## ret ## Method(env, obj, classID ## _ ## name); \
	}

#define JNI_METH_1(classID, name, ret, arg1, tp1) \
	jmethodID classID ## _ ## name; \
	inline j ## ret JNI_ ## classID ## _ ## name (JNIEnv* env, jobject obj, j ## tp1 arg1) { \
		return (*env)->Call ## ret ## Method(env, obj, classID ## _ ## name, arg1); \
	}

#endif /* INCLUDE_JNI_UTIL_H_ */
