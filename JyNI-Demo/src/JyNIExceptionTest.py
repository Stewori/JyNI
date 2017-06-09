'''
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
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


Created on 20.09.2013

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
