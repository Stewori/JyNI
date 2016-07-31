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
 * JyHash.c
 *
 *  Created on: 30.07.2016
 *      Author: Stefan Richthofer
 *
 *
 * This file implements a hash-table from scratch. It is intended for C pointer
 * hashing, and we attempt to optimize it for this purpose over time. E.g. it already
 * turned out that for pointers, which usually differ mainly in the lower bits
 * mapping the plain pointer value into table space (value % capacity) yield
 * sufficient table-fill results. Using Knuth's hash function (value * factor)
 * does not notably improve this behavior in this usecase.
 * k-th rehashing is done by k-left-bitshift before applying "modulo capacity".
 * We increase table size whenever rehashing-sequence exceeds a maximal count.
 * With maxCount == 40 we usually achieve 75%-90% table fill when resize is needed.
 * Currently maxCount is constant, but probably should be formulated as fraction of
 * tableSize.
 * For resizing we roughly double the size, but ensure table-size is odd (though
 * maybe not needed for the trivial hash-function we ended up with).
 * In the rare case that the resized table directly exceeds max rehash-count when
 * we init it with the old values, we abort the resize operation and make a new
 * attempt with additional 10% increased size.
 * As of this writing table-shrinking is never applied. (Will be added later.)
 * Deletion is applied by inserting a constant dummy element. Currently no cleaning
 * is applied and dummies are only removed on resize.
 * We plan to add some flags that would allow in-place resize, in-place dummy
 * clean-up and true deletion for elements that were never stepped over by rehash.
 * However so far delDummies tend not to accumulate critically, even on frequent
 * use of del-feature.
 *
 * JyNI uses this hashtable to track dynamically (m)allocated pointers, because there
 * is no good way to distinguish them from static memory positions. This discrimination
 * is (currently) crucial for AS_JY, FROM_JY and friends to work properly/determine when
 * they can be used safely.
 * We could have used a bloom-filter or something else to achieve this more efficiently,
 * as long as only membership-testing is required. However we plan to migrate the whole
 * lookup-mechanism, which is currently header-based, to this hashtable, vastly
 * simplifying various parts of JyNI. Using a similar mechanism for the other lookup
 * direction on native site would reduce JVM-callbacks and potentially improve performance;
 * however it might still be a bottle-neck to consistently obtain hash-values for jobjects,
 * because this is only doable on Java-site. We would later expand the interface to directly
 * pass corresponding hashes down to native site whenever Java-objects are handed down, but
 * that's a future optimization.
 */

#include "JyNI.h"

//#define initSize 50021
//#define initSize 11939
#define initSize 1019
#define maxRehash 40

//#define initSize 13
//#define maxRehash 10

size_t tableCapacity = initSize, tableSize = 0, delDummies = 0, collisions = 0;

#define increasedSize 2*tableCapacity-1

/*
 * Todo:
 * Add some flags, e.g. steppedOver, rehashed, reinserted
 * which allow some improvements (but increase size on the other hand)
 * - true deletion possible for never over-stepped bins
 * - table-cleaning (remove delDummies) possible in-place
 * - table-resize possible in-place module memmove by realloc
 */

typedef struct {
	void* ptr;
} TableEntry;

TableEntry* ptrTable;

// We use the table-pointer itself as delDummy, because it
// will surely never be inserted into the table as a regular value.
#define delDummy ptrTable

/*
 * It appears for memory pointers the identity modulo capacity
 * is actually a rather good hash function.
 */
//#define hash_func(i) ((jlong) i)*2654435761 % tableCapacity
#define hash_func(i) ((jlong) i) % tableCapacity

// For now we just hash the position repeatedly
//#define rehash_func(i, k, prev) hash_func(prev)
#define rehash_func(i, k, prev) hash_func((((jlong) i) << k))
//#define rehash_func(i, k, prev) hash_func((((jlong) i) << k)*((jlong) prev))
//#define rehash_func(i, k, prev) hash_func(i >> k)

inline void JyHash_init()
{
	ptrTable = (TableEntry*) calloc(tableCapacity, sizeof(TableEntry));
	if (!ptrTable) jputs("JyHash_init failed!");
}

inline void JyHash_printTable()
{
	int i;
	for (i = 0; i < tableCapacity; ++i)
	{
		if (ptrTable[i].ptr != delDummy) printf("%i: %i\n", i, ptrTable[i]);
		else printf("%i: %s\n", i, "--");
	}
}

static inline ssize_t getPos(void* ptr)
{
//	GET_POS(!ptrTable[pos].ptr)
	size_t pos = hash_func(ptr);
//	printf("%i %i\n", pos, ptrTable[pos].ptr);
	if (ptrTable[pos].ptr == ptr) return pos;
	if (!ptrTable[pos].ptr) return ~pos;
	int rehash = 1;
	pos = rehash_func(ptr, rehash, pos);
	while (ptrTable[pos].ptr != ptr)
	{
		if (!ptrTable[pos].ptr) return ~pos;
		if (++rehash > maxRehash) return ~tableCapacity;
		pos = rehash_func(ptr, rehash, pos);
	}
	return pos;
}

/*
 * Like getPos, but counts collisions.
 */
