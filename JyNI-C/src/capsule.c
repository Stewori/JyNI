/* This File is based on capsule.c from CPython 2.7.4 release.
 * It has been modified to suit JyNI needs.
 *
 * Copyright of the original file:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014 Stefan Richthofer.  All rights reserved.
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


/* Wrap void * pointers to be passed between C modules */

#include "JyNI.h"

/* Internal structure of PyCapsule */
typedef struct {
	PyObject_HEAD
	void *pointer;
	const char *name;
	void *context;
	PyCapsule_Destructor destructor;
} PyCapsule;



static int
_is_legal_capsule(PyCapsule *capsule, const char *invalid_capsule)
{
	if (!capsule || !PyCapsule_CheckExact(capsule) || capsule->pointer == NULL) {
		PyErr_SetString(PyExc_ValueError, invalid_capsule);
		return 0;
	}
	return 1;
}

#define is_legal_capsule(capsule, name) \
	(_is_legal_capsule(capsule, \
	 name " called with invalid PyCapsule object"))


static int
name_matches(const char *name1, const char *name2) {
	/* if either is NULL, */
	if (!name1 || !name2) {
		/* they're only the same if they're both NULL. */
		return name1 == name2;
	}
	return !strcmp(name1, name2);
}



PyObject *
PyCapsule_New(void *pointer, const char *name, PyCapsule_Destructor destructor)
{
	PyCapsule *capsule;

	if (!pointer) {
		PyErr_SetString(PyExc_ValueError, "PyCapsule_New called with null pointer");
		return NULL;
	}

	capsule = PyObject_NEW(PyCapsule, &PyCapsule_Type);
	if (capsule == NULL) {
		return NULL;
	}

	capsule->pointer = pointer;
	capsule->name = name;
	capsule->context = NULL;
	capsule->destructor = destructor;

	return (PyObject *)capsule;
}


int
PyCapsule_IsValid(PyObject *o, const char *name)
{
	PyCapsule *capsule = (PyCapsule *)o;

	return (capsule != NULL &&
			PyCapsule_CheckExact(capsule) &&
			capsule->pointer != NULL &&
			name_matches(capsule->name, name));
}


void *
PyCapsule_GetPointer(PyObject *o, const char *name)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!is_legal_capsule(capsule, "PyCapsule_GetPointer")) {
		return NULL;
	}

	if (!name_matches(name, capsule->name)) {
		PyErr_SetString(PyExc_ValueError, "PyCapsule_GetPointer called with incorrect name");
		return NULL;
	}

	return capsule->pointer;
}


const char *
PyCapsule_GetName(PyObject *o)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!is_legal_capsule(capsule, "PyCapsule_GetName")) {
		return NULL;
	}
	return capsule->name;
}


PyCapsule_Destructor
PyCapsule_GetDestructor(PyObject *o)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!is_legal_capsule(capsule, "PyCapsule_GetDestructor")) {
		return NULL;
	}
	return capsule->destructor;
}


void *
PyCapsule_GetContext(PyObject *o)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!is_legal_capsule(capsule, "PyCapsule_GetContext")) {
		return NULL;
	}
	return capsule->context;
}


int
PyCapsule_SetPointer(PyObject *o, void *pointer)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!pointer) {
		PyErr_SetString(PyExc_ValueError, "PyCapsule_SetPointer called with null pointer");
		return -1;
	}

	if (!is_legal_capsule(capsule, "PyCapsule_SetPointer")) {
		return -1;
	}

	capsule->pointer = pointer;
	return 0;
}


int
PyCapsule_SetName(PyObject *o, const char *name)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!is_legal_capsule(capsule, "PyCapsule_SetName")) {
		return -1;
	}

	capsule->name = name;
	return 0;
}


int
PyCapsule_SetDestructor(PyObject *o, PyCapsule_Destructor destructor)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!is_legal_capsule(capsule, "PyCapsule_SetDestructor")) {
		return -1;
	}

	capsule->destructor = destructor;
	return 0;
}


int
PyCapsule_SetContext(PyObject *o, void *context)
{
	PyCapsule *capsule = (PyCapsule *)o;

	if (!is_legal_capsule(capsule, "PyCapsule_SetContext")) {
		return -1;
	}

	capsule->context = context;
	return 0;
}


