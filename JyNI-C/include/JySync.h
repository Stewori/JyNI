/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
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
 * JySync.h
 *
 *  Created on: 23.04.2013, 05:17:18
 *      Author: Stefan Richthofer
 */

#ifndef JYSYNC_H_
#define JYSYNC_H_

PyObject* JySync_Init_PyTuple_From_JyTuple(jobject src);
jobject JySync_Init_JyTuple_From_PyTuple(PyObject* src);

PyObject* JySync_Init_PyString_From_JyString(jobject src);
jobject JySync_Init_JyString_From_PyString(PyObject* src);

PyObject* JySync_Init_PyUnicode_From_JyUnicode(jobject src);
jobject JySync_Init_JyUnicode_From_PyUnicode(PyObject* src);

PyObject* JySync_Init_PyInt_From_JyInt(jobject src);
jobject JySync_Init_JyInt_From_PyInt(PyObject* src);

PyObject* JySync_Init_PyFloat_From_JyFloat(jobject src);
jobject JySync_Init_JyFloat_From_PyFloat(PyObject* src);

PyObject* JySync_Init_PyComplex_From_JyComplex(jobject src);
jobject JySync_Init_JyComplex_From_PyComplex(PyObject* src);

PyObject* JySync_Init_PyLong_From_JyLong(jobject src);
jobject JySync_Init_JyLong_From_PyLong(PyObject* src);

jobject JySync_Init_JyList_From_PyList(PyObject* src);
PyObject* JySync_Init_PyList_From_JyList(jobject src);

PyObject* JySync_Init_PySet_From_JySet(jobject src);
PyObject* JySync_Init_PyFrozenSet_From_JyFrozenSet(jobject src);

jobject JySync_Init_JyClass_From_PyClass(PyObject* src);
PyObject* JySync_Init_PyClass_From_JyClass(jobject src);

jobject JySync_Init_JyInstance_From_PyInstance(PyObject* src);
PyObject* JySync_Init_PyInstance_From_JyInstance(jobject src);
PyObject* JySync_Init_Special_PyInstance(jobject src);

jobject JySync_Init_JyMethod_From_PyMethod(PyObject* src);
PyObject* JySync_Init_PyMethod_From_JyMethod(jobject src);

jobject JySync_Init_JyWeakReference_From_PyWeakReference(PyObject* src);
PyObject* JySync_Init_PyWeakReference_From_JyWeakReference(jobject src);

jobject JySync_Init_JyWeakProxy_From_PyWeakProxy(PyObject* src);
PyObject* JySync_Init_PyWeakProxy_From_JyWeakProxy(jobject src);

jobject JySync_Init_JyWeakCallableProxy_From_PyWeakCallableProxy(PyObject* src);
PyObject* JySync_Init_PyWeakCallableProxy_From_JyWeakCallableProxy(jobject src);

void JySync_PyCode_From_JyCode(jobject src, PyObject* dest);

//PyObject* JySync_Init_PyFunction_From_JyFunction(jobject src);
void JySync_PyFunction_From_JyFunction(jobject src, PyObject* dest);

void JySync_JyCell_From_PyCell(PyObject* src, jobject dest);
void JySync_PyCell_From_JyCell(jobject src, PyObject* dest);

void JySync_PyType_From_JyType(jobject src, PyObject* dest);

//Exception factories:
jobject JyExc_KeyErrorFactory();
jobject JyExc_SystemExitFactory();
jobject JyExc_EnvironmentErrorFactory();
jobject JyExc_SyntaxErrorFactory();

jobject JyExc_UnicodeErrorFactory();

#ifdef Py_USING_UNICODE
jobject JyExc_UnicodeEncodeErrorFactory();
jobject JyExc_UnicodeDecodeErrorFactory();
jobject JyExc_UnicodeTranslateErrorFactory();
#endif

#endif /* JYSYNC_H_ */
