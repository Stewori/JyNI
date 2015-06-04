/* This File is based on moduleobject.c from CPython 2.7.3 release.
 * It has been modified to suit JyNI needs.
 *
 * Copyright of the original file:
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


/* Module object implementation */

#include "JyNI.h"
#include "structmember_JyNI.h"

typedef struct {
    PyObject_HEAD
    PyObject *md_dict;
} PyModuleObject;

//static PyMemberDef module_members[] = {
//    {"__dict__", T_OBJECT, offsetof(PyModuleObject, md_dict), READONLY},
//    {0}
//};
static PyGetSetDef module_getsets[] = {
	{"__dict__", (getter)PyModule_GetDict},
	{NULL},
};

PyObject *
PyModule_New(const char *name)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
			(*env)->NewObject(env, pyModuleClass, pyModuleByStringConstructor,
			(*env)->CallObjectMethod(env, (*env)->NewStringUTF(env, name), stringIntern)));

    /*PyModuleObject *m;
    PyObject *nameobj;
    m = PyObject_GC_New(PyModuleObject, &PyModule_Type);
    if (m == NULL)
        return NULL;
    nameobj = PyString_FromString(name);
    m->md_dict = PyDict_New();
    if (m->md_dict == NULL || nameobj == NULL)
        goto fail;
    if (PyDict_SetItemString(m->md_dict, "__name__", nameobj) != 0)
        goto fail;
    if (PyDict_SetItemString(m->md_dict, "__doc__", Py_None) != 0)
        goto fail;
    if (PyDict_SetItemString(m->md_dict, "__package__", Py_None) != 0)
        goto fail;
    Py_DECREF(nameobj);
    PyObject_GC_Track(m);
    return (PyObject *)m;

 fail:
    Py_XDECREF(nameobj);
    Py_DECREF(m);
    return NULL;*/
}

PyObject *
PyModule_GetDict(PyObject *m)
{
	//puts("PyModule_GetDict");
	if (!PyModule_Check(m)) {
		PyErr_BadInternalCall();
		return NULL;
	}
	env(NULL);
	//jobject jm = (*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(m), pyModuleGetDict);
	PyObject* er = JyNI_PyObject_FromJythonPyObject(
			(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(m), pyModuleGetDict));
	return er;
    /*PyObject *d;
    if (!PyModule_Check(m)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    d = ((PyModuleObject *)m) -> md_dict;
    if (d == NULL)
        ((PyModuleObject *)m) -> md_dict = d = PyDict_New();
    return d;*/
}

/*
 * JyNI Warning: The returned pointer is only valid until the value
 * in the dict is changed! (Should usually not happen.)
 */
char *
PyModule_GetName(PyObject *m)
{
	if (!PyModule_Check(m)) {
		PyErr_BadArgument();
		return NULL;
	}
	//stringIntern
	env(NULL);
	jobject pyStr = (*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(m), pyObject__getattr__,
			(*env)->CallObjectMethod(env, (*env)->NewStringUTF(env, "__name__"), stringIntern));
	if (pyStr == NULL)
	{
		PyErr_SetString(PyExc_SystemError, "nameless module");
		return NULL;
	}
	jstring er = (*env)->CallObjectMethod(env, pyStr, pyStringAsString);
	global_cstr_from_jstring(cstr, er);
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY_WITH_GC(m), JyAttributeModuleName, cstr, JY_ATTR_OWNS_VALUE_FLAG_MASK);

	return cstr;

	/*
    PyObject *d;
    PyObject *nameobj;
    if (!PyModule_Check(m)) {
        PyErr_BadArgument();
        return NULL;
    }
    d = ((PyModuleObject *)m)->md_dict;
    if (d == NULL ||
        (nameobj = PyDict_GetItemString(d, "__name__")) == NULL ||
        !PyString_Check(nameobj))
    {
        PyErr_SetString(PyExc_SystemError, "nameless module");
        return NULL;
    }
    return PyString_AsString(nameobj);*/
}

/*
 * JyNI Warning: The returned pointer is only valid until the value
 * in the dict is changed! (Should usually not happen.)
 */
char *
PyModule_GetFilename(PyObject *m)
{
	if (!PyModule_Check(m)) {
		PyErr_BadArgument();
		return NULL;
	}
	//stringIntern
	env(NULL);
	//puts("bis hier...");
	jobject pyStr = (*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(m), pyObject__getattr__,
			(*env)->CallObjectMethod(env, (*env)->NewStringUTF(env, "__file__"), stringIntern));
	//puts("und noch etwas weiter");
	if (pyStr == NULL)
	{
		puts("module filename missing");
		//printf("PyExc_SystemError isException? %i\n", PyExceptionClass_Check2(PyExc_SystemError));
		//printf("PyExc_SystemError isType? %i\n", PyType_Check(PyExc_SystemError));
		//puts("hier nicht");
		//todo: fix this later
		//PyErr_SetString(PyExc_SystemError, "module filename missing");
		//puts("und hier0?");
		return NULL;
	}
	//puts("und hier?");
	jstring er = (*env)->CallObjectMethod(env, pyStr, pyStringAsString);
	global_cstr_from_jstring(cstr, er);
	//puts("fast fertig...");
	JyNI_AddOrSetJyAttributeWithFlags(AS_JY_WITH_GC(m), JyAttributeModuleFile, cstr, JY_ATTR_OWNS_VALUE_FLAG_MASK);

	return cstr;

	/*
    PyObject *d;
    PyObject *fileobj;
    if (!PyModule_Check(m)) {
        PyErr_BadArgument();
        return NULL;
    }
    d = ((PyModuleObject *)m)->md_dict;
    if (d == NULL ||
        (fileobj = PyDict_GetItemString(d, "__file__")) == NULL ||
        !PyString_Check(fileobj))
    {
        PyErr_SetString(PyExc_SystemError, "module filename missing");
        return NULL;
    }
    return PyString_AsString(fileobj);
    */
}

