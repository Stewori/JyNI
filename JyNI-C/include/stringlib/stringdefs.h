/* This File is based on stringdefs.h from CPython 2.7.4 release.
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


#ifndef STRINGLIB_STRINGDEFS_H
#define STRINGLIB_STRINGDEFS_H

/* this is sort of a hack.  there's at least one place (formatting
   floats) where some stringlib code takes a different path if it's
   compiled as unicode. */
#define STRINGLIB_IS_UNICODE     0

#define STRINGLIB_OBJECT         PyStringObject
#define STRINGLIB_CHAR           char
#define STRINGLIB_TYPE_NAME      "string"
#define STRINGLIB_PARSE_CODE     "S"
#define STRINGLIB_EMPTY          nullstring
#define STRINGLIB_ISSPACE        Py_ISSPACE
#define STRINGLIB_ISLINEBREAK(x) ((x == '\n') || (x == '\r'))
#define STRINGLIB_ISDECIMAL(x)   ((x >= '0') && (x <= '9'))
#define STRINGLIB_TODECIMAL(x)   (STRINGLIB_ISDECIMAL(x) ? (x - '0') : -1)
#define STRINGLIB_TOUPPER        Py_TOUPPER
#define STRINGLIB_TOLOWER        Py_TOLOWER
#define STRINGLIB_FILL           memset
#define STRINGLIB_STR            PyString_AS_STRING
#define STRINGLIB_LEN            PyString_GET_SIZE
#define STRINGLIB_NEW            PyString_FromStringAndSize
#define STRINGLIB_RESIZE         _PyString_Resize
#define STRINGLIB_CHECK          PyString_Check
#define STRINGLIB_CHECK_EXACT    PyString_CheckExact
#define STRINGLIB_TOSTR          PyObject_Str
#define STRINGLIB_GROUPING       _PyString_InsertThousandsGrouping
#define STRINGLIB_GROUPING_LOCALE _PyString_InsertThousandsGroupingLocale

#define STRINGLIB_WANT_CONTAINS_OBJ 1

#endif /* !STRINGLIB_STRINGDEFS_H */
