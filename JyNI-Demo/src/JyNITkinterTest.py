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


@author: Stefan Richthofer
'''

import sys

#Include native Tkinter:
sys.path.append('/usr/lib/python2.7/lib-dynload')
sys.path.append('/usr/lib/python2.7/lib-tk')

from Tkinter import Tk#, StringVar, Label, Entry, Button

root = Tk()
# txt = StringVar()
# txt.set("Hello World!")
# 
# def printText():
# 	print txt.get()
# 
# def printTimeStamp():
# 	from java.lang import System
# 	print "System.currentTimeMillis: "+str(System.currentTimeMillis())
# 
# Label(root, text = "Welcome to JyNI Tkinter-Demo!").pack()
# Entry(root, textvariable = txt).pack()
# Button(root, text = "print text", command = printText).pack()
# Button(root, text = "print timestamp", command = printTimeStamp).pack()
# Button(root, text = "Quit", command = root.destroy).pack()

root.mainloop()

print ""
print "===="
print "exit"
print "===="