void *
PyCapsule_Import(const char *name, int no_block)
{
	puts("PyCapsule_Import:");
	puts(name);
	if (no_block) puts("no block");
	else puts("block");
	PyObject *object = NULL;
	void *return_value = NULL;
	char *trace;
	size_t name_length = (strlen(name) + 1) * sizeof(char);
	char *name_dup = (char *)PyMem_MALLOC(name_length);

	if (!name_dup) {
		return NULL;
	}

	memcpy(name_dup, name, name_length);
	puts("aa");
	trace = name_dup;
	while (trace) {
		puts("Trace:");
		puts(trace);
		char *dot = strchr(trace, '.');
		if (dot) {
			*dot++ = '\0';
		}

		if (object == NULL) {
			if (no_block) {
				puts("aa no_block");
				object = PyImport_ImportModuleNoBlock(trace);
				puts("aa...done");
			} else {
				puts("aa block");
				object = PyImport_ImportModule(trace);
				puts("aa...done");
				if (!object) {
					puts("PyCapsule_Import could not import module");
					PyErr_Format(PyExc_ImportError, "PyCapsule_Import could not import module \"%s\"", trace);
				}
			}
		} else {
			puts("bb1");
			PyObject *object2 = PyObject_GetAttrString(object, trace);
			puts("bb2");
			if (object) puts(object->ob_type->tp_name);
			else puts("object NULL");
			Py_DECREF(object);
			//do {
//				if (--object->ob_refcnt != 0);
//				else
//					object->ob_type->tp_dealloc(object);
			//} while (0)

			puts("bb3");
			object = object2;
			puts("bb4");
			if (object) puts(object->ob_type->tp_name);
			else puts("object2 NULL");
		}
		if (!object) {
			goto EXIT;
		}

		trace = dot;
	}
	puts("cc");
	/* compare attribute name to module.name by hand */
	if (PyCapsule_IsValid(object, name)) {
		puts("valid!");
		PyCapsule *capsule = (PyCapsule *)object;
		return_value = capsule->pointer;
	} else {
		puts("PyCapsule_Import is not valid:");
		puts(name);
		PyErr_Format(PyExc_AttributeError,
			"PyCapsule_Import \"%s\" is not valid",
			name);
	}

EXIT:
	puts("exitt");
	Py_XDECREF(object);
	if (name_dup) {
		PyMem_FREE(name_dup);
	}
	return return_value;
}


static void
capsule_dealloc(PyObject *o)
{
	PyCapsule *capsule = (PyCapsule *)o;
	if (capsule->destructor) {
		capsule->destructor(o);
	}
	PyObject_DEL(o);
}


static PyObject *
capsule_repr(PyObject *o)
{
	PyCapsule *capsule = (PyCapsule *)o;
	const char *name;
	const char *quote;

	if (capsule->name) {
		quote = "\"";
		name = capsule->name;
	} else {
		quote = "";
		name = "NULL";
	}

	return PyString_FromFormat("<capsule object %s%s%s at %p>",
		quote, name, quote, capsule);
}



PyDoc_STRVAR(PyCapsule_Type__doc__,
"Capsule objects let you wrap a C \"void *\" pointer in a Python\n\
object.  They're a way of passing data through the Python interpreter\n\
without creating your own custom type.\n\
\n\
Capsules are used for communication between extension modules.\n\
They provide a way for an extension module to export a C interface\n\
to other extension modules, so that extension modules can use the\n\
Python import mechanism to link to one another.\n\
");

PyTypeObject PyCapsule_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"PyCapsule",		/*tp_name*/
	sizeof(PyCapsule),		/*tp_basicsize*/
	0,				/*tp_itemsize*/
	/* methods */
	capsule_dealloc, /*tp_dealloc*/
	0,				/*tp_print*/
	0,				/*tp_getattr*/
	0,				/*tp_setattr*/
	0,				/*tp_reserved*/
	capsule_repr, /*tp_repr*/
	0,				/*tp_as_number*/
	0,				/*tp_as_sequence*/
	0,				/*tp_as_mapping*/
	0,				/*tp_hash*/
	0,				/*tp_call*/
	0,				/*tp_str*/
	0,				/*tp_getattro*/
	0,				/*tp_setattro*/
	0,				/*tp_as_buffer*/
	0,				/*tp_flags*/
	PyCapsule_Type__doc__	/*tp_doc*/
};


