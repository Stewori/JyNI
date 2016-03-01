/* This File is based on partition.h from CPython 2.7.4 release.
 * It has been modified to suit JyNI needs.
 *
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
 * 2010, 2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
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


/* stringlib: partition implementation */

#ifndef STRINGLIB_PARTITION_H
#define STRINGLIB_PARTITION_H

#ifndef STRINGLIB_FASTSEARCH_H
#error must include "stringlib/fastsearch.h" before including this module
#endif

Py_LOCAL_INLINE(PyObject*)
stringlib_partition(PyObject* str_obj,
                    const STRINGLIB_CHAR* str, Py_ssize_t str_len,
                    PyObject* sep_obj,
                    const STRINGLIB_CHAR* sep, Py_ssize_t sep_len)
{
    PyObject* out;
    Py_ssize_t pos;

    if (sep_len == 0) {
        PyErr_SetString(PyExc_ValueError, "empty separator");
        return NULL;
    }

    out = PyTuple_New(3);
    if (!out)
        return NULL;

    pos = fastsearch(str, str_len, sep, sep_len, -1, FAST_SEARCH);

    if (pos < 0) {
#if STRINGLIB_MUTABLE
        PyTuple_SET_ITEM(out, 0, STRINGLIB_NEW(str, str_len));
        PyTuple_SET_ITEM(out, 1, STRINGLIB_NEW(NULL, 0));
        PyTuple_SET_ITEM(out, 2, STRINGLIB_NEW(NULL, 0));
#else
        Py_INCREF(str_obj);
        PyTuple_SET_ITEM(out, 0, (PyObject*) str_obj);
        Py_INCREF(STRINGLIB_EMPTY);
        PyTuple_SET_ITEM(out, 1, (PyObject*) STRINGLIB_EMPTY);
        Py_INCREF(STRINGLIB_EMPTY);
        PyTuple_SET_ITEM(out, 2, (PyObject*) STRINGLIB_EMPTY);
#endif
        return out;
    }

    PyTuple_SET_ITEM(out, 0, STRINGLIB_NEW(str, pos));
    Py_INCREF(sep_obj);
    PyTuple_SET_ITEM(out, 1, sep_obj);
    pos += sep_len;
    PyTuple_SET_ITEM(out, 2, STRINGLIB_NEW(str + pos, str_len - pos));

    if (PyErr_Occurred()) {
        Py_DECREF(out);
        return NULL;
    }

    return out;
}

Py_LOCAL_INLINE(PyObject*)
stringlib_rpartition(PyObject* str_obj,
                     const STRINGLIB_CHAR* str, Py_ssize_t str_len,
                     PyObject* sep_obj,
                     const STRINGLIB_CHAR* sep, Py_ssize_t sep_len)
{
    PyObject* out;
    Py_ssize_t pos;

    if (sep_len == 0) {
        PyErr_SetString(PyExc_ValueError, "empty separator");
        return NULL;
    }

    out = PyTuple_New(3);
    if (!out)
        return NULL;

    pos = fastsearch(str, str_len, sep, sep_len, -1, FAST_RSEARCH);

    if (pos < 0) {
#if STRINGLIB_MUTABLE
        PyTuple_SET_ITEM(out, 0, STRINGLIB_NEW(NULL, 0));
        PyTuple_SET_ITEM(out, 1, STRINGLIB_NEW(NULL, 0));
        PyTuple_SET_ITEM(out, 2, STRINGLIB_NEW(str, str_len));
#else
        Py_INCREF(STRINGLIB_EMPTY);
        PyTuple_SET_ITEM(out, 0, (PyObject*) STRINGLIB_EMPTY);
        Py_INCREF(STRINGLIB_EMPTY);
        PyTuple_SET_ITEM(out, 1, (PyObject*) STRINGLIB_EMPTY);
        Py_INCREF(str_obj);
        PyTuple_SET_ITEM(out, 2, (PyObject*) str_obj);
#endif
        return out;
    }

    PyTuple_SET_ITEM(out, 0, STRINGLIB_NEW(str, pos));
    Py_INCREF(sep_obj);
    PyTuple_SET_ITEM(out, 1, sep_obj);
    pos += sep_len;
    PyTuple_SET_ITEM(out, 2, STRINGLIB_NEW(str + pos, str_len - pos));

    if (PyErr_Occurred()) {
        Py_DECREF(out);
        return NULL;
    }

    return out;
}

#endif
