/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
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
 * JyNI_JNI.h
 *
 *  Created on: 17.08.2016
 *      Author: Stefan Richthofer
 */

#include <jni.h>

#ifndef INCLUDE_JYNI_JNI_H_
#define INCLUDE_JYNI_JNI_H_

#define pack_object                "java/lang/Object"
#define pack_string                "java/lang/String"
#define pack_class                 "java/lang/Class"
#define pack_bigInt                "java/math/BigInteger"
#define pack_arrayList             "java/util/ArrayList"
#define pack_collection            "java/util/Collection"
#define pack_list                  "java/util/List"
#define pack_set                   "java/util/Set"
#define pack_JyNI                  "JyNI/JyNI"
#define pack_JyTState              "JyNI/JyTState"
#define pack_JyNIDictNextResult    "JyNI/JyNIDictNextResult"
#define pack_JyNISetNextResult     "JyNI/JyNISetNextResult"
#define pack_JyList                "JyNI/JyList"
#define pack_JySet                 "JyNI/JySet"
#define pack_JyLock                "JyNI/JyLock"
#define pack_NativeAction          "JyNI/JyReferenceMonitor$NativeAction"
#define pack_JyReferenceMonitor    "JyNI/JyReferenceMonitor"
#define pack_pyCPeer               "JyNI/PyCPeer"
#define pack_pyCPeerGC             "JyNI/gc/PyCPeerGC"
#define pack_pyCPeerTypeGC         "JyNI/gc/PyCPeerTypeGC"
#define pack_pyDictCPeer           "JyNI/PyDictionaryCPeer"
#define pack_pyTupleCPeer          "JyNI/PyTupleCPeer"
#define pack_pyCPeerType           "JyNI/PyCPeerType"
#define pack_jyGCHead              "JyNI/gc/JyGCHead"
#define pack_pyObjectGCHead        "JyNI/gc/PyObjectGCHead"
#define pack_traversableGCHead     "JyNI/gc/TraversableGCHead"
#define pack_cPeer                 "JyNI/CPeerInterface"
#define pack_cPeerNativeDelegate   "JyNI/CPeerNativeDelegateSubtype"
#define pack_pyCMethodDef          "JyNI/CMethodDef"
#define pack_pyCFunction           "JyNI/PyCFunction"
#define pack_JyNIGlobalRef         "JyNI/JyNIGlobalRef"
#define pack_pyPy                  "org/python/core/Py"
#define pack_pyTraceback           "org/python/core/PyTraceback"
#define pack_pyException           "org/python/core/PyException"
#define pack_pyThreadState         "org/python/core/ThreadState"
#define pack_pyNone                "org/python/core/PyNone"
#define pack_pyObject              "org/python/core/PyObject"
#define pack_pyInstance            "org/python/core/PyInstance"
#define pack_pyFrame               "org/python/core/PyFrame"
#define pack_pyBoolean             "org/python/core/PyBoolean"
#define pack_pyInt                 "org/python/core/PyInteger"
#define pack_pyLong                "org/python/core/PyLong"
#define pack_pyUnicode             "org/python/core/PyUnicode"
#define pack_pyString              "org/python/core/PyString"
#define pack_pyFloat               "org/python/core/PyFloat"
#define pack_pyComplex             "org/python/core/PyComplex"
#define pack_pyTuple               "org/python/core/PyTuple"
#define pack_pyList                "org/python/core/PyList"
#define pack_pyAbstractDict        "org/python/core/AbstractDict"
#define pack_pyDict                "org/python/core/PyDictionary"
#define pack_pyStringMap           "org/python/core/PyStringMap"
#define pack_pySequence            "org/python/core/PySequence"
#define pack_pySequenceList        "org/python/core/PySequenceList"
#define pack_pyType                "org/python/core/PyType"
#define pack_pyCodecs              "org/python/core/codecs"
#define pack_pyNotImplemented      "org/python/core/PyNotImplemented"
#define pack_pyFile                "org/python/core/PyFile"
#define pack_pyModule              "org/python/core/PyModule"
#define pack_pyCell                "org/python/core/PyCell"
#define pack_pyClass               "org/python/core/PyClass"
#define pack_pyMethod              "org/python/core/PyMethod"
#define pack_pyFunction            "org/python/core/PyFunction"
#define pack_pyClassMethod         "org/python/core/PyClassMethod"
#define pack_pyStaticMethod        "org/python/core/PyStaticMethod"
#define pack_pyBuiltinCallableInfo "org/python/core/PyBuiltinCallable$Info"
#define pack_pyBuiltinCallable     "org/python/core/PyBuiltinCallable"
#define pack_pyDescr               "org/python/core/PyDescriptor"
#define pack_pyMethodDescr         "org/python/core/PyMethodDescr"
#define pack_pyClassMethodDescr    "org/python/core/PyClassMethodDescr"
#define pack_pyDictProxy           "org/python/core/PyDictProxy"
#define pack_pyProperty            "org/python/core/PyProperty"
#define pack_pySequenceIter        "org/python/core/PySequenceIter"
#define pack_baseSet               "org/python/core/BaseSet"
#define pack_pySet                 "org/python/core/PySet"
#define pack_pyFrozenSet           "org/python/core/PyFrozenSet"
#define pack_pySlice               "org/python/core/PySlice"
#define pack_pyEllipsis            "org/python/core/PyEllipsis"
#define pack_pyWeakReference       "org/python/modules/_weakref/ReferenceType"
#define pack_pyWeakProxy           "org/python/modules/_weakref/ProxyType"
#define pack_pyWeakCallableProxy   "org/python/modules/_weakref/CallableProxyType"
#define pack_pyCode                "org/python/core/PyCode"
#define pack_pyBaseCode            "org/python/core/PyBaseCode"
#define pack_pyBytecode            "org/python/core/PyBytecode"
#define pack_pyTableCode           "org/python/core/PyTableCode"
#define pack_GlobalRef             "org/python/modules/_weakref/GlobalRef"
#define pack_referenceBackend      "org/python/modules/_weakref/ReferenceBackend"
#define pack_AbstractReference     "org/python/modules/_weakref/AbstractReference"
#define pack_pyBaseException       "org/python/core/PyBaseException"
#define pack___builtin__           "org/python/core/__builtin__"
#define pack_imp                   "org/python/core/imp"
#define pack_exceptions            "org/python/core/exceptions"
#define pack_structModule          "org/python/modules/struct"
#define pack_structModuleFormatDef "org/python/modules/struct$FormatDef"
//#define pack_pyArray             "org/python/core/PyArray"
//#define pack_pyBaseString        "org/python/core/PyBaseString"
//#define pack_pyXRange            "org/python/core/PyXRange"
//#define pack_pyFastSequenceIter  "org/python/core/PyFastSequenceIter"
//#define pack_pyReversedIterator  "org/python/core/PyReversedIterator"
//#define pack_pyEnumeration       "org/python/core/PyEnumerate"
//#define pack_pyGenerator         "org/python/core/PyGenerator"
//#define pack_pyCallIter          "org/python/core/PyCallIter"
//#define pack_pySuper             "org/python/core/PySuper"
//#define pack_pyByteArray         "org/python/core/PyByteArray"
//#define pack_pyBuffer            "org/python/core/PyBuffer"
//#define pack_pyMemoryView        "org/python/core/PyMemoryView"


