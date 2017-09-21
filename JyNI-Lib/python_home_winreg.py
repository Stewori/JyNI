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

from _winreg import HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, QueryValue

key = "SOFTWARE\\Python\\PythonCore\\2.7\\InstallPath"
key32 = "SOFTWARE\\Wow6432Node\\Python\\PythonCore\\2.7\\InstallPath"

def python_home():
	try:
		# On 32 bit Java we prefer a Python in Wow6432Node if it exists:
		from java.lang import System
		if System.getProperty("sun.arch.data.model") == "32":
			try:
				return QueryValue(HKEY_CURRENT_USER, key32)
			except WindowsError:
				pass
			try:
				return QueryValue(HKEY_LOCAL_MACHINE, key32)
			except WindowsError:
				pass
	except:
		pass
	try:
		return QueryValue(HKEY_CURRENT_USER, key)
	except WindowsError:
		pass
	try:
		return QueryValue(HKEY_LOCAL_MACHINE, key)
	except WindowsError:
		pass

if __name__ == '__main__':
	print python_home()[:-1]
