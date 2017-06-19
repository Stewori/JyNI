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
 * JyNI_JNI.c
 *
 *  Created on: 17.08.2016
 *      Author: Stefan Richthofer
 */

#include <JyNI.h>
#include <JNI_util.h>
#include <JyNI_JNI.h>

//singletons:
JavaVM* java;
jweak JyNone;
jweak JyNotImplemented;
jweak JyEllipsis;
jweak JyEmptyFrozenSet;
jweak JyEmptyString;
jweak JyEmptyUnicode;
jweak JyEmptyTuple;
jweak JyTrue;
jweak JyFalse;
jweak JyEmptyStringArray;
jweak JyEmptyPyObjectArray;

jclass objectClass;
jmethodID object_toString;
jmethodID object_getClass;

jclass classClass;
jmethodID class_equals;

jclass arrayListClass;
jmethodID arrayList_Constructor;
jmethodID list_add;
jmethodID collection_size;
//jmethodID listClear;
//jmethodID listRemove;
//jmethodID listRemoveIndex;
//jmethodID listSet;

//jclass systemClass;
//jmethodID arraycopy;

jclass JyNIClass;
jmethodID JyNI_setNativeHandle;
//jmethodID JyNIRegisterNativeStaticTypeDict;
jmethodID JyNI_registerNativeStaticJyGCHead;
jmethodID JyNI_getNativeStaticJyGCHead;
jmethodID JyNI_lookupNativeHandle;
jmethodID JyNI_lookupCPeerFromHandle;
jmethodID JyNI_clearNativeHandle;
//jmethodID JyNIConstructDefaultObject;
jmethodID JyNI_getDLOpenFlags;
jmethodID JyNI_getJyObjectByName;
jmethodID JyNI_getPyObjectByName;
jmethodID JyNI_getPyType;
jmethodID JyNI_getNativeAvailableKeysAndValues;
jmethodID JyNI_exceptionByName;
//jmethodID JyErr_SetCurExc;
//jmethodID JyErr_GetCurExc;
jmethodID JyNI_JyErr_InsertCurExc;
jmethodID JyNI_JyErr_PrintEx;
//jmethodID JyNIPyErr_Restore;
//jmethodID JyNIPyErr_Clear;
//jmethodID JyNIPyErr_Occurred;
jmethodID JyNI_PyErr_ExceptionMatches;
//jmethodID JyNIPyErr_SetObject;
//jmethodID JyNIPyErr_SetString;
//jmethodID JyNIPyErr_SetNone;
//jmethodID JyNIPyErr_NoMemory;
//jmethodID JyNIPyErr_Fetch;
jmethodID JyNI_JyNI_PyTraceBack_Here;
jmethodID JyNI_PyErr_WriteUnraisable;
jmethodID JyNI_getDLVerbose;
jmethodID JyNI__PyImport_FindExtension;
jmethodID JyNI_getPyDictionary_Next;
jmethodID JyNI_getPySet_Next;
jmethodID JyNI_PyImport_GetModuleDict;
jmethodID JyNI_PyImport_AddModule;
jmethodID JyNI_PyImport_ImportModuleNoBlock;
jmethodID JyNI_JyNI_GetModule;
jmethodID JyNI_slice_compare;
jmethodID JyNI_printPyLong;
jmethodID JyNI_lookupNativeHandles;
jmethodID JyNI_prepareKeywordArgs;
jmethodID JyNI_getCurrentThreadID;
jmethodID JyNI_JyNI_pyCode_co_code;
jmethodID JyNI_JyNI_pyCode_co_flags;
jmethodID JyNI_JyNI_pyCode_co_lnotab;
jmethodID JyNI_jPrint;
jmethodID JyNI_jPrintLong;
jmethodID JyNI_jGetHash;
//jmethodID JyNIPySet_pop;
jmethodID JyNI_makeGCHead;
jmethodID JyNI_makeStaticGCHead;
jmethodID JyNI_gcDeletionReport;
jmethodID JyNI_waitForCStubs;
jmethodID JyNI_addJyNICriticalObject;
jmethodID JyNI_removeJyNICriticalObject;
jmethodID JyNI_suspendPyInstanceFinalizer;
jmethodID JyNI_restorePyInstanceFinalizer;
jmethodID JyNI_createWeakReferenceFromNative;
jmethodID JyNI_createProxyFromNative;
jmethodID JyNI_createCallableProxyFromNative;
jmethodID JyNI_getGlobalRef;
jmethodID JyNI_getTypeNameForNativeConversion;
jmethodID JyNI_getTypeOldStyleParent;
jmethodID JyNI_getJythonBuiltins;
jmethodID JyNI_getJythonLocals;
jmethodID JyNI_getJythonGlobals;
jmethodID JyNI_getJythonFrame;
jmethodID JyNI_getPlatform;

jclass JyTStateClass;
jmethodID JyTState_setRecursionLimit;
jmethodID JyTState_prepareNativeThreadState;
jfieldID JyTState_nativeRecursionLimitField;

jclass JyNIDictNextResultClass;
jfieldID JyNIDictNextResult_keyField;
jfieldID JyNIDictNextResult_valueField;
jfieldID JyNIDictNextResult_newIndexField;
jfieldID JyNIDictNextResult_keyHandleField;
jfieldID JyNIDictNextResult_valueHandleField;

jclass JyNISetNextResultClass;
jfieldID JyNISetNextResult_keyField;
jfieldID JyNISetNextResult_newIndexField;
jfieldID JyNISetNextResult_keyHandleField;

jclass JyListClass;
jmethodID JyList_fromBackendHandleConstructor;
//jmethodID JyListInstallToPyList;

jclass JySetClass;
jmethodID JySet_fromBackendHandleConstructor;
//jmethodID JySetInstallToPySet;

jclass JyLockClass;
jmethodID JyLock_Constructor;
jmethodID JyLock_acquire;
jmethodID JyLock_release;

jclass NativeActionClass;
jmethodID NativeAction_constructor;
jfieldID NativeAction_actionField;
jfieldID NativeAction_objField;
jfieldID NativeAction_nativeRef1Field;
jfieldID NativeAction_nativeRef2Field;
jfieldID NativeAction_cTypeNameField;
jfieldID NativeAction_cMethodField;
jfieldID NativeAction_cLineField;
jfieldID NativeAction_cFileField;

jclass JyReferenceMonitorClass;
jmethodID JyRefMonitorMakeDebugInfo;
jmethodID JyReferenceMonitor_addNativeAction;

jclass pyCPeerClass;
jmethodID pyCPeer_Constructor;
jfieldID pyCPeer_objectHandleField;
//jfieldID pyCPeerRefHandle;

jclass pyCPeerGCClass;
jmethodID pyCPeerGC_Constructor;
//jfieldID pyCPeerLinksHandle;

jclass pyCPeerTypeGCClass;
jmethodID pyCPeerTypeGC_Constructor;
jmethodID pyCPeerTypeGC_ConstructorSubtype;

// Subclasses:
jclass pyDictCPeerClass;
jclass pyTupleCPeerClass;
jmethodID pyTupleCPeer_Constructor;

jclass jyGCHeadClass;
jmethodID traversableGCHead_setLinks;
jmethodID traversableGCHead_setLink;
jmethodID traversableGCHead_insertLink;
jmethodID traversableGCHead_clearLink;
jmethodID traversableGCHead_clearLinksFromIndex;
jmethodID traversableGCHead_ensureSize;
jmethodID pyObjectGCHead_setPyObject;
jmethodID jyGCHead_getHandle;

jclass cPeerInterface;
jclass cPeerNativeDelegateInterface;
jmethodID super__call__;
jmethodID super__findattr_ex__;
jmethodID super__setattr__;
jmethodID super__str__;
jmethodID super__repr__;
jmethodID super__finditem__;
jmethodID super__setitem__;
jmethodID super__delitem__;
jmethodID super__len__;
jmethodID super_toString;

jclass pyCPeerTypeClass;
//jmethodID pyCPeerTypeConstructor;
jmethodID pyCPeerType_withNameAndDictConstructor;
jmethodID pyCPeerType_withNameDictTypeConstructor;
jfieldID pyCPeerType_objectHandleField;
jfieldID pyCPeerType_refHandleField;

jclass stringClass;
jmethodID string_fromBytesAndCharsetNameConstructor;
jmethodID string_toUpperCase;
jmethodID string_getBytes;
jmethodID string_intern;

