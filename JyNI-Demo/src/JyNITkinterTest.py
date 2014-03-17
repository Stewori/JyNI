'''
@author: Stefan Richthofer
'''

import sys

#Include native Tkinter:
sys.path.append('/usr/lib/python2.7/lib-dynload')
sys.path.append('/usr/lib/python2.7/lib-tk')

from Tkinter import *

def printTest():
	print "test"
	
def printTest2():
	print "test2"

def printTimeStamp():
	from java.lang import System
	print "System.currentTimeMillis: "+str(System.currentTimeMillis())

root = Tk()
Label(root, text = "Welcome to JyNI Tkinter-Demo!").pack()
Button(root, text="print \"test\"", command=printTest).pack()
Button(root, text="print timestamp", command=printTimeStamp).pack()
Button(root, text="print \"test2\"", command=printTest2).pack()
Button(root, text="Quit", command=root.destroy).pack()

root.mainloop()

print ""
print "===="
print "exit"
print "===="