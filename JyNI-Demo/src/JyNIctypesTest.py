'''
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
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

import sys

#Include native ctypes:
sys.path.append('/usr/lib/python2.7/lib-dynload')

import platform
isMac = platform.java_ver()[-1][0] == 'Mac OS X' or platform.mac_ver()[0] != ''
print "import ctypes..."
import ctypes
#import struct

print "Demo of ctypes with os.name: "+platform.os.name
print ""

if isMac:
	libc = ctypes.CDLL('libc.dylib')
else:
	libc = ctypes.CDLL('libc.so.6')

print libc
print type(libc.strlen)
print libc.strlen
print libc.strlen("abcdef")
print "--------------------"
printf = libc.printf
printf("%d bottles of beer\n", 42)

from ctypes import *
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
for i in range(8):
	print p.name,
	p = p.next[0]
print "\n--------------------"
print "exited normally"
print "i.e. demo successful"
print ""
print "======in JyNI-case expect native output after these lines on some consoles====="
print "=====(it is a JNI issue that native output is displayed after java output)====="
