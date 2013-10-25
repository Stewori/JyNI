'''
Created on 20.09.2013

@author: Stefan Richthofer
'''
import sys

#Since invalid paths do no harm, we add several possible paths here, where
#DemoExtension.so could be located in various build scenarios. If you use different
#scenarios in parallel, uncomment some of the paths to select,the one to be used.

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

#import java.lang.System as jsys
#jsys.out.println("Jython is running!")
#print "-------------------"

print "sys.exc_info, initial: "+str(sys.exc_info())
try:
	print "We call a native method that creates an exception. However, this time we handle it..."
	DemoExtension.exceptionTest()
except SystemError:
	print "...and entered except-area!"
	print "sys.exc_info: "+str(sys.exc_info())
print "Now lets look, what happens, if the exception is uncaught:"
DemoExtension.exceptionTest()
print "------------------- this line should never be printed"