/* JNI IDs for Jython-stuff: */

/* singletons: */
extern JavaVM* java;
extern jweak JyTrue;
extern jweak JyFalse;
extern jweak JyNone;
extern jweak JyNotImplemented;
extern jweak JyEllipsis;
extern jweak JyEmptyFrozenSet;
extern jweak JyEmptyString;
extern jweak JyEmptyUnicode;
extern jweak JyEmptyTuple;
extern jweak JyEmptyStringArray;
extern jweak JyEmptyPyObjectArray;

extern jclass objectClass;
extern jmethodID object_toString;
extern jmethodID object_getClass;

extern jclass classClass;
extern jmethodID class_equals;

extern jclass arrayListClass;
extern jmethodID arrayList_Constructor;
extern jmethodID list_add;
extern jmethodID collection_size;

extern jclass stringClass;
extern jmethodID string_toUpperCase;
extern jmethodID string_fromBytesAndCharsetNameConstructor;
extern jmethodID string_getBytes;
extern jmethodID string_intern;

extern jclass bigIntClass;
extern jmethodID bigInt_fromByteArrayConstructor;
extern jmethodID bigInt_toByteArray;

extern jclass JyNIClass;
extern jmethodID JyNI_setNativeHandle;
extern jmethodID JyNI_registerNativeStaticJyGCHead;
extern jmethodID JyNI_getNativeStaticJyGCHead;
extern jmethodID JyNI_lookupNativeHandle;
extern jmethodID JyNI_clearNativeHandle;
extern jmethodID JyNI_lookupCPeerFromHandle;
extern jmethodID JyNI_getDLOpenFlags;
extern jmethodID JyNI_getDLVerbose;
extern jmethodID JyNI_getJyObjectByName;
extern jmethodID JyNI_getPyObjectByName;
extern jmethodID JyNI__PyImport_FindExtension;
extern jmethodID JyNI_getPyType;
extern jmethodID JyNI_getNativeAvailableKeysAndValues;
extern jmethodID JyNI_getPyDictionary_Next;
extern jmethodID JyNI_getPySet_Next;
extern jmethodID JyNI_PyImport_GetModuleDict;
extern jmethodID JyNI_PyImport_AddModule;
extern jmethodID JyNI_PyImport_ImportModuleNoBlock;
extern jmethodID JyNI_JyNI_GetModule;
extern jmethodID JyNI_slice_compare;
extern jmethodID JyNI_printPyLong;
extern jmethodID JyNI_lookupNativeHandles;
extern jmethodID JyNI_prepareKeywordArgs;
extern jmethodID JyNI_getCurrentThreadID;
extern jmethodID JyNI_JyNI_pyCode_co_code;
extern jmethodID JyNI_JyNI_pyCode_co_flags;
extern jmethodID JyNI_JyNI_pyCode_co_lnotab;
extern jmethodID JyNI_jPrint;
extern jmethodID JyNI_jPrintLong;
extern jmethodID JyNI_jGetHash;
extern jmethodID JyNI_makeGCHead;
extern jmethodID JyNI_makeStaticGCHead;
extern jmethodID JyNI_gcDeletionReport;
extern jmethodID JyNI_waitForCStubs;
extern jmethodID JyNI_addJyNICriticalObject;
extern jmethodID JyNI_removeJyNICriticalObject;
extern jmethodID JyNI_suspendPyInstanceFinalizer;
extern jmethodID JyNI_restorePyInstanceFinalizer;
extern jmethodID JyNI_createWeakReferenceFromNative;
extern jmethodID JyNI_createProxyFromNative;
extern jmethodID JyNI_createCallableProxyFromNative;
extern jmethodID JyNI_getGlobalRef;
extern jmethodID JyNI_getTypeNameForNativeConversion;
extern jmethodID JyNI_getTypeOldStyleParent;
extern jmethodID JyNI_getJythonBuiltins;
extern jmethodID JyNI_getJythonLocals;
extern jmethodID JyNI_getJythonGlobals;
extern jmethodID JyNI_getJythonFrame;
extern jmethodID JyNI_getPlatform;