jclass bigIntClass;
//jmethodID bigIntFromStringConstructor;
//jmethodID bigIntFromStringRadixConstructor;
jmethodID bigInt_fromByteArrayConstructor;
//jmethodID bigIntegerFromSignByteArrayConstructor;
jmethodID bigInt_toByteArray;
//jmethodID bigIntSignum;
//jmethodID bigIntToStringRadix;

jclass pyPyClass;
jmethodID pyPy_getThreadState;
jmethodID pyPy_getFrame;
jmethodID pyPy_isSubClass;
jmethodID pyPy_warning;
jmethodID pyPy_newString;
jmethodID pyPy_newUnicode;
jmethodID pyPy_newIntFromInt;
jmethodID pyPy_newIntFromLong;
jmethodID pyPy_newLongFromBigInt;
jmethodID pyPy_newLongFromInt;
jmethodID pyPy_newLongFromLong;
jmethodID pyPy_newLongFromString;
jmethodID pyPy_newFloatFromDouble;
jmethodID pyPy_warningStck;
jmethodID pyPy_explicitWarning;
jfieldID pyPy_ImportErrorField;
jfieldID pyPy_SystemErrorField;
jfieldID pyPy_UnicodeErrorField;
jfieldID pyPy_UnicodeEncodeErrorField;
jfieldID pyPy_UnicodeDecodeErrorField;
jfieldID pyPy_UnicodeTranslateErrorField;
jfieldID pyPy_UnicodeWarningField;
jmethodID pyPy_UnicodeErrorFactory;
jmethodID pyPy_UnicodeEncodeErrorFactory;
jmethodID pyPy_UnicodeDecodeErrorFactory;
jmethodID pyPy_UnicodeTranslateErrorFactory;
jmethodID pyPy_raiseUnicodeWarning;
jmethodID pyPy_makeClass;
//pre-allocated:
jfieldID pyPy_integerCacheField;
jfieldID pyPy_lettersField;
//No unicode letters are cached in Jython

jclass pyObjectClass;
jmethodID pyObject_getType;
jmethodID pyObject_asString;
jfieldID pyObject_TYPEField;
jmethodID pyObject___getattr__;
jmethodID pyObject___findattr__;
jmethodID pyObject___setattr__;
jmethodID pyObject___repr__;
jmethodID pyObject___cmp__;
jmethodID pyObject___reduce__;
jmethodID pyObject___and__;
jmethodID pyObject___or__;
jmethodID pyObject___sub__;
jmethodID pyObject___xor__;
jmethodID pyObject___isub__;
jmethodID pyObject___call__;
jmethodID pyObject___str__;
jmethodID pyObject___finditem__;
//jmethodID pyObject__getitem__;
jmethodID pyObject___setitem__;
jmethodID pyObject___delitem__;
jmethodID pyObject___get__;
jmethodID pyObject___contains__;
jmethodID pyObject___len__;
jmethodID pyObject___getslice__;
jmethodID pyObject___nonzero__;
jmethodID pyObject___iter__;
jmethodID pyObject___iternext__;
jmethodID pyObject__cmp;
jmethodID pyObject_getDict;
jmethodID pyObject_fastGetDict;
jmethodID pyObject_setDict;
jmethodID pyObject_isCallable;
jmethodID object_hashCode;

jclass pyThreadStateClass;
jfieldID pyThreadState_exceptionField;
jfieldID pyThreadState_frameField;
//jfieldID pyThreadStateRecursionDepth;
//jmethodID pyThreadStateEnterRecursiveCall;
//jmethodID pyThreadStateLeaveRecursiveCall;
jmethodID pyThreadState_enterRepr;
jmethodID pyThreadState_exitRepr;
jmethodID pyThreadState_getCompareStateDict;

jclass pyInstanceClass;
jmethodID pyInstance_Constructor;
jfieldID pyInstance_instclassField;
jfieldID pyInstance___dict__Field;
jmethodID pyInstance_isSequenceType;

jclass pyFrameClass;
jmethodID pyFrame_Constructor;
jfieldID pyFrame_f_backField;
jfieldID pyFrame_f_builtinsField;
jfieldID pyFrame_f_linenoField;

jclass pyTracebackClass;
jmethodID pyTraceback_byTracebackFrameConstructor;

jclass pyExceptionClass;
jfieldID pyException_typeField;
jfieldID pyException_valueField;
jfieldID pyException_tracebackField;
jmethodID pyException_fullConstructor;
jmethodID pyException_typeValueConstructor;
jmethodID pyException_typeConstructor;
jmethodID pyException_normalize;
jmethodID pyException_match;
jmethodID pyException_isExceptionClass;
jmethodID pyException_isExceptionInstance;

jclass pyBooleanClass;
jfieldID pyBoolean_valueField;

//jclass pyArrayClass;
//jmethodID pyArrayGetTypecode;

jclass pyTupleClass;
jmethodID pyTuple_Constructor;
jmethodID pyTuple_byPyObjectArrayBooleanConstructor;
//jfieldID pyTupleArray;
//jmethodID pyTupleGetArray;

jclass pyListClass;
jmethodID pyList_Constructor;
jmethodID pyList_byPyObjectArrayConstructor;
jmethodID pyList_fromList;
jmethodID pyList_append;
jmethodID pyList_insert;
jmethodID pyList_sort;
jmethodID pyList_reverse;
jmethodID pyList_getArray;
jfieldID pyList_listField;

jclass pyAbstractDictClass;
jmethodID pyAbstractDict_copy;
jmethodID pyAbstractDict_merge;
jmethodID pyAbstractDict_mergeFromSeq;

jclass pyDictClass;
jmethodID pyDict_Constructor;
//jmethodID pyDictByPyObjectArrayConstructor;

jclass pyStringMapClass;

jclass pyIntClass;
jmethodID pyInt_Constructor;
jmethodID pyInt_getValue;
//jmethodID pyIntAsInt;
//jmethodID pyIntAsLong;

jclass pyLongClass;
jmethodID pyLong_byBigIntConstructor;
jmethodID pyLong_byLongConstructor;
jmethodID pyLong_asLong;
jmethodID pyLong_getLong;
jmethodID pyLong_getValue;
jmethodID pyLong_bit_length;
//jmethodID pyLongToString;

jclass pyUnicodeClass;
//jmethodID pyUnicode_byStringConstructor;

jclass pyStringClass;
//jmethodID pyString_byStringConstructor;

jclass pyFloatClass;
jmethodID pyFloat_byDoubleConstructor;
jmethodID pyFloat_asDouble;
jfieldID pyFloat_TYPEField;

jclass pyComplexClass;
jmethodID pyComplex_by2DoubleConstructor;
jfieldID pyComplexRealField;
jfieldID pyComplexImagField;

jclass pySequenceClass;
jmethodID pySequence_pyget;
//jmethodID pySequence_pyset;

jclass pyTypeClass;
jmethodID pyType_getName;
jmethodID pyType_setName;
jmethodID pyType_getBase;
jmethodID pyType_getBases;
jmethodID pyType_getMro;
jmethodID pyType_isSubType;
jmethodID pyType_lookup;
jfieldID pyType_mroField;
jfieldID pyType_nameField;

jclass pyCodecsClass;
jmethodID pyCodecs_decode;
jmethodID pyCodecs_encode;
jmethodID pyCodecs_getDefaultEncoding;
jmethodID pyCodecs_lookup;
jmethodID pyCodecs_register;
jmethodID pyCodecs_lookup_error;
jmethodID pyCodecs_register_error;

jclass pyFileClass;
jmethodID pyFile_write;
jmethodID pyFile_fd;

jclass pyNotImplementedClass;
jclass pyNoneClass;

jclass pyModuleClass;
jmethodID pyModule_byStringConstructor;
jmethodID pyModule_getDict;
jmethodID pyModule___setattr__;
jmethodID pyModule___delattr__;

jclass pyCellClass;
jmethodID pyCell_Constructor;
jfieldID pyCell_ob_refField;

jclass pyClassClass;
jmethodID pyClass_classobj___new__;
jfieldID pyClass___bases__Field;
jfieldID pyClass___dict__Field;
jfieldID pyClass___name__Field;

jclass pyMethodClass;
jmethodID pyMethod_Constructor;
jfieldID pyMethod___func__Field;
jfieldID pyMethod___self__Field;
jfieldID pyMethod_im_classField;

