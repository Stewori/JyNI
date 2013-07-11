/*
 * Copyright of JyNI:
 * Copyright (c) 2013 Stefan Richthofer.  All rights reserved.
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013 Python Software Foundation.  All rights reserved.
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

#endif /* JYSYNC_H_ */