extern jclass JyTStateClass;
extern jmethodID JyTState_setRecursionLimit;
extern jmethodID JyTState_prepareNativeThreadState;
extern jfieldID JyTState_nativeRecursionLimitField;

extern jclass JyNIDictNextResultClass;
extern jfieldID JyNIDictNextResult_keyField;
extern jfieldID JyNIDictNextResult_valueField;
extern jfieldID JyNIDictNextResult_newIndexField;
extern jfieldID JyNIDictNextResult_keyHandleField;
extern jfieldID JyNIDictNextResult_valueHandleField;

extern jclass JyNISetNextResultClass;
extern jfieldID JyNISetNextResult_keyField;
extern jfieldID JyNISetNextResult_newIndexField;
extern jfieldID JyNISetNextResult_keyHandleField;

extern jmethodID JyNI_exceptionByName;
extern jmethodID JyNI_JyErr_InsertCurExc;
extern jmethodID JyNI_JyErr_PrintEx;
extern jmethodID JyNI_PyErr_ExceptionMatches;
extern jmethodID JyNI_JyNI_PyTraceBack_Here;
extern jmethodID JyNI_PyErr_WriteUnraisable;

extern jclass JyListClass;
extern jmethodID JyList_fromBackendHandleConstructor;

extern jclass JySetClass;
extern jmethodID JySet_fromBackendHandleConstructor;

