'''
Created on 30.06.2013

@author: Stefan Richthofer
'''

import sys
#Since invalid paths do no harm, we add several possible paths here, where
#DemoExtension.so could be located in various build scenarios. If you use different
#scenarios in parallel, select the one to be used by setting some of the paths as comments.

#build with an IDE in debug mode:
sys.path.append('../../DemoExtension/Debug') #in case you run it from src dir
sys.path.append('./DemoExtension/Debug') #in case you run it from base dir
#build with an IDE in release mode:
sys.path.append('../../DemoExtension/Release') #in case you run it from src dir
sys.path.append('./DemoExtension/Release') #in case you run it from base dir
#build with setup.py on 64 bit machine:
sys.path.append('../../DemoExtension/build/lib.linux-x86_64-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.linux-x86_64-2.7') #in case you run it from base dir
#build with setup.py on 32 bit machine:
sys.path.append('../../DemoExtension/build/lib.linux-i686-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.linux-i686-2.7') #in case you run it from base dir

import DemoExtension


# a = 200000
# print str(a)+" ("+str(id(a))+")"
# b = 300000
# print str(b)+" ("+str(id(b))+")"
# b = 200000
# print str(b)+" ("+str(id(b))+")"

from JyNI import JyNI

l = ["Hello", "lovely", "world"]
DemoExtension.listModifyTest(l, 2)
print "Set mem debug..."
JyNI.JyRefMonitor_setMemDebugFlags(1)
print "done"
print l[1]
#print l
#self.assertEqual(l[2], "natively modified")
#self.assertEqual(len(l), 3)

DemoExtension.printInt(333)
DemoExtension.hello_world()
print "done2"
