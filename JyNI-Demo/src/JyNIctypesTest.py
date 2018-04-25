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


Created on 03.10.2015

@author: Stefan Richthofer
'''

import os, sys
import ctypes
from ctypes import *
import platform

isMac = platform.java_ver()[-1][0] == 'Mac OS X' or platform.mac_ver()[0] != ''
isWin = os.name in ("nt", "ce") or os.name == 'java' and os._name in ("nt", "ce")

# print "Demo of ctypes with os.name: "+platform.os.name
# print ""

if isMac:
	libc = CDLL('libc.dylib')
elif isWin:
	libc = cdll.msvcrt
else:
	libc = CDLL('libc.so.6')

class Bottles:
	def __init__(self, number):
		self._as_parameter_ = number

print libc
print type(libc.strlen)
print libc.strlen
print libc.strlen("abcdef")
print "--------------------"
printf = libc.printf
printf("%d bottles of beer\n", 42)
printf("%d bottles of beer\n", Bottles(73))

printf.argtypes = [c_char_p, c_char_p, c_int, c_double]
printf("String '%s'; Int %d; Double %f\n", "Hi", 10, 2.2)

printf.argtypes = [c_char_p, c_int]
printf("Int %d\n", False)

buffer = c_buffer("\000", 10)
libc.sprintf(buffer, "Spam%d", 776)

print repr(buffer.value)
# Currently fails:
# print repr(buffer.raw),
# print "(should be 'Spam776\\x00\\x00\\x00')"

class cell(Structure):
	pass

print "Testing ctypes-pointers:"

cell._fields_ = [("name", c_char_p), ("next", POINTER(cell))]
c1 = cell()
c1.name = "foo"
c2 = cell()
c2.name = "bar"
c1.next = pointer(c2)
c2.next = pointer(c1)
p = c1
print p.next[0]
for i in range(8):
	print p.name,
	p = p.next[0]

print ""
print "--------------------"

IntArray6 = c_int * 6

#print type(IntArray6)
#print IntArray6.mro()

ia = IntArray6(5, 1, 7, 33, 99, -7)
print ia
qsort = libc.qsort

def py_cmp_func(a, b):
	print "py_cmp_func", a[0], b[0]
	return a[0] - b[0]

CMPFUNC = CFUNCTYPE(c_int, POINTER(c_int), POINTER(c_int))
cmp_func = CMPFUNC(py_cmp_func)
qsort(ia, len(ia), sizeof(c_int), cmp_func)
for i in range(len(ia)):
	print ia[i],

# Iterator-related builtins not yet supported:
# for i in ia: print i,

print ""
print "===================="
print "exited normally"
print "i.e. demo successful"
print ""
print "======in JyNI-case expect native output after these lines on some consoles====="
print "=====(it is a JNI issue that native output is displayed after java output)====="
