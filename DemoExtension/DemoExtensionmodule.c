/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
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
 *
 *
 *
 * DemoExtensionmodule.c
 *
 *  Created on: 09.03.2013, 01:18:32
 *	  Author: Stefan Richthofer
 */

#include <Python.h>

#ifndef MS_WINDOWS
#define _alloca alloca
#endif

PyObject *
hello_world(PyObject *self, PyObject *args)
{
	puts("Hello World!");

	//If nothing shall be returned, do:
	//Py_INCREF(Py_None);
	//return Py_None;
	//PyThreadState_Get();

	Py_RETURN_NONE;
}

PyObject *
argCountToString(PyObject *self, PyObject *args)
{
	char str[6];
	PyOS_snprintf(str, 5, "(%u)", PyTuple_GET_SIZE(args));
	return PyString_FromString(str);
}

PyObject *
concatFirstWithLastString(PyObject *self, PyObject *args)
{
	//char* str1, str2, er;
	//if (PyTuple_GET_SIZE(args) < 2) return PyString_FromString("");
	char* str1 = PyString_AS_STRING(PyTuple_GET_ITEM(args, 0));
	char* str2 = PyString_AS_STRING(PyTuple_GET_ITEM(args, PyTuple_GET_SIZE(args)-1));
	//char er[500];// strlen(str1) + strlen(str2) + 1];
	char* er;// = _alloca(strlen(str1) + strlen(str2) + 1);
	PyObject* res;
	if (PyTuple_GET_SIZE(args) < 2) return PyString_FromString("");
	er = _alloca(strlen(str1) + strlen(str2) + 1);
	//char er[strlen(str1) + strlen(str2) + 1];
	strcpy(er, str1);
	strcat(er, str2);
	res = PyString_FromString(er);
	//free(er);
	return res;
}

PyObject *
printString(PyObject *self, PyObject *args)
{
	puts(PyString_AS_STRING(PyTuple_GET_ITEM(args, 0)));
	Py_RETURN_NONE;
}

PyObject *
printInt(PyObject *self, PyObject *args)
{
	printf("%i\n", PyInt_AS_LONG(PyTuple_GET_ITEM(args, 0)));
	Py_RETURN_NONE;
}

PyObject *
intSquare(PyObject *self, PyObject *args)
{
	//long l = PyInt_AS_LONG(PyTuple_GET_ITEM(args, 0));
	long int l;
	if (!PyArg_ParseTuple(args, "l", &l))
		return NULL;

	//printf("l: %i\n", l);
	return PyInt_FromLong(l*l);
}

PyObject *
intSquare1(PyObject *self, PyObject *integer)
{
	long int l = PyInt_AS_LONG(integer);
	return PyInt_FromLong(l*l);
}

PyObject *
refcount(PyObject *self, PyObject *object)
{
	return PyInt_FromLong(object->ob_refcnt);
}

PyObject*
keywordTest(PyObject* self, PyObject* args, PyObject* kw)
{
	puts("kewordTest called:");
	//if (kw != NULL) puts(kw->ob_type->tp_name);
	//else puts("keywords is NULL");
	if (kw != NULL)
	{
		PyObject* ar;
		//printf("Number of args: %i\n", PyTuple_GET_SIZE(args));
		puts(PyString_AS_STRING(PyTuple_GET_ITEM(args, 0)));
		puts(PyString_AS_STRING(PyTuple_GET_ITEM(args, 1)));

		//printf("Number of kw: %i\n", PyDict_Size(kw));

		ar = PyDict_GetItemString(kw, "right");
		if (ar != NULL) puts(PyString_AS_STRING(ar));
	}

	Py_RETURN_NONE;
}

PyObject*
longTests(PyObject* self, PyObject* args, PyObject* kw)
{
	long int l; unsigned long t;
	PyObject *pl, *pls, *pt, *pts;
	if (!PyArg_ParseTuple(args, "l", &l))
		return NULL;
	pl = PyLong_FromLong(l);
	printf("l: %i\n", l);
	pls = _PyLong_Format(pl, 10, 0, 0);
	printf("long to string: %u\n", pls);
	puts(PyString_AS_STRING(pls));

	t = 10000000000000000002;
	pt = PyLong_FromUnsignedLong(t);
	pts = _PyLong_Format(pt, 10, 0, 0);
	puts(PyString_AS_STRING(pts));
	Py_RETURN_NONE;
}

