'''
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


Created on 02.09.2014

@author: Stefan Richthofer
'''

import sys
import os
from os import name
import sys
import platform

# This will vastly simplify once Jython 2.7.1 is out and we have
# uname and mac_ver available in Jython:
if os.name == 'java':
	machine = platform.java_ver()[-1][-1]
	if machine == 'amd64':
		machine = 'x86_64'
	elif machine == 'x86':
		machine = 'i686'
	systm = platform.java_ver()[-1][0].lower().replace(' ', '')
	if systm == 'macosx':
		ver = platform.java_ver()[-1][1]
		ver = ver[:ver.rfind('.')]
		buildf = '-'.join((systm, ver, 'intel'))
	else:
		buildf = '-'.join((systm, machine))
else:
	systm = os.uname()[0].lower()
	if systm == 'darwin':
		ver = platform.mac_ver()[0]
		ver = ver[:ver.rfind('.')]
		buildf = '-'.join(('macosx', ver, 'intel'))
	else:
		buildf = '-'.join((systm, os.uname()[-1]))


#Since invalid paths do no harm, we add several possible paths here, where
#DemoExtension.so could be located in various build scenarios. If you use different
#scenarios in parallel, select the one to be used by setting some of the paths as comments.

#built with an IDE in debug mode:
sys.path.append('../../DemoExtension/Debug') #in case you run it from src dir
sys.path.append('./DemoExtension/Debug') #in case you run it from base dir
#built with an IDE in release mode:
sys.path.append('../../DemoExtension/Release') #in case you run it from src dir
sys.path.append('./DemoExtension/Release') #in case you run it from base dir
#built with setup.py:
sys.path.append('../../DemoExtension/build/lib.'+buildf+'-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.'+buildf+'-2.7') #in case you run it from base dir


# datetime_path is expected to be folder containing datetime.so

# This is the usual system path for datetime.so. On some distributions it actually
# does not contain datetime.so; i.e. then datetime.so is part of libpython2.7.so.
# (However JyNI cannot load libpython2.7.so for that purpose due to other symbol
#  conflicts; you need to provide datetime.so in some other way then, e.g. by
#  compiling CPython yourself)
datetime_path = '/usr/lib/python2.7/lib-dynload'

# This is an example-path for a self-compiled python:
#datetime_path = '/data/workspace/linux/Python-2.7.11/build/lib.linux-x86_64-2.7'

sys.path.insert(0, datetime_path)

def hasNativeDatetime():
	if os.name == 'java':
		from JyNI import JyNI
		return JyNI.isLibraryFileAvailable('datetime')
	else:
		# Let's assume native datetime is trivially available for non-Java case:
		return True

import DemoExtension
import datetime
import unittest

class TestJyNI(unittest.TestCase):

	def test_DemoExtension_doc(self):
		self.assertEqual(DemoExtension.__doc__, "This is a pure demo extension.")

	def test_hello_world_info(self):
		self.assertEqual(str(DemoExtension.hello_world), "<built-in function hello_world>")
		self.assertEqual(DemoExtension.hello_world.__doc__, "Hello World method.")

	def test_argument_passing(self):
		self.assertEqual(DemoExtension.concatFirstWithLastString.__doc__, "Concatenates first with last element. Returns empty string, if less than two args are available.")
		self.assertEqual(DemoExtension.concatFirstWithLastString("begin_", "ignore", "ignore too", "end"), "begin_end")
		self.assertEqual(DemoExtension.argCountToString.__doc__, "Returns number of arguments as string.")
		self.assertEqual(DemoExtension.argCountToString("a", "b", "c", "d", "e", "f"), "(6)")

	def test_integer_passing(self):
		self.assertEqual(DemoExtension.intSquare.__doc__, "Returns the square of the given int.")
		self.assertEqual(str(DemoExtension.intSquare), "<built-in function intSquare>")
		self.assertEqual(str(DemoExtension.intSquare.__class__), "<type 'builtin_function_or_method'>")
		self.assertEqual(DemoExtension.intSquare.__name__, "intSquare")
		self.assertEqual(DemoExtension.intSquare(16), 256)
		self.assertEqual(DemoExtension.intSquare(19), 361)
		self.assertEqual(DemoExtension.intSquare1(-19), 361)

	def test_boolean_passing(self):
		self.assertEqual(DemoExtension.booleanToInt(False), 0)
		self.assertEqual(DemoExtension.booleanToInt(True), 1)
		self.assertIsNone(DemoExtension.booleanToInt(99))
		self.assertTrue(DemoExtension.intToBoolean(1))
		self.assertFalse(DemoExtension.intToBoolean(0))
		self.assertIsNone(DemoExtension.intToBoolean(2))

	def test_native_list_access_writing(self):
		l = ["Hello", "lovely", "world"]
		DemoExtension.listModifyTest(l, 2)
		self.assertEqual(l[2], "natively modified")
		self.assertEqual(len(l), 3)

	#Passing self-containing objects to native code used to crash JyNI.
	#This test proves that this is fixed.
	def test_native_list_selfcontaining(self):
		l = ["Hello", "lovely", "world"]
		l[1] = l
		DemoExtension.listModifyTest(l, 0)
		self.assertEqual(l[0], "natively modified")
		self.assertEqual(len(l), 3)
		self.assertEqual(str(l), "['natively modified', [...], 'world']")

	def test_native_sequence_selfcontaining(self):
		# In early JyNI-days self-containing sequences used
		# to trigger infinite conversion loops
		l = ["Hello", "lovely", "world"]
		l[1] = (11, 12, l)
		DemoExtension.listModifyTest(l, 2)
		self.assertEqual(l[2], "natively modified")
		self.assertEqual(len(l), 3)
		self.assertEqual(str(l), "['Hello', (11, 12, [...]), 'natively modified']")

	def test_native_create_list_selfcontaining(self):
		l = DemoExtension.createListSelfContaining()
		self.assertEqual(str(l), "['element1', 'element2', [...]]")

	def test_native_create_tuple_selfcontaining(self):
		l = DemoExtension.createTupleSelfContaining()
		self.assertEqual(str(l[:2]), "('tp1', 'tp2')")
		self.assertEqual(str(l[2]), "['lst1', ('tp1', 'tp2', [...])]")
		#This output differs between Jython and CPython: (Why?)
		#(slightly different output-recursion depth)
