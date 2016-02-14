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


Created on 10.07.2015

@author: Stefan Richthofer
'''

import sys

#Since invalid paths do no harm, we add several possible paths here, where
#DemoExtension.so could be located in various build scenarios. If you use different
#scenarios in parallel, select the one to be used by setting some of the paths as comments.

#built with an IDE in debug mode:
sys.path.append('../../DemoExtension/Debug') #in case you run it from src dir
sys.path.append('./DemoExtension/Debug') #in case you run it from base dir
#built with an IDE in release mode:
sys.path.append('../../DemoExtension/Release') #in case you run it from src dir
sys.path.append('./DemoExtension/Release') #in case you run it from base dir
#built with setup.py on 64 bit machine:
sys.path.append('../../DemoExtension/build/lib.linux-x86_64-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.linux-x86_64-2.7') #in case you run it from base dir
#built with setup.py on 32 bit machine:
sys.path.append('../../DemoExtension/build/lib.linux-i686-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.linux-i686-2.7') #in case you run it from base dir
#built with setup.py on macosx 10.10:
sys.path.append('../../DemoExtension/build/lib.macosx-10.10-intel-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.macosx-10.10-intel-2.7') #in case you run it from base dir
#built with setup.py on macosx 10.11:
sys.path.append('../../DemoExtension/build/lib.macosx-10.10-intel-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.macosx-10.11-intel-2.7') #in case you run it from base dir

sys.path.insert(0, '/usr/lib/python2.7/lib-dynload')

import DemoExtension
import unittest
import time
import weakref

from JyNI import JyNI
from JyNI import JyReferenceMonitor as monitor
#from JyNI.gc import JyWeakReferenceGC
from java.lang import System
from java.lang.ref import WeakReference

def runGC():
	System.gc()
	time.sleep(1)

def clearCurrentLeaks():
	leaks = monitor.getCurrentNativeLeaks()
	if len(leaks) > 0:
		monitor.declareLeaksPermanent(leaks)

class TestJyNI_gc(unittest.TestCase):
	@classmethod
	def setUpClass(cls):
		JyNI.JyRefMonitor_setMemDebugFlags(1)
		#JyWeakReferenceGC.monitorNativeCollection = True

	@classmethod
	def tearDownClass(cls):
		JyNI.JyRefMonitor_setMemDebugFlags(0)
		#JyWeakReferenceGC.monitorNativeCollection = False

	def test_gc_doc(self):
		#print "test_gc_doc"
		doc = DemoExtension.argCountToString.__doc__
		#monitor.listLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)

	def test_gc_list_cycle(self):
		#print "test_gc_list_cycle"
		clearCurrentLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		l = (123, [0, "test1"])
		l[1][0] = l
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l)
		DemoExtension.argCountToString(l)
		del l
		self.assertIsNotNone(wkl.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 4)
		runGC()
		self.assertIsNone(wkl.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		del wkl

	def test_gc_list_cycle2(self):
		#print "test_gc_list_cycle2"
		clearCurrentLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		l2 = (127, [0, "test2"])
		l2[1][0] = l2
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l2)
		DemoExtension.argCountToString(l2)
		self.assertIsNotNone(wkl.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 4)
		runGC()
		self.assertIsNotNone(wkl.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 3)
		del l2
		runGC()
		self.assertIsNone(wkl.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		del wkl

	def test_gc_dict_cycle(self):
		#print "test_gc_dict_cycle"
		clearCurrentLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		l = (123, {'a': 0, 'b': "test3"})
		l[1]['a'] = l
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l)
		DemoExtension.argCountToString(l)
		del l
		self.assertIsNotNone(wkl.get())
		#monitor.listLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 3)
		runGC()
		self.assertIsNone(wkl.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		#monitor.listLeaks()
		del wkl

	def test_gc_list_modify_pre(self):
		#print "test_gc_list_modify_pre"
		clearCurrentLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		l = [0, "test1"]
		d = {'a': 7, 'b': "test6"}
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l)
		wkd = WeakReference(d)
		l[0] = d
		DemoExtension.argCountToString(l)
		#l[0] = d
		del d
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 4)
		runGC()
		self.assertIsNotNone(wkl.get())
		self.assertIsNotNone(wkd.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 3)
		del l
		runGC()
		self.assertIsNone(wkl.get())
		self.assertIsNone(wkd.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)

	def test_gc_list_modify_update(self):
		#print "test_gc_list_modify_update"
		clearCurrentLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		l = [0, "test1"]
		d = {'a': 7, 'b': "test6"}
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l)
		wkd = WeakReference(d)
		#l[0] = d
		DemoExtension.argCountToString(l)
		l[0] = d
		del d
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 4)
		runGC()
		self.assertIsNotNone(wkl.get())
		self.assertIsNotNone(wkd.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 3)
		del l
		runGC()
		self.assertIsNone(wkl.get())
		self.assertIsNone(wkd.get())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)

	def test_gc_list_modify_silent(self):
		#print "test_gc_list_modify_silent"
		clearCurrentLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		l = [0, "test1"]
		d = {'a': 7, 'b': "test6"}
		wkl = WeakReference(l)
		wkd = WeakReference(d)
		wkl2 = weakref.ref(l)
		wkd2 = weakref.ref(d)
		self.assertIsNotNone(wkl.get())
		self.assertIsNotNone(wkd.get())
		self.assertIsNotNone(wkl2())
		self.assertIsNotNone(wkd2())
		DemoExtension.listSetIndex(l, 0, d)
		del d
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 4)
		#monitor.listAll()
		runGC()
		#monitor.listAll()
		self.assertFalse(monitor.lastClearGraphValid)
		self.assertIsNotNone(wkl.get())
		self.assertIsNone(wkd.get())
		self.assertIsNotNone(wkl2())
		self.assertIsNotNone(wkd2())
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 3)
		self.assertIsNotNone(l[0])
		self.assertEqual(len(l[0]), 2)
		del l
		runGC()
		#monitor.listAll()
		self.assertTrue(monitor.lastClearGraphValid)

 		# For some reason resurrected objects persist one more
 		# gc-cycle in principle. So we have to run gc again before
 		# we can observe wkd2 to die. It is currently unclear whether
 		# this behavior is a JyNI-bug or natural Java-gc behavior,
 		# but for now (with some evidence) we assume the latter.
		# Note: Since WeakRef-support wkd2 actually keeps the native
		# referent alive for one more cycle. So also the monitor-refcount
		# test only passes after another gc-run now.
 		runGC()
 		#monitor.listAll()
		self.assertTrue(monitor.lastClearGraphValid)

		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		self.assertIsNone(wkl2())
		self.assertIsNone(wkd2())


if __name__ == '__main__':
	unittest.main()
