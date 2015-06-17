'''
 * Copyright of Python and Jython:
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
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.


Created on 02.09.2014

@author: Stefan Richthofer
'''

import sys

#Since invalid paths do no harm, we add several possible paths here, where
#DemoExtension.so could be located in various build scenarios. If you use different
#scenarios in parallel, select the one to be used by setting some of the paths as comments.

#build with an IDE in debug mode:
sys.path.append('../../DemoExtension/Debug') #in case you run it from src dir
sys.path.append('./DemoExtension/Debug') #in case you run it from base dir
#build with an IDE in release mode:
sys.path.append('../../DemoExtension/Release') #in case you run it from src dir
sys.path.append('./DemoExtension/Release') #in case you run it from base dir
#build with setup.py on 64 bit machine:
sys.path.append('../../DemoExtension/build/lib.linux-x86_64-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.linux-x86_64-2.7') #in case you run it from base dir
#build with setup.py on 32 bit machine:
sys.path.append('../../DemoExtension/build/lib.linux-i686-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.linux-i686-2.7') #in case you run it from base dir
#build with setup.py on macosx 10.10:
sys.path.append('../../DemoExtension/build/lib.macosx-10.10-intel-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.macosx-10.10-intel-2.7') #in case you run it from base dir

sys.path.insert(0, '/usr/lib/python2.7/lib-dynload')

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

	def test_native_list_access_writing(self):
		l = ["Hello", "lovely", "world"]
		DemoExtension.listModifyTest(l, 2)
		self.assertEqual(l[2], "natively modified")
		self.assertEqual(len(l), 3)

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
			self.assertEqual(1, 2) #always would fail, but is not reached, if everything works as expected
		except SystemError:
			exc = sys.exc_info()
			self.assertEqual(exc[0], SystemError)
			self.assertEqual(str(exc[1]), "This is a test exception message for JyNI.")

	def test_datetime(self):
		self.assertEqual(datetime.__doc__, "Fast implementation of the datetime type.")
		self.assertEqual(datetime.__name__, "datetime")
		now = datetime.datetime(2013, 11, 3, 20, 30, 45)
		self.assertEqual(str(now), "2013-11-03 20:30:45")
		self.assertEqual(repr(now), "datetime.datetime(2013, 11, 3, 20, 30, 45)")
		self.assertEqual(str(type(now)), "<type 'datetime.datetime'>")

if __name__ == '__main__':
	unittest.main()