# 		self.assertEqual(str(l), "('tp1', 'tp2', ['lst1', ('tp1', 'tp2', [...])])")

	def test_native_set(self):
		basket = ['apple', 'orange', 'apple', 'pear', 'orange', 'banana']
		fruit = set(basket)
		p = 2
		DemoExtension.setPopTest(fruit, p)
		self.assertEqual(len(fruit), 2)
		count = 0
		if "pear" in fruit:
			count += 1
		if "apple" in fruit:
			count += 1
		if "orange" in fruit:
			count += 1
		if "banana" in fruit:
			count += 1
		self.assertEqual(count, 2)

	def test_unicode(self):
		uc = u'a\xac\u1234\u20ac\U00008000'
		uc2 = DemoExtension.unicodeTest(uc)
		self.assertEqual(uc, uc2)

	def test_exception(self):
		self.assertRaisesRegexp(SystemError, "This is a test exception message for JyNI.", DemoExtension.exceptionTest)
		try:
			DemoExtension.exceptionTest()
			self.assertEqual(1, 2) #would always fail, but is not reached if everything works as expected
		except SystemError:
			exc = sys.exc_info()
			self.assertEqual(exc[0], SystemError)
			self.assertEqual(str(exc[1]), "This is a test exception message for JyNI.")

	def test_new_style_classes(self):

		class testnewstyle(object):
			pass

		class testnewstyleString(str):
			pass

		class testnewstyleInt(int):
			pass

		strType = type("")
		intType = type(6)

		nobj = testnewstyle()
		self.assertTrue(DemoExtension.newstyleCheck(nobj))
		self.assertFalse(DemoExtension.newstyleCheckSubtype(nobj, strType))
		self.assertFalse(DemoExtension.newstyleCheckSubtype(nobj, intType))

		nobj = testnewstyleString()
		self.assertTrue(DemoExtension.newstyleCheck(nobj))
		self.assertTrue(DemoExtension.newstyleCheckSubtype(nobj, strType))
		self.assertFalse(DemoExtension.newstyleCheckSubtype(nobj, intType))

		nobj = testnewstyleInt()
		self.assertTrue(DemoExtension.newstyleCheck(nobj))
		self.assertFalse(DemoExtension.newstyleCheckSubtype(nobj, strType))
		self.assertTrue(DemoExtension.newstyleCheckSubtype(nobj, intType))

	@unittest.skipUnless(os.name == 'java',
		'Not runnable with CPython, because it uses JyNI-classes for testing.')
	def test_custom_classes(self):
		from JyNI import JyNIImporter #, PyShadowString

		strType = type("")
		intType = type(6)

		nobj = JyNIImporter()
		self.assertTrue(DemoExtension.newstyleCheck(nobj))
		self.assertFalse(DemoExtension.newstyleCheckSubtype(nobj, strType))
		self.assertFalse(DemoExtension.newstyleCheckSubtype(nobj, intType))

# 		nobj = PyShadowString("a", "b")
# 		self.assertTrue(DemoExtension.newstyleCheck(nobj))
# 		self.assertTrue(DemoExtension.newstyleCheckSubtype(nobj, strType))
# 		self.assertFalse(DemoExtension.newstyleCheckSubtype(nobj, intType))

	@unittest.skipUnless(hasNativeDatetime(),
		'datetime.so not found (probably part of libpython2.7.so)')
	def test_datetime(self):
		self.assertEqual(datetime.__doc__, "Fast implementation of the datetime type.")
		self.assertEqual(datetime.__name__, "datetime")
		now = datetime.datetime(2013, 11, 3, 20, 30, 45)
		self.assertEqual(str(now), "2013-11-03 20:30:45")
		self.assertEqual(repr(now), "datetime.datetime(2013, 11, 3, 20, 30, 45)")
		self.assertEqual(str(type(now)), "<type 'datetime.datetime'>")

	@unittest.skipUnless(os.name == 'java',
		'Shuts down CPython silently for unknown reason; \
		the tested API is used by Cython, so should actually work.')
	def test_native_import_API_and_methoddescr(self):
		self.assertEqual(DemoExtension.importAPIandMethodDescrTest(), 0)


if __name__ == '__main__':
	unittest.main()
