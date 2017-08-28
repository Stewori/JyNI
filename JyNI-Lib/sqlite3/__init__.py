#-*- coding: ISO-8859-1 -*-
#
#  This File is based on sqlite3/__init__.py from CPython 2.7.9.
#  It has been modified to suit JyNI needs.
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


# pysqlite2/__init__.py: the pysqlite2 package.
#
# Copyright (C) 2005 Gerhard Häring <gh@ghaering.de>
#
# This file is part of pysqlite.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.


# JyNI-adjustment:
# Usually Jython does not have lib-dynload on its path, so ensure it here
import config_path
config_path.auto_config()
# lib_dynload = '/usr/lib/python2.7/lib-dynload'
# #lib_dynload = '/data/workspace/linux/Python-2.7.11/build/lib.linux-x86_64-2.7'
# import sys
# if not lib_dynload in sys.path:
#     sys.path.append(lib_dynload)

from dbapi2 import *
