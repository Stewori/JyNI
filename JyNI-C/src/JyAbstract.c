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

//Number protocol:

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

#define PyNumberMethod2(name) \
jobject JyNI_PyNumber_ ## name (jlong o1, jobject o2, jlong tstate) \
{ \
	ENTER_JyNI \
	PyObject* arg = JyNI_PyObject_FromJythonPyObject(o2); \
	PyObject* res = PyNumber_ ## name ((PyObject*) o1, arg); \
	jobject jres = JyNI_JythonPyObject_FromPyObject(res); \
	Py_XDECREF(arg); \
	Py_XDECREF(res); \
	LEAVE_JyNI \
	return jres; \
}

PyNumberMethod2(Add)
PyNumberMethod2(Subtract)
PyNumberMethod2(Multiply)
PyNumberMethod2(Divide)
PyNumberMethod2(FloorDivide)
PyNumberMethod2(TrueDivide)
PyNumberMethod2(Remainder)
PyNumberMethod2(Divmod)

jobject JyNI_PyNumber_Power(jlong o1, jobject o2, jobject o3, jlong tstate)
{
	ENTER_JyNI
	PyObject* arg2 = JyNI_PyObject_FromJythonPyObject(o2);
	PyObject* arg3 = JyNI_PyObject_FromJythonPyObject(o3);
	PyObject* res = PyNumber_Power((PyObject*) o1, arg2, arg3);
	jobject jres = JyNI_JythonPyObject_FromPyObject(res);
	Py_XDECREF(arg2);
	Py_XDECREF(arg3);
	Py_XDECREF(res);
	LEAVE_JyNI
	return jres;
}

PyNumberMethod1(Negative)
PyNumberMethod1(Positive)
PyNumberMethod1(Absolute)
PyNumberMethod1(Invert)

PyNumberMethod2(Lshift)
PyNumberMethod2(Rshift)
PyNumberMethod2(And)
PyNumberMethod2(Xor)
PyNumberMethod2(Or)