PyObject*
listReadTest(PyObject* self, PyObject* args)
{
	char* a, *b, *c;
	if (!PyArg_ParseTuple(args, "(sss)", &a, &b, &c))
		return NULL;
	puts("a:");
	puts(a);
	puts ("b:");
	puts(b);
	puts ("c:");
	puts(c);
	Py_RETURN_NONE;
}

PyObject*
listModifyTest(PyObject* self, PyObject* args)
{
	char* modText = "natively modified";
	char* appendText = "natively appended";
	PyObject* list;
	int i;
	if (!PyArg_ParseTuple(args, "O!i", &PyList_Type, &list, &i)) return NULL;
	Py_XDECREF(PyList_GET_ITEM(list, i));
	PyList_SET_ITEM(list, i, PyString_FromString(modText));
	//PyList_Append(list, PyString_FromString(appendText));
	Py_RETURN_NONE;
}

PyObject*
listSetIndex(PyObject* self, PyObject* args)
{
	PyObject* list = PyTuple_GET_ITEM(args, 0);
	PyObject* index = PyTuple_GET_ITEM(args, 1);
	PyObject* value = PyTuple_GET_ITEM(args, 2);
	long i = PyInt_AS_LONG(index);
	Py_XINCREF(value);

	Py_XDECREF(PyList_GET_ITEM(list, i));
	PyList_SET_ITEM(list, i, value);

	//PyList_SetItem(list, i, value);

	Py_RETURN_NONE;
}

PyObject*
setTest(PyObject* self, PyObject* args)
{
	char* plumText = "plum";
	PyObject* set;
	if (!PyArg_ParseTuple(args, "O!", &PySet_Type, &set)) return NULL;
	puts("Set parsed!");
	printf("length: %i\n", PySet_GET_SIZE(set));
	PySet_Add(set, PyString_FromString(plumText));
	printf("length after add: %i\n", PySet_GET_SIZE(set));
	//PyList_SET_ITEM(list, i, PyString_FromString(modText));
	//PyList_Append(list, PyString_FromString(appendText));
	Py_RETURN_NONE;
}

PyObject*
setPopTest(PyObject* self, PyObject* args)
{
	PyObject* set;
	int i, j;
	if (!PyArg_ParseTuple(args, "O!i", &PySet_Type, &set, &i)) return NULL;
	//printf("pop %i elements from the set...\n", i);
	for (j = 0; j < i; ++j)
	{
		PyObject* pop = PySet_Pop(set);
//		if (pop != NULL)
//			puts(PyString_AS_STRING(pop));
//		else
//			puts("No more elements in the set!");
	}
	Py_RETURN_NONE;
}

PyObject*
exceptionTest(PyObject* self, PyObject* args)
{
//	puts("exceptionTest:");
//	if (PyErr_Occurred()) puts("Error present");
//	else  puts("no Error present");
//	if (Py_IgnoreEnvironmentFlag) puts("ignore");
//	else puts("not ignore");
	PyErr_SetString(PyExc_SystemError, "This is a test exception message for JyNI.");
	//Py_RETURN_NONE;
	return NULL;
}

PyObject*
unicodeTest(PyObject* self, PyObject* args)
{
	PyObject* uni = PyTuple_GET_ITEM(args, 0);
//	puts("unicodeTest");
	//"str"
//	puts(uni->ob_type->tp_name);
	//puts(PyString_AS_STRING(PyObject_Repr(uni)));
//	puts(PyString_AS_STRING(uni->ob_type->tp_repr(uni)));

	Py_UNICODE* unc = PyUnicode_AS_UNICODE(uni);
	//PyObject* ret = PyUnicode_FromUnicode(unc, PyUnicode_GET_SIZE(uni));
	//Py_RETURN_NONE;
	return PyUnicode_FromUnicode(unc, PyUnicode_GET_SIZE(uni));
}

PyObject*
createListSelfContaining(PyObject* self, PyObject* args)
{
	char* text1 = "element1";
	char* text2 = "element2";
	PyObject* list = PyList_New(3);
	PyList_SET_ITEM(list, 0, PyString_FromString(text1));
	PyList_SET_ITEM(list, 1, PyString_FromString(text2));
	PyList_SET_ITEM(list, 2, list);
	return list;
}

