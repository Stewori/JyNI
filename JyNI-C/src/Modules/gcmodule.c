/* This File is based on gcmodule.c from CPython 2.7.3 release.
 * It has been modified to suit JyNI needs.
 *
 *
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

  Reference Cycle Garbage Collection
  ==================================

  Neil Schemenauer <nas@arctrix.com>

  Based on a post on the python-dev list.  Ideas from Guido van Rossum,
  Eric Tiedemann, and various others.

  http://www.arctrix.com/nas/python/gc/
  http://www.python.org/pipermail/python-dev/2000-March/003869.html
  http://www.python.org/pipermail/python-dev/2000-March/004010.html
  http://www.python.org/pipermail/python-dev/2000-March/004022.html

  For a highlevel view of the collection process, read the collect
  function.

*/

//#include "Python.h"
#include "JyNI.h"
#include "JyNI-Debug.h"
//#include "frameobject.h"		// for PyFrame_ClearFreeList


// Get an object's GC head
#define AS_GC(o) ((PyGC_Head *)(o)-1)

// Get the object given the GC head
#define FROM_GC(g) ((PyObject *)(((PyGC_Head *)g)+1))

// *** Global GC state ***

//struct gc_generation {
//	PyGC_Head head;
//	int threshold; // collection threshold
//	int count; // count of allocations or collections of younger
//			   // generations
//};

//#define NUM_GENERATIONS 3
//#define GEN_HEAD(n) (&generations[n].head)

// linked lists of container objects
//static struct gc_generation generations[NUM_GENERATIONS] = {
//	// PyGC_Head,							   threshold,	  count
//	{{{GEN_HEAD(0), GEN_HEAD(0), 0}},           700,            0},
//	{{{GEN_HEAD(1), GEN_HEAD(1), 0}},           10,             0},
//	{{{GEN_HEAD(2), GEN_HEAD(2), 0}},           10,             0},
//};
//static struct gc_generation generations = {{{GEN_HEAD, GEN_HEAD, 0}}, 700, 0};

static PyGC_Head generations = {{&generations, &generations, 0}};
PyGC_Head *_PyGC_generation0 = &generations;

/* Exploration-stack implementation.
 * Note that the stack explicitly
 * does *not* incref or decref!
 */
#define EX_STACK_BLOCK_SIZE 100

typedef struct Ex_Stack_Block Ex_Stack_Block; /* Forward declaration */
struct Ex_Stack_Block {
	int position;
	PyObject* stack[EX_STACK_BLOCK_SIZE];
	Ex_Stack_Block* next;
};

static Ex_Stack_Block explorationStack;
//static int exStackGapCount = 0;

static void pushExStack(PyObject* op) {
	if (explorationStack.position < EX_STACK_BLOCK_SIZE) {
		explorationStack.stack[explorationStack.position++] = op;
	} else {
		Ex_Stack_Block* stack = explorationStack.next;
		if (stack && stack->position < EX_STACK_BLOCK_SIZE) {
			stack->stack[stack->position++] = op;
		} else {
			Ex_Stack_Block* newNext = (Ex_Stack_Block*) malloc(sizeof(Ex_Stack_Block));
			newNext->stack[0] = op;
			newNext->position = 1;
			newNext->next = stack;
			explorationStack.next = newNext;
		}
	}
}
/*
static void cleanExStackTop() {
	if (exStackGapCount && explorationStack.position > 0) {
		if (explorationStack.next) {
			jputsLong(__LINE__);
			Ex_Stack_Block* stack = explorationStack.next;
			while (stack && stack->position > 0 && stack->stack[stack->position-1]) {
				--stack->position;
				--exStackGapCount;
				if (!stack->position) {
					explorationStack.next = stack->next;
					free(stack);
					stack = explorationStack.next;
				}
			}
			if (!explorationStack.next) {
				while (explorationStack.position > 0 && !explorationStack.stack[explorationStack.position-1]) {
					--explorationStack.position;
					--exStackGapCount;
				}
			}
		} else {
			jputsLong(__LINE__);
			jputsLong(explorationStack.position-1);
			while (explorationStack.position > 0 && !explorationStack.stack[explorationStack.position-1]) {
				--explorationStack.position;
				--exStackGapCount;
			}
		}
	}
}*/

static PyObject* popExStack() {
	//jputs(__FUNCTION__);
//	if (explorationStack.position < EX_STACK_BLOCK_SIZE) {
//		jputsLong(explorationStack.position);
//		return explorationStack.stack[--explorationStack.position];
//	} else
	if (explorationStack.next) {
		Ex_Stack_Block* stack = explorationStack.next;
		if (stack->position > 1) {
			return stack->stack[--stack->position];
		} else {
			PyObject* result = stack->stack[--stack->position];
			explorationStack.next = stack->next;
			free(stack);
			return result;
		}
	} else if (explorationStack.position > 0)//explorationStack.position == EX_STACK_BLOCK_SIZE
		return explorationStack.stack[--explorationStack.position];
	else
		return NULL;
}

/*static inline PyObject* popExStack() {
	if (!explorationStack.position) return NULL;
	PyObject* result = _popExStack();
	while (!result && explorationStack.position)
	{
		result = _popExStack();
		--exStackGapCount;
	}
	cleanExStackTop();
	return result;
}*/

static jboolean removeObjectExStack(PyObject* op) {
	//jputs(__FUNCTION__);
	int i;
	jboolean result = JNI_FALSE;
	for (i = explorationStack.position-1; i >= 0; --i) {
		if (explorationStack.stack[i] == op) {
			explorationStack.stack[i] = explorationStack.stack[--explorationStack.position];
			result = JNI_TRUE;
			//++exStackGapCount;
		}
	}
	Ex_Stack_Block* stack = explorationStack.next;
	while (stack) {
		for (i = stack->position-1; i >= 0; --i) {
			if (stack->stack[i] == op) {
				stack->stack[i] = stack->stack[--stack->position];
				result = JNI_TRUE;
				//++exStackGapCount;
			}
		}
		stack = stack->next;
	}
	return result;
}

static jboolean isExStackEmpty() {
	return !explorationStack.next && explorationStack.position == 0;
}

/* Nothing wrong with this method, but currently not used.
static jint exStackSize() {
	jint result = explorationStack.position;
	Ex_Stack_Block* stack = explorationStack.next;
	while(stack) {
		result += stack->position;
		stack = stack->next;
	}
	return result;//-exStackGapCount;
}
*/

static jint exStackBlockCount() {
	jint result = 1;
	Ex_Stack_Block* stack = explorationStack.next;
	while(stack)
		++result;
	return result;
}

//static int enabled = 1; // automatic collection enabled?
//
//// true if we are currently running the collector
//static int collecting = 0;
//
//// list of uncollectable objects
//static PyObject *garbage = NULL;
//
//// Python string to use if unhandled exception occurs
//static PyObject *gc_str = NULL;
//
//// Python string used to look for __del__ attribute.
//static PyObject *delstr = NULL;
//
//// This is the number of objects who survived the last full collection. It
//// approximates the number of long lived objects tracked by the GC.
//
//// (by "full collection", we mean a collection of the oldest generation).
//
//static Py_ssize_t long_lived_total = 0;
//
//// This is the number of objects who survived all "non-full" collections,
//// and are awaiting to undergo a full collection for the first time.
//
//static Py_ssize_t long_lived_pending = 0;


//   NOTE: about the counting of long-lived objects.
//
//   To limit the cost of garbage collection, there are two strategies;
//	 - make each collection faster, e.g. by scanning fewer objects
//	 - do less collections
//   This heuristic is about the latter strategy.
//
//   In addition to the various configurable thresholds, we only trigger a
//   full collection if the ratio
//	long_lived_pending / long_lived_total
//   is above a given value (hardwired to 25%).
//
//   The reason is that, while "non-full" collections (i.e., collections of
//   the young and middle generations) will always examine roughly the same
//   number of objects -- determined by the aforementioned thresholds --,
//   the cost of a full collection is proportional to the total number of
//   long-lived objects, which is virtually unbounded.
//
//   Indeed, it has been remarked that doing a full collection every
//   <constant number> of object creations entails a dramatic performance
//   degradation in workloads which consist in creating and storing lots of
//   long-lived objects (e.g. building a large list of GC-tracked objects would
//   show quadratic performance, instead of linear as expected: see issue #4074).
//
//   Using the above ratio, instead, yields amortized linear performance in
//   the total number of objects (the effect of which can be summarized
//   thusly: "each full garbage collection is more and more costly as the
//   number of objects grows, but we do fewer and fewer of them").
//
//   This heuristic was suggested by Martin von Löwis on python-dev in
//   June 2008. His original analysis and proposal can be found at:
//	http://mail.python.org/pipermail/python-dev/2008-June/080579.html


/* set for debugging information */
#define DEBUG_STATS             (1<<0) /* print collection statistics */
#define DEBUG_COLLECTABLE       (1<<1) /* print collectable objects */
#define DEBUG_UNCOLLECTABLE     (1<<2) /* print uncollectable objects */
#define DEBUG_INSTANCES         (1<<3) /* print instances */
#define DEBUG_OBJECTS           (1<<4) /* print other objects */
#define DEBUG_SAVEALL           (1<<5) /* save all garbage in gc.garbage */
#define DEBUG_LEAK              DEBUG_COLLECTABLE | \
                                DEBUG_UNCOLLECTABLE | \
                                DEBUG_INSTANCES | \
                                DEBUG_OBJECTS | \
                                DEBUG_SAVEALL
//static int debug;
//static PyObject *tmod = NULL;

//--------------------------------------------------------------------------
//gc_refs values.
//
//Between collections, every gc'ed object has one of two gc_refs values:
//
//GC_UNTRACKED
//	The initial state; objects returned by PyObject_GC_Malloc are in this
//	state.  The object doesn't live in any generation list, and its
//	tp_traverse slot must not be called.
//
//GC_REACHABLE
//	The object lives in some generation list, and its tp_traverse is safe to
//	call.  An object transitions to GC_REACHABLE when PyObject_GC_Track
//	is called.
//
//During a collection, gc_refs can temporarily take on other states:
//
//>= 0
//	At the start of a collection, update_refs() copies the true refcount
//	to gc_refs, for each object in the generation being collected.
//	subtract_refs() then adjusts gc_refs so that it equals the number of
//	times an object is referenced directly from outside the generation
//	being collected.
//	gc_refs remains >= 0 throughout these steps.
//
//GC_TENTATIVELY_UNREACHABLE
//	move_unreachable() then moves objects not reachable (whether directly or
//	indirectly) from outside the generation into an "unreachable" set.
//	Objects that are found to be reachable have gc_refs set to GC_REACHABLE
//	again.  Objects that are found to be unreachable have gc_refs set to
//	GC_TENTATIVELY_UNREACHABLE.  It's "tentatively" because the pass doing
//	this can't be sure until it ends, and GC_TENTATIVELY_UNREACHABLE may
//	transition back to GC_REACHABLE.
//
//	Only objects with GC_TENTATIVELY_UNREACHABLE still set are candidates
//	for collection.  If it's decided not to collect such an object (e.g.,
//	it has a __del__ method), its gc_refs is restored to GC_REACHABLE again.
//----------------------------------------------------------------------------

#define GC_UNTRACKED					_PyGC_REFS_UNTRACKED
#define GC_REACHABLE					_PyGC_REFS_REACHABLE
#define GC_TENTATIVELY_UNREACHABLE	  _PyGC_REFS_TENTATIVELY_UNREACHABLE

#define GC_UNEXPLORED _PyGC_REFS_UNEXPLORED
#define GC_EXPLORING _PyGC_REFS_EXPLORING
#define GC_EXPLORED _PyGC_REFS_EXPLORED

#define IS_UNEXPLORED(op) \
	(!IsReadyType(op) && (!PyObject_IS_GC(op) || (AS_GC(op)->gc.gc_refs < 0 && AS_GC(op)->gc.gc_refs > GC_EXPLORING)))

//not used
//#define IS_EXPLORED(op) \
//	(IsReadyType(op) || (AS_GC(op)->gc.gc_refs > 0 || AS_GC(op)->gc.gc_refs < GC_EXPLORING))

#define IS_TRACKED(o) ((AS_GC(o))->gc.gc_refs != GC_UNTRACKED)
#define IS_REACHABLE(o) ((AS_GC(o))->gc.gc_refs == GC_REACHABLE)
#define IS_TENTATIVELY_UNREACHABLE(o) ( \
	(AS_GC(o))->gc.gc_refs == GC_TENTATIVELY_UNREACHABLE)

// *** list functions ***

//static void
//gc_list_init(PyGC_Head *list)
//{
//	list->gc.gc_prev = list;
//	list->gc.gc_next = list;
//}

//static int
//gc_list_is_empty(PyGC_Head *list)
//{
//	return (list->gc.gc_next == list);
//}

//#if 0
//// This became unused after gc_list_move() was introduced.
//// Append `node` to `list`.
//static void
//gc_list_append(PyGC_Head *node, PyGC_Head *list)
//{
//	node->gc.gc_next = list;
//	node->gc.gc_prev = list->gc.gc_prev;
//	node->gc.gc_prev->gc.gc_next = node;
//	list->gc.gc_prev = node;
//}
//#endif

// Remove `node` from the gc list it's currently in.
//static void
//gc_list_remove(PyGC_Head *node)
//{
//	node->gc.gc_prev->gc.gc_next = node->gc.gc_next;
//	node->gc.gc_next->gc.gc_prev = node->gc.gc_prev;
//	node->gc.gc_next = NULL; // object is not currently tracked
//}

// Move `node` from the gc list it's currently in (which is not explicitly
// named here) to the end of `list`.  This is semantically the same as
// gc_list_remove(node) followed by gc_list_append(node, list).

//static void
//gc_list_move(PyGC_Head *node, PyGC_Head *list)
//{
//	PyGC_Head *new_prev;
//	PyGC_Head *current_prev = node->gc.gc_prev;
//	PyGC_Head *current_next = node->gc.gc_next;
//	// Unlink from current list.
//	current_prev->gc.gc_next = current_next;
//	current_next->gc.gc_prev = current_prev;
//	// Relink at end of new list.
//	new_prev = node->gc.gc_prev = list->gc.gc_prev;
//	new_prev->gc.gc_next = list->gc.gc_prev = node;
//	node->gc.gc_next = list;
//}

