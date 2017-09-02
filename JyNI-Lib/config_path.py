import os, sys

def auto_config():
	if os.name == 'java':
		if os._name == 'posix':
			lib_dynload = '/usr/lib/python2.7/lib-dynload'
		elif os._name in ("nt", "ce"):
			import python_home_winreg
			lib_dynload = python_home_winreg.python_home()+'DLLs'
		else:
			return
		if not lib_dynload in sys.path:
			sys.path.insert(0, lib_dynload)
