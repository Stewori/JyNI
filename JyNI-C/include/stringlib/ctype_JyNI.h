/* This File is based on ctype.h from CPython 2.7.4 release.
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


/*
 * JyNI comment:
 * We attempted to keep stringlib unmodified, but this file caused errors during
 * build.
 *
 * Adding the lines
 *
 * #ifndef CTYPE_H
 * #define CTYPE_H
 *
 * fixed the problem for build, but the definitions of following
 * pattern produced symbol lookup errors at runtime:
 *
 * #undef islower
 * #define islower(c) undefined_islower(c)
 *
 * For now we solved the issue by renaming this file, so that it does not overwrite
 * the system's ctype.h.
 */

/* NOTE: this API is -ONLY- for use with single byte character strings. */
/* Do not use it with Unicode. */

#include "bytes_methods_JyNI.h"

static PyObject*
stringlib_isspace(PyObject *self)
{
    return _Py_bytes_isspace(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}

static PyObject*
stringlib_isalpha(PyObject *self)
{
    return _Py_bytes_isalpha(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}

static PyObject*
stringlib_isalnum(PyObject *self)
{
    return _Py_bytes_isalnum(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}

static PyObject*
stringlib_isdigit(PyObject *self)
{
    return _Py_bytes_isdigit(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}

static PyObject*
stringlib_islower(PyObject *self)
{
    return _Py_bytes_islower(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}

static PyObject*
stringlib_isupper(PyObject *self)
{
    return _Py_bytes_isupper(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}

static PyObject*
stringlib_istitle(PyObject *self)
{
    return _Py_bytes_istitle(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}


/* functions that return a new object partially translated by ctype funcs: */

static PyObject*
stringlib_lower(PyObject *self)
{
    PyObject* newobj;
    newobj = STRINGLIB_NEW(NULL, STRINGLIB_LEN(self));
    if (!newobj)
            return NULL;
    _Py_bytes_lower(STRINGLIB_STR(newobj), STRINGLIB_STR(self),
                 STRINGLIB_LEN(self));
    return newobj;
}

static PyObject*
stringlib_upper(PyObject *self)
{
    PyObject* newobj;
    newobj = STRINGLIB_NEW(NULL, STRINGLIB_LEN(self));
    if (!newobj)
            return NULL;
    _Py_bytes_upper(STRINGLIB_STR(newobj), STRINGLIB_STR(self),
                 STRINGLIB_LEN(self));
    return newobj;
}

static PyObject*
stringlib_title(PyObject *self)
{
    PyObject* newobj;
    newobj = STRINGLIB_NEW(NULL, STRINGLIB_LEN(self));
    if (!newobj)
            return NULL;
    _Py_bytes_title(STRINGLIB_STR(newobj), STRINGLIB_STR(self),
                 STRINGLIB_LEN(self));
    return newobj;
}

static PyObject*
stringlib_capitalize(PyObject *self)
{
    PyObject* newobj;
    newobj = STRINGLIB_NEW(NULL, STRINGLIB_LEN(self));
    if (!newobj)
            return NULL;
    _Py_bytes_capitalize(STRINGLIB_STR(newobj), STRINGLIB_STR(self),
                      STRINGLIB_LEN(self));
    return newobj;
}

static PyObject*
stringlib_swapcase(PyObject *self)
{
    PyObject* newobj;
    newobj = STRINGLIB_NEW(NULL, STRINGLIB_LEN(self));
    if (!newobj)
            return NULL;
    _Py_bytes_swapcase(STRINGLIB_STR(newobj), STRINGLIB_STR(self),
                    STRINGLIB_LEN(self));
    return newobj;
}