// append list `from` onto list `to`; `from` becomes an empty list
//static void
//gc_list_merge(PyGC_Head *from, PyGC_Head *to)
//{
//	PyGC_Head *tail;
//	assert(from != to);
//	if (!gc_list_is_empty(from)) {
//		tail = to->gc.gc_prev;
//		tail->gc.gc_next = from->gc.gc_next;
//		tail->gc.gc_next->gc.gc_prev = tail;
//		to->gc.gc_prev = from->gc.gc_prev;
//		to->gc.gc_prev->gc.gc_next = to;
//	}
//	gc_list_init(from);
//}

//static Py_ssize_t
//gc_list_size(PyGC_Head *list)
//{
//	PyGC_Head *gc;
//	Py_ssize_t n = 0;
//	for (gc = list->gc.gc_next; gc != list; gc = gc->gc.gc_next) {
//		n++;
//	}
//	return n;
//}

// Append objects in a GC list to a Python list.
// Return 0 if all OK, < 0 if error (out of memory for list).
/*
static int
append_objects(PyObject *py_list, PyGC_Head *gc_list)
{
	PyGC_Head *gc;
	for (gc = gc_list->gc.gc_next; gc != gc_list; gc = gc->gc.gc_next) {
		PyObject *op = FROM_GC(gc);
		if (op != py_list) {
			if (PyList_Append(py_list, op)) {
				return -1; // exception
			}
		}
	}
	return 0;
}

// *** end of list stuff ***


// Set all gc_refs = ob_refcnt.  After this, gc_refs is > 0 for all objects
// in containers, and is GC_REACHABLE for all tracked gc objects not in
// containers.

static void
update_refs(PyGC_Head *containers)
{
	PyGC_Head *gc = containers->gc.gc_next;
	for (; gc != containers; gc = gc->gc.gc_next) {
		assert(gc->gc.gc_refs == GC_REACHABLE);
		gc->gc.gc_refs = Py_REFCNT(FROM_GC(gc));
//		 * Python's cyclic gc should never see an incoming refcount
//		 * of 0:  if something decref'ed to 0, it should have been
//		 * deallocated immediately at that time.
//		 * Possible cause (if the assert triggers):  a tp_dealloc
//		 * routine left a gc-aware object tracked during its teardown
//		 * phase, and did something-- or allowed something to happen --
//		 * that called back into Python.  gc can trigger then, and may
//		 * see the still-tracked dying object.  Before this assert
//		 * was added, such mistakes went on to allow gc to try to
//		 * delete the object again.  In a debug build, that caused
//		 * a mysterious segfault, when _Py_ForgetReference tried
//		 * to remove the object from the doubly-linked list of all
//		 * objects a second time.  In a release build, an actual
//		 * double deallocation occurred, which leads to corruption
//		 * of the allocator's internal bookkeeping pointers.  That's
//		 * so serious that maybe this should be a release-build
//		 * check instead of an assert?

		assert(gc->gc.gc_refs != 0);
	}
}

// A traversal callback for subtract_refs.
static int
visit_decref(PyObject *op, void *data)
{
	assert(op != NULL);
	if (PyObject_IS_GC(op)) {
		PyGC_Head *gc = AS_GC(op);
		// We're only interested in gc_refs for objects in the
		// generation being collected, which can be recognized
		// because only they have positive gc_refs.

		assert(gc->gc.gc_refs != 0); // else refcount was too small
		if (gc->gc.gc_refs > 0)
			gc->gc.gc_refs--;
	}
	return 0;
}

// Subtract internal references from gc_refs.  After this, gc_refs is >= 0
// for all objects in containers, and is GC_REACHABLE for all tracked gc
// objects not in containers.  The ones with gc_refs > 0 are directly
// reachable from outside containers, and so can't be collected.

static void
subtract_refs(PyGC_Head *containers)
{
	traverseproc traverse;
	PyGC_Head *gc = containers->gc.gc_next;
	for (; gc != containers; gc=gc->gc.gc_next) {
		traverse = Py_TYPE(FROM_GC(gc))->tp_traverse;
		(void) traverse(FROM_GC(gc),
					   (visitproc)visit_decref,
					   NULL);
	}
}

// A traversal callback for move_unreachable.
static int
visit_reachable(PyObject *op, PyGC_Head *reachable)
{
	if (PyObject_IS_GC(op)) {
		PyGC_Head *gc = AS_GC(op);
		const Py_ssize_t gc_refs = gc->gc.gc_refs;

		if (gc_refs == 0) {
			// This is in move_unreachable's 'young' list, but
			// the traversal hasn't yet gotten to it.  All
			// we need to do is tell move_unreachable that it's
			// reachable.

			gc->gc.gc_refs = 1;
		}
		else if (gc_refs == GC_TENTATIVELY_UNREACHABLE) {
//			* This had gc_refs = 0 when move_unreachable got
//			* to it, but turns out it's reachable after all.
//			* Move it back to move_unreachable's 'young' list,
//			* and move_unreachable will eventually get to it
//			* again.

			gc_list_move(gc, reachable);
			gc->gc.gc_refs = 1;
		}
//		 * Else there's nothing to do.
//		 * If gc_refs > 0, it must be in move_unreachable's 'young'
//		 * list, and move_unreachable will eventually get to it.
//		 * If gc_refs == GC_REACHABLE, it's either in some other
//		 * generation so we don't care about it, or move_unreachable
//		 * already dealt with it.
//		 * If gc_refs == GC_UNTRACKED, it must be ignored.

		 else {
			assert(gc_refs > 0
				   || gc_refs == GC_REACHABLE
				   || gc_refs == GC_UNTRACKED);
		 }
	}
	return 0;
}

// * Move the unreachable objects from young to unreachable.  After this,
// * all objects in young have gc_refs = GC_REACHABLE, and all objects in
// * unreachable have gc_refs = GC_TENTATIVELY_UNREACHABLE.  All tracked
// * gc objects not in young or unreachable still have gc_refs = GC_REACHABLE.
// * All objects in young after this are directly or indirectly reachable
// * from outside the original young; and all objects in unreachable are
// * not.

static void
move_unreachable(PyGC_Head *young, PyGC_Head *unreachable)
{
	PyGC_Head *gc = young->gc.gc_next;

//	 * Invariants:  all objects "to the left" of us in young have gc_refs
//	 * = GC_REACHABLE, and are indeed reachable (directly or indirectly)
//	 * from outside the young list as it was at entry.  All other objects
//	 * from the original young "to the left" of us are in unreachable now,
//	 * and have gc_refs = GC_TENTATIVELY_UNREACHABLE.  All objects to the
//	 * left of us in 'young' now have been scanned, and no objects here
//	 * or to the right have been scanned yet.


	while (gc != young) {
		PyGC_Head *next;

		if (gc->gc.gc_refs) {
//			 * gc is definitely reachable from outside the
//			 * original 'young'.  Mark it as such, and traverse
//			 * its pointers to find any other objects that may
//			 * be directly reachable from it.  Note that the
//			 * call to tp_traverse may append objects to young,
//			 * so we have to wait until it returns to determine
//			 * the next object to visit.

			PyObject *op = FROM_GC(gc);
			traverseproc traverse = Py_TYPE(op)->tp_traverse;
			assert(gc->gc.gc_refs > 0);
			gc->gc.gc_refs = GC_REACHABLE;
			(void) traverse(op,
							(visitproc)visit_reachable,
							(void *)young);
			next = gc->gc.gc_next;
			if (PyTuple_CheckExact(op)) {
				_PyTuple_MaybeUntrack(op);
			}
			else if (PyDict_CheckExact(op)) {
				_PyDict_MaybeUntrack(op);
			}
		}
		else {
//			 * This *may* be unreachable.  To make progress,
//			 * assume it is.  gc isn't directly reachable from
//			 * any object we've already traversed, but may be
//			 * reachable from an object we haven't gotten to yet.
//			 * visit_reachable will eventually move gc back into
//			 * young if that's so, and we'll see it again.

			next = gc->gc.gc_next;
			gc_list_move(gc, unreachable);
			gc->gc.gc_refs = GC_TENTATIVELY_UNREACHABLE;
		}
		gc = next;
	}
}

// * Return true if object has a finalization method.
// * CAUTION:  An instance of an old-style class has to be checked for a
// *__del__ method, and earlier versions of this used to call PyObject_HasAttr,
// * which in turn could call the class's __getattr__ hook (if any).  That
// * could invoke arbitrary Python code, mutating the object graph in arbitrary
// * ways, and that was the source of some excruciatingly subtle bugs.

static int
has_finalizer(PyObject *op)
{
	if (PyInstance_Check(op)) {
		assert(delstr != NULL);
		return _PyInstance_Lookup(op, delstr) != NULL;
	}
	else if (PyType_HasFeature(op->ob_type, Py_TPFLAGS_HEAPTYPE))
		return op->ob_type->tp_del != NULL;
	else if (PyGen_CheckExact(op))
		return PyGen_NeedsFinalizing((PyGenObject *)op);
	else
		return 0;
}

// * Move the objects in unreachable with __del__ methods into `finalizers`.
// * Objects moved into `finalizers` have gc_refs set to GC_REACHABLE; the
// * objects remaining in unreachable are left at GC_TENTATIVELY_UNREACHABLE.

static void
move_finalizers(PyGC_Head *unreachable, PyGC_Head *finalizers)
{
	PyGC_Head *gc;
	PyGC_Head *next;

	// March over unreachable.  Move objects with finalizers into
	// `finalizers`.

	for (gc = unreachable->gc.gc_next; gc != unreachable; gc = next) {
		PyObject *op = FROM_GC(gc);

		assert(IS_TENTATIVELY_UNREACHABLE(op));
		next = gc->gc.gc_next;

		if (has_finalizer(op)) {
			gc_list_move(gc, finalizers);
			gc->gc.gc_refs = GC_REACHABLE;
		}
	}
}

// A traversal callback for move_finalizer_reachable.
static int
visit_move(PyObject *op, PyGC_Head *tolist)
{
	if (PyObject_IS_GC(op)) {
		if (IS_TENTATIVELY_UNREACHABLE(op)) {
			PyGC_Head *gc = AS_GC(op);
			gc_list_move(gc, tolist);
			gc->gc.gc_refs = GC_REACHABLE;
		}
	}
	return 0;
}

// Move objects that are reachable from finalizers, from the unreachable set
// into finalizers set.

static void
move_finalizer_reachable(PyGC_Head *finalizers)
{
	traverseproc traverse;
	PyGC_Head *gc = finalizers->gc.gc_next;
	for (; gc != finalizers; gc = gc->gc.gc_next) {
		// Note that the finalizers list may grow during this.
		traverse = Py_TYPE(FROM_GC(gc))->tp_traverse;
		(void) traverse(FROM_GC(gc),
						(visitproc)visit_move,
						(void *)finalizers);
	}
}

// * Clear all weakrefs to unreachable objects, and if such a weakref has a
// * callback, invoke it if necessary.  Note that it's possible for such
// * weakrefs to be outside the unreachable set -- indeed, those are precisely
// * the weakrefs whose callbacks must be invoked.  See gc_weakref.txt for
// * overview & some details.  Some weakrefs with callbacks may be reclaimed
// * directly by this routine; the number reclaimed is the return value.  Other
// * weakrefs with callbacks may be moved into the `old` generation.  Objects
// * moved into `old` have gc_refs set to GC_REACHABLE; the objects remaining in
// * unreachable are left at GC_TENTATIVELY_UNREACHABLE.  When this returns,
// * no object in `unreachable` is weakly referenced anymore.

static int
handle_weakrefs(PyGC_Head *unreachable, PyGC_Head *old)
{
	PyGC_Head *gc;
	PyObject *op;			   // generally FROM_GC(gc)
	PyWeakReference *wr;		// generally a cast of op
	PyGC_Head wrcb_to_call;	 // weakrefs with callbacks to call
	PyGC_Head *next;
	int num_freed = 0;

	gc_list_init(&wrcb_to_call);

//	 * Clear all weakrefs to the objects in unreachable.  If such a weakref
//	 * also has a callback, move it into `wrcb_to_call` if the callback
//	 * needs to be invoked.  Note that we cannot invoke any callbacks until
//	 * all weakrefs to unreachable objects are cleared, lest the callback
//	 * resurrect an unreachable object via a still-active weakref.  We
//	 * make another pass over wrcb_to_call, invoking callbacks, after this
//	 * pass completes.

	for (gc = unreachable->gc.gc_next; gc != unreachable; gc = next) {
		PyWeakReference **wrlist;

		op = FROM_GC(gc);
		assert(IS_TENTATIVELY_UNREACHABLE(op));
		next = gc->gc.gc_next;

		if (! PyType_SUPPORTS_WEAKREFS(Py_TYPE(op)))
			continue;

		// It supports weakrefs.  Does it have any?
		wrlist = (PyWeakReference **)
								PyObject_GET_WEAKREFS_LISTPTR(op);

//		 * `op` may have some weakrefs.  March over the list, clear
//		 * all the weakrefs, and move the weakrefs with callbacks
//		 * that must be called into wrcb_to_call.

		for (wr = *wrlist; wr != NULL; wr = *wrlist) {
			PyGC_Head *wrasgc;				  // AS_GC(wr)

//			 * _PyWeakref_ClearRef clears the weakref but leaves
//			 * the callback pointer intact.  Obscure:  it also
//			 * changes *wrlist.

			assert(wr->wr_object == op);
			_PyWeakref_ClearRef(wr);
			assert(wr->wr_object == Py_None);
			if (wr->wr_callback == NULL)
				continue;					   // no callback

//	 * Headache time.  `op` is going away, and is weakly referenced by
//	 * `wr`, which has a callback.  Should the callback be invoked?  If wr
//	 * is also trash, no:
//	 *
//	 * 1. There's no need to call it.  The object and the weakref are
//	 *	both going away, so it's legitimate to pretend the weakref is
//	 *	going away first.  The user has to ensure a weakref outlives its
//	 *	referent if they want a guarantee that the wr callback will get
//	 *	invoked.
//	 *
//	 * 2. It may be catastrophic to call it.  If the callback is also in
//	 *	cyclic trash (CT), then although the CT is unreachable from
//	 *	outside the current generation, CT may be reachable from the
//	 *	callback.  Then the callback could resurrect insane objects.
//	 *
//	 * Since the callback is never needed and may be unsafe in this case,
//	 * wr is simply left in the unreachable set.  Note that because we
//	 * already called _PyWeakref_ClearRef(wr), its callback will never
//	 * trigger.
//	 *
//	 * OTOH, if wr isn't part of CT, we should invoke the callback:  the
//	 * weakref outlived the trash.  Note that since wr isn't CT in this
//	 * case, its callback can't be CT either -- wr acted as an external
//	 * root to this generation, and therefore its callback did too.  So
//	 * nothing in CT is reachable from the callback either, so it's hard
//	 * to imagine how calling it later could create a problem for us.  wr
//	 * is moved to wrcb_to_call in this case.

			if (IS_TENTATIVELY_UNREACHABLE(wr))
				continue;
			assert(IS_REACHABLE(wr));

//			 * Create a new reference so that wr can't go away
//			 * before we can process it again.

			Py_INCREF(wr);

			// Move wr to wrcb_to_call, for the next pass.
			wrasgc = AS_GC(wr);
			assert(wrasgc != next); // wrasgc is reachable, but
									// next isn't, so they can't
									// be the same
			gc_list_move(wrasgc, &wrcb_to_call);
		}
	}

//	 * Invoke the callbacks we decided to honor.  It's safe to invoke them
//	 * because they can't reference unreachable objects.

	while (! gc_list_is_empty(&wrcb_to_call)) {
		PyObject *temp;
		PyObject *callback;

		gc = wrcb_to_call.gc.gc_next;
		op = FROM_GC(gc);
		assert(IS_REACHABLE(op));
		assert(PyWeakref_Check(op));
		wr = (PyWeakReference *)op;
		callback = wr->wr_callback;
		assert(callback != NULL);

		// copy-paste of weakrefobject.c's handle_callback()
		temp = PyObject_CallFunctionObjArgs(callback, wr, NULL);
		if (temp == NULL)
			PyErr_WriteUnraisable(callback);
		else
			Py_DECREF(temp);

//		 * Give up the reference we created in the first pass.  When
//		 * op's refcount hits 0 (which it may or may not do right now),
//		 * op's tp_dealloc will decref op->wr_callback too.  Note
//		 * that the refcount probably will hit 0 now, and because this
//		 * weakref was reachable to begin with, gc didn't already
//		 * add it to its count of freed objects.  Example:  a reachable
//		 * weak value dict maps some key to this reachable weakref.
//		 * The callback removes this key->weakref mapping from the
//		 * dict, leaving no other references to the weakref (excepting
//		 * ours).

		Py_DECREF(op);
		if (wrcb_to_call.gc.gc_next == gc) {
			// object is still alive -- move it
			gc_list_move(gc, old);
		}
		else
			++num_freed;
	}

	return num_freed;
}

static void
debug_instance(char *msg, PyInstanceObject *inst)
{
	char *cname;
	// simple version of instance_repr
	PyObject *classname = inst->in_class->cl_name;
	if (classname != NULL && PyString_Check(classname))
		cname = PyString_AsString(classname);
	else
		cname = "?";
	PySys_WriteStderr("gc: %.100s <%.100s instance at %p>\n",
					  msg, cname, inst);
}

static void
debug_cycle(char *msg, PyObject *op)
{
	if ((debug & DEBUG_INSTANCES) && PyInstance_Check(op)) {
		debug_instance(msg, (PyInstanceObject *)op);
	}
	else if (debug & DEBUG_OBJECTS) {
		PySys_WriteStderr("gc: %.100s <%.100s %p>\n",
						  msg, Py_TYPE(op)->tp_name, op);
	}
}

// * Handle uncollectable garbage (cycles with finalizers, and stuff reachable
// * only from such cycles).
// * If DEBUG_SAVEALL, all objects in finalizers are appended to the module
// * garbage list (a Python list), else only the objects in finalizers with
// * __del__ methods are appended to garbage.  All objects in finalizers are
// * merged into the old list regardless.
// * Returns 0 if all OK, <0 on error (out of memory to grow the garbage list).
// * The finalizers list is made empty on a successful return.

static int
handle_finalizers(PyGC_Head *finalizers, PyGC_Head *old)
{
	PyGC_Head *gc = finalizers->gc.gc_next;

	if (garbage == NULL) {
		garbage = PyList_New(0);
		if (garbage == NULL)
			Py_FatalError("gc couldn't create gc.garbage list");
	}
	for (; gc != finalizers; gc = gc->gc.gc_next) {
		PyObject *op = FROM_GC(gc);

		if ((debug & DEBUG_SAVEALL) || has_finalizer(op)) {
			if (PyList_Append(garbage, op) < 0)
				return -1;
		}
	}

	gc_list_merge(finalizers, old);
	return 0;
}

// * Break reference cycles by clearing the containers involved.  This is
// * tricky business as the lists can be changing and we don't know which
// * objects may be freed.  It is possible I screwed something up here.

static void
delete_garbage(PyGC_Head *collectable, PyGC_Head *old)
{
	inquiry clear;

	while (!gc_list_is_empty(collectable)) {
		PyGC_Head *gc = collectable->gc.gc_next;
		PyObject *op = FROM_GC(gc);

		assert(IS_TENTATIVELY_UNREACHABLE(op));
		if (debug & DEBUG_SAVEALL) {
			PyList_Append(garbage, op);
		}
		else {
			if ((clear = Py_TYPE(op)->tp_clear) != NULL) {
				Py_INCREF(op);
				clear(op);
				Py_DECREF(op);
			}
		}
		if (collectable->gc.gc_next == gc) {
			// object is still alive, move it, it may die later
			gc_list_move(gc, old);
			gc->gc.gc_refs = GC_REACHABLE;
		}
	}
}

// * Clear all free lists
// * All free lists are cleared during the collection of the highest generation.
// * Allocated items in the free list may keep a pymalloc arena occupied.
// * Clearing the free lists may give back memory to the OS earlier.

static void
clear_freelists(void)
{
//	todo: fix
//	(void)PyMethod_ClearFreeList();
//	(void)PyFrame_ClearFreeList();
//	(void)PyCFunction_ClearFreeList();
//	(void)PyTuple_ClearFreeList();
//#ifdef Py_USING_UNICODE
//	(void)PyUnicode_ClearFreeList();
//#endif
//	(void)PyInt_ClearFreeList();
//	(void)PyFloat_ClearFreeList();
}

static double
get_time(void)
{
	double result = 0;
	if (tmod != NULL) {
		PyObject *f = PyObject_CallMethod(tmod, "time", NULL);
		if (f == NULL) {
			PyErr_Clear();
		}
		else {
			if (PyFloat_Check(f))
				result = PyFloat_AsDouble(f);
			Py_DECREF(f);
		}
	}
	return result;
}

// * This is the main function.  Read this to understand how the
// * collection process works.
static Py_ssize_t
collect(int generation)
{
	int i;
	Py_ssize_t m = 0; // # objects collected
	Py_ssize_t n = 0; // # unreachable objects that couldn't be collected
	PyGC_Head *young; // the generation we are examining
	PyGC_Head *old; // next older generation
	PyGC_Head unreachable; // non-problematic unreachable trash
	PyGC_Head finalizers;  // objects with, & reachable from, __del__
	PyGC_Head *gc;
	double t1 = 0.0;

	if (delstr == NULL) {
		delstr = PyString_InternFromString("__del__");
		if (delstr == NULL)
			Py_FatalError("gc couldn't allocate \"__del__\"");
	}

	if (debug & DEBUG_STATS) {
		PySys_WriteStderr("gc: collecting generation %d...\n",
						  generation);
		PySys_WriteStderr("gc: objects in each generation:");
		for (i = 0; i < NUM_GENERATIONS; i++)
			PySys_WriteStderr(" %" PY_FORMAT_SIZE_T "d",
							  gc_list_size(GEN_HEAD(i)));
		t1 = get_time();
		PySys_WriteStderr("\n");
	}

	// update collection and allocation counters
	if (generation+1 < NUM_GENERATIONS)
		generations[generation+1].count += 1;
	for (i = 0; i <= generation; i++)
		generations[i].count = 0;

	// merge younger generations with one we are currently collecting
	for (i = 0; i < generation; i++) {
		gc_list_merge(GEN_HEAD(i), GEN_HEAD(generation));
	}

	// handy references
	young = GEN_HEAD(generation);
	if (generation < NUM_GENERATIONS-1)
		old = GEN_HEAD(generation+1);
	else
		old = young;

//	 * Using ob_refcnt and gc_refs, calculate which objects in the
//	 * container set are reachable from outside the set (i.e., have a
//	 * refcount greater than 0 when all the references within the
//	 * set are taken into account).

	update_refs(young);
	subtract_refs(young);

//	 * Leave everything reachable from outside young in young, and move
//	 * everything else (in young) to unreachable.
//	 * NOTE:  This used to move the reachable objects into a reachable
//	 * set instead.  But most things usually turn out to be reachable,
//	 * so it's more efficient to move the unreachable things.

	gc_list_init(&unreachable);
	move_unreachable(young, &unreachable);

	// Move reachable objects to next generation.
	if (young != old) {
		if (generation == NUM_GENERATIONS - 2) {
			long_lived_pending += gc_list_size(young);
		}
		gc_list_merge(young, old);
	}
	else {
		long_lived_pending = 0;
		long_lived_total = gc_list_size(young);
	}

//	 * All objects in unreachable are trash, but objects reachable from
//	 * finalizers can't safely be deleted.  Python programmers should take
//	 * care not to create such things.  For Python, finalizers means
//	 * instance objects with __del__ methods.  Weakrefs with callbacks
//	 * can also call arbitrary Python code but they will be dealt with by
//	 * handle_weakrefs().

	gc_list_init(&finalizers);
	move_finalizers(&unreachable, &finalizers);
//	 * finalizers contains the unreachable objects with a finalizer;
//	 * unreachable objects reachable *from* those are also uncollectable,
//	 * and we move those into the finalizers list too.

	move_finalizer_reachable(&finalizers);

//	 * Collect statistics on collectable objects found and print
//	 * debugging information.

	for (gc = unreachable.gc.gc_next; gc != &unreachable;
					gc = gc->gc.gc_next) {
		m++;
		if (debug & DEBUG_COLLECTABLE) {
			debug_cycle("collectable", FROM_GC(gc));
		}
	}

	// Clear weakrefs and invoke callbacks as necessary.
	m += handle_weakrefs(&unreachable, old);

//	 * Call tp_clear on objects in the unreachable set.  This will cause
//	 * the reference cycles to be broken.  It may also cause some objects
//	 * in finalizers to be freed.

	delete_garbage(&unreachable, old);

//	 * Collect statistics on uncollectable objects found and print
//	 * debugging information.
	for (gc = finalizers.gc.gc_next;
		 gc != &finalizers;
		 gc = gc->gc.gc_next) {
		n++;
		if (debug & DEBUG_UNCOLLECTABLE)
			debug_cycle("uncollectable", FROM_GC(gc));
	}
	if (debug & DEBUG_STATS) {
		double t2 = get_time();
		if (m == 0 && n == 0)
			PySys_WriteStderr("gc: done");
		else
			PySys_WriteStderr(
				"gc: done, "
				"%" PY_FORMAT_SIZE_T "d unreachable, "
				"%" PY_FORMAT_SIZE_T "d uncollectable",
				n+m, n);
		if (t1 && t2) {
			PySys_WriteStderr(", %.4fs elapsed", t2-t1);
		}
		PySys_WriteStderr(".\n");
	}

//	 * Append instances in the uncollectable set to a Python
//	 * reachable list of garbage.  The programmer has to deal with
//	 * this if they insist on creating this type of structure.

	(void)handle_finalizers(&finalizers, old);

//	 * Clear free list only during the collection of the highest
//	 * generation
	if (generation == NUM_GENERATIONS-1) {
		clear_freelists();
	}

	if (PyErr_Occurred()) {
		if (gc_str == NULL)
			gc_str = PyString_FromString("garbage collection");
		PyErr_WriteUnraisable(gc_str);
		Py_FatalError("unexpected exception during garbage collection");
	}
	return n+m;
}

static Py_ssize_t
collect_generations(void)
{
	int i;
	Py_ssize_t n = 0;

//	 * Find the oldest generation (highest numbered) where the count
//	 * exceeds the threshold.  Objects in the that generation and
//	 * generations younger than it will be collected.
	for (i = NUM_GENERATIONS-1; i >= 0; i--) {
		if (generations[i].count > generations[i].threshold) {
			// Avoid quadratic performance degradation in number
			// of tracked objects. See comments at the beginning
			// of this file, and issue #4074.

			if (i == NUM_GENERATIONS - 1
				&& long_lived_pending < long_lived_total / 4)
				continue;
			n = collect(i);
			break;
		}
	}
	return n;
}

PyDoc_STRVAR(gc_enable__doc__,
"enable() -> None\n"
"\n"
"Enable automatic garbage collection.\n");

static PyObject *
gc_enable(PyObject *self, PyObject *noargs)
{
	enabled = 1;
	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(gc_disable__doc__,
"disable() -> None\n"
"\n"
"Disable automatic garbage collection.\n");

static PyObject *
gc_disable(PyObject *self, PyObject *noargs)
{
	enabled = 0;
	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(gc_isenabled__doc__,
"isenabled() -> status\n"
"\n"
"Returns true if automatic garbage collection is enabled.\n");

static PyObject *
gc_isenabled(PyObject *self, PyObject *noargs)
{
	return PyBool_FromLong((long)enabled);
}

PyDoc_STRVAR(gc_collect__doc__,
"collect([generation]) -> n\n"
"\n"
"With no arguments, run a full collection.  The optional argument\n"
"may be an integer specifying which generation to collect.  A ValueError\n"
"is raised if the generation number is invalid.\n\n"
"The number of unreachable objects is returned.\n");

static PyObject *
gc_collect(PyObject *self, PyObject *args, PyObject *kws)
{
	static char *keywords[] = {"generation", NULL};
	int genarg = NUM_GENERATIONS - 1;
	Py_ssize_t n;

	if (!PyArg_ParseTupleAndKeywords(args, kws, "|i", keywords, &genarg))
		return NULL;

	else if (genarg < 0 || genarg >= NUM_GENERATIONS) {
		PyErr_SetString(PyExc_ValueError, "invalid generation");
		return NULL;
	}

	if (collecting)
		n = 0; // already collecting, don't do anything
	else {
		collecting = 1;
		n = collect(genarg);
		collecting = 0;
	}

	return PyInt_FromSsize_t(n);
}

PyDoc_STRVAR(gc_set_debug__doc__,
"set_debug(flags) -> None\n"
"\n"
"Set the garbage collection debugging flags. Debugging information is\n"
"written to sys.stderr.\n"
"\n"
"flags is an integer and can have the following bits turned on:\n"
"\n"
"  DEBUG_STATS - Print statistics during collection.\n"
"  DEBUG_COLLECTABLE - Print collectable objects found.\n"
"  DEBUG_UNCOLLECTABLE - Print unreachable but uncollectable objects found.\n"
"  DEBUG_INSTANCES - Print instance objects.\n"
"  DEBUG_OBJECTS - Print objects other than instances.\n"
"  DEBUG_SAVEALL - Save objects to gc.garbage rather than freeing them.\n"
"  DEBUG_LEAK - Debug leaking programs (everything but STATS).\n");

static PyObject *
gc_set_debug(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, "i:set_debug", &debug))
		return NULL;

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(gc_get_debug__doc__,
"get_debug() -> flags\n"
"\n"
"Get the garbage collection debugging flags.\n");

static PyObject *
gc_get_debug(PyObject *self, PyObject *noargs)
{
	return Py_BuildValue("i", debug);
}

PyDoc_STRVAR(gc_set_thresh__doc__,
"set_threshold(threshold0, [threshold1, threshold2]) -> None\n"
"\n"
"Sets the collection thresholds.  Setting threshold0 to zero disables\n"
"collection.\n");

static PyObject *
gc_set_thresh(PyObject *self, PyObject *args)
{
	int i;
	if (!PyArg_ParseTuple(args, "i|ii:set_threshold",
						  &generations[0].threshold,
						  &generations[1].threshold,
						  &generations[2].threshold))
		return NULL;
	for (i = 2; i < NUM_GENERATIONS; i++) {
		// generations higher than 2 get the same threshold
		generations[i].threshold = generations[2].threshold;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(gc_get_thresh__doc__,
"get_threshold() -> (threshold0, threshold1, threshold2)\n"
"\n"
"Return the current collection thresholds\n");

static PyObject *
gc_get_thresh(PyObject *self, PyObject *noargs)
{
	return Py_BuildValue("(iii)",
						 generations[0].threshold,
						 generations[1].threshold,
						 generations[2].threshold);
}

PyDoc_STRVAR(gc_get_count__doc__,
"get_count() -> (count0, count1, count2)\n"
"\n"
"Return the current collection counts\n");

static PyObject *
gc_get_count(PyObject *self, PyObject *noargs)
{
	return Py_BuildValue("(iii)",
						 generations[0].count,
						 generations[1].count,
						 generations[2].count);
}

static int
referrersvisit(PyObject* obj, PyObject *objs)
{
	Py_ssize_t i;
	for (i = 0; i < PyTuple_GET_SIZE(objs); i++)
		if (PyTuple_GET_ITEM(objs, i) == obj)
			return 1;
	return 0;
}

static int
gc_referrers_for(PyObject *objs, PyGC_Head *list, PyObject *resultlist)
{
	PyGC_Head *gc;
	PyObject *obj;
	traverseproc traverse;
	for (gc = list->gc.gc_next; gc != list; gc = gc->gc.gc_next) {
		obj = FROM_GC(gc);
		traverse = Py_TYPE(obj)->tp_traverse;
		if (obj == objs || obj == resultlist)
			continue;
		if (traverse(obj, (visitproc)referrersvisit, objs)) {
			if (PyList_Append(resultlist, obj) < 0)
				return 0; // error
		}
	}
	return 1; // no error
}

PyDoc_STRVAR(gc_get_referrers__doc__,
"get_referrers(*objs) -> list\n\
Return the list of objects that directly refer to any of objs.");

static PyObject *
gc_get_referrers(PyObject *self, PyObject *args)
{
	int i;
	PyObject *result = PyList_New(0);
	if (!result) return NULL;

	for (i = 0; i < NUM_GENERATIONS; i++) {
		if (!(gc_referrers_for(args, GEN_HEAD(i), result))) {
			Py_DECREF(result);
			return NULL;
		}
	}
	return result;
}

// Append obj to list; return true if error (out of memory), false if OK.
static int
referentsvisit(PyObject *obj, PyObject *list)
{
	return PyList_Append(list, obj) < 0;
}

PyDoc_STRVAR(gc_get_referents__doc__,
"get_referents(*objs) -> list\n\
Return the list of objects that are directly referred to by objs.");

static PyObject *
gc_get_referents(PyObject *self, PyObject *args)
{
	Py_ssize_t i;
	PyObject *result = PyList_New(0);

	if (result == NULL)
		return NULL;

	for (i = 0; i < PyTuple_GET_SIZE(args); i++) {
		traverseproc traverse;
		PyObject *obj = PyTuple_GET_ITEM(args, i);

		if (! PyObject_IS_GC(obj))
			continue;
		traverse = Py_TYPE(obj)->tp_traverse;
		if (! traverse)
			continue;
		if (traverse(obj, (visitproc)referentsvisit, result)) {
			Py_DECREF(result);
			return NULL;
		}
	}
	return result;
}

PyDoc_STRVAR(gc_get_objects__doc__,
"get_objects() -> [...]\n"
"\n"
"Return a list of objects tracked by the collector (excluding the list\n"
"returned).\n");

static PyObject *
gc_get_objects(PyObject *self, PyObject *noargs)
{
	int i;
	PyObject* result;

	result = PyList_New(0);
	if (result == NULL)
		return NULL;
	for (i = 0; i < NUM_GENERATIONS; i++) {
		if (append_objects(result, GEN_HEAD(i))) {
			Py_DECREF(result);
			return NULL;
		}
	}
	return result;
}

PyDoc_STRVAR(gc_is_tracked__doc__,
"is_tracked(obj) -> bool\n"
"\n"
"Returns true if the object is tracked by the garbage collector.\n"
"Simple atomic objects will return false.\n"
);

static PyObject *
gc_is_tracked(PyObject *self, PyObject *obj)
{
	PyObject *result;

	if (PyObject_IS_GC(obj) && IS_TRACKED(obj))
		result = Py_True;
	else
		result = Py_False;
	Py_INCREF(result);
	return result;
}


PyDoc_STRVAR(gc__doc__,
"This module provides access to the garbage collector for reference cycles.\n"
"\n"
"enable() -- Enable automatic garbage collection.\n"
"disable() -- Disable automatic garbage collection.\n"
"isenabled() -- Returns true if automatic collection is enabled.\n"
"collect() -- Do a full collection right now.\n"
"get_count() -- Return the current collection counts.\n"
"set_debug() -- Set debugging flags.\n"
"get_debug() -- Get debugging flags.\n"
"set_threshold() -- Set the collection thresholds.\n"
"get_threshold() -- Return the current the collection thresholds.\n"
"get_objects() -- Return a list of all objects tracked by the collector.\n"
"is_tracked() -- Returns true if a given object is tracked.\n"
"get_referrers() -- Return the list of objects that refer to an object.\n"
"get_referents() -- Return the list of objects that an object refers to.\n");

static PyMethodDef GcMethods[] = {
	{"enable",			 gc_enable,	 METH_NOARGS,  gc_enable__doc__},
	{"disable",			gc_disable,	METH_NOARGS,  gc_disable__doc__},
	{"isenabled",		  gc_isenabled,  METH_NOARGS,  gc_isenabled__doc__},
	{"set_debug",		  gc_set_debug,  METH_VARARGS, gc_set_debug__doc__},
	{"get_debug",		  gc_get_debug,  METH_NOARGS,  gc_get_debug__doc__},
	{"get_count",		  gc_get_count,  METH_NOARGS,  gc_get_count__doc__},
	{"set_threshold",  gc_set_thresh, METH_VARARGS, gc_set_thresh__doc__},
	{"get_threshold",  gc_get_thresh, METH_NOARGS,  gc_get_thresh__doc__},
	{"collect",			(PyCFunction)gc_collect,
		METH_VARARGS | METH_KEYWORDS,		   gc_collect__doc__},
	{"get_objects",	gc_get_objects,METH_NOARGS,  gc_get_objects__doc__},
	{"is_tracked",	 gc_is_tracked, METH_O,	   gc_is_tracked__doc__},
	{"get_referrers",  gc_get_referrers, METH_VARARGS,
		gc_get_referrers__doc__},
	{"get_referents",  gc_get_referents, METH_VARARGS,
		gc_get_referents__doc__},
	{NULL,	  NULL}		   // Sentinel
};

PyMODINIT_FUNC
initgc(void)
{
	PyObject *m;

	m = Py_InitModule4("gc",
						  GcMethods,
						  gc__doc__,
						  NULL,
						  PYTHON_API_VERSION);
	if (m == NULL)
		return;

	if (garbage == NULL) {
		garbage = PyList_New(0);
		if (garbage == NULL)
			return;
	}
	Py_INCREF(garbage);
	if (PyModule_AddObject(m, "garbage", garbage) < 0)
		return;

//	 * Importing can't be done in collect() because collect()
//	 * can be called via PyGC_Collect() in Py_Finalize().
//	 * This wouldn't be a problem, except that <initialized> is
//	 * reset to 0 before calling collect which trips up
//	 * the import and triggers an assertion.

	if (tmod == NULL) {
		tmod = PyImport_ImportModuleNoBlock("time");
		if (tmod == NULL)
			PyErr_Clear();
	}

#define ADD_INT(NAME) if (PyModule_AddIntConstant(m, #NAME, NAME) < 0) return
	ADD_INT(DEBUG_STATS);
	ADD_INT(DEBUG_COLLECTABLE);
	ADD_INT(DEBUG_UNCOLLECTABLE);
	ADD_INT(DEBUG_INSTANCES);
	ADD_INT(DEBUG_OBJECTS);
	ADD_INT(DEBUG_SAVEALL);
	ADD_INT(DEBUG_LEAK);
#undef ADD_INT
}

// API to invoke gc.collect() from C
Py_ssize_t
PyGC_Collect(void)
{
	Py_ssize_t n;

	if (collecting)
		n = 0; // already collecting, don't do anything
	else {
		collecting = 1;
		n = collect(NUM_GENERATIONS - 1);
		collecting = 0;
	}

	return n;
}

// for debugging
void
_PyGC_Dump(PyGC_Head *g)
{
	_PyObject_Dump(FROM_GC(g));
}
*/
// extension modules might be compiled with GC support so these
// functions must always be available