extern jclass JyLockClass;
extern jmethodID JyLock_Constructor;
extern jmethodID JyLock_acquire;
extern jmethodID JyLock_release;

extern jclass NativeActionClass;
extern jmethodID NativeAction_constructor;
extern jfieldID NativeAction_actionField;
extern jfieldID NativeAction_objField;
extern jfieldID NativeAction_nativeRef1Field;
extern jfieldID NativeAction_nativeRef2Field;
extern jfieldID NativeAction_cTypeNameField;
extern jfieldID NativeAction_cMethodField;
extern jfieldID NativeAction_cLineField;
extern jfieldID NativeAction_cFileField;

extern jclass JyReferenceMonitorClass;
extern jmethodID JyRefMonitorMakeDebugInfo;
extern jmethodID JyReferenceMonitor_addNativeAction;

extern jclass pyCPeerClass;
extern jmethodID pyCPeer_Constructor;
extern jfieldID pyCPeer_objectHandleField;

extern jclass pyCPeerGCClass;
extern jmethodID pyCPeerGC_Constructor;

extern jclass pyCPeerTypeGCClass;
extern jmethodID pyCPeerTypeGC_Constructor;
extern jmethodID pyCPeerTypeGC_ConstructorSubtype;

extern jclass pyDictCPeerClass;
extern jclass pyTupleCPeerClass;
extern jmethodID pyTupleCPeer_Constructor;

extern jclass jyGCHeadClass;
extern jmethodID traversableGCHead_setLinks;
extern jmethodID traversableGCHead_setLink;
extern jmethodID traversableGCHead_insertLink;
extern jmethodID traversableGCHead_clearLink;
extern jmethodID traversableGCHead_clearLinksFromIndex;
extern jmethodID traversableGCHead_ensureSize;
extern jmethodID pyObjectGCHead_setPyObject;
extern jmethodID jyGCHead_getHandle;

extern jclass cPeerInterface;
extern jclass cPeerNativeDelegateInterface;
extern jmethodID super__call__;
extern jmethodID super__findattr_ex__;
extern jmethodID super__setattr__;
extern jmethodID super__str__;
extern jmethodID super__repr__;
extern jmethodID super__finditem__;
extern jmethodID super__setitem__;
extern jmethodID super__delitem__;
extern jmethodID super__len__;
extern jmethodID super_toString;

extern jclass pyCPeerTypeClass;
extern jmethodID pyCPeerType_withNameAndDictConstructor;
extern jmethodID pyCPeerType_withNameDictTypeConstructor;
extern jfieldID pyCPeerType_objectHandleField;
extern jfieldID pyCPeerType_refHandleField;

