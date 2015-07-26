'''
Created on 30.06.2013

@author: Stefan Richthofer
'''

# Note that this script currently has issues under 32bit systems.
# It appears that this is a JNI-bug; a workaround is known, but not yet implemented.

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

from JyNI import JyNI
from JyNI import JyReferenceMonitor as monitor
from JyNI.gc import JyWeakReferenceGC
from java.lang import System
from java.lang.ref import WeakReference


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
	# 
	# Todo: Don't count objects reachable from registered type-dicts or dynModule-list as leaks.
	
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


run2()
#System.getProperties().list(System.out)