#undef PyObject_GC_Track
#undef PyObject_GC_UnTrack
#undef PyObject_GC_Del
#undef _PyObject_GC_Malloc

static inline jsize JyObject_FixedGCSize(PyObject* pyObject)
{
	if (Py_TYPE(pyObject) == &PyTuple_Type)    return Py_SIZE(pyObject);
	if (Py_TYPE(pyObject) == &PyFunction_Type) return 5; //6
	if (Py_TYPE(pyObject) == &PyInstance_Type) return 2;
	if (Py_TYPE(pyObject) == &PyMethod_Type)   return 3;
	if (Py_TYPE(pyObject) == &PyClass_Type)    return 6;
	if (Py_TYPE(pyObject) == &PyCell_Type)     return 1;
	if (Is_StaticTypeObject(pyObject))         return 7;
	return UNKNOWN_FIXED_GC_SIZE;
}

static int
visit_count(PyObject *op, void *arg)
{
//	jputs(__FUNCTION__);
	int* count = (int*) arg;
	//assert(op != NULL);
	(*count) = (*count)+1;
	//jputs("done");
	return 0;
}

//static int
//visit_explore(PyObject *op, void *arg)
//{
//	if (IS_UNEXPLORED(op))
//		pushExStack(op);
//	return 0;
//}

