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
 * JyAbstract.c
 *
 * This file contains rather trivial push-through implementations of
 * several methods callable from Java. Methods in this class would
 * usually have been part of JyNI.c, but we decided to out-source them,
 * (since implementations are trivial) in order to unburden JyNI.c a bit.
 * Most methods here directly correspond to methods in abstract.c, which
 * gave the name to this file.
 *
 *  Created on: 14.03.2013, 00:49:46
 *	  Author: Stefan Richthofer
 */

#include <JyNI.h>

#define PyNumberMethod1(name) \
jobject JyNI_PyNumber_ ## name (jlong o, jlong tstate) \
{ \
	ENTER_JyNI \
	PyObject* res = PyNumber_ ## name ((PyObject*) o); \
	jobject jres = JyNI_JythonPyObject_FromPyObject(res); \
	Py_XDECREF(res); \
	LEAVE_JyNI \
	return jres; \
}

#define PyMethod2(pref, name) \
jobject JyNI_ ## pref ## _ ## name (jlong o1, jobject o2, jlong tstate) \
{ \
	ENTER_JyNI \
	PyObject* arg = JyNI_PyObject_FromJythonPyObject(o2); \
	PyObject* res = pref ## _ ## name ((PyObject*) o1, arg); \
	jobject jres = JyNI_JythonPyObject_FromPyObject(res); \
	Py_XDECREF(arg); \
	Py_XDECREF(res); \
	LEAVE_JyNI \
	return jres; \
}

#define PyNumberMethod2(name) PyMethod2(PyNumber, name)

#define PyNumberMethod3(name) \
jobject JyNI_PyNumber_ ## name (jlong o1, jobject o2, jobject o3, jlong tstate) \
{ \
	ENTER_JyNI \
	PyObject* arg2 = JyNI_PyObject_FromJythonPyObject(o2); \
	PyObject* arg3 = JyNI_PyObject_FromJythonPyObject(o3); \
	PyObject* res = PyNumber_ ## name ((PyObject*) o1, arg2, arg3); \
	jobject jres = JyNI_JythonPyObject_FromPyObject(res); \
	Py_XDECREF(arg2); \
	Py_XDECREF(arg3); \
	Py_XDECREF(res); \
	LEAVE_JyNI \
	return jres; \
}

#define PyLengthMethod(prefix) \
jint JyNI_ ## prefix ## _Length(jlong o, jlong tstate) \
{ \
	RE_ENTER_JyNI \
	jint res = prefix ## _Length((PyObject*) o); \
	RE_LEAVE_JyNI \
	return res; \
}

#define PySequenceSizeArgFunc(name) \
jobject JyNI_PySequence_ ## name (jlong o, jint l, jlong tstate) \
{ \
	ENTER_JyNI \
	PyObject* res = PySequence_ ## name ((PyObject*) o, l); \
	jobject jres = JyNI_JythonPyObject_FromPyObject(res); \
	Py_XDECREF(res); \
	LEAVE_JyNI \
	return jres; \
}


jint JyNI_PyObject_Compare(jlong handle, jobject o, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* obj = JyNI_PyObject_FromJythonPyObject(o);
	//Maybe use _PyObject_Compare?
	//jint res = _PyObject_Compare((PyObject*) handle, obj);
	jint res = Py_TYPE((PyObject*) handle)->tp_compare((PyObject*) handle, obj);
	Py_XDECREF(obj);
	RE_LEAVE_JyNI
	return res;
}

jobject JyNI_PyObject_RichCompare(jlong handle, jobject o, jint op, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* obj = JyNI_PyObject_FromJythonPyObject(o);
	//Maybe use PyObject_RichCompare?
	//PyObject* res = PyObject_RichCompare((PyObject*) handle, obj);
	PyObject* res = Py_TYPE((PyObject*) handle)->tp_richcompare((PyObject*) handle, obj, op);
	jobject jres = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(obj);
	Py_XDECREF(res);
	RE_LEAVE_JyNI
	return jres;
}

jobject JyNI_PyObject_GetIter(jlong handle, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* res = PyObject_GetIter((PyObject*) handle);
	jobject jres = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(res);
	RE_LEAVE_JyNI
	return jres;
}

jobject JyNI_PyIter_Next(jlong handle, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* res = PyIter_Next((PyObject*) handle);
	jobject jres = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(res);
	RE_LEAVE_JyNI
	return jres;
}


