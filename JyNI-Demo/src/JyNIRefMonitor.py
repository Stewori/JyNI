'''
Created on 30.06.2013

@author: Stefan Richthofer
'''

# Note that this script currently has issues under 32bit systems.
# It appears that this is a JNI-bug.

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

import time
import weakref

from JyNI import JyNI
from JyNI import JyReferenceMonitor as monitor
from JyNI.gc import JyWeakReferenceGC
from java.lang import System
from java.lang.ref import WeakReference
from org.python.modules._weakref import GlobalRef


def run1():
	JyNI.JyRefMonitor_setMemDebugFlags(1)
	JyWeakReferenceGC.monitorNativeCollection = True
	
	# PyType  <- Make native ref-cycle test with heap type to test partly-stub-mode.
	# PySet, but no native link to other PyObject
	# PyFrozenSet, "
	# PyCode, not GC-relevant in CPython
	
	import DemoExtension
	
	#Note:
	# For now we attempt to verify JyNI's GC-functionality independently from
	# Jython concepts like Jython weak references or Jython GC-module.
	# So we use java.lang.ref.WeakReference and java.lang.System.gc to monitor
	# and control Java-gc.
	
	#JyNI.JyRefMonitor_setMemDebugFlags(1)
	#JyWeakReferenceGC.monitorNativeCollection = True

	#l = (123,)
	l = ([0, "test"],)
	l[0][0] = l
	#l = (123, {'a': 0, 'b': "test"})
	#l[1]['a'] = l
	#We create weak reference to l to monitor collection by Java-GC:
	wkl = WeakReference(l)
	print "weak(l): "+str(wkl.get())
	
	# We pass down l to some native method. We don't care for the method itself,
	# but conversion to native side causes creation of native PyObjects that
	# correspond to l and its elements. We will then track the life-cycle of these.
	print "make l native..."
	
	DemoExtension.argCountToString(l)
	
	#print "argCountToString.__doc__-address: "+str(JyNI.lookupNativeHandle(DemoExtension.argCountToString.__doc__))
	#print "We access a method-doc as this used to cause problems with GC:"
	#print "    "+DemoExtension.argCountToString.__doc__
	#print "Native static objects so far:"
	#print JyNI.nativeStaticPyObjectHeads.keySet()
	
	print "Delete l... (but GC not yet ran)"
	del l
	#l = None
	print "weak(l) after del: "+str(wkl.get())
	print ""
	# monitor.list-methods display the following format:
	# [native pointer]{'' | '_GC_J' | '_J'} ([type]) #[native ref-count]: [repr] *[creation time]
	# _GC_J means that JyNI tracks the object
	# _J means that a JyNI-GC-head exists, but the object is not actually treated by GC
	# This can serve monitoring purposes or soft-keep-alive (c.f. java.lang.ref.SoftReference)
	# for caching.
	print "Leaks before GC:"
	monitor.listLeaks()
	print ""

	# By inserting this line you can confirm that native
	# leaks would persist if JyNI-GC is not working:
	#JyWeakReferenceGC.nativecollectionEnabled = False

	print "calling Java-GC..."
	System.gc()
	time.sleep(2)
	print "weak(l) after GC: "+str(wkl.get())
	print ""
	monitor.listWouldDeleteNative()
	print ""
	print "leaks after GC:"
	monitor.listLeaks()
	print ""
	print "    "+DemoExtension.argCountToString.__doc__
	monitor.listLeaks()
	#print "----"
	#print monitor.listAll()
	print ""
	print "===="
	print "exit"
	print "===="


def run2():
	JyNI.JyRefMonitor_setMemDebugFlags(1)
	JyWeakReferenceGC.monitorNativeCollection = True
	
	# PyType  <- Make native ref-cycle test with heap type to test partly-stub-mode.
	# PySet, but no native link to other PyObject
	# PyFrozenSet, "
	# PyCode, not GC-relevant in CPython

	import DemoExtension
	
	#Note:
	# For now we attempt to verify JyNI's GC-functionality independently from
	# Jython concepts like Jython weak references or Jython GC-module.
	# So we use java.lang.ref.WeakReference and java.lang.System.gc to monitor
	# and control Java-gc.
	
	#JyNI.JyRefMonitor_setMemDebugFlags(1)
	#JyWeakReferenceGC.monitorNativeCollection = True

	l = [0, "test1"]
	d = {'a': 7, 'b': "test6"}
	#We create weak reference to l to monitor collection by Java-GC:
	wkl = WeakReference(l)
	wkd = WeakReference(d)
	#l[0] = d
	print "weak(l): "+str(wkl.get())
	print "weak(d): "+str(wkd.get())

	# We pass down l to some native method. We don't care for the method itself,
	# but conversion to native side causes creation of native PyObjects that
	# correspond to l and its elements. We will then track the life-cycle of these.
	print "make l native..."

	# Note that it *does* matter whether we insert d into l before oder after this
	# native call. Inserting it after the call tests PyList's capability to update
	# the reference graph while inserting it before yields the should-be-result.
	# If both variants lead to the same post-GC output, JyNI works fine.
	#l[0] = d
	DemoExtension.argCountToString(l)
	l[0] = d
	#print "argCountToString.__doc__-address: "+str(JyNI.lookupNativeHandle(DemoExtension.argCountToString.__doc__))
	#print "We access a method-doc as this used to cause problems with GC:"
	#print "    "+DemoExtension.argCountToString.__doc__
	#print "Native static objects so far:"
	#print JyNI.nativeStaticPyObjectHeads.keySet()
	
	print "Delete l... (but GC not yet ran)"
	#del l
	del d
	#l = None
	#print "weak(l) after del: "+str(wkl.get())
	print "weak(d) after del: "+str(wkd.get())
	print ""
	# monitor.list-methods display the following format:
	# [native pointer]{'' | '_GC_J' | '_J'} ([type]) #[native ref-count]: [repr] *[creation time]
	# _GC_J means that JyNI tracks the object
	# _J means that a JyNI-GC-head exists, but the object is not actually treated by GC
	# This can serve monitoring purposes or soft-keep-alive (c.f. java.lang.ref.SoftReference)
	# for caching.
	print "Leaks before GC:"
	monitor.listLeaks()
	print ""
	
	# By inserting this line you can confirm that native
	# leaks would persist if JyNI-GC is not working:
	#JyWeakReferenceGC.nativecollectionEnabled = False
	
	print "calling Java-GC..."
	System.gc()
	time.sleep(2)
	print "weak(l) after GC: "+str(wkl.get())
	#print "l after GC: "+str(l)
	print "weak(d) after GC: "+str(wkd.get())
	print ""
	monitor.listWouldDeleteNative()
	print ""
	print "leaks after GC:"
	monitor.listLeaks()
	
	#print "------"
	#print DemoExtension.argCountToString.__doc__
	#monitor.listFreeStatus("dict")
	#monitor.listAll()
	print ""
	print "===="
	print "exit"
	print "===="

