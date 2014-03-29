/*
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014 Stefan Richthofer.  All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */

/*
 * JythonObjects.h
 *
 *  Created on: 18.03.2013, 01:30:18
 *      Author: Stefan Richthofer
 */

#ifndef JYNIOBJECTS_H_
#define JYNIOBJECTS_H_

#include "JyNI.h"

//singletons:
extern jobject JyNone;
extern jobject JyNotImplemented;
extern jobject JyEllipsis;
extern jobject JyEmptyFrozenSet;

extern jclass pyObjectClass;
extern jmethodID pyObjectGetType;
extern jmethodID pyObjectAsString;
extern jfieldID pyObjectType;
extern jmethodID pyObject__getattr__;
extern jmethodID pyObject__findattr__;
extern jmethodID pyObject__setattr__;
extern jmethodID pyObject__repr__;
extern jmethodID pyObject__cmp__;
extern jmethodID pyObject__reduce__;
extern jmethodID pyObject__and__;
extern jmethodID pyObject__or__;
extern jmethodID pyObject__sub__;
extern jmethodID pyObject__xor__;
extern jmethodID pyObject__isub__;
extern jmethodID pyObject__call__;
extern jmethodID pyObject__str__;
extern jmethodID pyObject__finditem__;
//extern jmethodID pyObject__getitem__;
extern jmethodID pyObject__setitem__;
extern jmethodID pyObject__delitem__;
extern jmethodID pyObject__len__;
extern jmethodID pyObject__getslice__;
extern jmethodID pyObject__nonzero__;
extern jmethodID pyObjectGetDict;
extern jmethodID pyObjectFastGetDict;
extern jmethodID pyObjectSetDict;
extern jmethodID pyObjectIsCallable;
extern jmethodID pyObjectHashCode;

extern jclass pyFrameClass;

extern jclass pyBooleanClass;
extern jmethodID pyBooleanConstructor;

extern jclass pyTupleClass;
extern jmethodID pyTupleConstructor;
extern jmethodID pyTupleByPyObjectArrayBooleanConstructor;
extern jmethodID pyTupleSize;
extern jmethodID pyTuplePyGet;
//extern jmethodID pyTupleGetArray;

extern jclass pyListClass;
extern jmethodID pyListConstructor;
extern jmethodID pyListByPyObjectArrayConstructor;
extern jmethodID pyListFromList;
extern jmethodID pyListSize;
extern jmethodID pyListPyGet;
extern jmethodID pyListPySet;
extern jmethodID pyListAppend;
extern jmethodID pyListInsert;
extern jmethodID pyListSort;
extern jmethodID pyListReverse;
extern jmethodID pyListGetArray;
extern jfieldID pyListBackend;

extern jclass pyDictClass;
extern jmethodID pyDictConstructor;
extern jmethodID pyDictByPyObjectArrayConstructor;
//extern jmethodID pyDictGet_PyObject;
//extern jmethodID pyDictGet_PyObjectWithDefault;
//extern jmethodID pyDict__setitem__;
//extern jmethodID pyDict__delitem__;
//extern jmethodID pyDictClear;
//extern jmethodID pyDictSize;

extern jclass pyStringMapClass;
//extern jmethodID pyStringMapClear;

extern jclass pyIntClass;
extern jmethodID pyIntConstructor;
extern jmethodID pyIntAsInt;
extern jmethodID pyIntAsLong;

extern jclass pyLongClass;
extern jmethodID pyLongByBigIntConstructor;
extern jmethodID pyLongByLongConstructor;
extern jmethodID pyLongAsLong;
extern jmethodID pyLongGetLong;
extern jmethodID pyLongGetValue;
extern jmethodID pyLongBit_length;
//extern jmethodID pyLongToString;

extern jclass pyUnicodeClass;
extern jmethodID pyUnicodeByJStringConstructor;
extern jmethodID pyUnicodeAsString;

extern jclass pyStringClass;
extern jmethodID pyStringByJStringConstructor;
extern jmethodID pyStringAsString;

extern jclass pyFloatClass;
extern jmethodID pyFloatByDoubleConstructor;
extern jmethodID pyFloatAsDouble;
extern jfieldID pyFloatTypeField;

