'''
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


Created on 09.02.2016

@author: Stefan Richthofer
'''

import sys

sys.path.insert(0, '/usr/lib/python2.7/lib-dynload')


import unittest
import platform
from ctypes import *

isMac = platform.java_ver()[-1][0] == 'Mac OS X' or platform.mac_ver()[0] != ''
if isMac:
	libc = CDLL('libc.dylib')
else:
	libc = CDLL('libc.so.6')

buffer = c_buffer("\000", 50)
			
class TestJyNI_ctypes(unittest.TestCase):

	def test_libc_sprintf(self):
		self.assertEqual(str(type(libc.strlen)), "<class 'ctypes._FuncPtr'>")
		self.assertTrue(str(libc.strlen).startswith("<_FuncPtr object at 0x"))
		self.assertEqual(libc.strlen("abcdef"), 6)
		libc.sprintf(buffer, "%d bottles of beer", 42)
		self.assertEqual(buffer.value, "42 bottles of beer")

		class Bottles:
			def __init__(self, number):
				self._as_parameter_ = number

		libc.sprintf(buffer, "%d bottles of beer", Bottles(73))
		self.assertEqual(buffer.value, "73 bottles of beer")


	def test_argtypes(self):
		# We must later restore old argtypes (i.e. probably None) to assure
		# that all tests are applied under same conditions.
		saved_argtypes = libc.sprintf.argtypes
		libc.sprintf.argtypes = [c_char_p, c_char_p, c_char_p, c_int, c_double]
		libc.sprintf(buffer, "String '%s'; Int %d; Double %f\n", "Hi", 10, 2.2)
		self.assertTrue(buffer.value.startswith("String 'Hi'; Int 10; Double 2"))
		libc.sprintf.argtypes = saved_argtypes


	def test_pointers(self):

		class cell(Structure):
			pass

		cell._fields_ = [("name", c_char_p), ("next", POINTER(cell))]
		c1 = cell()
		c1.name = "foo"
		c2 = cell()
		c2.name = "bar"
		c1.next = pointer(c2)
		c2.next = pointer(c1)
		p = c1
		sbuf = []
		for i in range(8):
			sbuf.append(p.name)
			p = p.next[0]
		self.assertEqual("_".join(sbuf), "foo_bar_foo_bar_foo_bar_foo_bar")


	def test_callback(self):
		IntArray6 = c_int * 6
		self.assertEqual(str(type(IntArray6)), "<type '_ctypes.PyCArrayType'>")
		ia = IntArray6(5, 1, 7, 33, 99, -8)
		
		def py_abs_cmp_func(a, b):
			return abs(a[0]) - abs(b[0])
		
		CMPFUNC = CFUNCTYPE(c_int, POINTER(c_int), POINTER(c_int))
		cmp_func = CMPFUNC(py_abs_cmp_func)
		libc.qsort(ia, len(ia), sizeof(c_int), cmp_func)
		ia_buf = []
		for i in range(len(ia)):
			ia_buf.append(ia[i])
		self.assertEqual(ia_buf, [1, 5, 7, -8, 33, 99])


if __name__ == '__main__':
	unittest.main()
