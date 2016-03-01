/* This File is based on Python.h from CPython 2.7.3 release.
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

/* JyNI comment:
 * Our long term goal is to build JyNI against the original Python headers.
 * In the current state we need the ability to exclude symbols from the headers
 * for debugging purposes. To distinguish between the original headers and our
 * modified ones, the modified ones have an appended "_JyNI" in their names.
 * We still keep the differences to the original headers as small as possible.
 * Using RTLD_LAZY for dynamic loading gives us a fail-late behavior if
 * extensions need any symbols excluded from our header versions. This way we
 * are able to test partly implemented protocols and built-in types, converging
 * to a full implementation over time.
 */
#ifndef Py_PYTHON_H
#define Py_PYTHON_H
/* Since this is a "meta-include" file, no #ifdef __cplusplus / extern "C" { */

/* Include nearly all Python header files */

//#include "patchlevel_JyNI.h"
#include "pyconfig.h"
//#include "pymacconfig_JyNI.h"

/* Cyclic gc is always enabled, starting with release 2.3a1.  Supply the
 * old symbol for the benefit of extension modules written before then
 * that may be conditionalizing on it.  The core doesn't use it anymore.
 */
#ifndef WITH_CYCLE_GC
#define WITH_CYCLE_GC 1
#endif

#include <limits.h>

#ifndef UCHAR_MAX
#error "Something's broken.  UCHAR_MAX should be defined in limits.h."
#endif

#if UCHAR_MAX != 255
#error "Python's source code assumes C's unsigned char is an 8-bit type."
#endif

#if defined(__sgi) && defined(WITH_THREAD) && !defined(_SGI_MP_SOURCE)
#define _SGI_MP_SOURCE
#endif

#include <stdio.h>
#ifndef NULL
#   error "Python.h requires that stdio.h define NULL."
#endif

#include <string.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* For size_t? */
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

/* CAUTION:  Build setups should ensure that NDEBUG is defined on the
 * compiler command line when building Python in release mode; else
 * assert() calls won't be removed.
 */
#include <assert.h>

#include "pyport_JyNI.h"

/* pyconfig.h or pyport.h may or may not define DL_IMPORT */
#ifndef DL_IMPORT	/* declarations for DLL import/export */
#define DL_IMPORT(RTYPE) RTYPE
#endif
#ifndef DL_EXPORT	/* declarations for DLL import/export */
#define DL_EXPORT(RTYPE) RTYPE
#endif

/* Debug-mode build with pymalloc implies PYMALLOC_DEBUG.
 *  PYMALLOC_DEBUG is in error if pymalloc is not in use.
 */
#if defined(Py_DEBUG) && defined(WITH_PYMALLOC) && !defined(PYMALLOC_DEBUG)
#define PYMALLOC_DEBUG
#endif
#if defined(PYMALLOC_DEBUG) && !defined(WITH_PYMALLOC)
#error "PYMALLOC_DEBUG requires WITH_PYMALLOC"
#endif
#include "pymath_JyNI.h"
#include "pymem_JyNI.h"

#include "object_JyNI.h"
#include "objimpl_JyNI.h"

#include "pydebug_JyNI.h"

#include "unicodeobject_JyNI.h"
#include "intobject_JyNI.h"
#include "boolobject_JyNI.h"
#include "longobject_JyNI.h"
#include "floatobject_JyNI.h"
#ifndef WITHOUT_COMPLEX
#include "complexobject_JyNI.h"
#endif
//#include "rangeobject_JyNI.h"
#include "stringobject_JyNI.h"
//#include "memoryobject_JyNI.h"
#include "bufferobject_JyNI.h"
#include "bytesobject_JyNI.h"
//#include "bytearrayobject_JyNI.h"
#include "tupleobject_JyNI.h"
#include "listobject_JyNI.h"
#include "dictobject_JyNI.h"
//#include "enumobject_JyNI.h"
#include "setobject_JyNI.h"
#include "methodobject_JyNI.h"
#include "moduleobject_JyNI.h"
#include "funcobject_JyNI.h"
#include "classobject_JyNI.h"
#include "fileobject_JyNI.h"
#include "cobject_JyNI.h"
#include "pycapsule_JyNI.h"
#include "traceback_JyNI.h"
#include "sliceobject_JyNI.h"
#include "cellobject_JyNI.h"
//#include "iterobject_JyNI.h"
//#include "genobject_JyNI.h"
#include "descrobject_JyNI.h"
#include "warnings_JyNI.h"
#include "weakrefobject_JyNI.h"

#include "codecs_JyNI.h"
#include "pyerrors_JyNI.h"

#include "pystate_JyNI.h"

//#include "pyarena_JyNI.h"
#include "modsupport_JyNI.h"
#include "pythonrun_JyNI.h"
#include "ceval_JyNI.h"
#include "sysmodule_JyNI.h"
//#include "intrcheck_JyNI.h"
#include "import_JyNI.h"

#include "abstract_JyNI.h"

//#include "compile_JyNI.h"
//#include "eval_JyNI.h"

#include "pyctype_JyNI.h"
#include "pystrtod_JyNI.h"
//#include "pystrcmp_JyNI.h"
#include "dtoa_JyNI.h"

/* _Py_Mangle is defined in compile.c */
//PyAPI_FUNC(PyObject*) _Py_Mangle(PyObject *p, PyObject *name);

/* PyArg_GetInt is deprecated and should not be used, use PyArg_Parse(). */
//#define PyArg_GetInt(v, a)	PyArg_Parse((v), "i", (a))

/* PyArg_NoArgs should not be necessary.
   Set ml_flags in the PyMethodDef to METH_NOARGS. */
//#define PyArg_NoArgs(v)		PyArg_Parse(v, "")

/* Argument must be a char or an int in [-128, 127] or [0, 255]. */
#define Py_CHARMASK(c)		((unsigned char)((c) & 0xff))

#include "pyfpe_JyNI.h"

/* These definitions must match corresponding definitions in graminit.h.
   There's code in compile.c that checks that they are the same. */
#define Py_single_input 256
#define Py_file_input 257
#define Py_eval_input 258

//#ifdef HAVE_PTH
/* GNU pth user-space thread support */
//#include <pth.h>
//#endif

/* Define macros for inline documentation. */
#define PyDoc_VAR(name) static char name[]
#define PyDoc_STRVAR(name,str) PyDoc_VAR(name) = PyDoc_STR(str)
#ifdef WITH_DOC_STRINGS
#define PyDoc_STR(str) str
#else
#define PyDoc_STR(str) ""
#endif

#endif /* !Py_PYTHON_H */