def run3():
	JyNI.JyRefMonitor_setMemDebugFlags(1)
	JyWeakReferenceGC.monitorNativeCollection = True
	import DemoExtension
	
	#JyNI.JyRefMonitor_setMemDebugFlags(1)
	#JyWeakReferenceGC.monitorNativeCollection = True

	l = [0, "test1"]
# 	print l
# 	DemoExtension.listSetIndex(l, 0, 100.7)
# 	print l
	d = {'a': 7, 'b': "test6"}
	#We create weak reference to l to monitor collection by Java-GC:
	wkl = WeakReference(l)
	wkd = WeakReference(d)
	wkl2 = weakref.ref(l)
	wkd2 = weakref.ref(d)

	#Note:
	#=====
	#To make this work we'll have to ensure that d gets a GCHead even though
	#l doesn't recognize the insertion of d and thus would not explore it.
	#In fact every C-stub object must get a GCHead when going native, regardless
	#of whether it is inserted somewhere or not, because it might be silently
	#inserted and a GCHead is the only way to detect this and fix it.
	#Add a call (e.g. to size) to the dict in l on native side to test that
	#d still works (which it currently shouldn't).
	#Later use d's GCHead to detect the invalid graph and resurrect d's java-part.
	#Take care of the GIL here. CStubs must release the GIL when they detect a
	#failing backend, so the GC-thread can acquire it and resurrect the backend.
	#How to fix the weak reference etc?

	#l[0] = d
	print "weak(l): "+str(wkl.get())
	print "weak(d): "+str(wkd.get())
	print "weak2(l): "+str(wkl2())
	print "weak2(d): "+str(wkd2())
	print "make l native..."
	#l[0] = d
	#DemoExtension.argCountToString(l)
	#l[0] = d
	DemoExtension.listSetIndex(l, 0, d)

	print "Delete l... (but GC not yet ran)"
	#del l
	del d
	#l = None
	#print "weak(l) after del: "+str(wkl.get())
	print "weak(d) after del: "+str(wkd.get())
	print "weak2(d) after del: "+str(wkd2())
	print ""
	print "Leaks before GC:"
	monitor.listLeaks()
	print ""
	
	print "calling Java-GC..."
	System.gc()
	time.sleep(2)
# 	if monitor.lastClearGraphValid:
# 		print "valid graph"
# 	else:
# 		print "invalid graph"
# 	monitor.lastClearGraphValid = False
	print "weak(l) after GC: "+str(wkl.get())
	#print "l after GC: "+str(l)
	print "weak(d) after GC: "+str(wkd.get())
	print "weak2(l) after GC: "+str(wkl2())
	print "weak2(d) after GC: "+str(wkd2())
	wkd = WeakReference(l[0])
	print ""
	#monitor.listWouldDeleteNative()
	print ""
	#print "leaks after GC:"
	#monitor.listLeaks()
	print "l[0], i.e. d after gc:"
	#print l[0]
	#print len(l[0])
	
	print "------"
	print "del l..."
	del l
	System.gc()
	time.sleep(2)
# 	if monitor.lastClearGraphValid:
# 		print "valid graph"
# 	else:
# 		print "invalid graph"
# 	monitor.lastClearGraphValid = False
	print ""
	monitor.listWouldDeleteNative()
	print ""
	#print "leaks after GC (l deleted):"
	#monitor.listLeaks()
	#print DemoExtension.argCountToString.__doc__
	#monitor.listFreeStatus("dict")
	#monitor.listAll()
	#GlobalRef.processDelayedCallbacks()
	print "weak(d) after GC2: "+str(wkd.get())
	print "weak2(l) after GC2: "+str(wkl2())
	print "weak2(d) after GC2: "+str(wkd2())
	System.gc()
	time.sleep(2)
	print "weak(d) after GC3: "+str(wkd.get())
	monitor.listWouldDeleteNative()
	print ""
	print "leaks after GC3:"
	monitor.listLeaks()
	print ""
	print "===="
	print "exit"
	print "===="

def run4():
	import DemoExtension

	JyNI.JyRefMonitor_setMemDebugFlags(1)
	l = ([0, "test"],)
	l[0][0] = l
	DemoExtension.argCountToString(l)
	del l
	#l[0][1] = None
	print "Leaks before GC:"
	monitor.listLeaks()
	System.gc()
	time.sleep(2)
	print "Leaks after GC:"
	monitor.listLeaks()

run4()
#System.getProperties().list(System.out)
