/* This File is based on warning.h from CPython 2.7.3 release.
 * It has been modified to suit JyNI needs.
 *
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
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


#ifndef Py_WARNINGS_H
#define Py_WARNINGS_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_FUNC(void) _PyWarnings_Init(void);

PyAPI_FUNC(int) PyErr_WarnEx(PyObject *, const char *, Py_ssize_t);
PyAPI_FUNC(int) PyErr_WarnExplicit(PyObject *, const char *, const char *, int,
                                    const char *, PyObject *);

#define PyErr_WarnPy3k(msg, stacklevel) \
  (Py_Py3kWarningFlag ? PyErr_WarnEx(PyExc_DeprecationWarning, msg, stacklevel) : 0)

/* DEPRECATED: Use PyErr_WarnEx() instead. */
#define PyErr_Warn(category, msg) PyErr_WarnEx(category, msg, 1)

#ifdef __cplusplus
}
#endif
#endif /* !Py_WARNINGS_H */

