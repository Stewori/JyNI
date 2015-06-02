/*
 * DemoExtensionmodule.c
 *
 *  Created on: 09.03.2013, 01:18:32
 *	  Author: Stefan Richthofer
 */

#include <Python.h>

PyObject *
hello_world(PyObject *self, PyObject *args)
{
	puts("Hello World!");

	//If nothing shall be returned, do:
	//Py_INCREF(Py_None);
	//return Py_None;

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
	if (PyTuple_GET_SIZE(args) < 2) return PyString_FromString("");
	char* str1 = PyString_AS_STRING(PyTuple_GET_ITEM(args, 0));
	char* str2 = PyString_AS_STRING(PyTuple_GET_ITEM(args, PyTuple_GET_SIZE(args)-1));
	char er[strlen(str1)+strlen(str2)+1];
	strcpy(er, str1);
	strcat(er, str2);
	return PyString_FromString(er);
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

PyObject*
keywordTest(PyObject* self, PyObject* args, PyObject* kw)
{
	puts("kewordTest called:");
	//if (kw != NULL) puts(kw->ob_type->tp_name);
	//else puts("keywords is NULL");
	if (kw != NULL)
	{
		//printf("Number of args: %i\n", PyTuple_GET_SIZE(args));
		puts(PyString_AS_STRING(PyTuple_GET_ITEM(args, 0)));
		puts(PyString_AS_STRING(PyTuple_GET_ITEM(args, 1)));

		//printf("Number of kw: %i\n", PyDict_Size(kw));

		PyObject* ar = PyDict_GetItemString(kw, "right");
		if (ar != NULL) puts(PyString_AS_STRING(ar));
	}

	Py_RETURN_NONE;
}

PyObject*
longTests(PyObject* self, PyObject* args, PyObject* kw)
{
	long int l;
	if (!PyArg_ParseTuple(args, "l", &l))
		return NULL;
	PyObject* pl = PyLong_FromLong(l);
	printf("l: %i\n", l);
	PyObject* pls = _PyLong_Format(pl, 10, 0, 0);
	printf("long to string: %u\n", pls);
	puts(PyString_AS_STRING(pls));

	unsigned long t = 10000000000000000002;
	PyObject* pt = PyLong_FromUnsignedLong(t);
	PyObject* pts = _PyLong_Format(pt, 10, 0, 0);
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
	int i;
	if (!PyArg_ParseTuple(args, "O!i", &PySet_Type, &set, &i)) return NULL;
	//printf("pop %i elements from the set...\n", i);
	int j;
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

PyMethodDef DemoExtensionMethods[] = {
	{"hello_world", hello_world, METH_NOARGS, "Hello World method."},
	{"longTests", longTests, METH_VARARGS, "Prints out some test-data about PyLong."},
	{"listReadTest", listReadTest, METH_VARARGS, "Prints out the strings from a three-string sequence."},
	{"listModifyTest", listModifyTest, METH_VARARGS, "Modifies a list."},
	{"setTest", setTest, METH_VARARGS, "Performs some tests on a given PySet object."},
	{"setPopTest", setPopTest, METH_VARARGS, "Pops the given number of elements from the set."},
	{"printInt", printInt, METH_VARARGS, "Prints out the int and returns nothing."},
	{"printString", printString, METH_VARARGS, "Prints out the string and returns nothing."},
	{"intSquare", intSquare, METH_VARARGS, "Returns the square of the given int."},
	{"argCountToString", argCountToString, METH_VARARGS, "Returns number of arguments as string."},
	{"concatFirstWithLastString", concatFirstWithLastString, METH_VARARGS, "Concatenates first with last element. Returns empty string, if less than two args are available."},
	{"keywordTest", keywordTest, METH_VARARGS | METH_KEYWORDS, "Tests working with keywords."},
	{"exceptionTest", exceptionTest, METH_NOARGS, "Raise an exception to test JyNI's exception support."},
	{"unicodeTest", unicodeTest, METH_VARARGS, "Test JyNI's unicode support by converting forth and back."},
	{NULL, NULL, 0, NULL}		/* Sentinel */
};

PyMODINIT_FUNC
initDemoExtension(void)
{
	(void) Py_InitModule3("DemoExtension", DemoExtensionMethods, "This is a pure demo extension.");
}