// PyNumber-methods:

PyNumberMethod2(Add)
PyNumberMethod2(Subtract)
PyNumberMethod2(Multiply)
PyNumberMethod2(Divide)
PyNumberMethod2(Remainder)
PyNumberMethod2(Divmod)
PyNumberMethod3(Power)
PyNumberMethod1(Negative)
PyNumberMethod1(Positive)
PyNumberMethod1(Absolute)

jboolean JyNI_PyNumber_NonZero(jlong o, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* obj = (PyObject*) o;
	jboolean res = Py_TYPE(obj)->tp_as_number->nb_nonzero(obj);
	RE_LEAVE_JyNI
	return res;
}

PyNumberMethod1(Invert)
PyNumberMethod2(Lshift)
PyNumberMethod2(Rshift)
PyNumberMethod2(And)
PyNumberMethod2(Xor)
PyNumberMethod2(Or)

jobject JyNI_PyNumber_Coerce(jlong o1, jobject o2, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* obj = (PyObject*) o1;
	PyObject* obj2 = JyNI_PyObject_FromJythonPyObject(o2);
	PyObject* obj2_tmp = obj2;
	int ret = Py_TYPE(obj)->tp_as_number->nb_coerce(&obj, &obj2_tmp);
	jobject res = o2;
	if (!ret)
	{
		Py_XDECREF(obj);
		if (obj2 != obj2_tmp) res = JyNI_JythonPyObject_FromPyObject(obj2_tmp);
		Py_XDECREF(obj2_tmp);
	}
	Py_XDECREF(obj2);
	RE_LEAVE_JyNI
	return ret ? JyNone : res;
}

PyNumberMethod1(Int)
PyNumberMethod1(Long)
PyNumberMethod1(Float)
PyNumberMethod1(Oct)
PyNumberMethod1(Hex)
PyNumberMethod2(InplaceAdd)
PyNumberMethod2(InplaceSubtract)
PyNumberMethod2(InplaceMultiply)
PyNumberMethod2(InplaceDivide)
PyNumberMethod2(InplaceRemainder)
PyNumberMethod3(InplacePower)
PyNumberMethod2(InplaceLshift)
PyNumberMethod2(InplaceRshift)
PyNumberMethod2(InplaceAnd)
PyNumberMethod2(InplaceXor)
PyNumberMethod2(InplaceOr)
PyNumberMethod2(FloorDivide)
PyNumberMethod2(TrueDivide)
PyNumberMethod2(InplaceFloorDivide)
PyNumberMethod2(InplaceTrueDivide)
PyNumberMethod1(Index)


// PySequence-methods:
PyLengthMethod(PySequence)
PyMethod2(PySequence, Concat)
PySequenceSizeArgFunc(Repeat)
PySequenceSizeArgFunc(GetItem)

jobject JyNI_PySequence_GetSlice(jlong o, jint l1, jint l2, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* res = PySequence_GetSlice((PyObject*) o, l1, l2);
	jobject jres = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(res);
	RE_LEAVE_JyNI
	return jres;
}

jint JyNI_PySequence_SetItem(jlong o1, jint l, jobject o2, jlong tstate)
{
	ENTER_JyNI
	PyObject* arg = JyNI_PyObject_FromJythonPyObject(o2);
	jint res = PySequence_SetItem((PyObject*) o1, l, arg);
	Py_XDECREF(arg);
	LEAVE_JyNI
	return res;
}

jint JyNI_PySequence_SetSlice(jlong o1, jint l1, jint l2, jobject o2, jlong tstate)
{
	ENTER_JyNI
	PyObject* arg = JyNI_PyObject_FromJythonPyObject(o2);
	jint res = PySequence_SetSlice((PyObject*) o1, l1, l2, arg);
	Py_XDECREF(arg);
	LEAVE_JyNI
	return res;
}

jint JyNI_PySequence_Contains(jlong o1, jobject o2, jlong tstate)
{
	RE_ENTER_JyNI
	PyObject* arg = JyNI_PyObject_FromJythonPyObject(o2);
	jint res = PySequence_Contains((PyObject*) o1, arg);
	Py_XDECREF(arg);
	RE_LEAVE_JyNI
	return res;
}

PyMethod2(PySequence, InplaceConcat)
PySequenceSizeArgFunc(InplaceRepeat)


// PyMapping-methods:
PyLengthMethod(PyMapping)