extern jclass pyPyClass;
extern jmethodID pyPy_getThreadState;
extern jmethodID pyPy_getFrame;
extern jmethodID pyPy_isSubClass;
extern jmethodID pyPy_warning;
extern jmethodID pyPy_newString;
extern jmethodID pyPy_newUnicode;
extern jmethodID pyPy_newIntFromInt;
extern jmethodID pyPy_newIntFromLong;
extern jmethodID pyPy_newLongFromBigInt;
extern jmethodID pyPy_newLongFromInt;
extern jmethodID pyPy_newLongFromLong;
extern jmethodID pyPy_newLongFromString;
extern jmethodID pyPy_newFloatFromDouble;
extern jmethodID pyPy_warningStck;
extern jmethodID pyPy_explicitWarning;
extern jfieldID pyPy_ImportErrorField;
extern jfieldID pyPy_SystemErrorField;
extern jfieldID pyPy_UnicodeErrorField;
extern jfieldID pyPy_UnicodeEncodeErrorField;
extern jfieldID pyPy_UnicodeDecodeErrorField;
extern jfieldID pyPy_UnicodeTranslateErrorField;
extern jfieldID pyPy_UnicodeWarningField;
extern jmethodID pyPy_UnicodeErrorFactory;
extern jmethodID pyPy_UnicodeEncodeErrorFactory;
extern jmethodID pyPy_UnicodeDecodeErrorFactory;
extern jmethodID pyPy_UnicodeTranslateErrorFactory;
extern jmethodID pyPy_raiseUnicodeWarning;
extern jmethodID pyPy_makeClass;
extern jfieldID pyPy_integerCacheField;
extern jfieldID pyPy_lettersField;

extern jclass pyCodecsClass;
extern jmethodID pyCodecs_decode;
extern jmethodID pyCodecs_encode;
extern jmethodID pyCodecs_getDefaultEncoding;
extern jmethodID pyCodecs_lookup;
extern jmethodID pyCodecs_register;
extern jmethodID pyCodecs_lookup_error;
extern jmethodID pyCodecs_register_error;

extern jclass pyThreadStateClass;
extern jfieldID pyThreadState_exceptionField;
extern jfieldID pyThreadState_frameField;
extern jmethodID pyThreadState_enterRepr;
extern jmethodID pyThreadState_exitRepr;
extern jmethodID pyThreadState_getCompareStateDict;

extern jclass pyExceptionClass;
extern jfieldID pyException_typeField;
extern jfieldID pyException_valueField;
extern jfieldID pyException_tracebackField;
extern jmethodID pyException_fullConstructor;
extern jmethodID pyException_typeValueConstructor;
extern jmethodID pyException_typeConstructor;
extern jmethodID pyException_normalize;
extern jmethodID pyException_match;
extern jmethodID pyException_isExceptionClass;
extern jmethodID pyException_isExceptionInstance;

extern jclass pyObjectClass;
extern jmethodID pyObject_getType;
extern jmethodID pyObject_asString;
extern jfieldID pyObject_TYPEField;
extern jmethodID pyObject___getattr__;
extern jmethodID pyObject___findattr__;
extern jmethodID pyObject___setattr__;
extern jmethodID pyObject___repr__;
extern jmethodID pyObject___cmp__;
extern jmethodID pyObject___reduce__;
extern jmethodID pyObject___and__;
extern jmethodID pyObject___or__;
extern jmethodID pyObject___sub__;
extern jmethodID pyObject___xor__;
extern jmethodID pyObject___isub__;
extern jmethodID pyObject___call__;
extern jmethodID pyObject___str__;
extern jmethodID pyObject___finditem__;
extern jmethodID pyObject___setitem__;
extern jmethodID pyObject___delitem__;
extern jmethodID pyObject___get__;
extern jmethodID pyObject___contains__;
extern jmethodID pyObject___len__;
extern jmethodID pyObject___getslice__;
extern jmethodID pyObject___nonzero__;
extern jmethodID pyObject___iter__;
extern jmethodID pyObject___iternext__;
extern jmethodID pyObject__cmp;
extern jmethodID pyObject_getDict;
extern jmethodID pyObject_fastGetDict;
extern jmethodID pyObject_setDict;
extern jmethodID pyObject_isCallable;
extern jmethodID object_hashCode;

extern jclass pyFrameClass;
extern jmethodID pyFrame_Constructor;
extern jfieldID pyFrame_f_backField;
extern jfieldID pyFrame_f_builtinsField;
extern jfieldID pyFrame_f_linenoField;

