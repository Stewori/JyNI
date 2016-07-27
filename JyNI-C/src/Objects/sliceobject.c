/* This File is based on sliceobject.c from CPython 2.7.4 release.
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


/*
Original file written by Jim Hugunin and Chris Chase.

This includes both the singular ellipsis object and slice objects.
*/

/*
Py_Ellipsis encodes the '...' rubber index token. It is similar to
the Py_NoneStruct in that there is no way to create other objects of
this type and there is exactly one in existence.
*/

#include "JyNI.h"
#include "structmember_JyNI.h"

static PyObject *
ellipsis_repr(PyObject *op)
{
	return PyString_FromString("Ellipsis");
}

PyTypeObject PyEllipsis_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"ellipsis",                         /* tp_name */
	0,                                  /* tp_basicsize */
	0,                                  /* tp_itemsize */
	0, /*never called*/                 /* tp_dealloc */
	0,                                  /* tp_print */
	0,                                  /* tp_getattr */
	0,                                  /* tp_setattr */
	0,                                  /* tp_compare */
	ellipsis_repr,                      /* tp_repr */
	0,                                  /* tp_as_number */
	0,                                  /* tp_as_sequence */
	0,                                  /* tp_as_mapping */
	0,                                  /* tp_hash */
	0,                                  /* tp_call */
	0,                                  /* tp_str */
	PyObject_GenericGetAttr,            /* tp_getattro */
	0,                                  /* tp_setattro */
	0,                                  /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                 /* tp_flags */
};

PyObject _Py_EllipsisObject = {
	_PyObject_EXTRA_INIT
	1, &PyEllipsis_Type
};


/* Slice object implementation

   start, stop, and step are python objects with None indicating no
   index is present.
*/

PyObject *
PySlice_New(PyObject *start, PyObject *stop, PyObject *step)
{
	if (step == NULL) step = Py_None;
	Py_INCREF(step);
	if (start == NULL) start = Py_None;
	Py_INCREF(start);
	if (stop == NULL) stop = Py_None;
	Py_INCREF(stop);
	env(NULL);
	PySliceObject *obj = (PySliceObject*) _JyObject_New(&PySlice_Type, &builtinTypes[TME_INDEX_Slice]);
	JyObject* jy = AS_JY_NO_GC(obj);
	jy->jy = (*env)->NewObject(env, pySliceClass, pySliceFromStartStopStepConstructor,
			JyNI_JythonPyObject_FromPyObject(start),
			JyNI_JythonPyObject_FromPyObject(stop),
			JyNI_JythonPyObject_FromPyObject(step));
	jy->flags |= JY_INITIALIZED_FLAG_MASK;
	/*
	 * Todo:
	 * - Maybe call JyNI_GC_EnsureHeadObject here.
	 * - Maybe call JyNI_GC_Track_CStub
	 */

//	PySliceObject *obj = PyObject_New(PySliceObject, &PySlice_Type);
//
//	if (obj == NULL)
//		return NULL;
//
//	if (step == NULL) step = Py_None;
//	Py_INCREF(step);
//	if (start == NULL) start = Py_None;
//	Py_INCREF(start);
//	if (stop == NULL) stop = Py_None;
//	Py_INCREF(stop);
//
//	obj->step = step;
//	obj->start = start;
//	obj->stop = stop;
//
	return (PyObject *) obj;
}

PyObject *
_PySlice_FromIndices(Py_ssize_t istart, Py_ssize_t istop)
{
	PyObject *start, *end, *slice;
	start = PyInt_FromSsize_t(istart);
	if (!start)
		return NULL;
	end = PyInt_FromSsize_t(istop);
	if (!end) {
		Py_DECREF(start);
		return NULL;
	}

	slice = PySlice_New(start, end, NULL);
	Py_DECREF(start);
	Py_DECREF(end);
	return slice;
}

int
PySlice_GetIndices(PySliceObject *r, Py_ssize_t length,
				   Py_ssize_t *start, Py_ssize_t *stop, Py_ssize_t *step)
{
	return PySlice_GetIndicesEx(r, length,
						 start, stop, step, NULL);
	/* XXX support long ints */
//	if (r->step == Py_None) {
//		*step = 1;
//	} else {
//		if (!PyInt_Check(r->step) && !PyLong_Check(r->step)) return -1;
//		*step = PyInt_AsSsize_t(r->step);
//	}
//	if (r->start == Py_None) {
//		*start = *step < 0 ? length-1 : 0;
//	} else {
//		if (!PyInt_Check(r->start) && !PyLong_Check(r->step)) return -1;
//		*start = PyInt_AsSsize_t(r->start);
//		if (*start < 0) *start += length;
//	}
//	if (r->stop == Py_None) {
//		*stop = *step < 0 ? -1 : length;
//	} else {
//		if (!PyInt_Check(r->stop) && !PyLong_Check(r->step)) return -1;
//		*stop = PyInt_AsSsize_t(r->stop);
//		if (*stop < 0) *stop += length;
//	}
//	if (*stop > length) return -1;
//	if (*start >= length) return -1;
//	if (*step == 0) return -1;
//	return 0;
}