static int
visit_exploreSingleLink(PyObject *op, void *arg)
{
	*((PyObject**) arg) = op;
	return 0;
}

typedef struct {
	JNIEnv* env;
	jobject dest;
	jint pos;
} exploreJNI;

void JyNI_GC_Explore()
{
//	jputs(__FUNCTION__);
//	jputs("stack-size:");
//	jputsLong(exStackSize());
	while (!isExStackEmpty()) {
		PyObject* toExplore = popExStack();
		if (!Is_Static_PyObject(toExplore)) {
//			jputs("JyNI-Warning: JyNI_GC_Explore found non-heap object.");
//			jputs(Py_TYPE(toExplore)->tp_name);
//		} else {
			if (IS_UNEXPLORED(toExplore)) {
				JyNI_GC_ExploreObject(toExplore);
			}
		}
	}
}

static int
statictype_traverse(PyObject* type, visitproc visit, void *arg)
{
	Py_VISIT(((PyTypeObject*) type)->tp_dict);
	Py_VISIT(((PyTypeObject*) type)->tp_bases);
	Py_VISIT(((PyTypeObject*) type)->tp_mro);
	Py_VISIT(((PyTypeObject*) type)->tp_base);
	Py_VISIT(((PyTypeObject*) type)->tp_subclasses);
	Py_VISIT(((PyTypeObject*) type)->tp_weaklist);
	Py_VISIT(((PyTypeObject*) type)->tp_cache);
	return 0;
}

