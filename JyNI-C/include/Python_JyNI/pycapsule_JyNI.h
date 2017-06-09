/* This File is based on pycapsule.h from CPython 2.7.4 release.
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


/* Capsule objects let you wrap a C "void *" pointer in a Python
   object.  They're a way of passing data through the Python interpreter
   without creating your own custom type.

   Capsules are used for communication between extension modules.
   They provide a way for an extension module to export a C interface
   to other extension modules, so that extension modules can use the
   Python import mechanism to link to one another.

   For more information, please see "c-api/capsule.html" in the
   documentation.
*/

#ifndef Py_CAPSULE_H
#define Py_CAPSULE_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_DATA(PyTypeObject) PyCapsule_Type;

typedef void (*PyCapsule_Destructor)(PyObject *);

#define PyCapsule_CheckExact(op) (Py_TYPE(op) == &PyCapsule_Type)


PyAPI_FUNC(PyObject *) PyCapsule_New(
    void *pointer,
    const char *name,
    PyCapsule_Destructor destructor);

PyAPI_FUNC(void *) PyCapsule_GetPointer(PyObject *capsule, const char *name);

PyAPI_FUNC(PyCapsule_Destructor) PyCapsule_GetDestructor(PyObject *capsule);

PyAPI_FUNC(const char *) PyCapsule_GetName(PyObject *capsule);

PyAPI_FUNC(void *) PyCapsule_GetContext(PyObject *capsule);

PyAPI_FUNC(int) PyCapsule_IsValid(PyObject *capsule, const char *name);

PyAPI_FUNC(int) PyCapsule_SetPointer(PyObject *capsule, void *pointer);

PyAPI_FUNC(int) PyCapsule_SetDestructor(PyObject *capsule, PyCapsule_Destructor destructor);

PyAPI_FUNC(int) PyCapsule_SetName(PyObject *capsule, const char *name);

PyAPI_FUNC(int) PyCapsule_SetContext(PyObject *capsule, void *context);

PyAPI_FUNC(void *) PyCapsule_Import(const char *name, int no_block);

#ifdef __cplusplus
}
#endif
#endif /* !Py_CAPSULE_H */
