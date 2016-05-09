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


Created on 01.05.2016

@author: Stefan Richthofer
'''

import sys
sys.path.insert(0, '/usr/lib/python2.7/lib-dynload')

import ctypes
import unittest
import time
#import weakref

from JyNI import JyNI
from JyNI import JyReferenceMonitor as monitor
from java.lang import System
#from java.lang.ref import WeakReference

def runGC():
	System.gc()
	time.sleep(1)

def clearCurrentLeaks():
	leaks = monitor.getCurrentNativeLeaks()
	if len(leaks) > 0:
		monitor.declareLeaksPermanent(leaks)

class TestJyNI_gc(unittest.TestCase):
# 	@classmethod
# 	def setUpClass(cls):
# 		JyNI.JyRefMonitor_setMemDebugFlags(1)
# 		#JyWeakReferenceGC.monitorNativeCollection = True
# 
# 	@classmethod
# 	def tearDownClass(cls):
# 		JyNI.JyRefMonitor_setMemDebugFlags(0)
# 		#JyWeakReferenceGC.monitorNativeCollection = False

	def test_ctypes_newstyle_dict(self):
		#print "test_ctypes_newstyle_dict"
		from ctypes import CFUNCTYPE, c_int

		test = CFUNCTYPE(c_int, c_int)()
		dct = test.__dict__
		test.__name__ = "testName"
		dct = None
		self.assertTrue(hasattr(test, "__name__"))
		self.assertEqual(test.__name__, "testName")
		runGC()
		runGC()
		self.assertTrue(hasattr(test, "__name__"))
		self.assertEqual(test.__name__, "testName")


if __name__ == '__main__':
	unittest.main()