extern jclass pyBooleanClass;
extern jfieldID pyBoolean_valueField;

extern jclass pyTupleClass;
extern jmethodID pyTuple_Constructor;
extern jmethodID pyTuple_byPyObjectArrayBooleanConstructor;

extern jclass pyListClass;
extern jmethodID pyList_Constructor;
extern jmethodID pyList_byPyObjectArrayConstructor;
extern jmethodID pyList_fromList;
extern jmethodID pyList_append;
extern jmethodID pyList_insert;
extern jmethodID pyList_sort;
extern jmethodID pyList_reverse;
extern jmethodID pyList_getArray;
extern jfieldID pyList_listField;

extern jclass pyAbstractDictClass;
extern jmethodID pyAbstractDict_copy;
extern jmethodID pyAbstractDict_merge;
extern jmethodID pyAbstractDict_mergeFromSeq;

extern jclass pyDictClass;
extern jmethodID pyDict_Constructor;

extern jclass pyStringMapClass;

extern jclass pyIntClass;
extern jmethodID pyInt_Constructor;
extern jmethodID pyInt_getValue;

extern jclass pyLongClass;
extern jmethodID pyLong_byBigIntConstructor;
extern jmethodID pyLong_byLongConstructor;
extern jmethodID pyLong_asLong;
extern jmethodID pyLong_getLong;
extern jmethodID pyLong_getValue;
extern jmethodID pyLong_bit_length;

extern jclass pyUnicodeClass;
extern jclass pyStringClass;

extern jclass pyFloatClass;
extern jmethodID pyFloat_byDoubleConstructor;
extern jmethodID pyFloat_asDouble;
extern jfieldID pyFloat_TYPEField;

extern jclass pyComplexClass;
extern jmethodID pyComplex_by2DoubleConstructor;
extern jfieldID pyComplexRealField;
extern jfieldID pyComplexImagField;

extern jclass pyTypeClass;
extern jmethodID pyType_getName;
extern jmethodID pyType_setName;
extern jmethodID pyType_getBase;
extern jmethodID pyType_getBases;
extern jmethodID pyType_getMro;
extern jmethodID pyType_isSubType;
extern jmethodID pyType_lookup;
extern jfieldID pyType_mroField;
extern jfieldID pyType_nameField;

extern jclass pySequenceClass;
extern jmethodID pySequence_pyget;
//extern jmethodID pySequence_pyset;

extern jclass pyInstanceClass;
extern jmethodID pyInstance_Constructor;
extern jfieldID pyInstance_instclassField;
extern jfieldID pyInstance___dict__Field;
extern jmethodID pyInstance_isSequenceType;

extern jclass pyFileClass;
extern jmethodID pyFile_write;
extern jmethodID pyFile_fd;

extern jclass pyTracebackClass;
extern jmethodID pyTraceback_byTracebackFrameConstructor;

extern jclass pyNotImplementedClass;
extern jclass pyNoneClass;
extern jclass pyFileClass;

extern jclass pyModuleClass;
extern jmethodID pyModule_byStringConstructor;
extern jmethodID pyModule_getDict;
extern jmethodID pyModule___setattr__;
extern jmethodID pyModule___delattr__;

extern jclass pyCellClass;
extern jmethodID pyCell_Constructor;
extern jfieldID pyCell_ob_refField;

extern jclass pyClassClass;
extern jmethodID pyClass_classobj___new__;
extern jfieldID pyClass___bases__Field;
extern jfieldID pyClass___dict__Field;
extern jfieldID pyClass___name__Field;

extern jclass pyMethodClass;
extern jmethodID pyMethod_Constructor;
extern jfieldID pyMethod___func__Field;
extern jfieldID pyMethod___self__Field;
extern jfieldID pyMethod_im_classField;

