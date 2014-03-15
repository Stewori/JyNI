'''
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