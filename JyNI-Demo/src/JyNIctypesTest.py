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
#sys.path.insert(0, '/home/stefan/eclipseWorkspace/ctypes/lib')

#from ctypes import cdll
#from _ctypes import _SimpleCData
import ctypes
#import struct

#print "py_object0..."

#class py_object0(_SimpleCData):
#	pass

print "import of ctypes successful"
print ""
#print ctypes.sizeof(ctypes.c_long)
#print struct.calcsize("l")#ctypes.c_long._type_)
#print ctypes.c_long._type_
#print py_object0

#libc = ctypes.cdll.LoadLibrary("libc.so.6")
libc = ctypes.CDLL('libc.so.6')
print libc
#print type(libc.strlen)
print libc.strlen
print libc.strlen("abcdef")
print "-----------"
printf = libc.printf
printf("%d bottles of beer\n", 42)
print "-----------"
