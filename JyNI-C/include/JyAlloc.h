/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
 * 2010, 2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
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

#ifndef INCLUDE_JYALLOC_H_
#define INCLUDE_JYALLOC_H_

#include <JyNI.h>

inline PyObject* JyNI_Alloc(TypeMapEntry* tme);
inline PyObject* JyNI_AllocVar(TypeMapEntry* tme, Py_ssize_t nitems);
inline PyObject* JyNI_AllocSubtypeVar(PyTypeObject* subtype, TypeMapEntry* tme, Py_ssize_t nitems);
inline PyObject* JyNI_AllocNativeVar(PyTypeObject* type, Py_ssize_t nitems);
inline PyObject* JyNI_InitPyObjectSubtype(jobject src, PyTypeObject* subtype);
inline PyObject* JyNI_ExceptionAlloc(ExceptionMapEntry* eme);
inline PyTypeObject* JyNI_AllocPyObjectNativeTypePeer(TypeMapEntry* tme, jobject src);


#endif /* INCLUDE_JYALLOC_H_ */
