JyNI
====

Enables Jython to load native CPython extensions.


Table of contents
---------------------------------------------

1. What is JyNI?
2. Current state
3. Building and testing
4. Roadmap
5. Binary compatibility
6. Summary of changes to Python-code
7. Copyright notice

---------------------------------------------




1. What is JyNI?
----------------

One well known current drawback of Jython is that it does not support native extensions written for CPython like NumPy and Scipy.
Since most scientific Python-code fundamentally depends on exactly such native extensions, it usually cannot be run with Jython.
JyNI aims to close this gap. It is a layer that enables Jython-users to 
load native CPython-extensions and access them from Jython
the same way as they would do in CPython. In order to leverage
the JyNI functionality, you just have to put it on the
java-classpath when Jython is launched.
It neither requires you to recompile the extension-code, nor to build
a customized Jython-fork. That means, it is binary compatible with existing extension-builds.

At the time when this is written, JyNI does not fully implement
the Python C-API and we are in fact just capable to load and run simple examples
that only invole most basic builtin-types.

The concept is rather complete though and our goal is to
provide the C-API needed to load NumPy as soon as possible.
After that we will focus on SciPy and others.




2. Current state
----------------

We are currently able to load a C-extension into Jython, call methods and access attributes,
provided that the extension uses only CPython-API we have already implemented.

The parse- and build-functions PyArg_ParseTuple(), PyArg_ParseTupleAndKeywords(), PyArg_Parse(), Py_BuildValue and related only work with format-strings as long as the corresponding builtin-types
are already supported. Alternatively, supported builtin types can be accessed via their c-api-functions
and even returned to Jython. Keywords work as well.


Builtin types already supported:

PyDict, PyTuple, PyString, PyInteger, PyLong, PyFloat, PyBoolean, PyList, PyComplex, PyModule, PySlice, PyCFunction, PyCapsule are almost fully supported.
PyType is supported as far as currently needed and possible.
Exception-types are conceptually supported, but are currently not usable due to bugs.

More coming soon.


JyNI has only been tested on linux mint debian edition 64 bit yet.
Theoretically, the current version should be buildable for systems with shlib.
We have not yet developed auto-tools-scripts.
So building/installing/testing JyNI can only be done by hand at the moment.
Section 3 contains a brief description of the necessary steps.




3. Building and testing
-----------------------

Easiest way to build JyNI is to setup C-, and Java-projects with an IDE of choice.
3 Projects are needed: JyNI-C, JyNI-Java and JyNI-Loader (and optionally PythonExtension).


*C-section*

JyNI-C and JyNI-Loader are C-projects and must be enabled to include some header
directories via gcc-option -I.

For JyNI-Loader:
- JNI
(usually located in /usr/lib/jvm/java-[version]-openjdk-[architecture]/include)
- "include"-subfolder from JyNI-Java

For JyNI-C:
- JNI (like above)
- its own "include"-subfolder and all subfolders within

Don't forget to use the fPIC-option.
Finally link JyNI-C and JyNI-Loader to following dynamic libraries

- libJyNI.so (JyNI-C)
- libJyNI-Loader.so (JyNI-Loader)

Linking is said to require -ldl, though I observed it to work without.


*Java-Section*

JyNI-Java needs to have Jython 2.7 beta on its build-time classpath.
We haven't yet implemented a smart way for JyNI-Java to find
libJyNI.so and libJyNI-Loader.so .
As a temporary hack, please modify the lines in the static initializer
of JyNI.java to contain the correct paths:

  System.load("[path to JyNI-Loader build-dir]/libJyNI-Loader.so");
  initJyNI("[path to JyNI-C build-dir]/libJyNI.so");

Put the resulting class-files into JyNI.jar and also include
the JyNI-Java-subfolder "META-INF".


*Test-Example*

"PythonExtension" is a CPython-extension that demonstrates the
already supported features of JyNI. It can be build like an
ordinary Python-extension as described in the tutorial
http://docs.python.org/2/extending/extending.html

Test.py demonstrates the use of PythonExtension from Python-side.
It should run perfectly with either CPython 2.7.x or Jython+JyNI.
To run it with Jython use the newest release of version 2.7
(currently beta) and be sure to have JyNI.jar on the run-time classpath.




4. Roadmap
----------

Support remaining crucial builtin types (we plan to implement them in the following order):
- PyUnicode
- PySet

Further steps:
- fix exception, error and warning support
- implement parameter access and object calls more cleanly
- fix garbage collection
- fix pydoc interoperation
- focus on numpy-specific needs
- test and support on other platforms
- provide autotools-support




5. Binary compatibility
-----------------------

CPython-extensions must be compiled with the flag WITH_PYMALLOC activated. Otherwise they may not use
the macros PyObject_MALLOC, PyObject_REALLOC and PyObject_FREE directly.
If they do, JyNI will most likely produce segmantation faults. This is because without the flag,
these macros directly map to the systems malloc-function family. In that case JyNI would not
be able to prepare the JyObject-header in every case.
However, it would be easy to recompile such extensions for JyNI. Simply add the WITH_PYMALLOC-definition
at compile-time or replace the macros by their corresponding function-calls.

In general, the less an extension hacks with CPython-specific internals, the greater the chance it will
run with JyNI. Especially allocation/deallocation should not be done by hand, since JyNI must be able to
setup the JyObject-headers.

We hope, extensions that suffer from this issue are very rare or not existent.




6. Summary of changes to Python-code
------------------------------------

Briefly speaking, we took the python files crucial for loading native C-extensions and modified them to perform the explained purpose of JyNI.
Mainly we changed alloc and dealloc behaviour of PyObjects consequently, also when occuring inline like in stringobject.c or intobject.c. The new allocation behaviour adds a header called JyObject before the PyObject header or even in front of PyGC_Head if present. This header contains information that allows to wrap or mirror a corresponding Jython jobject in a seamless way.

Mirror mode is used for builtin types that allow access to their data-structure via macros. To keep these accesses valid, the data structure mirrors the actual Jython jobject. Syncing is not an issue for immutable objects and can be done initially for each object. One mutable builtin object needing mirror-mode is PyList. We perform sync from java by inserting a special java.util.List implementation as a backend into the corresponding Jython PyList.
A bit more tricky is PyByteArray, which also features a direct-access-macro, namely PyByteArray_AS_STRING.
However a fix for this has low priority, as this macro is not used by NumPy. Nevertheless, we can say a bit
about this issue. As PyByteArray in Jython uses a primitive byte[]-array as backend, we can't replace the backend by a subclass. Our most promising idea to solve the issue anyway, is to insert a custom implementation of SequenceIndexDelegate as delegator (PyByteArray is a subclass of PySequence and thus offers a delegator-field, which allows to customize the indexed access-behaviour).

Builtins that don't provide access via macros can be wrappered. That means, the original CPython data structure is not used and not even allocated. All functions of such objects are rewritten to delegate their calls to Jython.
However, also in these cases, we usually don't rewrite everything. Functions that access the data only via other functions are mostly kept unchanged.




7. Copyright notice
-------------------

Copyright of Python and Jython:
Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
2011, 2012, 2013 Python Software Foundation.  All rights reserved.

Copyright of JyNI:
Copyright (c) 2013 Stefan Richthofer.  All rights reserved.