static jboolean hasJyGCHead(JNIEnv* env, PyObject* op, JyObject* jy)
{
	if (Is_Static_PyObject(op))
	{
		jobject result = (*env)->CallStaticObjectMethod(env, JyNIClass,
				JyNI_getNativeStaticJyGCHead, (jlong) op);
		return result != NULL;
	} else
		return JyObject_HasJyGCHead(op, jy);
}

/*
 * This method is intended for objects that already have a JyNI-GCHead,
 * but were not initialized properly when the head was created.
 * A result of false means that either the object is still not initialized
 * or doesn't have a head yet.
 */
jboolean JyNI_GC_EnsureHeadObject(JNIEnv* env, PyObject* op, JyObject* jy)
{
	if (!(jy->flags & JY_INITIALIZED_FLAG_MASK))
		return JNI_FALSE;

	if (JyObject_HasJyGCHead(op, jy))
	{
		jobject result = (*env)->NewLocalRef(env, JyObject_GetJyGCHead(op, jy));

		if (!result || (*env)->IsSameObject(env, result, NULL))
			return JNI_FALSE;

		jobject jjy = (*env)->NewLocalRef(env, jy->jy);
		if (!(*env)->IsSameObject(env, jjy, NULL)) {
//			jputsLong(__LINE__);
			(*env)->CallVoidMethod(env, result, pyObjectGCHeadSetObject, jjy);
		}
		else return JNI_FALSE;
		(*env)->DeleteLocalRef(env, jjy);
		return JNI_TRUE;
	} else
		return JNI_FALSE;
}

jobject JyNI_GC_ObtainJyGCHead(JNIEnv* env, PyObject* op, JyObject* jy)
{
//	jputs(__FUNCTION__);
//	jputs(op->ob_type->tp_name);
	if (Is_Static_PyObject(op))
	{
		//jputs("JyNI-Warning: JyNI_GC_ObtainJyGCHead was called with non-heap object.");
		jobject result = (*env)->CallStaticObjectMethod(env, JyNIClass,
				JyNI_getNativeStaticJyGCHead, (jlong) op);
		if (!result || (*env)->IsSameObject(env, result, NULL)) {
			result = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_makeStaticGCHead,
					(jlong) op, Is_StaticTypeObject(op) || PyObject_IS_GC(op));
			(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_registerNativeStaticJyGCHead,
					(jlong) op, result);
		}
		return result;
	}
	if (jy->flags & JY_CPEER_FLAG_MASK)
	{
//		if (!(jy->flags & JY_INITIALIZED_FLAG_MASK))
//			jputs("JyNI-Warning: uninitialized CPeer at explore!");
		jobject er = JyNI_JythonPyObject_FromPyObject(op);

		//Todo: This warning is not completely silent when commented-in.
		//Investigate!
//		if (!(*env)->IsInstanceOf(env, er, jyGCHeadClass))
//			jputs("Obtained a head that is not a JyGCHead!");

//		if (PyType_Check(op) && !(*env)->IsInstanceOf(env, er, pyTypeClass)) {
//			jputs("Obtained a head that is not a PyType!");
//			if ((*env)->IsInstanceOf(env, er, pyCPeerGCClass))
//				jputs("Created pyCPeerGC instead");
//		}
		//jputsLong(__LINE__);
		return er;
	} else
	{
		jboolean hasHeadAttr = JyObject_HasJyGCHead(op, jy);
		jobject result = NULL;
		if (hasHeadAttr)
		{
			result = (*env)->NewLocalRef(env, JyObject_GetJyGCHead(op, jy));
		}
		if (!result || (*env)->IsSameObject(env, result, NULL))
		{
			if (result)
			{
				jputs("Still needed JyGCHead was collected!");
				jputs("This should not happen and would be fatal for GIL-free mode. Debug!");
				(*env)->DeleteLocalRef(env, result);
			}
			//create the right c-head here...
			//First check for special-case PyList:
			if (Py_TYPE(op) == &PyList_Type)
			{
				/* Set up JyGCHead before we call track: */
				result = (*env)->NewObject(env, JyListClass, JyListFromBackendHandleConstructor, (jlong) op);
//				if ((*env)->ExceptionCheck(env)) {
//					jputs("exception occurred creating JyList:");
//					(*env)->ExceptionDescribe(env);
//				}
			} else
			{
				//Use DefaultTraversableGCHead for now unless mirror mode.
				result = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_makeGCHead,
						(jlong) op, JyObject_IS_MIRROR(op, jy), PyObject_IS_GC(op));
//				jputs("Created JyGCHead for ");
//				jputs(Py_TYPE(op)->tp_name);
//				jputsLong(op);
//				JyNI_jprintHash(result);
//				if (PyString_Check(op)) {
//					jputs(PyString_AS_STRING(op));
//				}
				if ((jy->flags & JY_INITIALIZED_FLAG_MASK))
				{
					jobject jjy = (*env)->NewLocalRef(env, jy->jy);
					//jobjectRefType rt = (*env)->GetObjectRefType(env, jy->jy);
					//jputs("RefType0");
					//jputsLong((*env)->GetObjectRefType(env, jy->jy));
					if (!(*env)->IsSameObject(env, jjy, NULL)) {
						//jputs("RefType1");
//						jputsLong(__LINE__);
						(*env)->CallVoidMethod(env, result, pyObjectGCHeadSetObject, jjy);
					}
					(*env)->DeleteLocalRef(env, jjy);
				} //else jputs("obj for head not yet initialized!");
			}
			//jputsLong(__LINE__);
			//Here we add another ref for the JyGCHead to let it keep the native object alive.
			Py_INCREF(op);
//			if ((jy->flags & JY_INITIALIZED_FLAG_MASK)) {
//				jputs("RefType:");
//				jputsLong((*env)->GetObjectRefType(env, jy->jy));
//			} else {
//				jputs("jy uninitialized");
//			}

			if (hasHeadAttr)
			{
				JyObject_AddOrSetJyGCHead(op, jy, (*env)->NewWeakGlobalRef(env, result));
			} else
			{
				JyObject_AddJyGCHead(op, jy, (*env)->NewWeakGlobalRef(env, result));
			}
		}
		return result;
	}
}

static int
visit_exploreArrayLink(PyObject *op, void *arg)
{
//	jputs(__FUNCTION__);
	/* JyNI-note:
	 * It is hard to decide whether to explore non-heap-objects here or not.
	 * On one hand they should be kept alive anyway. However there might be
	 * cases where this principle is broken and some of them could be avoided
	 * by exploring non-heap objects here. (So we do it.)
	 * Note that AS_JY would *not* segfault on non-heap objects, but it returns
	 * an invalid position that would cause a segfault if FROM_JY was applied
	 * on it again or one accessed it any other way (because FROM_JY must look
	 * at a flag; FROM_JY_WITH_GC or FROM_JY_NO_GC would not be affected, but
	 * are not feasible here).
	 * However, for non-heap objects JyNI_GC_ObtainJyGCHead ignores the JyObject-param,
	 * so an invalid pointer causes no harm.
	 */
	//if (!Is_Static_PyObject(op)) {
	jobject head = JyNI_GC_ObtainJyGCHead(((exploreJNI*) arg)->env, op, AS_JY(op));
	(*((exploreJNI*) arg)->env)->SetObjectArrayElement(((exploreJNI*) arg)->env,
			((exploreJNI*) arg)->dest, ((exploreJNI*) arg)->pos++, head);
	(*((exploreJNI*) arg)->env)->DeleteLocalRef(((exploreJNI*) arg)->env, head);
	//}
	return 0;
}

static int
visit_exploreListLink(PyObject *op, void *arg)
{
	/* JyNI-note:
	 * See note in visit_exploreArrayLink.
	 */
	//if (!Is_Static_PyObject(op)) {
	jobject head = JyNI_GC_ObtainJyGCHead(((exploreJNI*) arg)->env, op, AS_JY(op));
	(*((exploreJNI*) arg)->env)->CallBooleanMethod(((exploreJNI*) arg)->env,
			((exploreJNI*) arg)->dest, listAdd, head);
	(*((exploreJNI*) arg)->env)->DeleteLocalRef(((exploreJNI*) arg)->env, head);
	//}
	return 0;
}

/*
 * Excpects a traversable PyObject as obj.
 */
static jsize countReferences(PyObject* obj, traverseproc trav)
{
	if (!trav)
	{
//		jputsLong(__LINE__);
		if (PyType_CheckExact(obj)) {// && !Py_TYPE((PyObject*) op)->tp_traverse)
//			jputsLong(__LINE__);
			trav = statictype_traverse; //For now we use this traverse-method also for heap-types.
		} else trav = Py_TYPE((PyObject*) obj)->tp_traverse;
	}
	jsize result = 0;
	trav((PyObject*) obj, (visitproc) visit_count, &result);
	return result;
}

static int
visit_updateLinks(PyObject *op, void *arg)
{
//	debugContext("   ->", op, "|");
	jobject head = JyNI_GC_ObtainJyGCHead(((exploreJNI*) arg)->env, op, AS_JY(op));
	if ( (*((exploreJNI*) arg)->env)->CallIntMethod(((exploreJNI*) arg)->env,
			((exploreJNI*) arg)->dest, traversableGCHeadSetLink,
			((exploreJNI*) arg)->pos++, head)
			== -1)
	{
		PyObject* destObj = JyNI_PyObject_FromJythonPyObject(((exploreJNI*) arg)->dest);
		jsize size = countReferences(destObj, NULL);
//		jputs("Native ensure size:");
//		jputsLong(size);
		(*((exploreJNI*) arg)->env)->CallVoidMethod(((exploreJNI*) arg)->env,
				((exploreJNI*) arg)->dest, traversableGCHeadEnsureSize, size);
		(*((exploreJNI*) arg)->env)->CallIntMethod(((exploreJNI*) arg)->env,
					((exploreJNI*) arg)->dest, traversableGCHeadSetLink,
					((exploreJNI*) arg)->pos-1, head);
	}
	(*((exploreJNI*) arg)->env)->DeleteLocalRef(((exploreJNI*) arg)->env, head);
	return 0;
}

static jobject exploreJyGCHeadLinks(JNIEnv* env, PyObject* op, JyObject* jy) {
	//debugPy(op);
//	jboolean jdbg = (strcmp(Py_TYPE(op)->tp_name, "xxx") == 0);
//	if (jdbg) jputs(__FUNCTION__);
	traverseproc trav;
	if (PyType_Check(op)) {// && !Py_TYPE((PyObject*) op)->tp_traverse)
		//todo: use heap-types own traverse method for heap-types again.
		trav = statictype_traverse; //For now we use this traverse-method also for heap-types.
	} else trav = Py_TYPE((PyObject*) op)->tp_traverse;
	if (!(jy->flags & JY_GC_VAR_SIZE)) {
		jsize fixedSize = JyObject_FixedGCSize(op);
		if (fixedSize == UNKNOWN_FIXED_GC_SIZE &&
			Py_TYPE(op)->tp_itemsize < sizeof(PyObject*))
		{
//			jputsLong(Py_TYPE(op)->tp_itemsize);
			//Is most likely fixed-size anyway, but we must
			//obtain the size by traverse-counting.
			fixedSize = countReferences(op, trav);

//			trav((PyObject*) op, (visitproc)visit_count, &fixedSize);
//			jputs("Estimated fixed size:");
//			jputsLong(fixedSize);
		}
//		jputsLong(fixedSize);
		if (fixedSize != UNKNOWN_FIXED_GC_SIZE)
		{
			if (fixedSize == 1) {
				PyObject* singleLink = NULL;
				trav(op, (visitproc)visit_exploreSingleLink, &singleLink);
				if (singleLink) {// && !Is_Static_PyObject(singleLink)) {
					JyObject* jy = AS_JY(singleLink);
					jobject result0 = JyNI_GC_ObtainJyGCHead(env, singleLink, jy);
					return result0;
					//return JyNI_GC_ObtainJyGCHead(env, singleLink, AS_JY(singleLink));
				} else {
					return NULL;
				}
			} else {
				jobject result = (*env)->NewObjectArray(env, fixedSize, jyGCHeadClass, NULL);
				exploreJNI expl = {env, result, 0};
				trav(op, visit_exploreArrayLink, &expl);
				return result;
			}
		}
	}
	//Create var-size GC-head...
	//obtain initial size...
	jsize initSize;
	if (Py_TYPE(op)->tp_itemsize)
		initSize = Py_SIZE(op);
	else {
		initSize = 0;
		trav((PyObject*) op, (visitproc) visit_count, &initSize);
	}
	jobject result = (*env)->NewObject(env, arrayListClass, arrayListConstructor, initSize);
	exploreJNI expl = {env, result, 0};
	trav(op, visit_exploreListLink, &expl);
	return result;
}

int updateJyGCHeadLink(PyObject* op, JyObject* jy, jsize index,
		PyObject* newItem, JyObject* newItemJy)
{
	if (!newItem) {
		env(GC_OBJECT_JNIFAIL);
		jobject gcHead = JyNI_GC_ObtainJyGCHead(env, op, jy);
		return (*env)->CallIntMethod(env, gcHead, traversableGCHeadClearLink, index);
	}
	if (IS_UNEXPLORED(op))
	{
		/* Attempts to update are not necessarily good places to perform
		 * object exploration. For now we refrain from it.
		 */
		//jputs("JyNI-Warning: updateJyGCHeadLink called on unexplored object!");
		//JyNI_GC_ExploreObject(op);
		return GC_OBJECT_UNEXPLORED;
	} else {
		env(GC_OBJECT_JNIFAIL);
		jobject gcHead = JyNI_GC_ObtainJyGCHead(env, op, jy);
		jobject linkHead = JyNI_GC_ObtainJyGCHead(env, newItem, newItemJy);
		int result = (*env)->CallIntMethod(env, gcHead, traversableGCHeadSetLink, index, linkHead);
		if (result == -1)
		{
			jsize size = countReferences(op, NULL);
			(*env)->CallVoidMethod(env, gcHead, traversableGCHeadEnsureSize, size);
			result = (*env)->CallIntMethod(env, gcHead, traversableGCHeadInsertLink, index, linkHead);
		}
		return result;
	}
}