PyObject*
createTupleSelfContaining(PyObject* self, PyObject* args)
{
	char* text1 = "tp1";
	char* text2 = "tp2";
	char* textl1 = "lst1";
	PyObject* tuple = PyTuple_New(3);
	PyObject* list = PyList_New(2);
	//In Python it is not allowed for tuples to directly self-contain.
	//So we construct the loop indirectly via a list.
	PyTuple_SET_ITEM(tuple, 0, PyString_FromString(text1));
	PyTuple_SET_ITEM(tuple, 1, PyString_FromString(text2));
	PyTuple_SET_ITEM(tuple, 2, list);
	PyList_SET_ITEM(list, 0, PyString_FromString(textl1));
	PyList_SET_ITEM(list, 1, tuple);
	//printf("createTupleSelfContaining %llu  %llu\n", list, PyList_GET_ITEM(list, 0));
	return tuple;
}

PyObject*
booleanToInt(PyObject* self, PyObject* boolean)
{
	if (boolean == Py_True) return PyInt_FromLong(1);
	else if (boolean == Py_False) return PyInt_FromLong(0);
	else Py_RETURN_NONE;
}

PyObject*
intToBoolean(PyObject* self, PyObject* integer)
{
	long int l = PyInt_AS_LONG(integer);
	//For debugging reasons we make a strict 0/1 conversion here.
	//(rather than 1 for non-NULL)
	if (l == 1) Py_RETURN_TRUE;
	else if (l == 0) Py_RETURN_FALSE;
	else Py_RETURN_NONE;
}

PyObject*
nativeDictGet(PyObject* self, PyObject* args)
{
	PyObject* dict = PyTuple_GET_ITEM(args, 0);
	PyObject* key = PyTuple_GET_ITEM(args, 1);
	return PyDict_GetItem(dict, key);
}

PyObject*
nativeDictCreate(PyObject* self, PyObject* args)
{
	PyObject* dct = PyDict_New();
	Py_ssize_t ds = PyDict_Size(dct);
	return PyInt_FromLong(ds);
}

PyObject*
newstyleCheck(PyObject* self, PyObject* args)
{
	PyObject* nobj = PyTuple_GET_ITEM(args, 0);
//	if (nobj && PyString_Check(nobj))
//		puts(PyString_AS_STRING(nobj));
	if (nobj) Py_RETURN_TRUE;
	else if (nobj == NULL) Py_RETURN_FALSE;
	else Py_RETURN_NONE;
}

PyObject*
newstyleCheckSubtype(PyObject* self, PyObject* args)
{
	PyObject* nobj = PyTuple_GET_ITEM(args, 0);
	PyTypeObject* supertype = (PyTypeObject*) PyTuple_GET_ITEM(args, 1);
	if (nobj && supertype)
	{
		if (PyType_IsSubtype(Py_TYPE(nobj), supertype)) Py_RETURN_TRUE;
		else Py_RETURN_FALSE;
	} else
		Py_RETURN_NONE;
}

