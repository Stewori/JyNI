'''
Created on 30.06.2013

@author: Stefan Richthofer
'''

import sys

sys.path.append('/home/stefan/eclipseWorkspace/JyNI/DemoExtension/Debug')

import DemoExtension

print "--------Access Docstring----------"
print DemoExtension.__doc__

# If you run this via Jython, you can uncomment the following lines to prove that Jython is running.
# With these lines this demo becomes a program that neither Jython without JyNI,
# nor ordinary CPython could run.

print "To prove that Jython is running, we make a java call:"
from  java.lang import System
print "System.currentTimeMillis: "+str(System.currentTimeMillis())


print ""
print "--------Hello World----------"
print DemoExtension.hello_world
print DemoExtension.hello_world.__doc__
DemoExtension.hello_world()

print ""
print "--------Argument passing----------"
print DemoExtension.concatFirstWithLastString.__doc__
print DemoExtension.concatFirstWithLastString("begin_", "ignore", "ignore too", "end")
print DemoExtension.argCountToString.__doc__
print DemoExtension.argCountToString("a", "b", "c", "d", "e", "f")
 
print ""
print "--------Argument passing with keywords----------"
DemoExtension.keywordTest("first", "second", right = "Hey", wrong = "nothing")
print "(in JyNI-case see bottom for native outputs)"
 
print ""
print "----------------Integer passing-----------------"
print DemoExtension.intSquare.__doc__
print DemoExtension.intSquare
print DemoExtension.intSquare.__class__
print DemoExtension.intSquare.__name__
print "Native square result of 16: "+str(DemoExtension.intSquare(16))
print "Native square result of -19: "+str(DemoExtension.intSquare(19))
 
print ""
print "--------Native list access reading----------"
lst = ["Hello", "lovely", "world"]
print lst.__class__
DemoExtension.listReadTest(lst)
print "(in JyNI-case see bottom for native outputs)"
print ""
print "--------Native list access writing----------"
l = ["Hello", "lovely", "world"]
print "input: "+str(l)
DemoExtension.listModifyTest(l, 2)
print "output: "+str(l)
print ""
print "--------Set testing----------"
basket = ['apple', 'orange', 'apple', 'pear', 'orange', 'banana']
fruit = set(basket)
#print fruit
#DemoExtension.setTest(fruit);
print fruit
p = 2
DemoExtension.setPopTest(fruit, p);
print "after popping "+str(p)+" elements:"
print fruit

print ""
print "============in JyNI-case expect native output after these lines=================="
print "==(it is a JNI issue that native output is displayed after java output is done)=="