static inline ssize_t getPosForInsert0(void* ptr)
{
//	GET_POS(!ptrTable[pos].ptr)
	size_t pos = hash_func(ptr);
//	printf("%i %i\n", pos, ptrTable[pos].ptr);
	if (ptrTable[pos].ptr == ptr) return pos;
	if (!ptrTable[pos].ptr) return ~pos;
	int rehash = 1;
	++collisions;
	pos = rehash_func(ptr, rehash, pos);
	while (ptrTable[pos].ptr != ptr)
	{
		if (!ptrTable[pos].ptr) return ~pos;
		if (++rehash > maxRehash) return ~tableCapacity;
		++collisions;
		//printf("rehash0 %i over %i\n", ptr, ptrTable[pos].ptr);
		pos = rehash_func(ptr, rehash, pos);
	}
	return pos;
}

static inline ssize_t getPosForInsert(void* ptr)
{
//	GET_POS(!ptrTable[pos].ptr || ptrTable[pos].ptr == delDummy)
	ssize_t freePos = -1, pos = hash_func(ptr);
//	printf("%i %i\n", pos, pos);
	if (ptrTable[pos].ptr == ptr) return pos;
	else if (!ptrTable[pos].ptr) return ~pos;
	else if (ptrTable[pos].ptr == delDummy && freePos == -1) freePos = pos;
	int rehash = 1;
	++collisions;
	//printf("rehash %i over %i\n", ptr, ptrTable[pos].ptr);
	pos = rehash_func(ptr, rehash, pos);
	while (ptrTable[pos].ptr != ptr)
	{
		if (!ptrTable[pos].ptr) return freePos != -1 ? (--delDummies, ~freePos) : ~pos;
		else if (ptrTable[pos].ptr == delDummy && freePos == -1) freePos = pos;
		if (++rehash > maxRehash) return freePos != -1 ? (--delDummies, ~freePos) : ~tableCapacity;
		++collisions;
		//printf("rehash %i over %i\n", ptr, ptrTable[pos].ptr);
		pos = rehash_func(ptr, rehash, pos);
	}
	return pos;
}

inline static void resize_ptrTable(size_t newSize);

/*
 * insert0 doesn't care for delDummies, it's intended only for use
 * with fresh tables.
 */
static inline int insert0(void* ptr)
{
	ssize_t pos = getPosForInsert0(ptr);
	if (pos < 0) {
		if (pos == ~tableCapacity) {
//			printf("Warning: maximal rehash exhausted in insert0");
			return ~tableCapacity; //abort resize in this case
		}
		ptrTable[~pos].ptr = ptr;
		++tableSize;
	}
	return pos < 0;
}

inline static void resize_ptrTable(size_t newSize)
{
	TableEntry* oldTable = ptrTable;
	ssize_t i = tableCapacity-1, oldCapacity = tableCapacity;
	tableCapacity = newSize;
	ptrTable = (TableEntry*) calloc(tableCapacity, sizeof(TableEntry));
	if (!ptrTable) jputs("resize_ptrTable failed!");
	delDummies = 0;
	tableSize = 0;
	collisions = 0;
	for (; i >= 0; --i) {// oldTable counts as delDummy here
		//if (i < 0) printf("%i\n", i);
		if (oldTable[i].ptr && oldTable[i].ptr != oldTable)
		{
			if (insert0(oldTable[i].ptr) == ~tableCapacity)
			{
				// abort resize and choose different size
				// this will become much cheaper once we support
				// in-place resize via realloc
				tableCapacity *= 1.10;
//				printf("Adjusted capacity: %i\n", tableCapacity);
				// Ensure it's odd
				if (tableCapacity % 2 == 0) tableCapacity -= 1;
				i = oldCapacity-1;
				free(ptrTable);
				ptrTable = (TableEntry*) calloc(tableCapacity, sizeof(TableEntry));
			}
		}
	}
	free(oldTable);
}

inline int JyHash_insert(void* ptr)
{
	ssize_t pos = getPosForInsert(ptr);
	//printf("%i %i\n", pos, pos);//ptrTable[pos].ptr);
	if (pos < 0) {
		if (pos == ~tableCapacity) {
//			printf("Resize table to %i\n", increasedSize);
//			printf("Collisions %i\n", collisions);
//			printf("Size %i\n", tableSize);
//			printf("delDummies %i\n", delDummies);
//			printf("Load %f\%\n", 100.0*tableSize/tableCapacity);
			resize_ptrTable(increasedSize);
//			puts("resize done");
			return insert0(ptr); //resize in this case
		}
		ptrTable[~pos].ptr = ptr;
		++tableSize;
	}
	return pos < 0;
}

inline void clean_table()
{
	resize_ptrTable(tableCapacity);
}

inline int JyHash_contains(void* ptr)
{
	ssize_t pos = getPos(ptr);
//	printf("%i %i\n", pos, pos>=0);
	return pos >= 0;
}

inline int JyHash_delete(void* ptr)
{
	ssize_t pos = getPos(ptr);
	if (pos >= 0) {
		ptrTable[pos].ptr = delDummy;
		++delDummies;
		--tableSize;
	}
	return pos >= 0;
}