extern jclass pyComplexClass;
extern jmethodID pyComplexBy2DoubleConstructor;
extern jfieldID pyComplexRealField;
extern jfieldID pyComplexImagField;

extern jclass pyTypeClass;
extern jmethodID pyTypeGetName;
extern jmethodID pyTypeSetName;
extern jmethodID pyTypeGetBase;
extern jmethodID pyTypeGetBases;
//extern jmethodID pyTypeGetMro;
extern jmethodID pyTypeIsSubType;

extern jclass pySequenceClass;
extern jmethodID pySequenceLen;
extern jmethodID pySequenceGetItem;

extern jclass pyInstanceClass;
extern jmethodID pyInstanceConstructor;
extern jfieldID pyInstanceInstclassField;
extern jfieldID pyInstance__dict__;
extern jmethodID pyInstanceIsSequenceType;

extern jclass pyFileClass;
extern jmethodID pyFileWrite;

extern jclass pyTracebackClass;
extern jmethodID pyTracebackByTracebackFrameConstructor;

extern jclass pyNotImplementedClass;
extern jclass pyNoneClass;
extern jclass pyFileClass;

extern jclass pyModuleClass;
extern jmethodID pyModuleByStringConstructor;
extern jmethodID pyModuleGetDict;
extern jmethodID pyModule__setattr__;
extern jmethodID pyModule__delattr__;

extern jclass pyCellClass;
extern jmethodID pyCellConstructor;
extern jfieldID pyCell_ob_ref;

extern jclass pyClassClass;
extern jmethodID pyClassClassobj___new__;
extern jfieldID pyClass__bases__;
extern jfieldID pyClass__dict__;
extern jfieldID pyClass__name__;

extern jclass pyMethodClass;
extern jmethodID pyMethodConstructor;
extern jfieldID pyMethod__func__;
extern jfieldID pyMethod__self__;
extern jfieldID pyMethodImClass;

extern jclass pyFunctionClass;
extern jmethodID pyFunctionConstructor;
extern jfieldID pyFunction__code__;
extern jmethodID pyFunctionSetCode;
extern jmethodID pyFunctionGetFuncGlobals;
extern jfieldID pyFunction__module__;
extern jmethodID pyFunctionGetFuncDefaults;
extern jmethodID pyFunctionSetFuncDefaults;
extern jfieldID pyFunctionFuncClosure;
extern jfieldID pyFunction__doc__;
extern jfieldID pyFunction__name__;

extern jclass pyClassMethodClass;
extern jclass pyStaticMethodClass;
extern jclass pyMethodDescrClass;
extern jclass pyClassMethodDescrClass;
extern jclass pyDictProxyClass;
extern jclass pyPropertyClass;
extern jclass pyBaseStringClass;
extern jclass pyXRangeClass;
extern jclass pySequenceIterClass;
extern jclass pyFastSequenceIterClass;
extern jclass pyReversedIteratorClass;

extern jclass pyBaseSetClass;
extern jmethodID pyBaseSetSize;
extern jmethodID pyBaseSet_update;
//extern jmethodID pyBaseSetbaseset_union;
extern jmethodID pyBaseSetbaseset_issubset;
extern jmethodID pyBaseSetbaseset_issuperset;
extern jmethodID pyBaseSetbaseset_isdisjoint;
extern jmethodID pyBaseSetbaseset_difference;
extern jmethodID pyBaseSetbaseset_differenceMulti;
extern jmethodID pyBaseSetbaseset_symmetric_difference;
extern jmethodID pyBaseSetbaseset_intersection;
//extern jmethodID pyBaseSetbaseset_copy;
extern jmethodID pyBaseSetbaseset___contains__;

extern jclass pySetClass;
extern jmethodID pySetFromIterableConstructor;
extern jmethodID pySetset_pop;
extern jmethodID pySetset_clear;
extern jmethodID pySetset_discard;
extern jmethodID pySetset_add;
extern jmethodID pySetset_difference_update;
extern jmethodID pySetset_intersection_update;
extern jmethodID pySetset_symmetric_difference_update;

extern jclass pyFrozenSetClass;
extern jmethodID pyFrozenSetFromIterableConstructor;