int updateClearJyGCHeadLinks(PyObject* op, JyObject* jy, jsize startIndex)
{
	if (IS_UNEXPLORED(op))
	{
		/* Attempts to update are not necessarily good places to perform
		 * object exploration. For now we refrain from it.
		 */
		//jputs("JyNI-Warning: updateJyGCHeadLink called on unexplored object!");
		//JyNI_GC_ExploreObject(op);
		return GC_OBJECT_UNEXPLORED;
	}
	env(GC_OBJECT_JNIFAIL);
	if (!hasJyGCHead(env, op, jy))
	{
		return GC_OBJECT_NOHEAD;
	} else
	{
		/* In this method we must care for the case that the head was gc'ed and
		 * we don't wanna recreate it.
		 */

		jobject gcHead;
		if (Is_Static_PyObject(op))
		{
			gcHead = (*env)->CallStaticObjectMethod(env, JyNIClass,
					JyNI_getNativeStaticJyGCHead, (jlong) op);
		} else
		{
			gcHead = (*env)->NewLocalRef(env, JyObject_GetJyGCHead(op, jy));
		}
		if (!gcHead || (*env)->IsSameObject(env, gcHead, NULL))
		{
			(*env)->DeleteLocalRef(env, gcHead);
			return GC_OBJECT_INVALIDHEAD;
		}
		int result = (*env)->CallIntMethod(env, gcHead,
				traversableGCHeadClearLinksFromIndex, startIndex);
		(*env)->DeleteLocalRef(env, gcHead);
		return result;
	}
}

int updateInsertJyGCHeadLink(PyObject* op, JyObject* jy, jsize index,
		PyObject* newItem, JyObject* newItemJy)
{
	if (IS_UNEXPLORED(op))
	{
		/* Attempts to update are not necessarily good places to perform
		 * object exploration. For now we refrain from it.
		 */
		//jputs("JyNI-Warning: updateJyGCHeadLink called on unexplored object!");
		//JyNI_GC_ExploreObject(op);
		return GC_OBJECT_UNEXPLORED;
	} else
	{
		env(GC_OBJECT_JNIFAIL);
		jobject gcHead = JyNI_GC_ObtainJyGCHead(env, op, jy);
		jobject linkHead = JyNI_GC_ObtainJyGCHead(env, newItem, newItemJy);
		int result = (*env)->CallIntMethod(env, gcHead, traversableGCHeadInsertLink, index, linkHead);
		if (result == -1)
		{
			jsize size = countReferences(op, NULL);
			(*env)->CallVoidMethod(env, gcHead, traversableGCHeadEnsureSize, size);
			result = (*env)->CallIntMethod(env, gcHead, traversableGCHeadInsertLink, index, linkHead);
		}
		return result;
	}
}

int updateJyGCHeadLinks(PyObject* op, JyObject* jy) {
//	jputs(__FUNCTION__);
//	jputsLong(op);
//	jputs(Py_TYPE(op)->tp_name);
	if (IS_UNEXPLORED(op))
	{
		/* Attempts to update are not necessarily good places to perform
		 * object exploration. For now we refrain from it.
		 */
//		jputs("JyNI-Warning: updateJyGCHeadLinks called on unexplored object!");
//		jputsLong(op);
		//JyNI_GC_ExploreObject(op);
		return GC_OBJECT_UNEXPLORED;
	} else {
		env(GC_OBJECT_JNIFAIL);
		traverseproc trav;
		if (PyType_CheckExact(op))// && !Py_TYPE((PyObject*) op)->tp_traverse)
			trav = statictype_traverse; //For now we use this traverse-method also for heap-types.
		else trav = Py_TYPE((PyObject*) op)->tp_traverse;

		jobject destHead = JyNI_GC_ObtainJyGCHead(env, op, jy);
		exploreJNI expl = {env, destHead, 0};
		trav(op, visit_updateLinks, &expl);
		(*env)->CallIntMethod(env, destHead, traversableGCHeadClearLinksFromIndex, expl.pos);
		(*env)->DeleteLocalRef(env, destHead);
		return 0;
	}
}

void JyNI_GC_ExploreObject(PyObject* op)
{
//	jputs(__FUNCTION__);
//	jputsLong(op);
//	jputs(Py_TYPE(op)->tp_name);
	/*
	 * Note that not only GC-objects (in terms of PyObject_IS_GC) must be explored,
	 * but every traversable object. Non-heap types are an example of this. They
	 * are traversable, but don't have a gc-head. Neverteless JyNI needs to explore
	 * them to obtain a full reference graph.
	 * Such objects are a special case though, because without a GC-head we cannot
	 * set the GC_EXPLORED flag.
	 * So for IS_UNEXPLORED we check for the GC_EXPLORED-flag only if the object
	 * satisfies PyObject_IS_GC. Otherwise we still do a special check for
	 * Type-objects, whether they have their ready-flag set, assuming that "readied"
	 * types have also been explored by this method.
	 * For other traversable, but non-gc (in terms of PyObject_IS_GC) obects we
	 * assume unexplored state all the time, even if the object was explored. So we
	 * redo exploration whenever we encouter such an object
	 * (actually no cases of this are currently known).
	 */
//	if (Is_Static_PyObject(op)) {
//		//jputs("JyNI-Warning: JyNI_GC_ExploreObject called with non-heap object.");
//		//jputs(Py_TYPE(op)->tp_name);
//		return;
//	}
	if (Is_Static_PyObject(op) || IS_UNEXPLORED(op))
	{ //For now we force re-exploration of static PyObjects whenever it occurs.

		JyObject* jy;
		env();
		if (PyObject_IS_GC(op))
			AS_GC(op)->gc.gc_refs = GC_EXPLORED; // Todo: Use GC_EXPLORED vs GC_EXPLORING properly
		else if (!Is_Static_PyObject(op))
		{
			/* Pure CStub-case (as pure CStubs are the only tracked non-gc objects,
			 * except some static ones.) Just create GCHead and get out of here: */
			jy = AS_JY_NO_GC(op);
			JyNI_GC_ObtainJyGCHead(env, op, jy);
			return;
		}
		jy = AS_JY_WITH_GC(op);
		/* Note that JyNI_GC_ObtainJyGCHead checks again for staticness and doesn't
		 * use jy in static case (which would otherwise result in a memory flaw).*/
		jobject jyHead = JyNI_GC_ObtainJyGCHead(env, op, jy);
		/* perform exploration here and add all reachable JyGCHeads as links to jyHead.
		 * If the object is JyNI-GC-Var, use a list or something as head-links.*/
		jobject linkHeads = exploreJyGCHeadLinks(env, op, jy);
		(*env)->CallVoidMethod(env, jyHead, traversableGCHeadSetLinks, linkHeads);
	}
	/*
	 * The code above does the following:
	 *
	 * - checkout the JyObject
	 * - evaluate the GC-flag and the CPeer-flag and look whether the type is traversable
	 * - in non PyCPeer-case create a GCHead. Done: Make sure that JyNI.c creates PyCPeerGC's
	 *   for traversable objects rather than ordinary PyCPeers.
	 * - if the object is traversable *and* uses a traversable gc-head, traverse it and set
	 *   up the JyGCHead accordingly
	 * - Non-traversable objects should not be inserted into this method. Post a fatal error
	 *   if it occurs anyway.
	 * - During exploration, JyGCHeads of target-objects will be needed. Create these for the
	 *   target-objects on the fly if they were not yet explored.
	 * - Create own GC-Head. In non-PyCPeerGCHead-case save a link to GCHead in a JyObject, so
	 * - Other objects exploring this can use it.
	 *
	 * What about pure CStubs? These don't need native tracking. Check whether it is feasible
	 * to treat them as ordinary non-gc objects on native side. Done: Converted some objects
	 * like dict and module to non-gc.
	 * Update:
	 * Also pure CStubs need a GCHead (i.e. CStubGCHead or CStubSimpleGCHead) to secure them
	 * against invidiously timed gc runs. To avoid this effort for short-lived objects we
	 * add these heads in exploration phase; until exploration phase is reached a JNI local
	 * ref will keep the Java part alive.
	 */
//	debugContext("explore done", op, "");
}

static int
visit_refCheckDec(PyObject *op, void *arg)
{
	if (!Is_Static_PyObject(op))
	{
//		debugContext("    ", op, "");
		int pos = (int) JyNI_GetJyAttribute(AS_JY(op), JyAttributeJyGCRefTmp);
		if (pos > 0)
			--(((jint*) arg)[pos-1]);
	}
	return 0;
}

static int
visit_refCheckInc(PyObject *op, void *arg)
{
	if (!Is_Static_PyObject(op))
	{
		int pos = (int) JyNI_GetJyAttribute(AS_JY(op), JyAttributeJyGCRefTmp);
		if (pos > 0)
			++(((jint*) arg)[pos-1]);
	}
	return 0;
}

/*
 * This method checks the reference graph spanned  by the objects
 * in the PyObject*-array refPool for inner consistency.
 * This means that all reference-counts must be explainable within
 * this graph.
 *
 * A zero return-value indicates that there must be external
 * objects holding references to refPool.
 *
 * A non-zero return value indicates that refPool is self-contained
 * in terms of reference-counting. For GC this usually means it can
 * be safely deleted, regardless of various internal reference counts,
 * as these must be a system of cycles.
 *
 * 'result' is a pointer to an int-array that will be filled with boolean
 * values indicating object-wise whether the component's reference count
 * is explainable within refPool (0 means explainable).
 * This array needs not to be initialized;
 * it is guaranteed that every component of it will be explicitly set.
 *
 *
 * Note that one must be holding the GIL to call this method.
 * Also, 'result' won't contain actual refcount, but contains
 * zero if no refs exist and >0 if at least one ref exists.
 * The algorithm only looks for has/has not ref, but then does
 * *not* investigate the exact count. This is sufficient to
 * know what can be safely deleted and what needs to be explored
 * again. Exact ref-count is trivially available via ob_refcnt.
 */
