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

sys.path.insert(0, '/usr/lib/python2.7/lib-dynload')

import DemoExtension
import unittest
import time

from JyNI import JyNI
from JyNI import JyReferenceMonitor as monitor
#from JyNI.gc import JyWeakReferenceGC
from java.lang import System
from java.lang.ref import WeakReference

def runGC():
	System.gc()
	time.sleep(1)

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
		doc = DemoExtension.argCountToString.__doc__
		#monitor.listLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)

	def test_gc_list_cycle(self):
		l = (123, [0, "test1"])
		l[1][0] = l
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l)
		DemoExtension.argCountToString(l)
		del l
		self.assertNotEqual(wkl.get(), None)
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 4)
		runGC()
		self.assertEqual(wkl.get(), None)
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		del wkl

	def test_gc_list_cycle2(self):
		l2 = (127, [0, "test2"])
		l2[1][0] = l2
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l2)
		DemoExtension.argCountToString(l2)
		self.assertNotEqual(wkl.get(), None)
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 4)
		runGC()
		self.assertNotEqual(wkl.get(), None)
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 3)
		del l2
		runGC()
		self.assertEqual(wkl.get(), None)
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		del wkl

	def test_gc_dict_cycle(self):
		l = (123, {'a': 0, 'b': "test3"})
		l[1]['a'] = l
		#We create weak reference to l to monitor collection by Java-GC:
		wkl = WeakReference(l)
		DemoExtension.argCountToString(l)
		del l
		self.assertNotEqual(wkl.get(), None)
		#monitor.listLeaks()
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 3)
		runGC()
		self.assertEqual(wkl.get(), None)
		self.assertEqual(len(monitor.getCurrentNativeLeaks()), 0)
		#print ""
		#monitor.listLeaks()
		del wkl

if __name__ == '__main__':
	unittest.main()
