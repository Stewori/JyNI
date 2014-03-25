'''
@author: Stefan Richthofer
'''

import sys

#Include native Tkinter:
sys.path.append('/usr/lib/python2.7/lib-dynload')
sys.path.append('/usr/lib/python2.7/lib-tk')

from Tkinter import *

root = Tk()
txt = StringVar()
txt.set("Hello World!")

def printText():
	print txt.get()

def printTimeStamp():
	from java.lang import System
	print "System.currentTimeMillis: "+str(System.currentTimeMillis())

Label(root, text = "Welcome to JyNI Tkinter-Demo!").pack()
Entry(root, textvariable = txt).pack()
Button(root, text="print text", command = printText).pack()
Button(root, text="print timestamp", command = printTimeStamp).pack()
Button(root, text="Quit", command = root.destroy).pack()

root.mainloop()

print ""
print "===="
print "exit"
print "===="