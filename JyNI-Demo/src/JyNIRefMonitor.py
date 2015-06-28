'''
Created on 30.06.2013

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

import time

from JyNI import JyNI
from JyNI import JyReferenceMonitor as monitor
from java.lang import System
from java.lang.ref import WeakReference

import DemoExtension

# l = DemoExtension.createTupleSelfContaining()
# # l = ('tp1', 'tp2', ['lst1', 0])
# # l[2][1] = l
# print len(l)
# # n = l[0]
# # print n
# print l
# #print l[2]
# print "----"

print "Set mem debug..."
JyNI.JyRefMonitor_setMemDebugFlags(1)
print "done"

l = (123, ["test"])
l[1][0] = l
#l = ["test", 129]
#l[1] = l
wkt = WeakReference(l[1])
r = 124
wkl = WeakReference(l)
wkr = WeakReference(r)
print "wkl: "+str(wkl.get())
#DemoExtension.listModifyTest(l, 2)
DemoExtension.argCountToString(l)
#l = None
r = None
print "wkl2: "+str(wkl.get())
print "wkt2: "+str(wkt.get())
print "wkr2: "+str(wkr.get())
#print l[1]
#print l
#self.assertEqual(l[2], "natively modified")
#self.assertEqual(len(l), 3)

#DemoExtension.printInt(333)
#DemoExtension.hello_world()
#print "done2"
del l
#l = None
#print ""
monitor.listLeaks()
System.gc()
time.sleep(2)
print "wkl3: "+str(wkl.get())
print "wkt3: "+str(wkt.get())
print "wkr3: "+str(wkr.get())

# System.gc()
# time.sleep(2)
# print "wkl4: "+str(wkl.get())
#print ""

monitor.listLeaks()


print ""
print "----------native output:------------"
#print str(l)+"-@-"+str(JyNI.lookupNativeHandle(l))
#for x in l:
#	print str(x)+"-@-"+str(JyNI.lookupNativeHandle(x))
#DemoExtension.hello_world()
#print "done3"
