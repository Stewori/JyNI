#
#  This File is based on encodings/mbcs.py from CPython 2.7.13.
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

""" Python 'mbcs' Codec for Windows


Cloned by Mark Hammond (mhammond@skippinet.com.au) from ascii.py,
which was written by Marc-Andre Lemburg (mal@lemburg.com).

(c) Copyright CNRI, All Rights Reserved. NO WARRANTY.

"""
# Import them explicitly to cause an ImportError
# on non-Windows systems
# JyNI-note: This won't cause ImportError on non-Windows.
# However An error will be printed and a crash or NPE will occur.
# ToDo: Improve this exception.
from JyNI.JyNI import mbcs_encode, mbcs_decode
# for IncrementalDecoder, IncrementalEncoder, ...
import codecs

### Codec APIs

def encode(input, errors='strict'):
	return mbcs_encode(input, errors)

def decode(input, errors='strict'):
	return mbcs_decode(input, errors, True)

class IncrementalEncoder(codecs.IncrementalEncoder):
	def encode(self, input, final=False):
		return mbcs_encode(input, self.errors)[0]

class IncrementalDecoder(codecs.BufferedIncrementalDecoder):
	_buffer_decode = mbcs_decode

class StreamWriter(codecs.StreamWriter):
	encode = mbcs_encode

class StreamReader(codecs.StreamReader):
	decode = mbcs_decode

### encodings module API

def getregentry():
	return codecs.CodecInfo(
		name='mbcs',
		encode=encode,
		decode=decode,
		incrementalencoder=IncrementalEncoder,
		incrementaldecoder=IncrementalDecoder,
		streamreader=StreamReader,
		streamwriter=StreamWriter,
	)