PyObject*
importAPIandMethodDescrTest(PyObject* self, PyObject* args)
{
//	puts(__FUNCTION__);
	PyObject *module, *importFunction, *lock, *__enter__;
	{
		PyObject* moduleDict = PyImport_GetModuleDict();
		PyObject* builtinModule = PyDict_GetItemString(moduleDict, "__builtin__");
		PyObject* attrStr = PyString_FromString("__import__");
		PyTypeObject* tp = Py_TYPE(builtinModule);  //Borrowed ref, so don't decref!
		Py_XDECREF(moduleDict);
	//	puts(tp->tp_name);
	//	puts(PyString_AS_STRING(PyObject_Str(builtinModule)));
		if (strcmp(tp->tp_name, "module") != 0) return PyInt_FromLong(-__LINE__);
		if (strcmp(PyString_AS_STRING(PyObject_Str(builtinModule)),
				"<module '__builtin__' (built-in)>") != 0) return PyInt_FromLong(-__LINE__);
	//	PyObject* range = PyObject_GetAttrString(builtinModule, "range");
		if (tp->tp_getattro) {
			importFunction = tp->tp_getattro(builtinModule, attrStr);
		}
		Py_XDECREF(builtinModule);
		Py_XDECREF(attrStr);
	}
//	puts(PyString_AS_STRING(PyObject_Str(importFunction)));
//	puts(PyString_AS_STRING(PyObject_Str(Py_TYPE(importFunction))));
	if (strcmp(PyString_AS_STRING(PyObject_Str(importFunction)),
			"<built-in function __import__>") != 0) return PyInt_FromLong(-__LINE__);
	if (strcmp(PyString_AS_STRING(PyObject_Str(Py_TYPE(importFunction))),
			"<type 'builtin_function_or_method'>") != 0) return PyInt_FromLong(-__LINE__);

	{
		PyObject* threadingname = PyString_FromString("threading");
		PyObject* py_level = PyInt_FromLong(1);
		PyObject* global_dict = PyDict_New();
		PyObject* empty_dict = PyDict_New();
		PyObject* list = PyList_New(0);
		module = PyObject_CallFunctionObjArgs(importFunction,
			threadingname, global_dict, empty_dict, list, py_level, NULL);
		Py_XDECREF(threadingname);
		Py_XDECREF(py_level);
		Py_XDECREF(global_dict);
		Py_XDECREF(empty_dict);
		Py_XDECREF(list);
	}
//	puts(PyString_AS_STRING(PyObject_Str(module)));
	if (strncmp(PyString_AS_STRING(PyObject_Str(module)),
			"<module 'threading'", 19) != 0) return PyInt_FromLong(-__LINE__);

	{
		PyObject* lockname = PyString_FromString("Lock");
		PyObject* locktp = NULL;
		PyTypeObject* tpl = Py_TYPE(module); //Borrowed ref, so don't decref!
	//	puts(PyString_AS_STRING(PyObject_Str(tpl)));
		if (!locktp && tpl->tp_getattro) {
	//		printf("%s %i\n", __FUNCTION__, __LINE__);
			locktp = tpl->tp_getattro(module, lockname);
		}
		if (!locktp && tpl->tp_getattr) {
	//		printf("%s %i\n", __FUNCTION__, __LINE__);
			locktp = tpl->tp_getattr(module, PyString_AS_STRING(lockname));
		}
		if (!locktp)
			locktp = PyObject_GetAttr(module, lockname);
		Py_XDECREF(lockname);
		lock = PyObject_Call(locktp, PyTuple_New(0), NULL);
	//	puts("getting __enter__...");
		__enter__ = _PyType_Lookup(locktp, PyString_FromString("__enter__"));
		Py_XDECREF(locktp);
		Py_XDECREF(module);
	//	puts(PyString_AS_STRING(PyObject_Str(locktp)));
	//	puts(PyString_AS_STRING(PyObject_Str(Py_TYPE(lock))));
	//	puts(PyString_AS_STRING(PyObject_Str(lock)));
		if (strcmp(PyString_AS_STRING(PyObject_Str(locktp)),
					"<type '_threading.Lock'>") != 0) return PyInt_FromLong(-__LINE__);
	}
	if (strcmp(PyString_AS_STRING(PyObject_Str(Py_TYPE(lock))),
				"<type '_threading.Lock'>") != 0) return PyInt_FromLong(-__LINE__);
	if (strcmp(PyString_AS_STRING(PyObject_Str(lock)),
				"<_threading.Lock owner=None locked=False>") != 0)
		return PyInt_FromLong(-__LINE__);

	//PyObject* __enter__ = PyObject_GetAttrString(lock, "__enter__");
//	puts("got __enter__:");
//	puts(PyString_AS_STRING(PyObject_Str(__enter__)));
	if (strcmp(PyString_AS_STRING(PyObject_Str(__enter__)),
					"<method '__enter__' of 'Lock' objects>") != 0)
		return PyInt_FromLong(-__LINE__);
	if (__enter__) {
//		puts("__enter__ not NULL");
		descrgetfunc f = Py_TYPE(__enter__)->tp_descr_get;
		if (!f) {
			//puts("descrget is NULL");
			//Py_INCREF(__enter__);
			return PyInt_FromLong(-__LINE__);
		} //else {
//			puts("descrget is non-NULL");
//			//__enter__ = f(__enter__, obj, (PyObject *)tp);
//		}
	} else return PyInt_FromLong(-__LINE__);
//	puts(Py_TYPE(__enter__)->tp_name);
	if (strcmp(Py_TYPE(__enter__)->tp_name, "method_descriptor") != 0)
		return PyInt_FromLong(-__LINE__);
//	if (PyType_HasFeature(Py_TYPE(__enter__), Py_TPFLAGS_HEAPTYPE))
//		puts("heaptype");
	{
		PyObject *argSelf, *p7;
		argSelf = PyTuple_New(1);
		PyTuple_SetItem(argSelf, 0, lock);
		p7 = PyObject_Call(__enter__, argSelf, NULL);
	//	puts(PyString_AS_STRING(PyObject_Str(p7)));
		if (strcmp(PyString_AS_STRING(PyObject_Str(p7)),
				"<_threading.Lock owner='MainThread' locked=True>") != 0)
			return PyInt_FromLong(-__LINE__);
	//	puts("decref...");
		Py_XDECREF(p7);
		Py_XDECREF(argSelf);
	}
	Py_XDECREF(importFunction);
	Py_XDECREF(lock);
	Py_XDECREF(__enter__);
//	puts("conversion test done");
	return PyInt_FromLong(0);
}