jclass pyFunctionClass;
jmethodID pyFunction_Constructor;
jfieldID pyFunction___code__Field;
jmethodID pyFunction_setCode;
jmethodID pyFunction_getFuncGlobals;
jfieldID pyFunction___module__Field;
jmethodID pyFunction_getFuncDefaults;
jmethodID pyFunction_setFuncDefaults;
jfieldID pyFunction___closure__Field;
jfieldID pyFunction___doc__Field;
//jmethodID pyFunctionGetDict; use pyObjectGetDict
//jemthodID pyFunctionSetDict; use pyObjectSetDict
jfieldID pyFunction___name__Field;
//jmethodID pyFunctionCall; use pyObject__call__

jclass pyClassMethodClass;
jmethodID pyClassMethod_Constructor;
jfieldID pyClassMethod_callableField;

jclass pyStaticMethodClass;
jmethodID pyStaticMethod_Constructor;
jfieldID pyStaticMethod_callableField;

jclass pyBuiltinCallableInfoClass;
jmethodID pyBuiltinCallableInfo_getMinargs;
jmethodID pyBuiltinCallableInfo_getMaxargs;
jmethodID pyBuiltinCallableInfo_getName;

jclass pyBuiltinCallableClass;
jmethodID pyBuiltinCallable_getSelf;
jmethodID pyBuiltinCallable_getModule;
jmethodID pyBuiltinCallable_getDoc;
//jmethodID pyBuiltinCallable_fastGetName;
jfieldID pyBuiltinCallable_infoField;

jclass pyCMethodDefClass;
jmethodID pyCMethodDef_Constructor;

jclass pyCFunctionClass;
jmethodID pyCFunction_Constructor;

jclass pyDescrClass;
jfieldID pyDescr_dtypeField;

jclass pyMethodDescrClass;
jmethodID pyMethodDescr_Constructor;
jmethodID pyMethodDescr_getDoc;

jclass pyClassMethodDescrClass;

jclass pyDictProxyClass;
jmethodID pyDictProxy_Constructor;
jfieldID pyDictProxy_dictField;

jclass pyPropertyClass;
jmethodID pyProperty_Constructor;
jfieldID pyProperty_fgetField;
jfieldID pyProperty_fsetField;
jfieldID pyProperty_fdelField;
jfieldID pyProperty_docField;
jfieldID pyProperty_docFromGetterField;

//jclass pyBaseStringClass;
//jclass pyXRangeClass;

jclass pySequenceIterClass;
jmethodID pySequenceIter_Constructor;
jfieldID pySequenceIter_seqField;
jfieldID pySequenceIter_indexField;

//jclass pyFastSequenceIterClass;
//jclass pyReversedIteratorClass;

//jclass pyBaseSetClass;
jfieldID pyBaseSet__setField;
//jmethodID pyBaseSetClear;
//jmethodID pyBaseSetContains;
//jmethodID pyBaseSetRemove;
//jmethodID pyBaseSetAdd;
jmethodID pyBaseSet__update;//+
//jmethodID pyBaseSetbaseset_union;//-
jmethodID pyBaseSet_baseset_issubset;//+
jmethodID pyBaseSet_baseset_issuperset;//-
jmethodID pyBaseSet_baseset_isdisjoint;//+
jmethodID pyBaseSet_baseset_difference;//+
jmethodID pyBaseSet_baseset_differenceMulti;//+
jmethodID pyBaseSet_baseset_symmetric_difference;//-
jmethodID pyBaseSet_baseset_intersection;//-
//jmethodID pyBaseSetbaseset_copy;//-
jmethodID pyBaseSet_baseset___contains__;//+
//jmethodID pyBaseSetbaseset_difference_multi;
//jmethodID pyBaseSetbaseset_intersection_multi;
//jmethodID pyBaseSetbaseset___iter__;

jclass pySetClass;
jmethodID pySet_fromIterableConstructor;
jmethodID pySet_set_pop;//+
jmethodID pySet_set_clear;//+
//jmethodID pySetset_remove;
jmethodID pySet_set_discard;//+
jmethodID pySet_set_add;//+
jmethodID pySet_set_difference_update;//+
jmethodID pySet_set_intersection_update;//+
jmethodID pySet_set_symmetric_difference_update;//+
//jmethodID pySetset___isub__;
//jmethodID pySetset___iand__;
//jmethodID pySetset___ixor__;

jclass pyFrozenSetClass;
jmethodID pyFrozenSet_fromIterableConstructor;

//jclass pyEnumerationClass;

jclass pySliceClass;
jmethodID pySlice_fromStartStopStepConstructor;
jmethodID pySlice_getStart;
jmethodID pySlice_getStop;
jmethodID pySlice_getStep;
jmethodID pySlice_indicesEx;

jclass pyEllipsisClass;
//jclass pyGeneratorClass;

jclass pyWeakReferenceClass;
jclass pyWeakProxyClass;
jclass pyWeakCallableProxyClass;

//jclass pyCodeClass;
jfieldID pyCode_co_nameField;

//jclass pyBaseCodeClass;
jfieldID pyBaseCode_co_argcountField;
jfieldID pyBaseCode_co_cellvarsField;
jfieldID pyBaseCode_co_filenameField;
jfieldID pyBaseCode_co_firstlinenoField;
//jfieldID pyBaseCode_co_flags;
jfieldID pyBaseCode_co_freevarsField;
jfieldID pyBaseCode_co_nlocalsField;
jfieldID pyBaseCode_co_varnamesField;

jclass pyBytecodeClass;
jmethodID pyBytecode_Constructor;
//jfieldID pyBytecode_co_code;
jfieldID pyBytecode_co_constsField;
//jfieldID pyBytecode_co_lnotab;
jfieldID pyBytecode_co_namesField;
jfieldID pyBytecode_co_stacksizeField;

jclass pyTableCodeClass;
//jfieldID pyTableCode_co_code;

//jclass pyCallIterClass;
//jclass pySuperClass;

jclass GlobalRefClass;
jmethodID GlobalRef_retryFactory;

jclass AbstractReferenceClass;
jmethodID AbstractReference_get;

jclass JyNIGlobalRefClass;
jmethodID JyNIGlobalRef_initNativeHandle;

jclass pyBaseExceptionClass;
//jfieldID pyBaseException__dict__;
jmethodID pyBaseException_emptyConstructor;
jmethodID pyBaseException_subTypeConstructor;
jmethodID pyBaseException___init__;
jmethodID pyBaseException___setstate__;
jmethodID pyBaseException___unicode__;
jfieldID pyBaseException_argsField;
jmethodID pyBaseException_setArgs;
jmethodID pyBaseException_getMessage;
jmethodID pyBaseException_setMessage;

//jclass pyByteArrayClass;
//jclass pyBufferClass;
//jclass pyMemoryViewClass;

jclass __builtin__Class;
//jmethodID __builtin__Import;
jmethodID __builtin___ImportLevel;

jclass impClass;
jmethodID imp_importName;
jmethodID imp_reload;

jclass exceptionsClass;
jmethodID exceptions_KeyError;
jmethodID exceptions_KeyError__str__;
jmethodID exceptions_EnvironmentError;
jmethodID exceptions_EnvironmentError__init__;
jmethodID exceptions_EnvironmentError__str__;
jmethodID exceptions_EnvironmentError__reduce__;
jmethodID exceptions_SyntaxError;
jmethodID exceptions_SyntaxError__init__;
jmethodID exceptions_SyntaxError__str__;
jmethodID exceptions_SystemExit;
jmethodID exceptions_SystemExit__init__;

jmethodID exceptions_UnicodeError;
#ifdef Py_USING_UNICODE
jmethodID exceptions_getStart;
jmethodID exceptions_getEnd;
jmethodID exceptions_getString;
jmethodID exceptions_getUnicode;
jmethodID exceptions_UnicodeError__init__;
jmethodID exceptions_UnicodeEncodeError;
jmethodID exceptions_UnicodeEncodeError__init__;
jmethodID exceptions_UnicodeEncodeError__str__;
jmethodID exceptions_UnicodeDecodeError;
jmethodID exceptions_UnicodeDecodeError__init__;
jmethodID exceptions_UnicodeDecodeError__str__;
jmethodID exceptions_UnicodeTranslateError;
jmethodID exceptions_UnicodeTranslateError__init__;
jmethodID exceptions_UnicodeTranslateError__str__;
#endif