extern jclass pyEnumerationClass;

extern jclass pySliceClass;
extern jmethodID pySliceFromStartStopStepConstructor;
extern jmethodID pySliceToString;
extern jmethodID pySliceGetStart;
extern jmethodID pySliceGetStop;
extern jmethodID pySliceGetStep;
extern jmethodID pySliceIndicesEx;

extern jclass pyEllipsisClass;
extern jclass pyGeneratorClass;

//extern jclass pyCodeClass;
extern jfieldID pyCode_co_name;

//extern jclass pyBaseCodeClass;
extern jfieldID pyBaseCode_co_argcount;
extern jfieldID pyBaseCode_co_cellvars;
extern jfieldID pyBaseCode_co_filename;
extern jfieldID pyBaseCode_co_firstlineno;
//extern jfieldID pyBaseCode_co_flags;
extern jfieldID pyBaseCode_co_freevars;
extern jfieldID pyBaseCode_co_nlocals;
extern jfieldID pyBaseCode_co_varnames;

extern jclass pyBytecodeClass;
extern jmethodID pyBytecodeConstructor;
//extern jfieldID pyBytecode_co_code;
extern jfieldID pyBytecode_co_consts;
//extern jfieldID pyBytecode_co_lnotab;
extern jfieldID pyBytecode_co_names;
extern jfieldID pyBytecode_co_stacksize;

extern jclass pyTableCodeClass;
//extern jfieldID pyTableCode_co_code;

extern jclass pyCallIterClass;
extern jclass pySuperClass;

extern jclass pyBaseExceptionClass;
extern jmethodID pyBaseExceptionEmptyConstructor;
extern jmethodID pyBaseExceptionSubTypeConstructor;
extern jmethodID pyBaseException__init__;
extern jmethodID pyBaseException__setstate__;
extern jmethodID pyBaseException__unicode__;
extern jfieldID pyBaseExceptionArgs;
extern jmethodID pyBaseExceptionSetArgs;
extern jmethodID pyBaseExceptionGetMessage;
extern jmethodID pyBaseExceptionSetMessage;

extern jclass pyByteArrayClass;
extern jclass pyBufferClass;
extern jclass pyMemoryViewClass;

extern jclass __builtin__Class;
extern jmethodID __builtin__Import;
extern jmethodID __builtin__ImportLevel;

extern jclass threadStateCurExcInfoClass;
extern jfieldID threadStateCurExcInfo_curexc_type;
extern jfieldID threadStateCurExcInfo_curexc_value;
extern jfieldID threadStateCurExcInfo_curexc_traceback;

extern jclass exceptionsClass;
extern jmethodID exceptionsKeyError;
extern jmethodID exceptionsKeyError__str__;
extern jmethodID exceptionsEnvironmentError;
extern jmethodID exceptionsEnvironmentError__init__;
extern jmethodID exceptionsEnvironmentError__str__;
extern jmethodID exceptionsEnvironmentError__reduce__;
extern jmethodID exceptionsSyntaxError;
extern jmethodID exceptionsSyntaxError__init__;
extern jmethodID exceptionsSyntaxError__str__;
extern jmethodID exceptionsSystemExit;
extern jmethodID exceptionsSystemExit__init__;

extern jmethodID exceptionsUnicodeError;
#ifdef Py_USING_UNICODE
extern jmethodID exceptionsGetStart;
extern jmethodID exceptionsGetEnd;
extern jmethodID exceptionsGetString;
extern jmethodID exceptionsGetUnicode;
extern jmethodID exceptionsUnicodeError__init__;
extern jmethodID exceptionsUnicodeEncodeError;
extern jmethodID exceptionsUnicodeEncodeError__init__;
extern jmethodID exceptionsUnicodeEncodeError__str__;
extern jmethodID exceptionsUnicodeDecodeError;
extern jmethodID exceptionsUnicodeDecodeError__init__;
extern jmethodID exceptionsUnicodeDecodeError__str__;
extern jmethodID exceptionsUnicodeTranslateError;
extern jmethodID exceptionsUnicodeTranslateError__init__;
extern jmethodID exceptionsUnicodeTranslateError__str__;
#endif

#endif /* JYNIOBJECTS_H_ */
