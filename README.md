# JyNI – Jython Native Interface

If not yet done, make sure to visit our project homepage at [www.jyni.org](http://www.jyni.org).

![JyNI Logo](http://www.jyni.org/_static/JyNILogoShadow_s.png)


## Table of contents
---------------------------------------------

1. [What is JyNI?](#1-what-is-jyni)
2. [Current state](#2-current-state)
3. [Installing JyNI](#3-installing-jyni)
4. [Building and testing](#4-building-and-testing)
5. [Roadmap](#5-roadmap)
6. [Binary compatibility](#6-binary-compatibility)
7. [Summary of changes to Python code](#7-summary-of-changes-to-python-code)
8. [Copyright notice](#8-copyright-notice)
9. [License](#9-license)
10. [Contact](#10-contact)

---------------------------------------------




## 1. What is JyNI?

A current issue of Jython hindering its wider adoption is that it does not
support native extensions written for CPython like NumPy and SciPy. Since
most scientific Python code fundamentally depends on exactly these native
extensions, it usually cannot be run with Jython. JyNI aims to bridge this
gap. It is a layer allowing Jython users to load native CPython extensions
and access them from Jython the same way they would in CPython. In order to
enable JyNI, you only have to put it on the Java classpath when Jython is
launched. It neither requires recompiling the extension code, nor building
a customized Jython fork. That means it is binary-compatible with existing
extension builds.

As of this writing, JyNI does not fully implement the Python C API but it
does support some important extensions, most notably the core
functionalities of ctypes and NumPy.

We are constantly working to improve this support and our goal is to
provide the C API needed to use the scientific stack and other extensions
as soon as possible.

JyNI runs on Linux, OS-X and theoretically on various other POSIX systems
(not tested). From JyNI-alpha.5 onwards, it also runs on Windows.




## 2. Current state

We are currently able to load a C extension into Jython, call methods and access
attributes, provided that the extension uses only CPython API we have already
implemented.

As of JyNI-alpha.3 JyNI performs a method for garbage collection that bridges
Java's and Jython's mark-and-sweep-based approach and CPython's
reference-counting-based approach as used int the C extensions. JyNI's approach
is explained in detail in the publication
[Garbage Collection in JyNI - How to bridge Mark/Sweep and Reference Counting GC.,
Proceedings of the 8th European Conference on Python in Science (EuroSciPy 2015),
CoRR abs/1607.00825 (2016), arxiv.org/abs/1607.00825](https://arxiv.org/abs/1607.00825)

As of JyNI-alpha.4 JyNI supports new-style classes. Support for defining new types
via C API that extend existing built-in types is still limited.

As of JyNI-alpha.5 JyNI supports Windows and properly manages built-in extensions.
Most notably, `datetime` and `_winreg` are now built-in. When running on Windows,
JyNI enables support for the `mbcs` encoding in Jython.

Parse and build functions `PyArg_ParseTuple()`, `PyArg_ParseTupleAndKeywords()`,
`PyArg_Parse()`, `Py_BuildValue` and related work with format strings corresponding
to built-in types that are already supported. Alternatively, supported built-in types
can be accessed via their C API functions.


The following built-in types are already supported:

* Number types PyInt, PyLong, PyFloat, PyComplex
* Sequence types PyTuple, PyList, PySlice, PyString, PyUnicode
* Data structure types PyDict, PySet, PyFrozenSet, PyDictProxy
* Operational types PyModule, PyFunction, PyCode, PyFrame, PyFile (partly), PyCell (partly)
* OOP types PyClass, PyInstance, PyMethod, PyClassMethod, PyStaticMethod, PyProperty
* Singleton types PyNone, PyNotImplemented, PyEllipsis, PyBool
* Native types PyCFunction, PyCapsule, PyCObject
* Iterator types PySeqIter
* Natively defined custom types
* Exception types and instances
* PyType as static type or heap type
* Weak reference types _PyWeakref_RefType, _PyWeakref_ProxyType, _PyWeakref_CallableProxyType
* New-style classes/instances

Planned:

* Support for remaining iterator types, e.g. TupleIter, ListIter
* Buffer protocol
* Support for PyByteArray (along with buffer protocol)
* Improve support for PyFile


JyNI has been tested on

* Linux Mint Debian Edition (LMDE) (32 bit)
* Linux Mint 18 (64 bit)
* Mac OS-X (10.11)
* Windows 10 (64 bit)
* Windows 7 (64 bit)

It would presumably work also on other POSIX systems.
If you try it on further distributions, please consider reporting your results
(see [contact section](#10-contact)).




## 3. Installing JyNI

This section explains how to install JyNI using prebuilt binary files.
If you want to built JyNI yourself, skip to section [4. Building and testing](#4-building-and-testing).

* Download `JyNI.jar` and the binaries for your system.

* Make the binary files available to your JVM.
  - On Linux, OS-X and other systems using `libLD` to load binaries, place
    `libJyNI.so` and `libJyNI-loader.so`
    (OS-X: `libJyNI.dylib` and `libJyNI-loader.dylib`) somewhere on your `LD_LIBRARY_PATH`
    or tell the JVM via `-Djava.library.path` where to find them or place them on the
    Java classpath.
  - On Windows, `JyNI.dll` will be a folder containing `python27.dll`.
    This is due to technical reasons explained on the
    [JyNI GSoC-2017 blog](http://gsoc2017-jyni.blogspot.de/2017/08/major-milestone-achieved-jyni-builds.html).
    Treat the folder `JyNI.dll` as if it were the binary dll file itself.
    It (i.e. its parent directory) must be available on the Java classpath.

* Have `JyNI.jar` on the Java classpath when calling Jython. For details and examples
  see the following section [Running JyNI](#running-jyni).


### Running JyNI

To run Jython with JyNI support, call (from JyNI base dir)

```
java -cp jython.jar:build/JyNI.jar org.python.util.jython someFile.py
```

On Windows use `;` instead of `:` and `\` instead of `/`:

```
java -cp jython.jar;build\JyNI.jar org.python.util.jython someFile.py
```

Alternatively, one can use Jython's start script
(assuming you copied it to the JyNI base directory):

```
jython -J-cp build/JyNI.jar
```

To run it from a different place, you just need to adjust the paths for
JyNI and Jython in the above commands.


Note that

```
java -cp build/JyNI.jar -jar jython.jar
```

is NOT suitable to run Jython with JyNI support. It does not pass the
provided classpath to Jython.

To run tests and demos, see section [Test Example](#test-example)




## 4. Building and testing

* Download the sources from the latest release or clone the [github repository](https://github.com/Stewori/JyNI).

* Provide Java:

  Make sure that Java 7 JDK or newer is installed.

  As of 2.7-alpha.4 JyNI attempts to guess the JDK location from the
  JRE location. This requires the `java` and `javac` commands to be
  configured properly on the `path` variable and assumes that `java` links to
  the JRE bundled with the JDK.

  Alternatively, you can explicitly configure the `JAVA_HOME` variable to point
  to the JDK.

  On POSIX systems, JyNI also looks for the symlink `/usr/lib/jvm/default-java`,
  which is provided by some systems (e.g. Linux MINT) and points to the
  specific Java folder name composed of Java version and architecture.

  If the above attempts fail for some reason, you can still adjust the JyNI
  `makefile` directly: Open the `makefile` appropriate for your system
  (located in the JyNI base folder) and adjust the variable `JAVA_HOME` as
  desired.

* Provide Jython:

  You have two ways to provide Jython. Either you copy
  (on non-Windows you can also (sym)link) `jython.jar` into the JyNI base
  directory or you edit `makefile` and adjust the `JYTHON`-variable to point
  to `jython.jar`.

  To use JyNIDemo.sh or other other demonstration shell scripts/batch files
  you need to adjust Jython path in that file(s) respectively.

  Note that current JyNI requires
  [Jython 2.7.1](http://fwierzbicki.blogspot.de/2017/07/jython-271-final-released.html).

* If not yet done, install the dev-package of Python 2.7 if you build on
  Linux, OS-X or likewise or install CPython 2.7 if you build on Windows.
  JyNI only needs `pyconfig.h` from that package/installation.

  Alternatively, (if you know what you're doing) you can provide your own `pyconfig.h`.
  In that case you'll have to adjust the corresponding include-path in the makefile.

* Make sure that a C compiler is installed.

  On Linux, JyNI supports building with GCC or CLANG, on OS-X only CLANG is supported.

  To build JyNI on Windows, install
  [MSVC for Python 2.7](https://www.microsoft.com/en-us/download/details.aspx?id=44266).

* You need to have a proper implementation of the `make` command on your
  system `path`.

  On Windows, we use [GNU Make](https://www.gnu.org/software/make/), but CMake might
  work as well (not tested).

* Build JyNI:

  Open a terminal and enter the JyNI base directory.
  - for Linux/GCC type `make`
  - for Linux/CLANG type `make -f makefile.clang`
  - for OS-X type `make -f makefile.osx`
  - for Windows (64 bit) type `make -f makefile.win64`
  - for Windows (32 bit) type `make -f makefile.win32`

  Optionally run `make clean` or e.g. `make -f makefile.osx clean` respectively.
  The resulting binaries are placed in the subfolder named `build`.

* Make the binary files available to your JVM.
  - On Linux, OS-X and other systems that use `libLD` to load binaries, place
    `libJyNI.so` and `libJyNI-loader.so`
    (OS-X: `libJyNI.dylib` and `libJyNI-loader.dylib`)
    somewhere on your `LD_LIBRARY_PATH` or tell the JVM via `-Djava.library.path`
    where to find them or place them on the Java classpath.
  - On Windows, `JyNI.dll` will be a folder containing `python27.dll`. This has
    technical reasons explained on the [JyNI GSoC-2017 blog](http://gsoc2017-jyni.blogspot.de/2017/08/major-milestone-achieved-jyni-builds.html).
    Treat the folder `JyNI.dll` as if it were the binary dll file.
    It (i.e. its parent directory) must be available on the Java classpath.

* For running JyNI see section [Running JyNI](#running-jyni).
  To run tests read the section following section [Test Example](#test-example).


### Test Example

`DemoExtension` is a CPython extension that demonstrates some
basic features of JyNI. It can be built like an
ordinary CPython extension as described in the
[Python tutorial](http://docs.python.org/2/extending/extending.html).

Enter the folder `DemoExtension` and type `python setup.py build`.
On Windows make sure that [MSVC for Python 2.7](https://www.microsoft.com/en-us/download/details.aspx?id=44266)
is installed.
Optionally type `python setup.py clean`.

`JyNI-Demo/src/test_JyNI.py` runs tests for some basic JyNI features.
You can run it via
```
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/test_JyNI.py
```
or
`./JyNI_unittest.sh` / `JyNI_unittest.bat`
assuming that `jython.jar` is placed or (sym)linked in JyNI's directory.
Otherwise you need to fix the path of `jython.jar` in the command.

`JyNI-Demo/src/JyNIDemo.py` demonstrates the use of `DemoExtension` from
Python side. It should run perfectly with either CPython 2.7.x or
Jython+JyNI. To run it, type
```
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIDemo.py
```
You can alternatively run
`./JyNIDemo.sh` / `JyNIDemo.bat`.

To see a basic demonstration that JyNI is capable of using the original
`datetime` module (a built-in module as of JyNI-alpha.5), run
```
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIDatetimeTest.py
```

To see a demonstration of exception support (on native level), run
```
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIExceptionTest.py
```

To see a demonstration of Tkinter support, run
```
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNITkinterTest.py
```
or
`./JyNITkinterDemo.sh` / `JyNITkinterDemo.bat`.

For a demonstration of ctypes support, run
```
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIctypesTest.py
```
or
`./JyNIctypesDemo.sh` / `JyNIctypesDemo.bat`.




## 5. Roadmap

Further steps:
- improve NumPy support
- support buffer protocol
- support CFFI and SciPy
- support further extensions
- JyNI 3




## 6. Binary compatibility

CPython extensions must be compiled with the flag `WITH_PYMALLOC` activated
(which is actually the default anyway). Otherwise they must not use
the macros `PyObject_MALLOC`, `PyObject_REALLOC` and `PyObject_FREE` directly.
If they use these macros with `WITH_PYMALLOC` not activated, JyNI will most
likely produce segmentation faults. Without the flag these macros directly
map to the system's `malloc` function family. In that case JyNI would not be
able to prepare the `JyObject` header in every case.
However, it would be easy to recompile such extensions for JyNI. Simply add
the `WITH_PYMALLOC` definition at compile time or replace the macros by their
corresponding function calls.

In general, the less an extension hacks with CPython-specific internals,
the greater the chance it will run with JyNI. Especially
allocation/deallocation should not be done by hand, since JyNI must be able
to setup the `JyObject` headers.

We hope that most extensions are not affected by this issue.




## 7. Summary of changes to Python code

Briefly, we took the Python files crucial for loading native
C extensions and modified them to perform the explained purpose of JyNI.
Mainly we changed alloc and dealloc behavior of `PyObject`s consequently, also
when inlined like in `stringobject.c` or `intobject.c`. The new allocation
behavior adds a header called `JyObject` before the `PyObject` header or even in
front of `PyGC_Head` if present. This header contains information that allows to
wrap or mirror a corresponding Jython `jobject` in a seamless way.

Mirror mode is used for built-in types that allow access to their data structure
via macros. To keep these accesses valid, the data structure mirrors the actual
Jython `jobject`. Syncing is not an issue for immutable objects and can be done
initially for each object. One mutable built-in object needing mirror mode is
`PyList`. We perform sync from Java by inserting a special `java.util.List`
implementation as a backend into the corresponding Jython `PyList`.
A bit more tricky is `PyByteArray`, which also features a direct access macro,
namely `PyByteArray_AS_STRING`. However, a fix for this has low priority, as this
macro is not used by NumPy. Nevertheless, we can say a bit about this issue. As
`PyByteArray` in Jython uses a primitive `byte[]` array as backend, we cannot replace
the backend by a subclass. Our most promising idea to solve the issue anyway is
to insert a custom implementation of `SequenceIndexDelegate` as delegator.
`PyByteArray` is a subclass of `PySequence` and thus offers a delegator field
that allows to customize the indexed access behavior.

Built-ins not providing access via macros can be wrapped. The original
CPython data structure is not used and not even allocated. All functions of
such objects are rewritten to delegate their calls to Jython.
However, also in these cases, we usually do not rewrite everything. Functions
that access the data only via other functions are mostly kept unchanged.




## 8. Copyright notice

Copyright of Python and Jython:
Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
2011, 2012, 2013, 2014, 2015, 2016, 2017 Python Software Foundation.
All rights reserved.

Copyright of JyNI:
Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
All rights reserved.




## 9. License

The software in this package is distributed under the
GNU Lesser General Public License.

A copy of GNU Lesser General Public License (LGPL) is included in this
distribution in the files
[COPYING](https://github.com/Stewori/JyNI/blob/master/COPYING) and
[COPYING.LESSER](https://github.com/Stewori/JyNI/blob/master/COPYING.LESSER).
If you do not have the source code, it is available at:

[https://github.com/Stewori/JyNI](https://github.com/Stewori/JyNI)


JyNI is partly based on source files from CPython 2.7.3, 2.7.4, 2.7.5, 2.7.6,
2.7.7, 2.7.8, 2.7.9, 2.7.10, 2.7.11, 2.7.12 and 2.7.13.
As such, it includes the common license of CPython
2.7.3, 2.7.4, 2.7.5, 2.7.6, 2.7.7, 2.7.8, 2.7.9, 2.7.10, 2.7.11, 2.7.12,
2.7.13 and Jython in the file
[PSF-LICENSE-2](https://github.com/Stewori/JyNI/blob/master/PSF-LICENSE-2).
Whether a source file is directly based on CPython
source code is documented at the beginning of each file.

For compliance with PSF LICENSE AGREEMENT FOR PYTHON 2, section 3,
the required overview of changes done to CPython code is given
in [section 7](#7-summary-of-changes-to-python-code) of this file.

For convenience, a copy of the current section is provided in the file
[LICENSE](https://github.com/Stewori/JyNI/blob/master/LICENSE).




## 10. Contact

Please use the contact information provided on [www.jyni.org/#contact](http://www.jyni.org/#contact).