inline jint initJNI(JNIEnv *env)
{
	JNI_CLASS(object)
	JNI_METH_CLASS(object, toString, string)
	JNI_METH_CLASS(object, getClass, class)
	JNI_METH_CLASS(object, hashCode, int)

	JNI_CLASS(class)
	JNI_METH_CLASS(class, equals, boolean, object);

	JNI_CLASS(string)
	JNI_CONSTRUCTOR(string, fromBytesAndCharsetNameConstructor, byte array, string)
	JNI_METH_CLASS(string, toUpperCase, string)
	JNI_METH_CLASS(string, getBytes, byte array, string)
	JNI_METH_CLASS(string, intern, string)

	JNI_CLASS(bigInt)
	JNI_CONSTRUCTOR(bigInt, fromByteArrayConstructor, byte array)
	JNI_METH_CLASS(bigInt, toByteArray, byte array)

	JNI_CLASS(arrayList)
	JNI_CONSTRUCTOR(arrayList, Constructor, int)

	jclass collectionClass = (*env)->FindClass(env, pack_collection);
	JNI_METH_CLASS(collection, size, int)
	(*env)->DeleteLocalRef(env, collectionClass);

	jclass listClass = (*env)->FindClass(env, pack_list);
	JNI_METH_CLASS(list, add, boolean, object)
	(*env)->DeleteLocalRef(env, listClass);

	return JNI_VERSION_1_2;
}

inline jint initJyNI(JNIEnv *env)
{
	JNI_CLASS(JyNI)
	JNI_METH_STATIC(JyNI, setNativeHandle, void, pyObject, long)
	JNI_METH_STATIC(JyNI, registerNativeStaticJyGCHead, void, long, jyGCHead)
	JNI_METH_STATIC(JyNI, getNativeStaticJyGCHead, jyGCHead, long)
	JNI_METH_STATIC(JyNI, lookupNativeHandle, long, pyObject)
	JNI_METH_STATIC(JyNI, clearNativeHandle, void, pyObject)
	JNI_METH_STATIC(JyNI, lookupCPeerFromHandle, pyObject, long)
	JNI_METH_STATIC(JyNI, getDLOpenFlags, int)
	JNI_METH_STATIC(JyNI, getDLVerbose, int)
	JNI_METH_STATIC(JyNI, getJyObjectByName, long, string)
	JNI_METH_STATIC(JyNI, getPyObjectByName, pyObject, string)
	JNI_METH_STATIC(JyNI, _PyImport_FindExtension, pyObject, string, string)
	JNI_METH_STATIC(JyNI, getNativeAvailableKeysAndValues, long array, pyDict)
	JNI_METH_STATIC(JyNI, getPyDictionary_Next, JyNIDictNextResult, pyDict, int)
	JNI_METH_STATIC(JyNI, getPySet_Next, JyNISetNextResult, baseSet, int)
	JNI_METH_STATIC(JyNI, PyImport_GetModuleDict, pyObject)
	JNI_METH_STATIC(JyNI, PyImport_AddModule, pyObject, string)
	JNI_METH_STATIC(JyNI, PyImport_ImportModuleNoBlock, pyObject, string, boolean)
	JNI_METH_STATIC(JyNI, JyNI_GetModule, pyObject, string)
	JNI_METH_STATIC(JyNI, slice_compare, int, pySlice, pySlice)
	JNI_METH_STATIC(JyNI, printPyLong, void, pyObject)
	JNI_METH_STATIC(JyNI, lookupNativeHandles, long array, pyList)
	JNI_METH_STATIC(JyNI, prepareKeywordArgs, string array, pyObject array, pyDict)
	JNI_METH_STATIC(JyNI, getCurrentThreadID, long)
	JNI_METH_STATIC(JyNI, JyNI_pyCode_co_code, string, pyBaseCode)
	JNI_METH_STATIC(JyNI, JyNI_pyCode_co_flags, int, pyBaseCode)
	JNI_METH_STATIC(JyNI, JyNI_pyCode_co_lnotab, string, pyBytecode)
	JNI_METH_STATIC(JyNI, jPrint, void, string)
	JNI_METH_STATIC2(JyNI, jPrint, jPrintLong, void, long)
	JNI_METH_STATIC(JyNI, jGetHash, int, object)
	JNI_METH_STATIC(JyNI, makeGCHead, pyObjectGCHead, long, boolean, boolean)
	JNI_METH_STATIC(JyNI, makeStaticGCHead, jyGCHead, long, boolean)
	JNI_METH_STATIC(JyNI, gcDeletionReport, void, long array, long array)
	JNI_METH_STATIC(JyNI, waitForCStubs, void)
	JNI_METH_STATIC(JyNI, addJyNICriticalObject, void, long)
	JNI_METH_STATIC(JyNI, removeJyNICriticalObject, void, long)
	JNI_METH_STATIC(JyNI, suspendPyInstanceFinalizer, void, pyInstance)
	JNI_METH_STATIC(JyNI, restorePyInstanceFinalizer, void, pyInstance)
	JNI_METH_STATIC(JyNI, createWeakReferenceFromNative, pyWeakReference,
			pyObject, long, pyObject)
	JNI_METH_STATIC(JyNI, createProxyFromNative, pyWeakProxy,
			pyObject, long, pyObject)
	JNI_METH_STATIC(JyNI, createCallableProxyFromNative,
			pyWeakCallableProxy, pyObject, long, pyObject)
	JNI_METH_STATIC(JyNI, getGlobalRef, referenceBackend, pyObject)
	JNI_METH_STATIC(JyNI, getTypeNameForNativeConversion, string, pyObject)
	JNI_METH_STATIC(JyNI, getTypeOldStyleParent, pyClass, pyObject)
	JNI_METH_STATIC(JyNI, getJythonBuiltins, pyObject)
	JNI_METH_STATIC(JyNI, getJythonLocals, pyObject)
	JNI_METH_STATIC(JyNI, getJythonGlobals, pyObject)
	JNI_METH_STATIC(JyNI, getJythonFrame, pyFrame)
	JNI_METH_STATIC(JyNI, getPlatform, string)

	//Error stuff:
	JNI_METH_STATIC(JyNI, JyErr_InsertCurExc, void,
			pyThreadState, pyObject, pyObject, pyTraceback)
	JNI_METH_STATIC(JyNI, JyErr_PrintEx, void,
			boolean, pyThreadState, pyObject, pyObject, pyTraceback)
	JNI_METH_STATIC(JyNI, PyErr_ExceptionMatches, boolean,
			pyObject, pyObject, pyObject, pyTraceback)
	JNI_METH_STATIC(JyNI, JyNI_PyTraceBack_Here, pyTraceback, pyFrame, pyThreadState)
	JNI_METH_STATIC(JyNI, PyErr_WriteUnraisable, void, pyObject)
	JNI_METH_STATIC(JyNI, exceptionByName, pyObject, string)
	JNI_METH_STATIC(JyNI, getPyType, pyType, class)

	//ThreadState stuff:
	JNI_CLASS(JyTState)
	JNI_METH_STATIC(JyTState, setRecursionLimit, void, int)
	JNI_METH_STATIC(JyTState, prepareNativeThreadState, long)
	JNI_FIELD_STATIC(JyTState, nativeRecursionLimit, int)

	JNI_CLASS(JyNIDictNextResult)
	JNI_FIELD(JyNIDictNextResult, key, pyObject)
	JNI_FIELD(JyNIDictNextResult, value, pyObject)
	JNI_FIELD(JyNIDictNextResult, newIndex, int)
	JNI_FIELD(JyNIDictNextResult, keyHandle, long)
	JNI_FIELD(JyNIDictNextResult, valueHandle, long)

	JNI_CLASS(JyNISetNextResult)
	JNI_FIELD(JyNISetNextResult, key, pyObject)
	JNI_FIELD(JyNISetNextResult, newIndex, int)
	JNI_FIELD(JyNISetNextResult, keyHandle, long)

	JNI_CLASS(JyList)
	JNI_CONSTRUCTOR(JyList, fromBackendHandleConstructor, long)

	JNI_CLASS(JySet)
	JNI_CONSTRUCTOR(JySet, fromBackendHandleConstructor, set, long)

	JNI_CLASS(JyLock)
	JNI_CONSTRUCTOR(JyLock, Constructor)
	JNI_METH_CLASS(JyLock, acquire, boolean, boolean)
	JNI_METH_CLASS(JyLock, release, void)

	JNI_CLASS(NativeAction)
	JNI_CONSTRUCTOR(NativeAction, constructor)
	JNI_FIELD(NativeAction, action, short)
	JNI_FIELD(NativeAction, obj, pyObject)
	JNI_FIELD(NativeAction, nativeRef1, long)
	JNI_FIELD(NativeAction, nativeRef2, long)
	JNI_FIELD(NativeAction, cTypeName, string)
	JNI_FIELD(NativeAction, cMethod, string)
	JNI_FIELD(NativeAction, cLine, int)
	JNI_FIELD(NativeAction, cFile, string)

	JNI_CLASS(JyReferenceMonitor)
	JNI_METH_STATIC(JyReferenceMonitor, addNativeAction, void, NativeAction)

	//Peer stuff:
	JNI_CLASS(pyCPeer)
	JNI_CONSTRUCTOR(pyCPeer, Constructor, long, pyType)
	JNI_FIELD(pyCPeer, objectHandle, long)

	JNI_CLASS(pyCPeerGC)
	JNI_CONSTRUCTOR(pyCPeerGC, Constructor, long, pyType)

	JNI_CLASS(pyCPeerTypeGC)
	JNI_CONSTRUCTOR(pyCPeerTypeGC, Constructor,
			long, string, pyObject, long)
	JNI_CONSTRUCTOR(pyCPeerTypeGC, ConstructorSubtype,
			long, string, pyObject, long, pyType)

	JNI_CLASS(pyDictCPeer)

	JNI_CLASS(pyTupleCPeer)
	JNI_CONSTRUCTOR(pyTupleCPeer, Constructor, long, pyCPeerType, pyObject array)

	JNI_CLASS(pyCPeerType)
	JNI_CONSTRUCTOR(pyCPeerType, withNameAndDictConstructor,
			long, string, pyObject, long)
	JNI_CONSTRUCTOR(pyCPeerType, withNameDictTypeConstructor,
			long, string, pyObject, long, pyType)
	JNI_FIELD(pyCPeerType, objectHandle, long)
	JNI_FIELD(pyCPeerType, refHandle, long)

	jclass traversableGCHeadInterface = (*env)->FindClass(env, pack_traversableGCHead);
	JNI_METH_INTERFACE(traversableGCHead, setLinks, void, object)
	JNI_METH_INTERFACE(traversableGCHead, setLink, int, int, jyGCHead)
	JNI_METH_INTERFACE(traversableGCHead, insertLink, int, int, jyGCHead)
	JNI_METH_INTERFACE(traversableGCHead, clearLink, int, int)
	JNI_METH_INTERFACE(traversableGCHead, clearLinksFromIndex, int, int)
	JNI_METH_INTERFACE(traversableGCHead, ensureSize, void, int)
	(*env)->DeleteLocalRef(env, traversableGCHeadInterface);

	JNI_CLASS(jyGCHead)
	JNI_METH_CLASS(jyGCHead, getHandle, long)

	jclass pyObjectGCHeadInterface = (*env)->FindClass(env, pack_pyObjectGCHead);
	JNI_METH_INTERFACE(pyObjectGCHead, setPyObject, void, pyObject)
	(*env)->DeleteLocalRef(env, pyObjectGCHeadInterface);

	JNI_INTERFACE1(cPeer)

	JNI_INTERFACE1(cPeerNativeDelegate)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__call__, Interface,
			pyObject, pyObject array, string array)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__findattr_ex__,
			Interface, pyObject, string)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__setattr__,
			Interface, void, string, pyObject)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__str__, Interface, pyString)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__repr__, Interface, pyString)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__finditem__,
			Interface, pyObject, pyObject)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__setitem__,
			Interface, void, pyObject, pyObject)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__delitem__,
			Interface, void, pyObject)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super__len__, Interface, int)
	JNI_METH_CLS_NOPREFIX(cPeerNativeDelegate, super_toString, Interface, string)

	return JNI_VERSION_1_2;
}