extern jclass pyFunctionClass;
extern jmethodID pyFunction_Constructor;
extern jfieldID pyFunction___code__Field;
extern jmethodID pyFunction_setCode;
extern jmethodID pyFunction_getFuncGlobals;
extern jfieldID pyFunction___module__Field;
extern jmethodID pyFunction_getFuncDefaults;
extern jmethodID pyFunction_setFuncDefaults;
extern jfieldID pyFunction___closure__Field;
extern jfieldID pyFunction___doc__Field;
extern jfieldID pyFunction___name__Field;

extern jclass pyClassMethodClass;
extern jmethodID pyClassMethod_Constructor;
extern jfieldID pyClassMethod_callableField;

extern jclass pyStaticMethodClass;
extern jmethodID pyStaticMethod_Constructor;
extern jfieldID pyStaticMethod_callableField;

extern jclass pyBuiltinCallableInfoClass;
extern jmethodID pyBuiltinCallableInfo_getMinargs;
extern jmethodID pyBuiltinCallableInfo_getMaxargs;
jmethodID pyBuiltinCallableInfo_getName;

extern jclass pyBuiltinCallableClass;
extern jmethodID pyBuiltinCallable_getSelf;
extern jmethodID pyBuiltinCallable_getModule;
extern jmethodID pyBuiltinCallable_getDoc;
extern jfieldID pyBuiltinCallable_infoField;

extern jclass pyCMethodDefClass;
extern jmethodID pyCMethodDef_Constructor;

extern jclass pyCFunctionClass;
extern jmethodID pyCFunction_Constructor;

extern jclass pyDescrClass;
extern jfieldID pyDescr_dtypeField;

extern jclass pyMethodDescrClass;
extern jmethodID pyMethodDescr_Constructor;
extern jmethodID pyMethodDescr_getDoc;

extern jclass pyClassMethodDescrClass;

extern jclass pyDictProxyClass;
extern jmethodID pyDictProxy_Constructor;
extern jfieldID pyDictProxy_dictField;

extern jclass pyPropertyClass;
extern jmethodID pyProperty_Constructor;
extern jfieldID pyProperty_fgetField;
extern jfieldID pyProperty_fsetField;
extern jfieldID pyProperty_fdelField;
extern jfieldID pyProperty_docField;
extern jfieldID pyProperty_docFromGetterField;

//extern jclass pyBaseStringClass;
//extern jclass pyXRangeClass;

extern jclass pySequenceIterClass;
extern jmethodID pySequenceIter_Constructor;
extern jfieldID pySequenceIter_seqField;
extern jfieldID pySequenceIter_indexField;

//extern jclass pyFastSequenceIterClass;
//extern jclass pyReversedIteratorClass;

//extern jclass pyBaseSetClass;
extern jfieldID pyBaseSet__setField;
extern jmethodID pyBaseSet__update;
extern jmethodID pyBaseSet_baseset_issubset;
extern jmethodID pyBaseSet_baseset_issuperset;
extern jmethodID pyBaseSet_baseset_isdisjoint;
extern jmethodID pyBaseSet_baseset_difference;
extern jmethodID pyBaseSet_baseset_differenceMulti;
extern jmethodID pyBaseSet_baseset_symmetric_difference;
extern jmethodID pyBaseSet_baseset_intersection;
extern jmethodID pyBaseSet_baseset___contains__;

extern jclass pySetClass;
extern jmethodID pySet_fromIterableConstructor;
extern jmethodID pySet_set_pop;
extern jmethodID pySet_set_clear;
extern jmethodID pySet_set_discard;
extern jmethodID pySet_set_add;
extern jmethodID pySet_set_difference_update;
extern jmethodID pySet_set_intersection_update;
extern jmethodID pySet_set_symmetric_difference_update;

extern jclass pyFrozenSetClass;
extern jmethodID pyFrozenSet_fromIterableConstructor;

//extern jclass pyEnumerationClass;

extern jclass pySliceClass;
extern jmethodID pySlice_fromStartStopStepConstructor;
extern jmethodID pySlice_getStart;
extern jmethodID pySlice_getStop;
extern jmethodID pySlice_getStep;
extern jmethodID pySlice_indicesEx;