PyMethodDef DemoExtensionMethods[] = {
	{"hello_world", hello_world, METH_NOARGS, "Hello World method."},
	{"longTests", longTests, METH_VARARGS, "Prints out some test-data about PyLong."},
	{"listReadTest", listReadTest, METH_VARARGS, "Prints out the strings from a three-string sequence."},
	{"listModifyTest", listModifyTest, METH_VARARGS, "Modifies a list."},
	{"listSetIndex", listSetIndex, METH_VARARGS, "Sets an element of a list to a new value."},
	{"setTest", setTest, METH_VARARGS, "Performs some tests on a given PySet object."},
	{"setPopTest", setPopTest, METH_VARARGS, "Pops the given number of elements from the set."},
	{"printInt", printInt, METH_VARARGS, "Prints out the int and returns nothing."},
	{"printString", printString, METH_VARARGS, "Prints out the string and returns nothing."},
	{"intSquare", intSquare, METH_VARARGS, "Returns the square of the given int."},
	{"intSquare1", intSquare1, METH_O, "Variant of intSquare using the METH_O flag."},
	{"argCountToString", argCountToString, METH_VARARGS, "Returns number of arguments as string."},
	{"concatFirstWithLastString", concatFirstWithLastString, METH_VARARGS, "Concatenates first with last element. Returns empty string, if less than two args are available."},
	{"keywordTest", keywordTest, METH_VARARGS | METH_KEYWORDS, "Tests working with keywords."},
	{"exceptionTest", exceptionTest, METH_NOARGS, "Raise an exception to test JyNI's exception support."},
	{"unicodeTest", unicodeTest, METH_VARARGS, "Test JyNI's unicode support by converting forth and back."},
	{"createListSelfContaining", createListSelfContaining, METH_NOARGS, "Natively create a self-containing list."},
	{"createTupleSelfContaining", createTupleSelfContaining, METH_NOARGS, "Natively create a self-containing tuple."},
	{"booleanToInt", booleanToInt, METH_O, "Converts True to one, False to zero, everything else to None."},
	{"intToBoolean", intToBoolean, METH_O, "Converts one to True, zero to False, everything else to None."},
	{"nativeDictGet", nativeDictGet, METH_VARARGS, "Looks up a key in a dict."},
	{"nativeDictCreate", nativeDictCreate, METH_NOARGS, "Creates a dict on native site."},
	{"newstyleCheck", newstyleCheck, METH_VARARGS, "Checks integrity of new-style instance conversion."},
	{"newstyleCheckSubtype", newstyleCheckSubtype, METH_VARARGS, "Checks subtype consistence new-style conversion."},
	{"refcount", refcount, METH_VARARGS, "Provides the current native refcount of the given object."},
	{"importAPIandMethodDescrTest", importAPIandMethodDescrTest, METH_NOARGS, "Tests some aspects of native import API."},
	{NULL, NULL, 0, NULL}		/* Sentinel */
};

PyMODINIT_FUNC
initDemoExtension(void)
{
	//puts("initDemoExtension...");
	//PyErr_Format(PyExc_ImportError, "test-error");
	(void)Py_InitModule3("DemoExtension", DemoExtensionMethods, "This is a pure demo extension.");
}