int
PySlice_GetIndicesEx(PySliceObject *r, Py_ssize_t length,
					 Py_ssize_t *start, Py_ssize_t *stop, Py_ssize_t *step, Py_ssize_t *slicelength)
{
	env(-1);
	jarray res = (*env)->CallObjectMethod(env,
			JyNI_JythonPyObject_FromPyObject((PyObject*) r),
			pySliceIndicesEx, (jint) length);
	jint* arr = (*env)->GetIntArrayElements(env, res, NULL);
	if (arr == NULL)
	{
		//(*env)->ReleaseIntArrayElements(env, res, arr, JNI_ABORT);
		return -1;
	} else
	{
		//if (start != NULL)
			*start = arr[0];
		//if (stop != NULL)
			*stop = arr[1];
		//if (step != NULL)
			*step = arr[2];
		if (slicelength != NULL) *slicelength = arr[3];
		(*env)->ReleaseIntArrayElements(env, res, arr, JNI_ABORT);
		return 0;
	}

	/* this is harder to get right than you might think */

//	Py_ssize_t defstart, defstop;
//
//	if (r->step == Py_None) {
//		*step = 1;
//	}
//	else {
//		if (!_PyEval_SliceIndex(r->step, step)) return -1;
//		if (*step == 0) {
//			PyErr_SetString(PyExc_ValueError,
//							"slice step cannot be zero");
//			return -1;
//		}
//	}
//
//	defstart = *step < 0 ? length-1 : 0;
//	defstop = *step < 0 ? -1 : length;
//
//	if (r->start == Py_None) {
//		*start = defstart;
//	}
//	else {
//		if (!_PyEval_SliceIndex(r->start, start)) return -1;
//		if (*start < 0) *start += length;
//		if (*start < 0) *start = (*step < 0) ? -1 : 0;
//		if (*start >= length)
//			*start = (*step < 0) ? length - 1 : length;
//	}
//
//	if (r->stop == Py_None) {
//		*stop = defstop;
//	}
//	else {
//		if (!_PyEval_SliceIndex(r->stop, stop)) return -1;
//		if (*stop < 0) *stop += length;
//		if (*stop < 0) *stop = (*step < 0) ? -1 : 0;
//		if (*stop >= length)
//			*stop = (*step < 0) ? length - 1 : length;
//	}
//
//	if ((*step < 0 && *stop >= *start)
//		|| (*step > 0 && *start >= *stop)) {
//		*slicelength = 0;
//	}
//	else if (*step < 0) {
//		*slicelength = (*stop-*start+1)/(*step)+1;
//	}
//	else {
//		*slicelength = (*stop-*start-1)/(*step)+1;
//	}
//
//	return 0;
}

static PyObject *
slice_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	PyObject *start, *stop, *step;

	start = stop = step = NULL;

	if (!_PyArg_NoKeywords("slice()", kw))
		return NULL;

	if (!PyArg_UnpackTuple(args, "slice", 1, 3, &start, &stop, &step))
		return NULL;

	/* This swapping of stop and start is to maintain similarity with
	   range(). */
	if (stop == NULL) {
		stop = start;
		start = NULL;
	}
	return PySlice_New(start, stop, step);
}

PyDoc_STRVAR(slice_doc,
"slice(stop)\n\
slice(start, stop[, step])\n\
\n\
Create a slice object.  This is used for extended slicing (e.g. a[0:10:2]).");

static void
slice_dealloc(PySliceObject *r)
{
	JyObject* jy = AS_JY_NO_GC(r);
	if (JyObject_IS_INITIALIZED(jy))
	{
		env();
		Py_DECREF(JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, jy->jy, pySliceGetStart)));
		Py_DECREF(JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, jy->jy, pySliceGetStop)));
		Py_DECREF(JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env, jy->jy, pySliceGetStep)));
//		Py_DECREF(r->step);
//		Py_DECREF(r->start);
//		Py_DECREF(r->stop);
//		PyObject_Del(r);
	}
	JyNI_CleanUp_JyObject(jy);
	//PyObject_Del(jy);
	PyObject_RawFree(jy);
}

static PyObject *
slice_repr(PySliceObject *r)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
			(*env)->CallObjectMethod(env,
					JyNI_JythonPyObject_FromPyObject((PyObject*) r),
					pySliceToString));

//	PyObject *s, *comma;
//
//	s = PyString_FromString("slice(");
//	comma = PyString_FromString(", ");
//	PyString_ConcatAndDel(&s, PyObject_Repr(r->start));
//	PyString_Concat(&s, comma);
//	PyString_ConcatAndDel(&s, PyObject_Repr(r->stop));
//	PyString_Concat(&s, comma);
//	PyString_ConcatAndDel(&s, PyObject_Repr(r->step));
//	PyString_ConcatAndDel(&s, PyString_FromString(")"));
//	Py_DECREF(comma);
//	return s;
}

//static PyMemberDef slice_members[] = {
//	{"start", T_OBJECT, offsetof(PySliceObject, start), READONLY},
//	{"stop", T_OBJECT, offsetof(PySliceObject, stop), READONLY},
//	{"step", T_OBJECT, offsetof(PySliceObject, step), READONLY},
//	{0}
//};

