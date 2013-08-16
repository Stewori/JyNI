/* This File is based on classobject.h from CPython 2.7.5 release.
 * It has been modified to suite JyNI needs.
 *
 * Copyright of the original file:
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


/* Class object interface */

/* Revealing some structures (not for general use) */

#ifndef Py_CLASSOBJECT_H
#define Py_CLASSOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	PyObject_HEAD
	PyObject	*cl_bases;	/* A tuple of class objects */
	PyObject	*cl_dict;	/* A dictionary */
	PyObject	*cl_name;	/* A string */
	/* The following three are functions or NULL */
	PyObject	*cl_getattr;
	PyObject	*cl_setattr;
	PyObject	*cl_delattr;
	PyObject	*cl_weakreflist; /* List of weak references */
} PyClassObject;

typedef struct {
	PyObject_HEAD
	PyClassObject *in_class;	/* The class object */
	PyObject	  *in_dict;	/* A dictionary */
	PyObject	  *in_weakreflist; /* List of weak references */
} PyInstanceObject;

typedef struct {
	PyObject_HEAD
	PyObject *im_func;   /* The callable object implementing the method */
	PyObject *im_self;   /* The instance it is bound to, or NULL */
	PyObject *im_class;  /* The class that asked for the method */
	PyObject *im_weakreflist; /* List of weak references */
} PyMethodObject;

PyAPI_DATA(PyTypeObject) PyClass_Type, PyInstance_Type, PyMethod_Type;

#define PyClass_Check(op) ((op)->ob_type == &PyClass_Type)
#define PyInstance_Check(op) ((op)->ob_type == &PyInstance_Type)
#define PyMethod_Check(op) ((op)->ob_type == &PyMethod_Type)

PyAPI_FUNC(PyObject *) PyClass_New(PyObject *, PyObject *, PyObject *);
PyAPI_FUNC(PyObject *) PyInstance_New(PyObject *, PyObject *,
											PyObject *);
PyAPI_FUNC(PyObject *) PyInstance_NewRaw(PyObject *, PyObject *);
PyAPI_FUNC(PyObject *) PyMethod_New(PyObject *, PyObject *, PyObject *);

PyAPI_FUNC(PyObject *) PyMethod_Function(PyObject *);
PyAPI_FUNC(PyObject *) PyMethod_Self(PyObject *);
PyAPI_FUNC(PyObject *) PyMethod_Class(PyObject *);

/* Look up attribute with name (a string) on instance object pinst, using
 * only the instance and base class dicts.  If a descriptor is found in
 * a class dict, the descriptor is returned without calling it.
 * Returns NULL if nothing found, else a borrowed reference to the
 * value associated with name in the dict in which name was found.
 * The point of this routine is that it never calls arbitrary Python
 * code, so is always "safe":  all it does is dict lookups.  The function
 * can't fail, never sets an exception, and NULL is not an error (it just
 * means "not found").
 */
PyAPI_FUNC(PyObject *) _PyInstance_Lookup(PyObject *pinst, PyObject *name);

/* Macros for direct access to these values. Type checks are *not*
   done, so use with care. */
#define PyMethod_GET_FUNCTION(meth) \
	(((PyMethodObject *)meth) -> im_func)
#define PyMethod_GET_SELF(meth) \
	(((PyMethodObject *)meth) -> im_self)
#define PyMethod_GET_CLASS(meth) \
	(((PyMethodObject *)meth) -> im_class)

PyAPI_FUNC(int) PyClass_IsSubclass(PyObject *, PyObject *);

PyAPI_FUNC(int) PyMethod_ClearFreeList(void);

#ifdef __cplusplus
}
#endif
#endif /* !Py_CLASSOBJECT_H */