static inline jboolean checkReferenceGraph(PyObject** refPool, jsize size, jint* result) {
	//JyAttributeJyGCRefTmp
	int i;
	//jint* refTmp;
	//First we initialize refTmp-attributes with current object index
	//and result with current refcount.
//	jputs("Initial ref");
	JyObject* jy;
	for (i = 0; i < size; ++i)
	{
//		if (PyObject_IS_GC(refPool[i]))
//		{
		jy = AS_JY(refPool[i]);
		result[i] = refPool[i]->ob_refcnt;
		if (jy->flags & JY_CACHE_ETERNAL_FLAG_MASK)
			--result[i];
//		jputsLong(refPool[i]);
//		jputsLong(result[i]);
		//We store 'i+i' so that '0' can stand for uninitialized.
		JyNI_AddOrSetJyAttribute(jy, JyAttributeJyGCRefTmp, (void*) (i+1));
//		}
	}
	//Now we decref the tmp-refcount copy (i.e. result) of all referees.
	for (i = 0; i < size; ++i)
	{
		if (PyObject_IS_GC(refPool[i]))
		{
//			jputs("DecTraverse:");
//			jputsLong(refPool[i]);
//			debugContext("", refPool[i], "->");
			Py_TYPE(refPool[i])->tp_traverse(refPool[i], (visitproc)visit_refCheckDec, result);
		}
	}
//	jputs(__FUNCTION__);
//	for (i = 0; i < size; ++i)
//	{
//		jputsLong(refPool[i]);
//		jputsLong(result[i]);
//	}
//	jputs("");
	jboolean graphInvalid = JNI_FALSE;
	for (i = 0; i < size; ++i)
	{
//		jputs("referent:");
//		jputsLong(refPool[i]);
//		jputsLong(result[i]);
//		jputs("weak:");
//		jputsLong(getWeakRefCount(AS_JY(refPool[i])));
		if (result[i]-getWeakRefCount(AS_JY(refPool[i])) > 1)
		{
			graphInvalid = JNI_TRUE;
//			jputs("Invalid referent:");
//			jputsLong(refPool[i]);
//			jputsLong(result[i]);
			break;
		} else if (result[i] < 1) {
			jputs("JyNI-Error: negative ref-count encountered!");
			jputs(Py_TYPE(refPool[i])->tp_name);
		}
	}
	if (graphInvalid)
	{
		// We perform a breadth-first search to fully provide result-array.
		//jputs("JyNI-Note: Invalid reference graph encountered!");
		jint restoreStack[size];
		jint restoreRefs[size];
		int stackTop = 0;
		//jputsLong(__LINE__);
		for (i = 0; i < size; ++i)
		{
			if (result[i] > 1 && PyObject_IS_GC(refPool[i]))
				restoreStack[stackTop++] = i;
			restoreRefs[i] = 0;
		}
//		jputsLong(__LINE__);
//		jputs("stackTop:");
//		jputsLong(stackTop);
//		jputsLong(refPool[restoreStack[stackTop-1]]);
//		jputs(Py_TYPE(refPool[restoreStack[stackTop-1]])->tp_name);
//		jputsLong(Py_TYPE(refPool[restoreStack[stackTop-1]])->tp_traverse);
		while (stackTop)
		{
//			jputsLong(stackTop);
			//jputsLong(__LINE__);
			for (i = 0; i < stackTop; ++i)
			{
				Py_TYPE(refPool[restoreStack[i]])->tp_traverse(refPool[restoreStack[i]],
						(visitproc)visit_refCheckInc, restoreRefs);
			}
			//jputsLong(__LINE__);
			stackTop = 0;
			for (i = 0; i < size; ++i)
			{
				/* In our BFS we check for non-initial state
				 * on result side and for non-zero on ref-side
				 * to identify not yet visited nodes:
				 */
				if (result[i] == 1 && restoreRefs[i] > 0)
				{
					result[i] += restoreRefs[i];
					if (PyObject_IS_GC(refPool[i]))
						restoreStack[stackTop++] = i;
				}
				restoreRefs[i] = 0;
			}
		}
	}
	//jputsLong(__LINE__);
	for (i = 0; i < size; ++i)
	{
//		if (PyObject_IS_GC(refPool[i]))
//		{
		//result[i] = refPool[i]->ob_refcnt;
		JyNI_AddOrSetJyAttribute(AS_JY(refPool[i]), JyAttributeJyGCRefTmp, (void*) (-1));
//		}
	}
	return graphInvalid;
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_clearNativeReferences
 * Signature: ([JJ)Z
 */
jboolean JyGC_clearNativeReferences(JNIEnv *env, jclass class, jlongArray references, jlong tstate)
{
	//jputs(__FUNCTION__);
	ENTER_JyNI
	inquiry clear;
	//Here we decref the associated native objects.
	jsize size = (*env)->GetArrayLength(env, references);
	jsize i;
	PyObject* refPool[size];
	jint graphResult[size];
	jlong* arr = (*env)->GetLongArrayElements(env, references, NULL);
#if __SIZEOF_POINTER__ == 8
	/* Regardless of the platform jlong is always 8 bytes. So this branch is processed
	 * whenever jlong and PyObject* are of same size, i.e. memcpy can be used:
	 */
	Py_MEMCPY(refPool, arr, size*sizeof(PyObject*));
#else
	for (i = 0; i < size; ++i) refPool[i] = (PyObject*) arr[i];
#endif

	(*env)->ReleaseLongArrayElements(env, references, arr, JNI_ABORT);
	jboolean graphInvalid = checkReferenceGraph(refPool, size, graphResult);
	if (graphInvalid)
	{
		//jputs("Invalid graph!!");
		jsize confirmCount = 0, resurrectCount = 0;
		for (i = 0; i < size; ++i)
		{
			if (PyObject_IS_TRUNCATED(refPool[i])) {
				if (graphResult[i] > 1) ++resurrectCount;
				else ++confirmCount;
			}
		}
		jlongArray confirmArr = confirmCount ? (*env)->NewLongArray(env, confirmCount) : NULL;
		jlongArray resurrectArr = resurrectCount ? (*env)->NewLongArray(env, resurrectCount) : NULL;
		jsize confirmPos = 0, resurrectPos = 0;
		jlong* confirmArr2 = confirmArr ? (*env)->GetLongArrayElements(env, confirmArr, NULL) : NULL;
		jlong* resurrectArr2 = resurrectArr ? (*env)->GetLongArrayElements(env, resurrectArr, NULL) : NULL;
		for (i = 0; i < size; ++i)
		{
			if (PyObject_IS_TRUNCATED(refPool[i])) {
				if (graphResult[i] > 1) resurrectArr2[resurrectPos++] = (jlong) refPool[i];
				else confirmArr2[confirmPos++] = (jlong) refPool[i];
			}
		}
		if (resurrectArr) (*env)->ReleaseLongArrayElements(env, resurrectArr, resurrectArr2, JNI_COMMIT);
		if (confirmArr) (*env)->ReleaseLongArrayElements(env, confirmArr, confirmArr2, JNI_COMMIT);
		(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_gcDeletionReport, confirmArr, resurrectArr);
		if (resurrectArr) {
			/* We wait for finalizers to be done. Problem here is that some
			 * finalizers might call into native code (e.g. by looking up some
			 * attribute of a CPeer). In that case they can deadlock on the GIL
			 * with this waiting point here.
			 * Py_BEGIN_ALLOW_THREADS/Py_END_ALLOW_THREADS can solve this, but
			 * might also allow other threads to interfere with gc-process
			 * causing unforeseeable trouble.
			 *
			 * I currently see two options:
			 * 1) Go with Py_BEGIN_ALLOW_THREADS/Py_END_ALLOW_THREADS and keep an
			 *    eye on it via testing
			 * 2) Strictly require finalizers not to call into native code.
			 *
			 * Since I see no way how to enforce option 2) or even just detect
			 * violations properly, it could be a constant source of deadlocks.
			 *
			 * So I'll apply 1) and hope it won't hurt too much (still, finalizers
			 * are at least _recommended_ not to call into native code).
			 *
			 * Edit: This should be solvable by replacing ENTER_JYNI with RE_ENTER_JYNI etc.
			 *       Then it should be okay to remove Py_BEGIN_ALLOW_THREADS-block here.
			 */
			Py_BEGIN_ALLOW_THREADS
			(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_waitForCStubs);
			Py_END_ALLOW_THREADS
		}
	} else
	{
		//jputs("Valid graph");
//		jputsLong(size);
		//Todo: Provide a quicker gcDeletionReport-method for this "trivial" case.
		jsize confirmCount = 0;
		for (i = 0; i < size; ++i)
		{
			if (PyObject_IS_TRUNCATED(refPool[i])) {
				++confirmCount;
			}
		}
		jlongArray confirmArr = confirmCount ? (*env)->NewLongArray(env, confirmCount) : NULL;
		jsize confirmPos = 0;
		jlong* confirmArr2 = confirmArr ? (*env)->GetLongArrayElements(env, confirmArr, NULL) : NULL;
		for (i = 0; i < size; ++i)
		{
			if (PyObject_IS_TRUNCATED(refPool[i])) {
				confirmArr2[confirmPos++] = (jlong) refPool[i];
			}
		}
		if (confirmArr) (*env)->ReleaseLongArrayElements(env, confirmArr, confirmArr2, JNI_COMMIT);
		(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_gcDeletionReport, confirmArr, NULL);
		//(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_waitForCStubs);
	}
//	jputs("delete native objects...");
	for (i = 0; i < size; ++i)
	{
		//todo: implement a JyNI-compliant version of this stuff
	//	assert(IS_TENTATIVELY_UNREACHABLE(op));
	//	if (debug & DEBUG_SAVEALL) {
	//		PyList_Append(garbage, op);
	//	}
		if (graphResult[i] == 1 && (refPool[i])->ob_refcnt > 1 &&
		//if ((refPool[i])->ob_refcnt > 1 &&
				(clear = Py_TYPE(refPool[i])->tp_clear))
			clear(refPool[i]);
		Py_DECREF(refPool[i]);
	}
	LEAVE_JyNI
	//jputsLong(__LINE__);
	return !graphInvalid;
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_restoreCStubBackend
 * Signature: (JLorg/python/core/PyObject;LJyNI/gc/JyGCHead;)V
 */
void JyGC_restoreCStubBackend(JNIEnv *env, jclass class, jlong handle,
		jobject backend, jobject newHead)
{
	/*
	 * Actually it appears that native weak references persist object
	 * resurrection, but just in case this behavior differs between
	 * implementations, we explicitly check/re-create the weak references
	 * here. Also. The gc-head must be replaced in any case.
	 */
//	jputs(__FUNCTION__);
	Py_INCREF((PyObject*) handle);

	//Something of these commands causes sporadical JVM-crashes
	//(segfault in JVM-code (V))
	JyObject* jy = AS_JY((PyObject*) handle);
	//if ((*env)->IsSameObject(env, jy->jy, NULL)) {
	jobject tmp = jy->jy;
	jy->jy = (*env)->NewWeakGlobalRef(env, backend);
	(*env)->DeleteWeakGlobalRef(env, tmp);
	//}
	JyObject_AddOrSetJyGCHead((PyObject*) handle, jy,
			(*env)->NewWeakGlobalRef(env, newHead));
}

typedef struct {
	jlong* oldLinks;
	jsize size;
	jsize pos;
	jint changes;
} countChanges;

typedef struct {
	jlong* oldLinks;
	jlong* changedNewLinks;
	jsize size;
	jsize pos;
	jint changePos;
} findChanges;

static int
visit_countChanges(PyObject *op, void *arg)
{
	countChanges* ec = (countChanges*) arg;
	if (ec->pos >= ec->size || op != ec->oldLinks[ec->pos++])
		++(ec->changes);
	return 0;
}

static int
visit_findChanges(PyObject *op, void *arg)
{
	findChanges* ec = (findChanges*) arg;
	if (ec->pos >= ec->size || op != ec->oldLinks[ec->pos++])
		ec->changedNewLinks[ec->changePos++] = (jlong) op;
	return 0;
}

static jboolean longArrayContains(jlong* array, jsize size, jlong value)
{
	jsize i = 0;
	for (; i < size; ++i) {
		if (array[i] == value) return JNI_TRUE;
	}
	return JNI_FALSE;
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_validateGCHead
 * Signature: (J[J)Z
 */
jboolean JyGC_validateGCHead(JNIEnv *env, jclass class, jlong handle, jlongArray oldLinks)
{
	//jputs(__FUNCTION__);
	assert(handle);

	traverseproc trav;
	if (PyType_CheckExact((PyObject*) handle))// && !Py_TYPE((PyObject*) op)->tp_traverse)
		trav = statictype_traverse; //For now we use this traverse-method also for heap-types.
	else trav = Py_TYPE((PyObject*) handle)->tp_traverse;

	countChanges changeCount = {NULL, 0, 0, 0};
	if (oldLinks) {
		changeCount.size = (*env)->GetArrayLength(env, oldLinks);
		if (changeCount.size)
			changeCount.oldLinks = (*env)->GetLongArrayElements(env, oldLinks, NULL);
	}
	trav((PyObject*) handle, visit_countChanges, &changeCount);
	if (!changeCount.changes) {
		//nothing to do...
		if (changeCount.size)
			(*env)->ReleaseLongArrayElements(env, oldLinks, changeCount.oldLinks, JNI_ABORT);
		//jputs("nothing to do");
		return JNI_FALSE;
	}
	//jputsLong(__LINE__);
	//jputsLong(changeCount.changes);
	jlong* potentialChanges[changeCount.changes];
	findChanges changes = {changeCount.oldLinks, potentialChanges, changeCount.size, 0, 0};
	trav((PyObject*) handle, visit_findChanges, &changes);
	jboolean result = JNI_FALSE;
	if (changeCount.size) { //Otherwise all potential changes are actual changes.
		for (changeCount.pos = 0; changeCount.pos < changeCount.changes; ++changeCount.pos)
		{
			if (!longArrayContains(changeCount.oldLinks, changeCount.size,
					changes.changedNewLinks[changeCount.pos]))
			{
				result = JNI_TRUE;
				break;
			}
		}
	} else
		result = JNI_TRUE;
	if (changeCount.size)
		(*env)->ReleaseLongArrayElements(env, oldLinks, changeCount.oldLinks, JNI_ABORT);
	/* We do this update as soon as possible, but not while the JVM is exposing an array
	   (i.e. might not be at full power): */
	updateJyGCHeadLinks((PyObject*) handle, AS_JY((PyObject*) handle));
	//jputs("validateGCHead done");
	return result;
}

// Former, misconcepted approach:
///*
// * Class:     JyNI_JyNI
// * Method:    JyGC_validateGCHead
// * Signature: (J[J)[J
// */
//jlongArray JyGC_validateGCHead(JNIEnv *env, jclass class, jlong handle, jlongArray oldLinks)
//{
//	assert(handle);
//
//	traverseproc trav;
//	if (PyType_CheckExact((PyObject*) handle))// && !Py_TYPE((PyObject*) op)->tp_traverse)
//		trav = statictype_traverse; //For now we use this traverse-method also for heap-types.
//	else trav = Py_TYPE((PyObject*) handle)->tp_traverse;
//
//	jlong* newLinks;
//	jsize newLinkCount = 0;
//	countChanges changeCount = {NULL,0 , 0, 0};
//	if (oldLinks) {
//		changeCount.size = (*env)->GetArrayLength(env, oldLinks);
//		if (changeCount.size)
//			changeCount.oldLinks = (*env)->GetLongArrayElements(env, oldLinks, NULL);
//	}
//	trav((PyObject*) handle, visit_countChanges, &changeCount);
//	if (!changeCount.changes) {
//		//nothing to do...
//		(*env)->ReleaseLongArrayElements(env, oldLinks, changeCount.oldLinks, JNI_ABORT);
//		return NULL;
//	}
//	jlong* potentialChanges[changeCount.changes];
//	findChanges changes = {changeCount.oldLinks, potentialChanges, changeCount.size, 0, 0};
//	trav((PyObject*) handle, visit_findChanges, &changes);
//	if (changeCount.size) { //Otherwise all potential changes are actual changes.
//		for (changeCount.pos = 0; changeCount.pos < changeCount.changes; ++changeCount.pos)
//		{
//			if (longArrayContains(changeCount.oldLinks, changeCount.size,
//					changes.changedNewLinks[changeCount.pos]))
//			{
//				--changeCount.changes;
//				changes.changedNewLinks[changeCount.pos] = changes.changedNewLinks[changeCount.changes];
//				--changeCount.pos;
//			} else
//				++newLinkCount;
//		}
//	}
//	(*env)->ReleaseLongArrayElements(env, oldLinks, changeCount.oldLinks, JNI_ABORT);
//	//We do this update as soon as possible, but not while the JVM is exposing an array
//	//(i.e. might not be at full power):
//	updateJyGCHeadLinks((PyObject*) handle, AS_JY((PyObject*) handle));
//	if (newLinks)
//	{
//		jlongArray result = (*env)->NewLongArray(env, newLinks);
//		jlong* result2 = (*env)->GetLongArrayElements(env, result, NULL);
//		memcpy(result2, changes.changedNewLinks, newLinkCount*sizeof(jlong));
//		(*env)->ReleaseLongArrayElements(env, result, result2, JNI_COMMIT);
//		return result;
//	} else
//		return NULL;
//}

typedef struct {
	jlong* dest;
	jsize pos;
} exploreTrav;

static int
visit_nativeTraverse(PyObject *op, void *arg)
{
	((exploreTrav*) arg)->dest[((exploreTrav*) arg)->pos++] = (jlong) op;
	return 0;
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyGC_nativeTraverse
 * Signature: (J)[J
 */
jlongArray JyGC_nativeTraverse(JNIEnv *env, jclass class, jlong handle)
{
	assert(handle);

	traverseproc trav;
	if (PyType_CheckExact((PyObject*) handle))// && !Py_TYPE((PyObject*) op)->tp_traverse)
		trav = statictype_traverse; //For now we use this traverse-method also for heap-types.
	else trav = Py_TYPE((PyObject*) handle)->tp_traverse;
	if (!trav) return NULL;

	jsize size = JyObject_FixedGCSize((PyObject*) handle);
	if (size == UNKNOWN_FIXED_GC_SIZE)
	{
		size = 0;
		trav((PyObject*) handle, (visitproc)visit_count, &size);
	}
	jlongArray result = (*env)->NewLongArray(env, size);
	exploreTrav expl = {(*env)->GetLongArrayElements(env, result, NULL), 0};
	trav((PyObject*) handle, visit_nativeTraverse, &expl);
	(*env)->ReleaseLongArrayElements(env, result, expl.dest, JNI_COMMIT);
	return result;
}

void
JyNI_GC_Track_CStub(PyObject* op)
{
//	jputs(__FUNCTION__);
	pushExStack(op);
}

void
PyObject_GC_Track(void *op)
{
//	jputs(__FUNCTION__);
//	jputs(Py_TYPE(op)->tp_name);
//	jputsLong(op);

	/*
	 * Doing an explore here can cause the following problems:
	 * Usually we take care to initialize lists or tuples to
	 * have non-NULL items before we explore. But during this
	 * initialization nested calls to PyTupleNew create and track
	 * new tuples.
	 */
	//JyNI_GC_Explore();
	if (Is_Static_PyObject(op)) {
//		jputs("JyNI-Warning: PyObject_GC_Track called with non-heap object.");
		return;
	}
	JyNIDebugOp(JY_NATIVE_GC_TRACK, (PyObject*) op, -1);
	//todo: Explore _PyGC_generation0 from time to time to catch objects that
	//  were tracked by some nasty extension via the _PyObject_GC_TRACK(op)-macro
	//  rather than PyObject_GC_Track.
	_PyObject_GC_TRACK(op);
	if (Is_JyNICriticalType(Py_TYPE((PyObject*) op)))
	{
		env();
		(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_addJyNICriticalObject, (jlong) op);
	}
	//if ((AS_JY_WITH_GC(op)->flags & GC_NO_INITIAL_EXPLORE) == 0)
	//	JyNI_GC_Explore((PyObject*) op);
	pushExStack((PyObject*) op);
//	jputs("Track done");
//	jputs("new Stack-size:");
//	jputsLong(exStackSize());
}

void
PyObject_GC_Track_NoExplore(void *op)
{
//	jputs(__FUNCTION__);
//	jputs(Py_TYPE(op)->tp_name);
//	jputsLong(op);
	JyNIDebugOp(JY_NATIVE_GC_TRACK, (PyObject*) op, -1);
	_PyObject_GC_TRACK(op);
}

/* for binary compatibility with 2.2 */
void
_PyObject_GC_Track(PyObject *op)
{
	PyObject_GC_Track(op);
}

void
PyObject_GC_UnTrack(void *op)
{
	// Obscure:  the Py_TRASHCAN mechanism requires that we be able to
	// call PyObject_GC_UnTrack twice on an object.
	if (IS_TRACKED(op)) {
//		if (IS_UNEXPLORED(op)) {
//			//jputs("unexplored, remove from exstack");
//			//jputsLong(removeObjectExStack(op));
//			removeObjectExStack(op);
//		}
		/*
		 * We must remove the object from exStack in any case, because
		 * _PyObject_GC_UNTRACK(op) would set it UNTRACKED, which would
		 * also imply UNEXPLORED, not allowing the explore method to
		 * filter it out as intended.
		 */
		removeObjectExStack(op);
		if (Is_JyNICriticalType(Py_TYPE((PyObject*) op)))
		{
			env();
			(*env)->CallStaticVoidMethod(env, JyNIClass, JyNI_removeJyNICriticalObject, (jlong) op);
		}
		JyNIDebugOp(JY_NATIVE_GC_UNTRACK, (PyObject*) op, -1);
		_PyObject_GC_UNTRACK(op);
	}
}

// for binary compatibility with 2.2
void
_PyObject_GC_UnTrack(PyObject *op)
{
	PyObject_GC_UnTrack(op);
}

/*
 * JyNI-note:
 * Note that this function returns an uninitialized ref, i.e. with refcount not set.
 * One will have to init it via something like PyObject_INIT or PyObject_INIT_VAR.
 */
PyObject *
_PyObject_GC_Malloc(size_t basicsize)
{
	PyObject *op;
	PyGC_Head *g;
	JyObject *jy;
	if (basicsize > PY_SSIZE_T_MAX - sizeof(PyGC_Head) - sizeof(JyObject))
		return PyErr_NoMemory();
	//jy = (JyObject*) PyObject_MALLOC(sizeof(JyObject) + sizeof(PyGC_Head) + basicsize);
	jy = (JyObject*) PyObject_RawMalloc(sizeof(JyObject) + sizeof(PyGC_Head) + basicsize);
//	jputs(__FUNCTION__);
//	jputsLong((jlong) jy);
//	jputsLong(sizeof(JyObject) + sizeof(PyGC_Head) + basicsize);
//	jputs("sizes:");
//	jputsLong(sizeof(PyGC_Head));
//	jputsLong(sizeof(JyObject));
	//jy = (JyObject*) malloc(sizeof(JyObject) + sizeof(PyGC_Head) + basicsize);
	//printf("_PyObject_GC_Malloc %u\n", sizeof(JyObject) + sizeof(PyGC_Head) + basicsize);
	//printf("address: %u\n", (jlong) jy);
	if (jy == NULL) return PyErr_NoMemory();
	jy->flags = JY_GC_FLAG_MASK;
	jy->jy = NULL;
	jy->attr = NULL;
	//g = (PyGC_Head *)PyObject_MALLOC(sizeof(PyGC_Head) + basicsize);
	g = GC_FROM_JY(jy);
//	jputs("g");
//	jputsLong((jlong) g);
	//if (g == NULL) return PyErr_NoMemory();
	g->gc.gc_refs = GC_UNTRACKED;
// JyNI-todo: Implement a JyNI-compliant variant of the collection block.
//	generations[0].count++; // number of allocated GC objects
//	if (generations[0].count > generations[0].threshold &&
//		enabled &&
//		generations[0].threshold &&
//		!collecting &&
//		!PyErr_Occurred()) {
//		collecting = 1;
//		collect_generations();
//		collecting = 0;
//	}
	op = FROM_GC(g);
//	jputs("op");
//	jputsLong((jlong) op);
	JyNIDebug(JY_NATIVE_ALLOC_GC, op, AS_JY_WITH_GC(op), basicsize, NULL);
	//Shortcut not feasible because generic AS_JY not yet works as it depends
	//on a properly configured Py_TYPE(op):
	//JyNIDebugOp(JY_NATIVE_ALLOC_GC, op, basicsize);
	return op;
}

PyObject *
_PyObject_GC_New(PyTypeObject *tp)
{
	return _JyObject_GC_New(tp, JyNI_JythonTypeEntry_FromPyType(tp));
}

/*
 * This method is also intended for "public" (in the sense of not in gcmodule.c)
 * use in some cases. Whenever GC-relevant objects use a free_list this method
 * should be used to re-initialize the corresponding JyObject.
 */
inline void _PyObject_GC_InitJy(PyObject *op, TypeMapEntry* tme)
{
	JyObject* jy = AS_JY_WITH_GC(op);
	/*
	 * We abuse the jy-field here to cache the already
	 * looked-up tme for later use. Methods in JyNI are
	 * aware of this and check for a cached tme in the
	 * jy-field before another look-up is performed.
	 * A non-NULL jy-field and lacking the INITIALIZED-flag
	 * indicate that a tme was stored in the jy-field.
	 */
	if (tme)
	{
		jy->jy = (jweak) tme;
		jy->flags = JY_GC_FLAG_MASK | tme->flags;
	} else
		jy->flags = JY_GC_FLAG_MASK;
	//jy->attr = NULL;
	//JyNI_SetUpJyObject((JyObject*) op);
}

PyObject *
_JyObject_GC_New(PyTypeObject *tp, TypeMapEntry* tme)
{
	PyObject *op;
	if (tme)
	{
		//printf("PyObject_GC_New-size: %u\n", (jlong) (tme->flags & JY_TRUNCATE_FLAG_MASK) ? sizeof(PyObject) : _PyObject_SIZE(tp));
		op = _PyObject_GC_Malloc((tme->flags & JY_TRUNCATE_FLAG_MASK) ? sizeof(PyObject)+tme->truncate_trailing : _PyObject_SIZE(tp));
		if (op != NULL)
		{
			_PyObject_GC_InitJy(op, tme);
			op = PyObject_INIT(op, tp);
		}
	}
	else
	{
		op = _PyObject_GC_Malloc(_PyObject_SIZE(tp));
		if (op != NULL) op = PyObject_INIT(op, tp);
	}
	return op;
}

PyVarObject *
_PyObject_GC_NewVar(PyTypeObject *tp, Py_ssize_t nitems)
{
	return _JyObject_GC_NewVar(tp, nitems, JyNI_JythonTypeEntry_FromPyType(tp));
}

PyVarObject *
_JyObject_GC_NewVar(PyTypeObject *tp, Py_ssize_t nitems, TypeMapEntry* tme)
{
	PyVarObject *op;
	if (tme != NULL)
	{
		op = (PyVarObject *) _PyObject_GC_Malloc((tme->flags & JY_TRUNCATE_FLAG_MASK) ? sizeof(PyVarObject)+tme->truncate_trailing : _PyObject_VAR_SIZE(tp, nitems));
		if (op != NULL)
		{
			JyObject* jy = AS_JY_WITH_GC(op);
			/*
			 * We abuse the jy-field here to cache the already
			 * looked-up tme for later use. Methods in JyNI are
			 * aware of this and check for a cached tme in the
			 * jy-field before another look-up is performed.
			 * A non-NULL jy-field and lacking the INITIALIZED-flag
			 * indicate that a tme was stored in the jy-field.
			 */
			jy->jy = (jweak) tme;
			jy->flags |= tme->flags;
			op = PyObject_INIT_VAR(op, tp, nitems);
		}
		//JyNI_SetUpJyVarObject((JyVarObject*) op);
	} else
	{
		const size_t size = _PyObject_VAR_SIZE(tp, nitems);
		op = (PyVarObject *) _PyObject_GC_Malloc(size);
		if (op != NULL) op = PyObject_INIT_VAR(op, tp, nitems);
	}
	return op;
}

PyVarObject *
_PyObject_GC_Resize(PyVarObject *op, Py_ssize_t nitems)
{
	JyObject* jy = AS_JY(op);
	if (JyObject_IS_TRUNCATED(jy))
	{
		//do nothing since JyObjects are always of same size:
		//(also when truncateTrailing != 0, as trailing size does not change.)
		/*const size_t basicsize = JyObjectBasicSize;
		PyGC_Head *g = AS_GC(op);
		if (basicsize > PY_SSIZE_T_MAX - sizeof(PyGC_Head))
			return (PyVarObject *)PyErr_NoMemory();
		g = (PyGC_Head *)PyObject_REALLOC(g,  sizeof(PyGC_Head) + basicsize);
		if (g == NULL)
			return (PyVarObject *)PyErr_NoMemory();
		op = (PyVarObject *) FROM_GC(g);
		Py_SIZE(op) = nitems;*/

		return op;
	}
	else
	{
		const size_t basicsize = _PyObject_VAR_SIZE(Py_TYPE(op), nitems);
		//PyGC_Head *q = AS_GC(op);
		//JyObject* jy2 = AS_JY(op);
		if (basicsize > PY_SSIZE_T_MAX - sizeof(PyGC_Head) - sizeof(JyObject))
			return (PyVarObject *)PyErr_NoMemory();
		//jy = PyObject_REALLOC(jy,  sizeof(JyObject) + sizeof(PyGC_Head) + basicsize);
		jy = PyObject_RawRealloc(jy,  sizeof(JyObject) + sizeof(PyGC_Head) + basicsize);
		if (jy == NULL) return (PyVarObject *) PyErr_NoMemory();

		//PyGC_Head *q = GC_FROM_JY(jy);
		//if (q == NULL) return (PyVarObject *) PyErr_NoMemory();
		//op = (PyVarObject *) FROM_GC(q);
		PyVarObject *op0 = op; //Only needed in debug-mode!
		op = (PyVarObject *) FROM_JY(jy);
		Py_SIZE(op) = nitems;
		//if (jy->jy) jputs("GC resize, but handle is initialized");
		if (jy->flags & JY_INITIALIZED_FLAG_MASK && jy->jy) //jputs("really initialized");
		{
			/* take care to correct the handle on java-side */
			env(NULL);
			(*env)->CallStaticVoidMethod(env, JyNIClass, JyNISetNativeHandle, jy->jy, (jlong) op);//, (jy->flags & JY_TRUNCATE_FLAG_MASK) != 0);
			//todo: maybe do sync here
		}
		//JyNIDebug2(JY_NATIVE_REALLOC_GC, AS_JY_WITH_GC(op0), jy, basicsize, NULL);
		JyNIDebugOp2(JY_NATIVE_REALLOC_GC, op0, jy, basicsize);
		return op;
	}
}

void
PyObject_GC_Del(void *op)
{
	/*if (JyNI_IsJyObject(op))
	{
		if (((PyObject*) op)->ob_type->tp_itemsize == 0)
			JyNI_CleanUpJyObject((JyObject*) op);
		else
			JyNI_CleanUpJyVarObject((JyVarObject*) op);
	}*/
	//JyObject* jy = AS_JY(op);
	JyObject* jy = AS_JY_WITH_GC(op);
	JyNIDebugOp(JY_NATIVE_FREE_GC, op, -1);
	JyNI_CleanUp_JyObject(jy);
	PyGC_Head *g = AS_GC(op);
// todo: Write a JyNI-compliant version of this inline clean-up:
//	if (IS_TRACKED(op))
//		gc_list_remove(g);
//	if (generations[0].count > 0) {
//		generations[0].count--;
//	}
	//PyObject_FREE(jy);
	PyObject_RawFree(jy);
}

// for binary compatibility with 2.2
#undef _PyObject_GC_Del
void
_PyObject_GC_Del(PyObject *op)
{
	PyObject_GC_Del(op);
}
