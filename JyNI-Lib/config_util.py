#
#  Copyright of JyNI:
#  Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
#  All rights reserved.
#
#
#  Copyright of Python and Jython:
#  Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
#  2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
#  Python Software Foundation.
#  All rights reserved.
#
#
#  This file is part of JyNI.
#
#  JyNI is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as
#  published by the Free Software Foundation, either version 3 of
#  the License, or (at your option) any later version.
#
#  JyNI is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with JyNI.  If not, see <http://www.gnu.org/licenses/>.

import os, sys

def autoconfig_dynload_path():
	'''Automatically adds the dynamic library folder of the system's
	installed CPython to sys.path.
	'''
	if os.name == 'java':
		if os._name == 'posix':
			lib_dynload = '/usr/lib/python2.7/lib-dynload'
		elif os._name in ("nt", "ce"):
			import python_home_winreg
			py_home = python_home_winreg.python_home()
			if py_home is None:
				return
			lib_dynload = py_home+'DLLs'
		else:
			return
		if not lib_dynload in sys.path:
			sys.path.append(lib_dynload)

def autoconfig_lib_tk_path():
	'''Automatically adds the lib-tk folder of the system's
	installed CPython to sys.path.
	'''
	if os.name == 'java':
		if os._name == 'posix':
			lib_tk = '/usr/lib/python2.7/lib-tk'
		elif os._name in ("nt", "ce"):
			import python_home_winreg
			py_home = python_home_winreg.python_home()
			if py_home is None:
				return
			lib_tk = py_home+'Lib\\lib-tk'
		else:
			return
		if not lib_tk in sys.path:
			sys.path.append(lib_tk)

def _monkeypatch_os_path_for_Tk(py_home):
	'''For internal use only. Do not call.'''
	_os_path_join = os.path.join
	def _join(a, *p):
		# This strange-looking check assumes that a PyShadowString
		# has been inserted into sys.prefix. We use the
		# PyShadowString magic here to assert that only very
		# specific modules (i.e. FixTk) are affected by this hack.
		if sys.prefix==a and sys.prefix==py_home:
			return _os_path_join(py_home, *p)
		else:
			return _os_path_join(a, *p)
	os.path.join = _join

def _register_mbcs_encoding():
	'''For internal use only. Do not call.'''
	import mbcs_JyNI, codecs
	def searchfunc_mbcs(encoding):
		if encoding == 'mbcs':
			return mbcs_JyNI.getregentry()
		else:
			return None
	codecs.register(searchfunc_mbcs)

def _patch_os_environ():
	'''For internal use only. Do not call.'''
	from JyNI import JyNI

	def unsetenv(key):
		JyNI.putenv(key, "")

# This variant fails with read-only attribute error:
# 	if os._name in ('os2', 'nt'):  # Where Env Var Names Must Be UPPERCASE
# 		# But we store them as upper case
# 		def __setitem__(self, key, item):
# 			JyNI.putenv(key, item)
# 			self.data[key.upper()] = item
# 		def __delitem__(self, key):
# 			unsetenv(key)
# 			del self.data[key.upper()]
# 		def clear(self):
# 			for key in self.data.keys():
# 				unsetenv(key)
# 				del self.data[key]
# 		def pop(self, key, *args):
# 			unsetenv(key)
# 			return self.data.pop(key.upper(), *args)
# 
# 	else:  # Where Env Var Names Can Be Mixed Case
# 		def __setitem__(self, key, item):
# 			JyNI.putenv(key, item)
# 			self.data[key] = item
# 		def __delitem__(self, key):
# 			unsetenv(key)
# 			del self.data[key]
# 		def clear(self):
# 			for key in self.data.keys():
# 				unsetenv(key)
# 				del self.data[key]
# 		def pop(self, key, *args):
# 			unsetenv(key)
# 			return self.data.pop(key, *args)
# 
# 	os.environ.__setitem__ = __setitem__
# 	os.environ.__delitem__ = __delitem__
# 	os.environ.clear = clear
# 	os.environ.pop = pop


# Variant that patches entire os.environ:
	import UserDict
	if os._name in ('os2', 'nt'):  # Where Env Var Names Must Be UPPERCASE
		# But we store them as upper case
		class _Environ(UserDict.IterableUserDict):
			def __init__(self, environ):
				UserDict.UserDict.__init__(self)
				data = self.data
				for k, v in environ.items():
					data[k.upper()] = v
			def __setitem__(self, key, item):
				JyNI.putenv(key, item)
				self.data[key.upper()] = item
			def __getitem__(self, key):
				return self.data[key.upper()]
			try:
				unsetenv
			except NameError:
				def __delitem__(self, key):
					del self.data[key.upper()]
			else:
				def __delitem__(self, key):
					unsetenv(key)
					del self.data[key.upper()]
				def clear(self):
					for key in self.data.keys():
						unsetenv(key)
						del self.data[key]
				def pop(self, key, *args):
					unsetenv(key)
					return self.data.pop(key.upper(), *args)
			def has_key(self, key):
				return key.upper() in self.data
			def __contains__(self, key):
				return key.upper() in self.data
			def get(self, key, failobj=None):
				return self.data.get(key.upper(), failobj)
			def update(self, dict=None, **kwargs):
				if dict:
					try:
						keys = dict.keys()
					except AttributeError:
						# List of (key, value)
						for k, v in dict:
							self[k] = v
					else:
						# got keys
						# cannot use items(), since mappings
						# may not have them.
						for k in keys:
							self[k] = dict[k]
				if kwargs:
					self.update(kwargs)
			def copy(self):
				return dict(self)

	else:  # Where Env Var Names Can Be Mixed Case
		class _Environ(UserDict.IterableUserDict):
			def __init__(self, environ):
				UserDict.UserDict.__init__(self)
				self.data = environ
			def __setitem__(self, key, item):
				JyNI.putenv(key, item)
				self.data[key] = item
			def update(self,  dict=None, **kwargs):
				if dict:
					try:
						keys = dict.keys()
					except AttributeError:
						# List of (key, value)
						for k, v in dict:
							self[k] = v
					else:
						# got keys
						# cannot use items(), since mappings
						# may not have them.
						for k in keys:
							self[k] = dict[k]
				if kwargs:
					self.update(kwargs)
			try:
				unsetenv
			except NameError:
				pass
			else:
				def __delitem__(self, key):
					unsetenv(key)
					del self.data[key]
				def clear(self):
					for key in self.data.keys():
						unsetenv(key)
						del self.data[key]
				def pop(self, key, *args):
					unsetenv(key)
					return self.data.pop(key, *args)
			def copy(self):
				return dict(self)

	os.environ = _Environ(os.environ)
