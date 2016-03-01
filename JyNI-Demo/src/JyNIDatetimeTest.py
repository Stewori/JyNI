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


Created on 21.09.2013

@author: Stefan Richthofer
'''
import sys
#    exclude the following line to switch back to Jython's datetime:
sys.path.insert(0, '/usr/lib/python2.7/lib-dynload')

#    This path should be okay for linux systems and maybe some bsd-oids;
#    if it does not work (i.e. doc below reads different than
#    "Fast implementation of the datetime type."), check that the path is
#    correct and the original datetime module is present there.

import datetime

print "datetime successfully imported."
print ""
#    you can see that CPython's datetime is used, if the doc reads
#    "Fast implementation of the datetime type."
print "-----datetime doc-----"
print datetime.__doc__
print "----------------------"
print ""

#print datetime.__findattr__("datetime")
print datetime.__name__
dt = datetime.datetime
#print dt
now = datetime.datetime(2013, 11, 3, 20, 30, 45)

print now
print repr(now)
print type(now)
#print now.year, now.month, now.day
# print now.hour, now.minute, now.second
# print now.microsecond
print "--------------------------------------"