static PyObject *
slice_get_start(PySliceObject *m, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
			(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) m), pySliceGetStart));
}

static PyObject *
slice_get_stop(PySliceObject *m, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
			(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) m), pySliceGetStop));
}

static PyObject *
slice_get_step(PySliceObject *m, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
			(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) m), pySliceGetStep));
}

static PyGetSetDef slice_getsets [] = {
	{"start",  (getter)slice_get_start,  NULL, NULL},
	{"stop", (getter)slice_get_stop, NULL, NULL},
	{"step", (getter)slice_get_step, NULL, NULL},
	{0}
};

static PyObject*
slice_indices(PySliceObject* self, PyObject* len)
{
	Py_ssize_t ilen, start, stop, step, slicelength;

	ilen = PyNumber_AsSsize_t(len, PyExc_OverflowError);

	if (ilen == -1 && PyErr_Occurred()) {
		return NULL;
	}

	if (PySlice_GetIndicesEx(self, ilen, &start, &stop,
							 &step, &slicelength) < 0) {
		return NULL;
	}

	return Py_BuildValue("(nnn)", start, stop, step);
}

PyDoc_STRVAR(slice_indices_doc,
"S.indices(len) -> (start, stop, stride)\n\
\n\
Assuming a sequence of length len, calculate the start and stop\n\
indices, and the stride length of the extended slice described by\n\
S. Out of bounds indices are clipped in a manner consistent with the\n\
handling of normal slices.");

static PyObject *
slice_reduce(PySliceObject* self)
{
	env(NULL);
	return Py_BuildValue("O(OOO)", Py_TYPE(self), //self->start, self->stop, self->step);
			JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
					JyNI_JythonPyObject_FromPyObject((PyObject*) self), pySliceGetStart)),
			JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
					JyNI_JythonPyObject_FromPyObject((PyObject*) self), pySliceGetStop)),
			JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
					JyNI_JythonPyObject_FromPyObject((PyObject*) self), pySliceGetStep)));
}

PyDoc_STRVAR(reduce_doc, "Return state information for pickling.");

static PyMethodDef slice_methods[] = {
	{"indices",		 (PyCFunction)slice_indices,
	 METH_O,			slice_indices_doc},
	{"__reduce__",	  (PyCFunction)slice_reduce,
	 METH_NOARGS,	   reduce_doc},
	{NULL, NULL}
};

static int
slice_compare(PySliceObject *v, PySliceObject *w)
{
	env(-2);
	return (*env)->CallIntMethod(env, JyNIClass, JyNISlice_compare,
			JyNI_JythonPyObject_FromPyObject((PyObject*) v),
			JyNI_JythonPyObject_FromPyObject((PyObject*) w));
//	int result = 0;
//
//	if (v == w)
//		return 0;
//
//	if (PyObject_Cmp(v->start, w->start, &result) < 0)
//		return -2;
//	if (result != 0)
//		return result;
//	if (PyObject_Cmp(v->stop, w->stop, &result) < 0)
//		return -2;
//	if (result != 0)
//		return result;
//	if (PyObject_Cmp(v->step, w->step, &result) < 0)
//		return -2;
//	return result;
}

static long
slice_hash(PySliceObject *v)
{
	PyErr_SetString(PyExc_TypeError, "unhashable type");
	return -1L;
}

PyTypeObject PySlice_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"slice",                    /* Name of this type */
	sizeof(PySliceObject),      /* Basic object size */
	0,                          /* Item size for varobject */
	(destructor)slice_dealloc,                /* tp_dealloc */
	0,                                        /* tp_print */
	0,                                        /* tp_getattr */
	0,                                        /* tp_setattr */
	(cmpfunc)slice_compare,                   /* tp_compare */
	(reprfunc)slice_repr,                     /* tp_repr */
	0,                                        /* tp_as_number */
	0,                                        /* tp_as_sequence */
	0,                                        /* tp_as_mapping */
	(hashfunc)slice_hash,                     /* tp_hash */
	0,                                        /* tp_call */
	0,                                        /* tp_str */
	PyObject_GenericGetAttr,                  /* tp_getattro */
	0,                                        /* tp_setattro */
	0,                                        /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                       /* tp_flags */
	slice_doc,                                /* tp_doc */
	0,                                        /* tp_traverse */
	0,                                        /* tp_clear */
	0,                                        /* tp_richcompare */
	0,                                        /* tp_weaklistoffset */
	0,                                        /* tp_iter */
	0,                                        /* tp_iternext */
	slice_methods,                            /* tp_methods */
	0,//slice_members,                        /* tp_members */
	slice_getsets,                            /* tp_getset */
	0,                                        /* tp_base */
	0,                                        /* tp_dict */
	0,                                        /* tp_descr_get */
	0,                                        /* tp_descr_set */
	0,                                        /* tp_dictoffset */
	0,                                        /* tp_init */
	0,                                        /* tp_alloc */
	slice_new,                                /* tp_new */
};
