/* This File is based on datetime.h from CPython 2.7.13 release.
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

/*  datetime.h
 */

#ifndef DATETIME_H
#define DATETIME_H
#ifdef __cplusplus
extern "C" {
#endif

/* Fields are packed into successive bytes, each viewed as unsigned and
 * big-endian, unless otherwise noted:
 *
 * byte offset
 *  0           year     2 bytes, 1-9999
 *  2           month    1 byte, 1-12
 *  3           day      1 byte, 1-31
 *  4           hour     1 byte, 0-23
 *  5           minute   1 byte, 0-59
 *  6           second   1 byte, 0-59
 *  7           usecond  3 bytes, 0-999999
 * 10
 */

/* # of bytes for year, month, and day. */
#define _PyDateTime_DATE_DATASIZE 4

/* # of bytes for hour, minute, second, and usecond. */
#define _PyDateTime_TIME_DATASIZE 6

/* # of bytes for year, month, day, hour, minute, second, and usecond. */
#define _PyDateTime_DATETIME_DATASIZE 10


typedef struct
{
    PyObject_HEAD
    long hashcode;              /* -1 when unknown */
    int days;                   /* -MAX_DELTA_DAYS <= days <= MAX_DELTA_DAYS */
    int seconds;                /* 0 <= seconds < 24*3600 is invariant */
    int microseconds;           /* 0 <= microseconds < 1000000 is invariant */
} PyDateTime_Delta;

typedef struct
{
    PyObject_HEAD               /* a pure abstract base class */
} PyDateTime_TZInfo;


/* The datetime and time types have hashcodes, and an optional tzinfo member,
 * present if and only if hastzinfo is true.
 */
#define _PyTZINFO_HEAD          \
    PyObject_HEAD               \
    long hashcode;              \
    char hastzinfo;             /* boolean flag */

/* No _PyDateTime_BaseTZInfo is allocated; it's just to have something
 * convenient to cast to, when getting at the hastzinfo member of objects
 * starting with _PyTZINFO_HEAD.
 */
typedef struct
{
    _PyTZINFO_HEAD
} _PyDateTime_BaseTZInfo;

/* All time objects are of PyDateTime_TimeType, but that can be allocated
 * in two ways, with or without a tzinfo member.  Without is the same as
 * tzinfo == None, but consumes less memory.  _PyDateTime_BaseTime is an
 * internal struct used to allocate the right amount of space for the
 * "without" case.
 */
#define _PyDateTime_TIMEHEAD    \
    _PyTZINFO_HEAD              \
    unsigned char data[_PyDateTime_TIME_DATASIZE];

typedef struct
{
    _PyDateTime_TIMEHEAD
} _PyDateTime_BaseTime;         /* hastzinfo false */

typedef struct
{
    _PyDateTime_TIMEHEAD
    PyObject *tzinfo;
} PyDateTime_Time;              /* hastzinfo true */


/* All datetime objects are of PyDateTime_DateTimeType, but that can be
 * allocated in two ways too, just like for time objects above.  In addition,
 * the plain date type is a base class for datetime, so it must also have
 * a hastzinfo member (although it's unused there).
 */
typedef struct
{
    _PyTZINFO_HEAD
    unsigned char data[_PyDateTime_DATE_DATASIZE];
} PyDateTime_Date;

#define _PyDateTime_DATETIMEHEAD        \
    _PyTZINFO_HEAD                      \
    unsigned char data[_PyDateTime_DATETIME_DATASIZE];

typedef struct
{
    _PyDateTime_DATETIMEHEAD
} _PyDateTime_BaseDateTime;     /* hastzinfo false */

typedef struct
{
    _PyDateTime_DATETIMEHEAD
    PyObject *tzinfo;
} PyDateTime_DateTime;          /* hastzinfo true */


/* Apply for date and datetime instances. */
#define PyDateTime_GET_YEAR(o)     ((((PyDateTime_Date*)o)->data[0] << 8) | \
                     ((PyDateTime_Date*)o)->data[1])
#define PyDateTime_GET_MONTH(o)    (((PyDateTime_Date*)o)->data[2])
#define PyDateTime_GET_DAY(o)      (((PyDateTime_Date*)o)->data[3])

#define PyDateTime_DATE_GET_HOUR(o)        (((PyDateTime_DateTime*)o)->data[4])
#define PyDateTime_DATE_GET_MINUTE(o)      (((PyDateTime_DateTime*)o)->data[5])
#define PyDateTime_DATE_GET_SECOND(o)      (((PyDateTime_DateTime*)o)->data[6])
#define PyDateTime_DATE_GET_MICROSECOND(o)              \
    ((((PyDateTime_DateTime*)o)->data[7] << 16) |       \
     (((PyDateTime_DateTime*)o)->data[8] << 8)  |       \
      ((PyDateTime_DateTime*)o)->data[9])

/* Apply for time instances. */
#define PyDateTime_TIME_GET_HOUR(o)        (((PyDateTime_Time*)o)->data[0])
#define PyDateTime_TIME_GET_MINUTE(o)      (((PyDateTime_Time*)o)->data[1])
#define PyDateTime_TIME_GET_SECOND(o)      (((PyDateTime_Time*)o)->data[2])
#define PyDateTime_TIME_GET_MICROSECOND(o)              \
    ((((PyDateTime_Time*)o)->data[3] << 16) |           \
     (((PyDateTime_Time*)o)->data[4] << 8)  |           \
      ((PyDateTime_Time*)o)->data[5])