inline jint initJythonSite(JNIEnv *env)
{
	JNI_CLASS(pyPy)
	JNI_METH_STATIC(pyPy, getThreadState, pyThreadState)
	JNI_METH_STATIC(pyPy, getFrame, pyFrame)
	JNI_METH_STATIC(pyPy, isSubClass, boolean, pyObject, pyObject)
	JNI_METH_STATIC(pyPy, warning, void, pyObject, string)
	JNI_METH_STATIC2(pyPy, warning, warningStck, void, pyObject, string, int)
	JNI_METH_STATIC2(pyPy, warning, explicitWarning, void,
			pyObject, string, string, int, string, pyObject)
	JNI_FIELD_STATIC(pyPy, ImportError, pyObject)
	JNI_FIELD_STATIC(pyPy, SystemError, pyObject)
	JNI_METH_STATIC(pyPy, newString, pyString, string)
	JNI_METH_STATIC(pyPy, newUnicode, pyUnicode, string)
	JNI_METH_STATIC2(pyPy, newInteger, newIntFromInt, pyInt, int)
	JNI_METH_STATIC2(pyPy, newInteger, newIntFromLong, pyObject, long)
	JNI_METH_STATIC2(pyPy, newLong, newLongFromBigInt, pyLong, bigInt)
	JNI_METH_STATIC2(pyPy, newLong, newLongFromInt, pyLong, int)
	JNI_METH_STATIC2(pyPy, newLong, newLongFromLong, pyLong, long)
	JNI_METH_STATIC2(pyPy, newLong, newLongFromString, pyLong, string)
	JNI_METH_STATIC2(pyPy, newFloat, newFloatFromDouble, pyFloat, double)
	JNI_FIELD_STATIC(pyPy, UnicodeError, pyObject)
	JNI_FIELD_STATIC(pyPy, UnicodeEncodeError, pyObject)
	JNI_FIELD_STATIC(pyPy, UnicodeDecodeError, pyObject)
	JNI_FIELD_STATIC(pyPy, UnicodeTranslateError, pyObject)
	JNI_FIELD_STATIC(pyPy, UnicodeWarning, pyObject)
	JNI_METH_STATIC2(pyPy, UnicodeError, UnicodeErrorFactory, pyException, string)
	JNI_METH_STATIC2(pyPy, UnicodeEncodeError, UnicodeEncodeErrorFactory,
			pyException, string, string, int, int, string)
	JNI_METH_STATIC2(pyPy, UnicodeDecodeError, UnicodeDecodeErrorFactory,
			pyException, string, string, int, int, string)
	JNI_METH_STATIC2(pyPy, UnicodeTranslateError, UnicodeTranslateErrorFactory,
			pyException, string, int, int, string)
	JNI_METH_STATIC2(pyPy, UnicodeWarning, raiseUnicodeWarning, void, string)
	JNI_METH_STATIC(pyPy, makeClass, pyObject, string, pyObject array, pyObject)
	JNI_FIELD_STATIC(pyPy, integerCache, pyInt array)
	JNI_FIELD_STATIC(pyPy, letters, pyString array)

	JNI_CLASS(pyTraceback)
	JNI_CONSTRUCTOR(pyTraceback, byTracebackFrameConstructor, pyTraceback, pyFrame)

	JNI_CLASS(pyException)
	JNI_FIELD(pyException, type, pyObject)
	JNI_FIELD(pyException, value, pyObject)
	JNI_FIELD(pyException, traceback, pyTraceback)
	JNI_CONSTRUCTOR(pyException, fullConstructor, pyObject, pyObject, pyTraceback)
	JNI_CONSTRUCTOR(pyException, typeValueConstructor, pyObject, pyObject)
	JNI_CONSTRUCTOR(pyException, typeConstructor, pyObject)
	JNI_METH_CLASS(pyException, normalize, void)
	JNI_METH_CLASS(pyException, match, boolean, pyObject)
	JNI_METH_STATIC(pyException, isExceptionClass, boolean, pyObject)
	JNI_METH_STATIC(pyException, isExceptionInstance, boolean, pyObject)

	JNI_CLASS(pyThreadState)
	JNI_FIELD(pyThreadState, exception, pyException)
	JNI_FIELD(pyThreadState, frame, pyFrame)
	JNI_METH_CLASS(pyThreadState, enterRepr, boolean, pyObject)
	JNI_METH_CLASS(pyThreadState, exitRepr, void, pyObject)
	JNI_METH_CLASS(pyThreadState, getCompareStateDict, pyDict)

	return JNI_VERSION_1_2;
}