//void
//_PyModule_Clear(PyObject *m)
//{
////       To make the execution order of destructors for global
////       objects a bit more predictable, we first zap all objects
////       whose name starts with a single underscore, before we clear
////       the entire dictionary.  We zap them by replacing them with
////       None, rather than deleting them from the dictionary, to
////       avoid rehashing the dictionary (to some extent).
//
//    Py_ssize_t pos;
//    PyObject *key, *value;
//    PyObject *d;
//
//    d = ((PyModuleObject *)m)->md_dict;
//    if (d == NULL)
//        return;
//
//    // First, clear only names starting with a single underscore
//    pos = 0;
//    while (PyDict_Next(d, &pos, &key, &value)) {
//        if (value != Py_None && PyString_Check(key)) {
//            char *s = PyString_AsString(key);
//            if (s[0] == '_' && s[1] != '_') {
//                if (Py_VerboseFlag > 1)
//                    PySys_WriteStderr("#   clear[1] %s\n", s);
//                PyDict_SetItem(d, key, Py_None);
//            }
//        }
//    }
//
//    // Next, clear all names except for __builtins__
//    pos = 0;
//    while (PyDict_Next(d, &pos, &key, &value)) {
//        if (value != Py_None && PyString_Check(key)) {
//            char *s = PyString_AsString(key);
//            if (s[0] != '_' || strcmp(s, "__builtins__") != 0) {
//                if (Py_VerboseFlag > 1)
//                    PySys_WriteStderr("#   clear[2] %s\n", s);
//                PyDict_SetItem(d, key, Py_None);
//            }
//        }
//    }
//
////       Note: we leave __builtins__ in place, so that destructors
////       of non-global objects defined in this module can still use
////       builtins, in particularly 'None'.
//
//}

// Methods
//
//static int
//module_init(PyModuleObject *m, PyObject *args, PyObject *kwds)
//{
//    static char *kwlist[] = {"name", "doc", NULL};
//    PyObject *dict, *name = Py_None, *doc = Py_None;
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "S|O:module.__init__",
//                                     kwlist, &name, &doc))
//        return -1;
//    dict = m->md_dict;
//    if (dict == NULL) {
//        dict = PyDict_New();
//        if (dict == NULL)
//            return -1;
//        m->md_dict = dict;
//    }
//    if (PyDict_SetItemString(dict, "__name__", name) < 0)
//        return -1;
//    if (PyDict_SetItemString(dict, "__doc__", doc) < 0)
//        return -1;
//    return 0;
//}

static void
module_dealloc(PyModuleObject *m)
{
    PyObject_GC_UnTrack(m);
//    if (m->md_dict != NULL) {
//        _PyModule_Clear((PyObject *)m);
//        Py_DECREF(m->md_dict);
//    }
    Py_TYPE(m)->tp_free((PyObject *)m);
}

static PyObject *
module_repr(PyModuleObject *m)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
		JyNI_JythonPyObject_FromPyObject((PyObject*) m), pyObject__repr__));
//    char *name;
//    char *filename;
//
//    name = PyModule_GetName((PyObject *)m);
//    if (name == NULL) {
//        PyErr_Clear();
//        name = "?";
//    }
//    filename = PyModule_GetFilename((PyObject *)m);
//    if (filename == NULL) {
//        PyErr_Clear();
//        return PyString_FromFormat("<module '%s' (built-in)>", name);
//    }
//    return PyString_FromFormat("<module '%s' from '%s'>", name, filename);
}

////   We only need a traverse function, no clear function: If the module
////   is in a cycle, md_dict will be cleared as well, which will break
////   the cycle.
//static int
//module_traverse(PyModuleObject *m, visitproc visit, void *arg)
//{
//    Py_VISIT(m->md_dict);
//    return 0;
//}

PyDoc_STRVAR(module_doc,
"module(name[, doc])\n\
\n\
Create a module object.\n\
The name must be a string; the optional doc argument can have any type.");

PyTypeObject PyModule_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "module",                                   // tp_name
    sizeof(PyModuleObject),                     // tp_size
    0,                                          // tp_itemsize
    (destructor)module_dealloc,                 // tp_dealloc
    0,                                          // tp_print
    0,                                          // tp_getattr
    0,                                          // tp_setattr
    0,                                          // tp_compare
    (reprfunc)module_repr,                      // tp_repr
    0,                                          // tp_as_number
    0,                                          // tp_as_sequence
    0,                                          // tp_as_mapping
    0,                                          // tp_hash
    0,                                          // tp_call
    0,                                          // tp_str
    PyObject_GenericGetAttr,                    // tp_getattro
    PyObject_GenericSetAttr,                    // tp_setattro
    0,                                          // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
        Py_TPFLAGS_BASETYPE,                    // tp_flags
    module_doc,                                 // tp_doc
    0,//(traverseproc)module_traverse,              // tp_traverse
    0,                                          // tp_clear
    0,                                          // tp_richcompare
    0,                                          // tp_weaklistoffset
    0,                                          // tp_iter
    0,                                          // tp_iternext
    0,                                          // tp_methods
    0,//module_members,                         // tp_members
    module_getsets,                             // tp_getset
    0,                                          // tp_base
    0,                                          // tp_dict
    0,                                          // tp_descr_get
    0,                                          // tp_descr_set
    0,//offsetof(PyModuleObject, md_dict),      // tp_dictoffset
    0,//(initproc)module_init,                  // tp_init
    PyType_GenericAlloc,                        // tp_alloc
    PyType_GenericNew,                          // tp_new
    PyObject_GC_Del,                            // tp_free
};
