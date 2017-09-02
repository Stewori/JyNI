/*
 * Compat_C89.h
 *
 *  Created on: 19.06.2017
 *      Author: Stefan
 */

#ifndef INCLUDE_COMPAT_C89_MSVC_H_
#define INCLUDE_COMPAT_C89_MSVC_H_

#ifdef MS_WINDOWS
#define inline
#define VLA_DECL(type, name) type * name
#define VLA(type, name, size) name = _alloca(sizeof(type)*(size))
#else
#define VLA_DECL(type, name)
#define VLA(type, name, size) type name[size]
#endif

#endif /* INCLUDE_COMPAT_C89_MSVC_H_ */