/* Define structure for C API. */
typedef struct {
    /* type objects */
    PyTypeObject *DateType;
    PyTypeObject *DateTimeType;
    PyTypeObject *TimeType;
    PyTypeObject *DeltaType;
    PyTypeObject *TZInfoType;

    /* constructors */
    PyObject *(*Date_FromDate)(int, int, int, PyTypeObject*);
    PyObject *(*DateTime_FromDateAndTime)(int, int, int, int, int, int, int,
        PyObject*, PyTypeObject*);
    PyObject *(*Time_FromTime)(int, int, int, int, PyObject*, PyTypeObject*);
    PyObject *(*Delta_FromDelta)(int, int, int, int, PyTypeObject*);

    /* constructors for the DB API */
    PyObject *(*DateTime_FromTimestamp)(PyObject*, PyObject*, PyObject*);
    PyObject *(*Date_FromTimestamp)(PyObject*, PyObject*);

} PyDateTime_CAPI;

#define PyDateTime_CAPSULE_NAME "datetime.datetime_CAPI"


/* "magic" constant used to partially protect against developer mistakes. */
#define DATETIME_API_MAGIC 0x414548d5

#ifdef Py_BUILD_CORE

/* Macros for type checking when building the Python core. */
#define PyDate_Check(op) PyObject_TypeCheck(op, &PyDateTime_DateType)
#define PyDate_CheckExact(op) (Py_TYPE(op) == &PyDateTime_DateType)

#define PyDateTime_Check(op) PyObject_TypeCheck(op, &PyDateTime_DateTimeType)
#define PyDateTime_CheckExact(op) (Py_TYPE(op) == &PyDateTime_DateTimeType)

#define PyTime_Check(op) PyObject_TypeCheck(op, &PyDateTime_TimeType)
#define PyTime_CheckExact(op) (Py_TYPE(op) == &PyDateTime_TimeType)

#define PyDelta_Check(op) PyObject_TypeCheck(op, &PyDateTime_DeltaType)
#define PyDelta_CheckExact(op) (Py_TYPE(op) == &PyDateTime_DeltaType)

#define PyTZInfo_Check(op) PyObject_TypeCheck(op, &PyDateTime_TZInfoType)
#define PyTZInfo_CheckExact(op) (Py_TYPE(op) == &PyDateTime_TZInfoType)

#else

/* Define global variable for the C API and a macro for setting it. */
static PyDateTime_CAPI *PyDateTimeAPI = NULL;

#define PyDateTime_IMPORT \
    PyDateTimeAPI = (PyDateTime_CAPI *)PyCapsule_Import(PyDateTime_CAPSULE_NAME, 0)

/* Macros for type checking when not building the Python core. */
#define PyDate_Check(op) PyObject_TypeCheck(op, PyDateTimeAPI->DateType)
#define PyDate_CheckExact(op) (Py_TYPE(op) == PyDateTimeAPI->DateType)

#define PyDateTime_Check(op) PyObject_TypeCheck(op, PyDateTimeAPI->DateTimeType)
#define PyDateTime_CheckExact(op) (Py_TYPE(op) == PyDateTimeAPI->DateTimeType)

#define PyTime_Check(op) PyObject_TypeCheck(op, PyDateTimeAPI->TimeType)
#define PyTime_CheckExact(op) (Py_TYPE(op) == PyDateTimeAPI->TimeType)

#define PyDelta_Check(op) PyObject_TypeCheck(op, PyDateTimeAPI->DeltaType)
#define PyDelta_CheckExact(op) (Py_TYPE(op) == PyDateTimeAPI->DeltaType)

#define PyTZInfo_Check(op) PyObject_TypeCheck(op, PyDateTimeAPI->TZInfoType)
#define PyTZInfo_CheckExact(op) (Py_TYPE(op) == PyDateTimeAPI->TZInfoType)

/* Macros for accessing constructors in a simplified fashion. */
#define PyDate_FromDate(year, month, day) \
    PyDateTimeAPI->Date_FromDate(year, month, day, PyDateTimeAPI->DateType)

#define PyDateTime_FromDateAndTime(year, month, day, hour, min, sec, usec) \
    PyDateTimeAPI->DateTime_FromDateAndTime(year, month, day, hour, \
        min, sec, usec, Py_None, PyDateTimeAPI->DateTimeType)

#define PyTime_FromTime(hour, minute, second, usecond) \
    PyDateTimeAPI->Time_FromTime(hour, minute, second, usecond, \
        Py_None, PyDateTimeAPI->TimeType)

#define PyDelta_FromDSU(days, seconds, useconds) \
    PyDateTimeAPI->Delta_FromDelta(days, seconds, useconds, 1, \
        PyDateTimeAPI->DeltaType)

/* Macros supporting the DB API. */
#define PyDateTime_FromTimestamp(args) \
    PyDateTimeAPI->DateTime_FromTimestamp( \
        (PyObject*) (PyDateTimeAPI->DateTimeType), args, NULL)

#define PyDate_FromTimestamp(args) \
    PyDateTimeAPI->Date_FromTimestamp( \
        (PyObject*) (PyDateTimeAPI->DateType), args)

#endif  /* Py_BUILD_CORE */

#ifdef __cplusplus
}
#endif
#endif
