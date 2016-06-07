/* This File is based on traceback.c from CPython 2.7.3 release.
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


/* Traceback implementation */

#include "JyNI.h"

#include "code_JyNI.h"
#include "frameobject_JyNI.h"
#include "structmember_JyNI.h"
//#include "osdefs.h"
//#include "traceback.h"

//#define OFF(x) offsetof(PyTracebackObject, x)
//
//static PyMemberDef tb_memberlist[] = {
//	{"tb_next", 	    T_OBJECT,       OFF(tb_next), READONLY},
//	{"tb_frame",        T_OBJECT,       OFF(tb_frame), READONLY},
//	{"tb_lasti",        T_INT,          OFF(tb_lasti), READONLY},
//	{"tb_lineno",       T_INT,          OFF(tb_lineno), READONLY},
//	{NULL}      /* Sentinel */
//};

static void
tb_dealloc(PyTracebackObject *tb)
{
	PyObject_Free(tb);
//	PyObject_GC_UnTrack(tb);
//	Py_TRASHCAN_SAFE_BEGIN(tb)
//	Py_XDECREF(tb->tb_next);
//	Py_XDECREF(tb->tb_frame);
//	PyObject_GC_Del(tb);
//	Py_TRASHCAN_SAFE_END(tb)
}

//static int
//tb_traverse(PyTracebackObject *tb, visitproc visit, void *arg)
//{
//	Py_VISIT(tb->tb_next);
//	Py_VISIT(tb->tb_frame);
//	return 0;
//}
//
//static void
//tb_clear(PyTracebackObject *tb)
//{
//	Py_CLEAR(tb->tb_next);
//	Py_CLEAR(tb->tb_frame);
//}

PyTypeObject PyTraceBack_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"traceback",
	sizeof(PyTracebackObject),
	0,
	(destructor)tb_dealloc,                     // tp_dealloc
	0,                                          // tp_print
	0,                                          // tp_getattr
	0,                                          // tp_setattr
	0,                                          // tp_compare
	0,                                          // tp_repr
	0,                                          // tp_as_number
	0,                                          // tp_as_sequence
	0,                                          // tp_as_mapping
	0,                                          // tp_hash
	0,                                          // tp_call
	0,                                          // tp_str
	0,                                          // tp_getattro
	0,                                          // tp_setattro
	0,                                          // tp_as_buffer
	Py_TPFLAGS_DEFAULT,//| Py_TPFLAGS_HAVE_GC,  // tp_flags
	0,                                          // tp_doc
	0,//(traverseproc)tb_traverse,              // tp_traverse
	0,//(inquiry)tb_clear,                      // tp_clear
	0,                                          // tp_richcompare
	0,                                          // tp_weaklistoffset
	0,                                          // tp_iter
	0,                                          // tp_iternext
	0,                                          // tp_methods
	0,//tb_memberlist,                          // tp_members
	0,                                          // tp_getset
	0,                                          // tp_base
	0,                                          // tp_dict
};
/*
static PyTracebackObject *
newtracebackobject(PyTracebackObject *next, PyFrameObject *frame)
{
	PyTracebackObject *tb;
	if ((next != NULL && !PyTraceBack_Check(next)) ||
					frame == NULL || !PyFrame_Check(frame)) {
		PyErr_BadInternalCall();
		return NULL;
	}
	tb = PyObject_GC_New(PyTracebackObject, &PyTraceBack_Type);
	if (tb != NULL) {
		Py_XINCREF(next);
		tb->tb_next = next;
		Py_XINCREF(frame);
		tb->tb_frame = frame;
		tb->tb_lasti = frame->f_lasti;
		tb->tb_lineno = PyFrame_GetLineNumber(frame);
		PyObject_GC_Track(tb);
	}
	return tb;
}*/

int
PyTraceBack_Here(PyFrameObject *frame)
{
//	jputs(__FUNCTION__);
	PyThreadState *tstate = PyThreadState_GET();
	PyTracebackObject *oldtb = (PyTracebackObject *) tstate->curexc_traceback;
	env(-1);
	jobject new_tb = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_PyTraceBack_Here,
			JyNI_JythonPyObject_FromPyObject(frame), TS_GET_JY(tstate));
	if ((*env)->IsSameObject(env, new_tb, NULL))
	{
		if ((*env)->ExceptionCheck(env))
			(*env)->ExceptionDescribe(env);
	}