inline jint initJythonObjects(JNIEnv *env)
{
	JNI_CLASS(pyNone)

	JNI_CLASS(pyObject)
	JNI_METH_CLASS(pyObject, getType, pyType)
	JNI_METH_CLASS(pyObject, asString, string)
	JNI_FIELD_STATIC(pyObject, TYPE, pyType)
	JNI_METH_CLASS(pyObject, __getattr__, pyObject, string)
	JNI_METH_CLASS(pyObject, __findattr__, pyObject, string)
	JNI_METH_CLASS(pyObject, __setattr__, void, string, pyObject)
	JNI_METH_CLASS(pyObject, __repr__, pyString)
	JNI_METH_CLASS(pyObject, __cmp__, int, pyObject)
	JNI_METH_CLASS(pyObject, __reduce__, pyObject)
	JNI_METH_CLASS(pyObject, __and__, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __or__, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __sub__, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __xor__, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __isub__, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __call__, pyObject, pyObject array, string array)
	JNI_METH_CLASS(pyObject, __str__, pyString)
	JNI_METH_CLASS(pyObject, __finditem__, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __setitem__, void, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __delitem__, void, pyObject)
	JNI_METH_CLASS(pyObject, __get__, pyObject, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __contains__, boolean, pyObject)
	JNI_METH_CLASS(pyObject, __len__, int)
	JNI_METH_CLASS(pyObject, __getslice__, pyObject, pyObject, pyObject)
	JNI_METH_CLASS(pyObject, __nonzero__, boolean)
	JNI_METH_CLASS(pyObject, __iter__, pyObject)
	JNI_METH_CLASS(pyObject, __iternext__, pyObject)
	JNI_METH_CLASS(pyObject, _cmp, int, pyObject)
	JNI_METH_CLASS(pyObject, getDict, pyObject)
	JNI_METH_CLASS(pyObject, fastGetDict, pyObject)
	JNI_METH_CLASS(pyObject, setDict, void, pyObject)
	JNI_METH_CLASS(pyObject, isCallable, boolean)

	JNI_CLASS(pyInstance)
	JNI_CONSTRUCTOR(pyInstance, Constructor, pyClass, pyObject)
	JNI_FIELD(pyInstance, instclass, pyClass)
	JNI_FIELD(pyInstance, __dict__, pyObject)
	JNI_METH_CLASS(pyInstance, isSequenceType, boolean)

	JNI_CLASS(pyFrame)
	JNI_CONSTRUCTOR(pyFrame, Constructor, pyBaseCode, pyObject, pyObject, pyObject)
	JNI_FIELD(pyFrame, f_back, pyFrame)
	JNI_FIELD(pyFrame, f_builtins, pyObject)
	JNI_FIELD(pyFrame, f_lineno, int)

	JNI_CLASS(pyBoolean)
	JNI_FIELD(pyBoolean, value, boolean)

	JNI_CLASS(pyInt)
	JNI_CONSTRUCTOR(pyInt, Constructor, int)
	JNI_METH_CLASS(pyInt, getValue, int)

	JNI_CLASS(pyLong)
	JNI_CONSTRUCTOR(pyLong, byLongConstructor, long)
	JNI_CONSTRUCTOR(pyLong, byBigIntConstructor, bigInt)
	JNI_METH_CLASS(pyLong, asLong, long)
	JNI_METH_CLASS(pyLong, getLong, long, long, long)
	JNI_METH_CLASS(pyLong, getValue, bigInt)
	JNI_METH_CLASS(pyLong, bit_length, int)

	JNI_CLASS(pyUnicode)
//	JNI_CONSTRUCTOR(pyUnicode, byStringConstructor, string)

	JNI_CLASS(pyString)
//	JNI_CONSTRUCTOR(pyString, byStringConstructor, string)

	JNI_CLASS(pyFloat)
	JNI_CONSTRUCTOR(pyFloat, byDoubleConstructor, double)
	JNI_METH_CLASS(pyFloat, asDouble, double)
	JNI_FIELD_STATIC(pyFloat, TYPE, pyType)

	JNI_CLASS(pyComplex)
	JNI_CONSTRUCTOR(pyComplex, by2DoubleConstructor, double, double)

//	JNI_CLASS(pyArray)
//	JNI_METH_CLASS(pyArray, getTypecode, string)

	JNI_CLASS(pyTuple)
	JNI_CONSTRUCTOR(pyTuple, Constructor)
	JNI_CONSTRUCTOR(pyTuple, byPyObjectArrayBooleanConstructor, pyObject array, boolean)

	JNI_CLASS(pyList)
	JNI_CONSTRUCTOR(pyList, Constructor)
	JNI_CONSTRUCTOR(pyList, byPyObjectArrayConstructor, pyObject array)
	JNI_METH_STATIC(pyList, fromList, pyList, list)
	JNI_METH_CLASS(pyList, append, void, pyObject)
	JNI_METH_CLASS(pyList, insert, void, int, pyObject)
	JNI_METH_CLASS(pyList, sort, void)
	JNI_METH_CLASS(pyList, reverse, void)
	JNI_METH_CLASS(pyList, getArray, pyObject array)
	JNI_FIELD(pyList, list, list)

	JNI_CLASS(pyAbstractDict)
	JNI_METH_CLASS(pyAbstractDict, copy, pyAbstractDict)
	JNI_METH_CLASS(pyAbstractDict, merge, void, pyObject, boolean)
	JNI_METH_CLASS(pyAbstractDict, mergeFromSeq, void, pyObject, boolean)

	JNI_CLASS(pyDict)
	JNI_CONSTRUCTOR(pyDict, Constructor)

	JNI_CLASS(pyStringMap)

	JNI_CLASS(pySequence)
	JNI_METH_CLASS(pySequence, pyget, pyObject, int)
//	JNI_METH_CLASS(pySequence, pyset, void, int, pyObject)

	JNI_CLASS(pyType)
	JNI_METH_CLASS(pyType, getName, string)
	JNI_METH_CLASS(pyType, setName, void, string)
	JNI_METH_CLASS(pyType, getBase, pyObject)
	JNI_METH_CLASS(pyType, getBases, pyObject)
	JNI_METH_CLASS(pyType, getMro, pyTuple)
	JNI_METH_CLASS(pyType, isSubType, boolean, pyType)
	JNI_METH_CLASS(pyType, lookup, pyObject, string)
	JNI_FIELD(pyType, mro, pyObject array)
	JNI_FIELD(pyType, name, string)

	JNI_CLASS(pyCodecs)
	JNI_METH_STATIC(pyCodecs, decode, pyObject, pyString, string, string)
	JNI_METH_STATIC(pyCodecs, encode, string, pyString, string, string)
	JNI_METH_STATIC(pyCodecs, getDefaultEncoding, string)
	JNI_METH_STATIC(pyCodecs, lookup, pyTuple, string)
	JNI_METH_STATIC(pyCodecs, register, void, pyObject)
	JNI_METH_STATIC(pyCodecs, lookup_error, pyObject, string)
	JNI_METH_STATIC(pyCodecs, register_error, void, string, pyObject)

	JNI_CLASS(pyNotImplemented)

	JNI_CLASS(pyFile)
	JNI_METH_CLASS(pyFile, write, void, string)
  JNI_METH_CLASS(pyFile, fd, int)

	JNI_CLASS(pyModule)
	JNI_CONSTRUCTOR(pyModule, byStringConstructor, string)
	JNI_METH_CLASS(pyModule, getDict, pyObject)
	JNI_METH_CLASS(pyModule, __setattr__, void, string, pyObject)
	JNI_METH_CLASS(pyModule, __delattr__, void, string)

	JNI_CLASS(pyCell)
	JNI_CONSTRUCTOR(pyCell, Constructor)
	JNI_FIELD(pyCell, ob_ref, pyObject)

	JNI_CLASS(pyClass)
	JNI_METH_STATIC(pyClass, classobj___new__, pyObject, pyObject, pyObject, pyObject)
	JNI_FIELD(pyClass, __bases__, pyTuple)
	JNI_FIELD(pyClass, __dict__, pyObject)
	JNI_FIELD(pyClass, __name__, string)

	JNI_CLASS(pyMethod)
	JNI_CONSTRUCTOR(pyMethod, Constructor, pyObject, pyObject, pyObject)
	JNI_FIELD(pyMethod, __func__, pyObject)
	JNI_FIELD(pyMethod, __self__, pyObject)
	JNI_FIELD(pyMethod, im_class, pyObject)

	JNI_CLASS(pyFunction)
	JNI_CONSTRUCTOR(pyFunction, Constructor, pyObject, pyObject array, pyCode)
	JNI_FIELD(pyFunction, __code__, pyCode)
	JNI_METH_CLASS(pyFunction, setCode, void, pyCode)
	JNI_METH_CLASS(pyFunction, getFuncGlobals, pyObject)
	JNI_FIELD(pyFunction, __module__, pyObject)
	JNI_METH_CLASS(pyFunction, getFuncDefaults, pyObject)
	JNI_METH_CLASS(pyFunction, setFuncDefaults, void, pyObject)
	JNI_FIELD(pyFunction, __closure__, pyObject)
	JNI_FIELD(pyFunction, __doc__, pyObject)
	JNI_FIELD(pyFunction, __name__, string)

	JNI_CLASS(pyClassMethod)
	JNI_CONSTRUCTOR(pyClassMethod, Constructor, pyObject)
	JNI_FIELD(pyClassMethod, callable, pyObject)

	JNI_CLASS(pyStaticMethod)
	JNI_CONSTRUCTOR(pyStaticMethod, Constructor, pyObject)
	JNI_FIELD(pyStaticMethod, callable, pyObject)

	JNI_CLASS(pyBuiltinCallableInfo)
	JNI_METH_CLASS(pyBuiltinCallableInfo, getMinargs, int)
	JNI_METH_CLASS(pyBuiltinCallableInfo, getMaxargs, int)
	JNI_METH_CLASS(pyBuiltinCallableInfo, getName, string)

	JNI_CLASS(pyBuiltinCallable)
	JNI_METH_CLASS(pyBuiltinCallable, getSelf, pyObject)
	JNI_METH_CLASS(pyBuiltinCallable, getModule, pyObject)
	JNI_METH_CLASS(pyBuiltinCallable, getDoc, string)
	JNI_FIELD(pyBuiltinCallable, info, pyBuiltinCallableInfo)

	JNI_CLASS(pyCMethodDef)
	JNI_CONSTRUCTOR(pyCMethodDef, Constructor, long, string, boolean, string)

	JNI_CLASS(pyCFunction)
	JNI_CONSTRUCTOR(pyCFunction, Constructor, long, pyType, string, boolean, string)

	JNI_CLASS(pyDescr)
	JNI_FIELD(pyDescr, dtype, pyType)

	JNI_CLASS(pyMethodDescr)
	JNI_CONSTRUCTOR(pyMethodDescr, Constructor, pyType, pyBuiltinCallable)
	JNI_METH_CLASS(pyMethodDescr, getDoc, string)

	JNI_CLASS(pyClassMethodDescr)

	JNI_CLASS(pyDictProxy)
	JNI_CONSTRUCTOR(pyDictProxy, Constructor, pyObject)
	JNI_FIELD(pyDictProxy, dict, pyObject)

	JNI_CLASS(pyProperty)
	JNI_CONSTRUCTOR(pyProperty, Constructor)
	JNI_FIELD(pyProperty, fget, pyObject)
	JNI_FIELD(pyProperty, fset, pyObject)
	JNI_FIELD(pyProperty, fdel, pyObject)
	JNI_FIELD(pyProperty, doc, pyObject)
	JNI_FIELD(pyProperty, docFromGetter, boolean)

//	JNI_CLASS(pyBaseString)
//	JNI_CLASS(pyXRange)

	JNI_CLASS(pySequenceIter)
	JNI_CONSTRUCTOR(pySequenceIter, Constructor, pyObject)
	JNI_FIELD(pySequenceIter, seq, pyObject)
	JNI_FIELD(pySequenceIter, index, int)

//	JNI_CLASS(pyFastSequenceIter)
//	JNI_CLASS(pyReversedIterator)

	jclass pyBaseSetClass = (*env)->FindClass(env, pack_baseSet);
	JNI_FIELD(pyBaseSet, _set, set)
	JNI_METH_CLASS(pyBaseSet, _update, void, pyObject)
	JNI_METH_CLASS(pyBaseSet, baseset_issubset, pyObject, pyObject)
	JNI_METH_CLASS(pyBaseSet, baseset_issuperset, pyObject, pyObject)
	JNI_METH_CLASS(pyBaseSet, baseset_isdisjoint, pyObject, pyObject)
	JNI_METH_CLASS(pyBaseSet, baseset_difference, pyObject, pyObject)
	JNI_METH_CLASS2(pyBaseSet, baseset_difference, baseset_differenceMulti,
			pyObject, pyObject array)
	JNI_METH_CLASS(pyBaseSet, baseset_symmetric_difference, pyObject, pyObject)
	JNI_METH_CLASS(pyBaseSet, baseset_intersection, pyObject, pyObject)
	JNI_METH_CLASS(pyBaseSet, baseset___contains__, boolean, pyObject)
	(*env)->DeleteLocalRef(env, pyBaseSetClass);

	JNI_CLASS(pySet)
	JNI_CONSTRUCTOR(pySet, fromIterableConstructor, pyObject)
	JNI_METH_CLASS(pySet, set_pop, pyObject)
	JNI_METH_CLASS(pySet, set_clear, void)
	JNI_METH_CLASS(pySet, set_discard, void, pyObject)
	JNI_METH_CLASS(pySet, set_add, void, pyObject)
	JNI_METH_CLASS(pySet, set_difference_update, void, pyObject array, string array)
	JNI_METH_CLASS(pySet, set_intersection_update, void, pyObject array, string array)
	JNI_METH_CLASS(pySet, set_symmetric_difference_update, void, pyObject)

	JNI_CLASS(pyFrozenSet)
	JNI_CONSTRUCTOR(pyFrozenSet, fromIterableConstructor, pyObject)

//	JNI_CLASS(pyEnumeration)

	JNI_CLASS(pySlice)
	JNI_CONSTRUCTOR(pySlice, fromStartStopStepConstructor, pyObject, pyObject, pyObject)
	JNI_METH_CLASS(pySlice, getStart, pyObject)
	JNI_METH_CLASS(pySlice, getStop, pyObject)
	JNI_METH_CLASS(pySlice, getStep, pyObject)
	JNI_METH_CLASS(pySlice, indicesEx, int array, int)

	JNI_CLASS(pyEllipsis)
//	JNI_CLASS(pyGenerator)
	JNI_CLASS(pyWeakReference)
	JNI_CLASS(pyWeakProxy)
	JNI_CLASS(pyWeakCallableProxy)

	jclass pyCodeClass = (*env)->FindClass(env, pack_pyCode);
	JNI_FIELD(pyCode, co_name, string)
	(*env)->DeleteLocalRef(env, pyCodeClass);

	jclass pyBaseCodeClass = (*env)->FindClass(env, pack_pyBaseCode);
	JNI_FIELD(pyBaseCode, co_argcount, int)
	JNI_FIELD(pyBaseCode, co_cellvars, string array)
	JNI_FIELD(pyBaseCode, co_filename, string)
	JNI_FIELD(pyBaseCode, co_firstlineno, int)
	JNI_FIELD(pyBaseCode, co_freevars, string array)
	JNI_FIELD(pyBaseCode, co_nlocals, int)
	JNI_FIELD(pyBaseCode, co_varnames, string array)
	(*env)->DeleteLocalRef(env, pyBaseCodeClass);

	JNI_CLASS(pyBytecode)
	JNI_CONSTRUCTOR(pyBytecode, Constructor, int, int, int, int, string, pyObject array,
			string array, string array, string, string, int, string, string array, string array)
	JNI_FIELD(pyBytecode, co_consts, pyObject array)
	JNI_FIELD(pyBytecode, co_names, string array)
	JNI_FIELD(pyBytecode, co_stacksize, int)

	JNI_CLASS(pyTableCode)

//	JNI_CLASS(pyCallIter)
//	JNI_CLASS(pySuper)

	JNI_CLASS(GlobalRef)
	JNI_METH_CLASS(GlobalRef, retryFactory, referenceBackend)

	JNI_CLASS(AbstractReference)
	JNI_METH_CLASS(AbstractReference, get, pyObject)

	JNI_CLASS(JyNIGlobalRef)
	JNI_METH_CLASS(JyNIGlobalRef, initNativeHandle, void, long)

	JNI_CLASS(pyBaseException)
	JNI_CONSTRUCTOR(pyBaseException, emptyConstructor)
	JNI_CONSTRUCTOR(pyBaseException, subTypeConstructor, pyType)
	JNI_METH_CLASS(pyBaseException, __init__, void, pyObject array, string array)
	JNI_METH_CLASS(pyBaseException, __setstate__, pyObject, pyObject)
	JNI_METH_CLASS(pyBaseException, __unicode__, pyUnicode)
	JNI_FIELD(pyBaseException, args, pyObject)
	JNI_METH_CLASS(pyBaseException, setArgs, void, pyObject)
	JNI_METH_CLASS(pyBaseException, getMessage, pyObject)
	JNI_METH_CLASS(pyBaseException, setMessage, void, pyObject)

//	JNI_CLASS(pyByteArray)
//	JNI_CLASS(pyBuffer)
//	JNI_CLASS(pyMemoryView)

	JNI_CLASS(__builtin__)
	JNI_METH_STATIC2(__builtin__, __import__, ImportLevel, pyObject,
			string, pyObject, pyObject, pyObject, int)

	JNI_CLASS(imp)
	JNI_METH_STATIC(imp, importName, pyObject, string, boolean)
	JNI_METH_STATIC(imp, reload, pyObject, pyModule)

	JNI_CLASS(exceptions)
	JNI_METH_STATIC(exceptions, KeyError, pyObject)
	JNI_METH_STATIC(exceptions, KeyError__str__, pyObject,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, EnvironmentError, pyObject)
	JNI_METH_STATIC(exceptions, EnvironmentError__init__, void,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, EnvironmentError__str__, pyObject,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, EnvironmentError__reduce__, pyObject,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, SyntaxError, pyObject)
	JNI_METH_STATIC(exceptions, SyntaxError__init__, void,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, SyntaxError__str__, pyString,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, SystemExit, pyObject)
	JNI_METH_STATIC(exceptions, SystemExit__init__, void,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, UnicodeError, pyObject)
	#ifdef Py_USING_UNICODE
	JNI_METH_STATIC(exceptions, getStart, int, pyObject, boolean)
	JNI_METH_STATIC(exceptions, getEnd, int, pyObject, boolean)
	JNI_METH_STATIC(exceptions, getString, pyString, pyObject, string)
	JNI_METH_STATIC(exceptions, getUnicode, pyUnicode, pyObject, string)
	JNI_METH_STATIC(exceptions, UnicodeError__init__, void,
			pyObject, pyObject array, string array, pyType)
	JNI_METH_STATIC(exceptions, UnicodeEncodeError, pyObject)
	JNI_METH_STATIC(exceptions, UnicodeEncodeError__init__, void,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, UnicodeEncodeError__str__, pyString,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, UnicodeDecodeError, pyObject)
	JNI_METH_STATIC(exceptions, UnicodeDecodeError__init__, void,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, UnicodeDecodeError__str__, pyString,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, UnicodeTranslateError, pyObject)
	JNI_METH_STATIC(exceptions, UnicodeTranslateError__init__, void,
			pyObject, pyObject array, string array)
	JNI_METH_STATIC(exceptions, UnicodeTranslateError__str__, pyString,
			pyObject, pyObject array, string array)
	#endif

	return JNI_VERSION_1_2;
}

