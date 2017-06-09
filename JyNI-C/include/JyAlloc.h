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

#ifndef INCLUDE_JYALLOC_H_
#define INCLUDE_JYALLOC_H_

#include <JyNI.h>

#define INIT_JY_BASIC(_jy, tme, _flags) \
	_jy->jy = (jweak) tme; \
	_jy->flags = _flags;

#define ORDINARY_ALLOC_FULL(size, allocFlags, tme, dest, type) \
	jy = (JyObject *) PyObject_RawMalloc(size+sizeof(JyObject)); \
	if (jy == NULL) return (PyObject *) PyErr_NoMemory(); \
	INIT_JY_BASIC(jy, tme, allocFlags) \
	jy->attr = NULL; \
	dest = (PyObject*) FROM_JY_NO_GC(jy); \
	JyNIDebug(JY_NATIVE_ALLOC, dest, jy, size+sizeof(JyObject), ((PyTypeObject*) type)->tp_name);

#define ALLOC_FULL(size, allocFlags, tme, type) \
	PyObject *obj; \
	if (PyType_IS_GC(type)) \
	{ \
		obj = _PyObject_GC_Malloc(size); \
		if (obj == NULL) return PyErr_NoMemory(); \
		AS_JY_WITH_GC(obj)->flags |= allocFlags; \
	} else \
	{ \
		JyObject* jy; \
		ORDINARY_ALLOC_FULL(size, allocFlags, tme, obj, type) \
	} \
	memset(obj, '\0', size);

#define INIT(obj, type, nitems) \
	if (type->tp_itemsize == 0) PyObject_INIT(obj, type); \
	else PyObject_INIT_VAR((PyVarObject *)obj, type, nitems); \
	type->tp_flags |= Jy_TPFLAGS_DYN_OBJECTS;

#define INIT_FULL(obj, type, nitems) \
	if (type->tp_flags & Py_TPFLAGS_HEAPTYPE) \
		Py_INCREF(type); \
	INIT(obj, type, nitems) \
	if (PyType_Check(obj)) \
		((PyTypeObject*) obj)->tp_flags |= Py_TPFLAGS_HEAPTYPE; \
	if (PyType_IS_GC(type)) \
		_JyNI_GC_TRACK(obj);


inline PyObject* JyNI_Alloc(TypeMapEntry* tme);
inline PyObject* JyNI_AllocVar(TypeMapEntry* tme, Py_ssize_t nitems);
inline PyObject* JyNI_AllocSubtypeVar(PyTypeObject* subtype, TypeMapEntry* tme, Py_ssize_t nitems);
inline PyObject* JyNI_AllocNativeVar(PyTypeObject* type, Py_ssize_t nitems);
inline PyObject* JyNI_ExceptionAlloc(ExceptionMapEntry* eme);
inline PyTypeObject* JyNI_AllocPyObjectNativeTypePeer(TypeMapEntry* tme, jobject src);

inline PyObject* JyNI_InitPyObject(TypeMapEntry* tme, jobject src);
inline PyObject* JyNI_InitPyObjectSubtype(jobject src, PyTypeObject* subtype);
inline PyObject* JyNI_InitPyException(ExceptionMapEntry* eme, jobject src);
inline PyTypeObject* JyNI_InitPyObjectNativeTypePeer(jobject srctype);

inline jobject JyNI_InitStaticJythonPyObject(PyObject* src);
inline jobject JyNI_InitJythonPyObject(TypeMapEntry* tme, PyObject* src, JyObject* srcJy);


#endif /* INCLUDE_JYALLOC_H_ */