//	PyTracebackObject *tb = newtracebackobject(oldtb, frame);
	PyTracebackObject* tb = (PyTracebackObject*) JyNI_PyObject_FromJythonPyObject(new_tb);
	if (tb == NULL)
		return -1;
	tstate->curexc_traceback = (PyObject *)tb;
	Py_XDECREF(oldtb);
	return 0;
}
/*
int
_Py_DisplaySourceLine(PyObject *f, const char *filename, int lineno, int indent)
{
	int err = 0;
	FILE *xfp = NULL;
	char linebuf[2000];
	int i;
	char namebuf[MAXPATHLEN+1];

	if (filename == NULL)
		return -1;
	// This is needed by Emacs' compile command
#define FMT "  File \"%.500s\", line %d, in %.500s\n"
	xfp = fopen(filename, "r" PY_STDIOTEXTMODE);
	if (xfp == NULL) {
		// Search tail of filename in sys.path before giving up
		PyObject *path;
		const char *tail = strrchr(filename, SEP);
		if (tail == NULL)
			tail = filename;
		else
			tail++;
		path = PySys_GetObject("path");
		if (path != NULL && PyList_Check(path)) {
			Py_ssize_t _npath = PyList_Size(path);
			int npath = Py_SAFE_DOWNCAST(_npath, Py_ssize_t, int);
			size_t taillen = strlen(tail);
			for (i = 0; i < npath; i++) {
				PyObject *v = PyList_GetItem(path, i);
				if (v == NULL) {
					PyErr_Clear();
					break;
				}
				if (PyString_Check(v)) {
					size_t len;
					len = PyString_GET_SIZE(v);
					if (len + 1 + taillen >= MAXPATHLEN)
						continue; // Too long
					strcpy(namebuf, PyString_AsString(v));
					if (strlen(namebuf) != len)
						continue; // v contains '\0'
					if (len > 0 && namebuf[len-1] != SEP)
						namebuf[len++] = SEP;
					strcpy(namebuf+len, tail);
					xfp = fopen(namebuf, "r" PY_STDIOTEXTMODE);
					if (xfp != NULL) {
						break;
					}
				}
			}
		}
	}

	if (xfp == NULL)
		return err;
	if (err != 0) {
		fclose(xfp);
		return err;
	}

	for (i = 0; i < lineno; i++) {
		char* pLastChar = &linebuf[sizeof(linebuf)-2];
		do {
			*pLastChar = '\0';
			if (Py_UniversalNewlineFgets(linebuf, sizeof linebuf, xfp, NULL) == NULL)
				break;
			// fgets read *something*; if it didn't get as
			// far as pLastChar, it must have found a newline
		    // or hit the end of the file	           if pLastChar is \n,
			// it obviously found a newline; else we haven't
			// yet seen a newline, so must continue
		} while (*pLastChar != '\0' && *pLastChar != '\n');
	}
	if (i == lineno) {
		char buf[11];
		char *p = linebuf;
		while (*p == ' ' || *p == '\t' || *p == '\014')
			p++;

		// Write some spaces before the line
		strcpy(buf, "          ");
		assert (strlen(buf) == 10);
		while (indent > 0) {
			if(indent < 10)
				buf[indent] = '\0';
			err = PyFile_WriteString(buf, f);
			if (err != 0)
				break;
			indent -= 10;
		}

		if (err == 0)
			err = PyFile_WriteString(p, f);
		if (err == 0 && strchr(p, '\n') == NULL)
			err = PyFile_WriteString("\n", f);
	}
	fclose(xfp);
	return err;
}

static int
tb_displayline(PyObject *f, const char *filename, int lineno, const char *name)
{
	int err = 0;
	char linebuf[2000];

	if (filename == NULL || name == NULL)
		return -1;
	// This is needed by Emacs' compile command
#define FMT "  File \"%.500s\", line %d, in %.500s\n"
	PyOS_snprintf(linebuf, sizeof(linebuf), FMT, filename, lineno, name);
	err = PyFile_WriteString(linebuf, f);
	if (err != 0)
		return err;
	return _Py_DisplaySourceLine(f, filename, lineno, 4);
}

static int
tb_printinternal(PyTracebackObject *tb, PyObject *f, long limit)
{
	int err = 0;
	long depth = 0;
	PyTracebackObject *tb1 = tb;
	while (tb1 != NULL) {
		depth++;
		tb1 = tb1->tb_next;
	}
	while (tb != NULL && err == 0) {
		if (depth <= limit) {
			err = tb_displayline(f,
				PyString_AsString(
					tb->tb_frame->f_code->co_filename),
				tb->tb_lineno,
				PyString_AsString(tb->tb_frame->f_code->co_name));
		}
		depth--;
		tb = tb->tb_next;
		if (err == 0)
			err = PyErr_CheckSignals();
	}
	return err;
}

int
PyTraceBack_Print(PyObject *v, PyObject *f)
{
	int err;
	PyObject *limitv;
	long limit = 1000;
	if (v == NULL)
		return 0;
	if (!PyTraceBack_Check(v)) {
		PyErr_BadInternalCall();
		return -1;
	}
	limitv = PySys_GetObject("tracebacklimit");
	if (limitv && PyInt_Check(limitv)) {
		limit = PyInt_AsLong(limitv);
		if (limit <= 0)
			return 0;
	}
	err = PyFile_WriteString("Traceback (most recent call last):\n", f);
	if (!err)
		err = tb_printinternal((PyTracebackObject *)v, f, limit);
    return err;
}
*/
