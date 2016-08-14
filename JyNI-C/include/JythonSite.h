/*
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
 */


/*
 * JythonSite.h
 *
 *  Created on: 18.03.2013, 01:40:10
 *      Author: Stefan Richthofer
 */

#ifndef JYTHONOBJECTS_H_
#define JYTHONOBJECTS_H_

#include "JythonObjects.h"
#include "JyNI.h"

extern jclass pyPyClass;
extern jmethodID pyPyGetThreadState;
extern jmethodID pyPyGetFrame;
extern jmethodID pyPyIsSubClass;
extern jmethodID pyPyWarning;
extern jmethodID pyPyNewString;
extern jmethodID pyPyNewUnicode;
extern jmethodID pyPyNewIntFromInt;
extern jmethodID pyPyNewIntFromLong;
extern jmethodID pyPyNewLongFromBigInt;
extern jmethodID pyPyNewLongFromInt;
extern jmethodID pyPyNewLongFromLong;
extern jmethodID pyPyNewLongFromString;
extern jmethodID pyPyNewFloatFromDouble;
extern jmethodID pyPyWarningStck;
extern jmethodID pyPyExplicitWarning;
extern jfieldID pyPyImportError;
extern jfieldID pyPySystemError;
extern jfieldID pyPyUnicodeError;
extern jfieldID pyPyUnicodeEncodeError;
extern jfieldID pyPyUnicodeDecodeError;
extern jfieldID pyPyUnicodeTranslateError;
extern jfieldID pyPyUnicodeWarning;
extern jmethodID pyPyUnicodeErrorFactory;
extern jmethodID pyPyUnicodeEncodeErrorFactory;
extern jmethodID pyPyUnicodeDecodeErrorFactory;
extern jmethodID pyPyUnicodeTranslateErrorFactory;
extern jmethodID pyPyRaiseUnicodeWarning;
extern jmethodID pyPyMakeClass;
extern jfieldID pyPyIntegerCache;
extern jfieldID pyPyLetters;

//extern jclass pyArrayClass;
//extern jmethodID pyArrayGetTypecode;

extern jclass pySequenceListClass;
extern jmethodID pySequenceListSize;
extern jmethodID pySequenceListPyGet;

extern jclass pyCodecsClass;
extern jmethodID pyCodecsDecode;
extern jmethodID pyCodecsEncode;
extern jmethodID pyCodecsGetDefaultEncoding;
extern jmethodID pyCodecsLookup;
extern jmethodID pyCodecsRegister;
extern jmethodID pyCodecsLookupError;
extern jmethodID pyCodecsRegisterError;

extern jclass pyThreadStateClass;
extern jfieldID pyThreadStateExceptionField;
extern jfieldID pyThreadStateFrameField;
//extern jfieldID pyThreadStateRecursionDepth;
//extern jmethodID pyThreadStateEnterRecursiveCall;
//extern jmethodID pyThreadStateLeaveRecursiveCall;
extern jmethodID pyThreadStateEnterRepr;
extern jmethodID pyThreadStateExitRepr;
extern jmethodID pyThreadStateGetCompareStateDict;

extern jclass pyExceptionClass;
extern jfieldID pyExceptionTypeField;
extern jfieldID pyExceptionValueField;
extern jfieldID pyExceptionTracebackField;
extern jmethodID pyExceptionFullConstructor;
extern jmethodID pyExceptionTypeValueConstructor;
extern jmethodID pyExceptionTypeConstructor;
extern jmethodID pyExceptionNormalize;
extern jmethodID pyExceptionMatch;
extern jmethodID pyExceptionIsExceptionClass;
extern jmethodID pyExceptionIsExceptionInstance;

#endif /* JYTHONOBJECTS_H_ */
