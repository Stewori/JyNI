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


Created on 30.06.2013

@author: Stefan Richthofer
'''

import sys
import os
import platform

# This will vastly simplify once Jython 2.7.1 is out and we have
# uname and mac_ver available in Jython:
if os.name == 'java':
	machine = platform.java_ver()[-1][-1]
	if machine == 'amd64':
		machine = 'x86_64'
	elif machine == 'x86':
		machine = 'i686'
	systm = platform.java_ver()[-1][0].lower().replace(' ', '')
	if systm == 'macosx':
		ver = platform.java_ver()[-1][1]
		ver = ver[:ver.rfind('.')]
		buildf = '-'.join((systm, ver, 'intel'))
	else:
		buildf = '-'.join((systm, machine))
else:
	systm = os.uname()[0].lower()
	if systm == 'darwin':
		ver = platform.mac_ver()[0]
		ver = ver[:ver.rfind('.')]
		buildf = '-'.join(('macosx', ver, 'intel'))
	else:
		buildf = '-'.join((systm, os.uname()[-1]))


#Since invalid paths do no harm, we add several possible paths here, where
#DemoExtension.so could be located in various build scenarios. If you use different
#scenarios in parallel, select the one to be used by setting some of the paths as comments.

#built with an IDE in debug mode:
sys.path.append('../../DemoExtension/Debug') #in case you run it from src dir
sys.path.append('./DemoExtension/Debug') #in case you run it from base dir
#built with an IDE in release mode:
sys.path.append('../../DemoExtension/Release') #in case you run it from src dir
sys.path.append('./DemoExtension/Release') #in case you run it from base dir
#built with setup.py:
sys.path.append('../../DemoExtension/build/lib.'+buildf+'-2.7') #in case you run it from src dir
sys.path.append('./DemoExtension/build/lib.'+buildf+'-2.7') #in case you run it from base dir

import DemoExtension


print "--------Access Docstring----------"
print DemoExtension
print DemoExtension.__doc__

# If you run this via Jython, you can uncomment the following lines to prove that Jython is running.
# With these lines this demo becomes a program that neither Jython without JyNI,
# nor ordinary CPython could run.

#print "To prove that Jython is running, we make a java call:"
#from  java.lang import System
#print "System.currentTimeMillis: "+str(System.currentTimeMillis())


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
print fruit
p = 2
DemoExtension.setPopTest(fruit, p)
print "after popping "+str(p)+" elements:"
print fruit

print ""
print "--------Unicode testing----------"
uc = u'a\xac\u1234\u20ac\U00008000'
print type(uc)
print uc
uc2 = DemoExtension.unicodeTest(uc)
print uc2

print ""
print "--------New-style classes---------"

class testnewstyle(str):
	pass

nobj = testnewstyle()
print "Check conversion of new-style class: "+str(DemoExtension.newstyleCheck(nobj))
print "Check new-style is string-subtype: "+str(DemoExtension.newstyleCheckSubtype(nobj, type("")))

# Let's test instances of custom type that is not necessarily new-style class:
from JyNI import PyShadowString
nobj2 = PyShadowString("a", "b")

print "Check conversion of Java-custom class: "+str(DemoExtension.newstyleCheck(nobj2))
print "Check custom class is string-subtype: "+str(DemoExtension.newstyleCheckSubtype(nobj2, type("")))
print "Check custom class is int-subtype (shouldn't be): "+str(DemoExtension.newstyleCheckSubtype(nobj2, type(6)))

print ""
print "======in JyNI-case expect native output after these lines on some consoles======"
print "=====(it is JNI-behavior that native output is displayed after Java output)====="
