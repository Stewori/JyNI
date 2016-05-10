#
#  This File is based on ctypes/_endian.py from CPython 2.7.8.
#  It has been modified to suit JyNI needs.
#
#  Copyright of JyNI:
#  Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
#  All rights reserved.
#
#
#  Copyright of Python and Jython:
#  Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
#  2010, 2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
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


#  JyNI-Note: So far this file was actually not modified apart from adding
#  this header. It is bundled with JyNI for convenience, such that ctypes
#  can work out of the box.


######################################################################
#  This file should be kept compatible with Python 2.3, see PEP 291. #
######################################################################
import sys
from ctypes import *

_array_type = type(Array)

def _other_endian(typ):
    """Return the type with the 'other' byte order.  Simple types like
    c_int and so on already have __ctype_be__ and __ctype_le__
    attributes which contain the types, for more complicated types
    arrays and structures are supported.
    """
    # check _OTHER_ENDIAN attribute (present if typ is primitive type)
    if hasattr(typ, _OTHER_ENDIAN):
        return getattr(typ, _OTHER_ENDIAN)
    # if typ is array
    if isinstance(typ, _array_type):
        return _other_endian(typ._type_) * typ._length_
    # if typ is structure
    if issubclass(typ, Structure):
        return typ
    raise TypeError("This type does not support other endian: %s" % typ)

class _swapped_meta(type(Structure)):
    def __setattr__(self, attrname, value):
        if attrname == "_fields_":
            fields = []
            for desc in value:
                name = desc[0]
                typ = desc[1]
                rest = desc[2:]
                fields.append((name, _other_endian(typ)) + rest)
            value = fields
        super(_swapped_meta, self).__setattr__(attrname, value)

################################################################

# Note: The Structure metaclass checks for the *presence* (not the
# value!) of a _swapped_bytes_ attribute to determine the bit order in
# structures containing bit fields.

if sys.byteorder == "little":
    _OTHER_ENDIAN = "__ctype_be__"

    LittleEndianStructure = Structure

    class BigEndianStructure(Structure):
        """Structure with big endian byte order"""
        __metaclass__ = _swapped_meta
        _swappedbytes_ = None

elif sys.byteorder == "big":
    _OTHER_ENDIAN = "__ctype_le__"

    BigEndianStructure = Structure

    class LittleEndianStructure(Structure):
        """Structure with little endian byte order"""
        __metaclass__ = _swapped_meta
        _swappedbytes_ = None

else:
    raise RuntimeError("Invalid byteorder")