inline jint initSingletons(JNIEnv *env)
{
	JNI_SINGLETON(pyPy, None, pyObject)
	JNI_SINGLETON(pyPy, NotImplemented, pyObject)
	JNI_SINGLETON(pyPy, Ellipsis, pyObject)
	JNI_SINGLETON(pyPy, EmptyFrozenSet, pyFrozenSet)
	JNI_SINGLETON(pyPy, EmptyString, pyString)
	JNI_SINGLETON(pyPy, EmptyUnicode, pyUnicode)
	JNI_SINGLETON(pyPy, EmptyTuple, pyTuple)
	JNI_SINGLETON2(pyPy, NoKeywords, EmptyStringArray, string array)
	JNI_SINGLETON2(pyPy, EmptyObjects, EmptyPyObjectArray, pyObject array)
	JNI_SINGLETON(pyPy, True, pyBoolean)
	JNI_SINGLETON(pyPy, False, pyBoolean)

	return JNI_VERSION_1_2;
}

#define native_table_index_long 10
#define native_table_index_ulong 11
static inline void patchJythonStructModule(JNIEnv *env)
{
	if (sizeof(long) != 4)
	{
		jclass structModuleClass = (*env)->FindClass(env, pack_structModule);
		jfieldID JNI_FIELD_STATIC(structModule, native_table, structModuleFormatDef array)
		jclass structModuleFormatDefClass = (*env)->FindClass(env, pack_structModuleFormatDef);
		jfieldID JNI_FIELD(structModuleFormatDef, size, int)
		jfieldID JNI_FIELD(structModuleFormatDef, alignment, int)

		jarray struct_native_table = (*env)->GetStaticObjectField(env, structModuleClass, structModule_native_tableField);
		jobject long_FormatDef = (*env)->GetObjectArrayElement(env, struct_native_table, native_table_index_long);
		jobject ulong_FormatDef = (*env)->GetObjectArrayElement(env, struct_native_table, native_table_index_ulong);
		(*env)->SetIntField(env, long_FormatDef, structModuleFormatDef_sizeField, sizeof(long));
		(*env)->SetIntField(env, long_FormatDef, structModuleFormatDef_alignmentField, sizeof(long));
		(*env)->SetIntField(env, ulong_FormatDef, structModuleFormatDef_sizeField, sizeof(long));
		(*env)->SetIntField(env, ulong_FormatDef, structModuleFormatDef_alignmentField, sizeof(long));
		(*env)->DeleteLocalRef(env, structModuleClass);
		(*env)->DeleteLocalRef(env, structModuleFormatDefClass);
	}
}

