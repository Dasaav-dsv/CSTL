#pragma once

#ifndef CSTL_LIST_H
#define CSTL_LIST_H

#include "alloc.h"
#include "type.h"

#if defined(__cplusplus)
#include <cassert>
#include <cstddef>
extern "C" {
#else
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#endif

/**
 * Internal node structure for the doubly-linked list.
 *
 * Contains pointers to the next and previous nodes. The stored value
 * follows this structure in memory.
 *
 */
typedef struct CSTL_ListNode {
    struct CSTL_ListNode* next;
    struct CSTL_ListNode* prev;
} CSTL_ListNode;

/**
 * STL ABI `std::list` layout.
 *
 * Does not include the allocator, which nonetheless is a part of the `std::list`
 * structure! You are responsible for including it, since it can take on any form.
 *
 * Do not manipulate the members directly, use the associated functions!
 *
 */
typedef struct CSTL_ListVal {
    CSTL_ListNode* sentinel;
    size_t size;
} CSTL_ListVal;

/**
 * Reference to a const `CSTL_ListVal`.
 *
 * Must not be null.
 *
 */
typedef const CSTL_ListVal* CSTL_ListCRef;

/**
 * Reference to a mutable `CSTL_ListVal`.
 *
 * Must not be null.
 *
 */
typedef CSTL_ListVal* CSTL_ListRef;

/**
 * An iterator over elements of a list.
 *
 * Contains a reference to the owning list and a pointer to the current node.
 *
 * Not ABI-compatible with `std::list::iterator`.
 *
 */
typedef struct CSTL_ListIter {
    CSTL_ListCRef owner;
    const CSTL_ListNode* pointer;
} CSTL_ListIter;

/**
 * Initializes the list pointed to by `new_instance`, allocating the sentinel node.
 *
 * An initialized list is empty. It can be trivially destroyed without leaks as long
 * as no functions that allocate (push, insert, etc.) have been called on it.
 *
 * Re-initializing a list with existing nodes will leak the old nodes.
 *
 */
void CSTL_list_construct(CSTL_ListRef new_instance, CSTL_Alloc* alloc);

/**
 * Destroys the list pointed to by `instance`, destroying all elements
 * and freeing all node storage, including the sentinel node.
 *
 */
void CSTL_list_destroy(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with a copy of the contents of `other_instance`.
 *
 * If `propagate_alloc == true && alloc != other_alloc`, then storage
 * is freed with `alloc`, `other_alloc` is copied to `alloc`, and then new nodes
 * are allocated using the new allocator.
 *
 * If `propagate_alloc == false`, `instance` keeps using `alloc` as its allocator.
 *
 * You are responsible for replacing the allocator outside of `CSTL_ListVal` if applicable.
 *
 */
bool CSTL_list_copy_assign(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_ListCRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Moves the contents of `other_instance` to the contents of `instance`.
 *
 * If `propagate_alloc == true` or if `alloc == other_alloc`, the contents are swapped in O(1) time.
 * If `propagate_alloc == true`, `instance` also takes on `other_alloc`.
 *
 * If `propagate_alloc == false && alloc != other_alloc`, then elements are moved one-by-one.
 *
 * You are responsible for replacing the allocator outside of `CSTL_ListVal` if applicable.
 *
 */
bool CSTL_list_move_assign(CSTL_ListRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_ListRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Destroys list contents, replacing them with `new_size` copies of `value`.
 *
 * If `new_size > CSTL_list_max_size(type)`, this function has no effect and returns `false`.
 * If allocation fails, returns `false`, otherwise returns `true`.
 *
 */
bool CSTL_list_assign_n(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc);

/**
 * Returns a pointer to the first element in the list.
 *
 * If `CSTL_list_empty(instance) == true`, the behavior is undefined.
 *
 */
void* CSTL_list_front(CSTL_ListRef instance);

/**
 * Returns a const pointer to the first element in the list.
 *
 * If `CSTL_list_empty(instance) == true`, the behavior is undefined.
 *
 */
const void* CSTL_list_const_front(CSTL_ListCRef instance);

/**
 * Returns a pointer to the last element in the list.
 *
 * If `CSTL_list_empty(instance) == true`, the behavior is undefined.
 *
 */
void* CSTL_list_back(CSTL_ListRef instance);

/**
 * Returns a const pointer to the last element in the list.
 *
 * If `CSTL_list_empty(instance) == true`, the behavior is undefined.
 *
 */
const void* CSTL_list_const_back(CSTL_ListCRef instance);

/**
 * Constructs an iterator to the first element of the list.
 *
 * If the list is empty, the returned iterator will equal `CSTL_list_end(instance)`.
 *
 */
CSTL_ListIter CSTL_list_begin(CSTL_ListCRef instance);

/**
 * Constructs an iterator to the past-the-end element of the list.
 *
 * This iterator acts as a placeholder and does not point to any element.
 *
 */
CSTL_ListIter CSTL_list_end(CSTL_ListCRef instance);

/**
 * Seeks the iterator forwards by `n` elements.
 *
 * Returns a new iterator at the resulting position.
 *
 */
CSTL_ListIter CSTL_list_iterator_add(CSTL_ListIter iterator, ptrdiff_t n);

/**
 * Seeks the iterator backwards by `n` elements.
 *
 * Returns a new iterator at the resulting position.
 *
 */
CSTL_ListIter CSTL_list_iterator_sub(CSTL_ListIter iterator, ptrdiff_t n);

/**
 * Dereferences the iterator at the element it's pointing to.
 *
 * Returns a pointer to the element. `iterator` must be dereferenceable.
 *
 */
void* CSTL_list_iterator_deref(CSTL_ListIter iterator);

/**
 * Subtracts two iterators and returns the distance measured in elements.
 *
 * The iterators must belong to the same list.
 *
 */
ptrdiff_t CSTL_list_iterator_distance(CSTL_ListIter lhs, CSTL_ListIter rhs);

/**
 * Compares iterators for equality.
 *
 * The iterators must belong to the same list.
 *
 */
bool CSTL_list_iterator_eq(CSTL_ListIter lhs, CSTL_ListIter rhs);

/**
 * Returns `true` if the list is empty, `false` otherwise.
 *
 */
bool CSTL_list_empty(CSTL_ListCRef instance);

/**
 * Returns the number of elements in the list.
 *
 */
size_t CSTL_list_size(CSTL_ListCRef instance);

/**
 * Returns the maximum possible number of elements in the list.
 *
 */
size_t CSTL_list_max_size(CSTL_Type type);

/**
 * Erases all elements from the list.
 *
 */
void CSTL_list_clear(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc);

/**
 * Swaps the contents of two lists in O(1) time.
 *
 * You are responsible for swapping the allocators if necessary.
 *
 */
void CSTL_list_swap(CSTL_ListRef instance, CSTL_ListRef other_instance);

/**
 * Resizes the list to contain `new_size` elements.
 *
 * If `new_size` is smaller than the current size, excess elements are destroyed.
 * If `new_size` is larger, new elements are appended and copy-constructed from `value`.
 *
 */
bool CSTL_list_resize(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc);

/**
 * Inserts `count` copies of `value` into the list before `where`.
 *
 * Returns an iterator to the first newly inserted element, or `where` if `count` is 0.
 *
 */
CSTL_ListIter CSTL_list_insert_n(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_ListIter where, size_t count, const void* value, CSTL_Alloc* alloc);

/**
 * Inserts a copy of `value` into the list before `where`.
 *
 * Returns an iterator to the newly inserted element.
 *
 */
CSTL_ListIter CSTL_list_copy_insert(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_ListIter where, const void* value, CSTL_Alloc* alloc);

/**
 * Inserts `value` into the list before `where` by moving it.
 *
 * Returns an iterator to the newly inserted element.
 *
 */
CSTL_ListIter CSTL_list_move_insert(CSTL_ListRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_ListIter where, void* value, CSTL_Alloc* alloc);

/**
 * Appends a copy of `value` to the end of the list.
 *
 */
bool CSTL_list_copy_push_back(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* value, CSTL_Alloc* alloc);

/**
 * Appends `value` to the end of the list by moving it.
 *
 */
bool CSTL_list_move_push_back(CSTL_ListRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, void* value, CSTL_Alloc* alloc);

/**
 * Removes the last element from the list.
 *
 * If the list is empty, the behavior is undefined.
 *
 */
void CSTL_list_pop_back(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc);

/**
 * Prepends a copy of `value` to the beginning of the list.
 *
 */
bool CSTL_list_copy_push_front(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* value, CSTL_Alloc* alloc);

/**
 * Prepends `value` to the beginning of the list by moving it.
 *
 */
bool CSTL_list_move_push_front(CSTL_ListRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, void* value, CSTL_Alloc* alloc);

/**
 * Removes the first element from the list.
 *
 * If the list is empty, the behavior is undefined.
 *
 */
void CSTL_list_pop_front(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc);

/**
 * Removes the element at `where`.
 *
 * Returns an iterator following the removed element.
 * The iterator `where` must be valid and dereferenceable.
 *
 */
CSTL_ListIter CSTL_list_erase(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc, CSTL_ListIter where);

/**
 * Removes elements in the range `[first, last)`.
 *
 * Returns an iterator following the last removed element (`last`).
 *
 */
CSTL_ListIter CSTL_list_erase_range(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc, CSTL_ListIter first, CSTL_ListIter last);

#if defined(__cplusplus)
}
#endif

#endif