extern jclass pyEllipsisClass;
//extern jclass pyGeneratorClass;

extern jclass pyWeakReferenceClass;
extern jclass pyWeakProxyClass;
extern jclass pyWeakCallableProxyClass;

//extern jclass pyCodeClass;
extern jfieldID pyCode_co_nameField;

//extern jclass pyBaseCodeClass;
extern jfieldID pyBaseCode_co_argcountField;
extern jfieldID pyBaseCode_co_cellvarsField;
extern jfieldID pyBaseCode_co_filenameField;
extern jfieldID pyBaseCode_co_firstlinenoField;
extern jfieldID pyBaseCode_co_freevarsField;
extern jfieldID pyBaseCode_co_nlocalsField;
extern jfieldID pyBaseCode_co_varnamesField;

extern jclass pyBytecodeClass;
extern jmethodID pyBytecode_Constructor;
extern jfieldID pyBytecode_co_constsField;
extern jfieldID pyBytecode_co_namesField;
extern jfieldID pyBytecode_co_stacksizeField;

extern jclass pyTableCodeClass;

extern jclass GlobalRefClass;
extern jmethodID GlobalRef_retryFactory;

extern jclass AbstractReferenceClass;
extern jmethodID AbstractReference_get;

extern jclass JyNIGlobalRefClass;
extern jmethodID JyNIGlobalRef_initNativeHandle;

extern jclass pyBaseExceptionClass;
extern jmethodID pyBaseException_emptyConstructor;
extern jmethodID pyBaseException_subTypeConstructor;
extern jmethodID pyBaseException___init__;
extern jmethodID pyBaseException___setstate__;
extern jmethodID pyBaseException___unicode__;
extern jfieldID pyBaseException_argsField;
extern jmethodID pyBaseException_setArgs;
extern jmethodID pyBaseException_getMessage;
extern jmethodID pyBaseException_setMessage;

extern jclass __builtin__Class;
extern jmethodID __builtin___ImportLevel;

extern jclass impClass;
extern jmethodID imp_importName;
extern jmethodID imp_reload;

extern jclass threadStateCurExcInfoClass;
extern jfieldID threadStateCurExcInfo_curexc_type;
extern jfieldID threadStateCurExcInfo_curexc_value;
extern jfieldID threadStateCurExcInfo_curexc_traceback;

extern jclass exceptionsClass;
extern jmethodID exceptions_KeyError;
extern jmethodID exceptions_KeyError__str__;
extern jmethodID exceptions_EnvironmentError;
extern jmethodID exceptions_EnvironmentError__init__;
extern jmethodID exceptions_EnvironmentError__str__;
extern jmethodID exceptions_EnvironmentError__reduce__;
extern jmethodID exceptions_SyntaxError;
extern jmethodID exceptions_SyntaxError__init__;
extern jmethodID exceptions_SyntaxError__str__;
extern jmethodID exceptions_SystemExit;
extern jmethodID exceptions_SystemExit__init__;

extern jmethodID exceptions_UnicodeError;
#ifdef Py_USING_UNICODE
extern jmethodID exceptions_getStart;
extern jmethodID exceptions_getEnd;
extern jmethodID exceptions_getString;
extern jmethodID exceptions_getUnicode;
extern jmethodID exceptions_UnicodeError__init__;
extern jmethodID exceptions_UnicodeEncodeError;
extern jmethodID exceptions_UnicodeEncodeError__init__;
extern jmethodID exceptions_UnicodeEncodeError__str__;
extern jmethodID exceptions_UnicodeDecodeError;
extern jmethodID exceptions_UnicodeDecodeError__init__;
extern jmethodID exceptions_UnicodeDecodeError__str__;
extern jmethodID exceptions_UnicodeTranslateError;
extern jmethodID exceptions_UnicodeTranslateError__init__;
extern jmethodID exceptions_UnicodeTranslateError__str__;
#endif

#endif /* INCLUDE_JYNI_JNI_H_ */