//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
jint JyNI_init(JavaVM *jvm)
{
	java = jvm; // cache the JavaVM pointer
	JNIEnv *env;

	if ((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_2)) {
		return JNI_ERR; // JNI version not supported
	}
	JyHash_init();
	//Py_Py3kWarningFlag
	if (initJNI(env) == JNI_ERR) return JNI_ERR;
	if (initJyNI(env) == JNI_ERR) return JNI_ERR;
	if (initJythonSite(env) == JNI_ERR) return JNI_ERR;
	if (initJythonObjects(env) == JNI_ERR) return JNI_ERR;
	initBuiltinTypes();
	initBuiltinExceptions();
	if (initSingletons(env) == JNI_ERR) return JNI_ERR;

	patchJythonStructModule(env);

	//init native objects where necessary:
	PyEval_InitThreads();
	_PyInt_Init();
	PyType_Ready(&PyBool_Type);

	return JNI_VERSION_1_2;
}

void JyNI_unload(JavaVM *jvm)
{
	jputs("JyNI_unload");
	PyString_Fini();
	PyInt_Fini();
	PyTuple_Fini();
	PyDict_Fini();
	PyCFunction_Fini();
	PySet_Fini();

	int i;
	for (i = 0; i < builtinTypeCount; ++i)
	{
		if (builtinTypes[i].sync != NULL) free(builtinTypes[i].sync);
		if (builtinTypes[i].type_name != NULL) free(builtinTypes[i].type_name);
	}

	//free(specialPyInstance.sync);

	env();
	(*env)->DeleteWeakGlobalRef(env, JyNone);
	(*env)->DeleteWeakGlobalRef(env, JyNotImplemented);
	(*env)->DeleteWeakGlobalRef(env, JyEllipsis);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyFrozenSet);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyString);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyUnicode);
	(*env)->DeleteWeakGlobalRef(env, JyTrue);
	(*env)->DeleteWeakGlobalRef(env, JyFalse);

	(*env)->DeleteWeakGlobalRef(env, JyEmptyStringArray);
	(*env)->DeleteWeakGlobalRef(env, JyEmptyPyObjectArray);
}
