#pragma once

#ifndef CSTL_VECTOR_H
#define CSTL_VECTOR_H

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
 * STL ABI `std::vector` layout.
 * 
 * Does not include the allocator, which nonetheless is a part of the `std::vector`
 * structure! You are responsible for including it, since it can take on any form.
 * 
 * Do not manipulate the members directly, use the associated functions!
 * 
 */
typedef struct CSTL_VectorVal {
    void* first;
    void* last;
    void* end;
} CSTL_VectorVal;

/**
 * Reference to a mutable `CSTL_VectorVal`.
 * 
 * Must not be null.
 * 
 */
typedef CSTL_VectorVal* CSTL_VectorRef;

/**
 * Reference to a const `CSTL_VectorVal`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_VectorVal* CSTL_VectorCRef;

/**
 * An iterator over elements of a vector.
 * 
 * Contains a reference to a `CSTL_Type` which must outlive it.
 * 
 * Not ABI-compatible with `std::vector::iterator`.
 * 
 */
typedef struct CSTL_VectorIter {
    const void* pointer;
    size_t size;
#ifndef NDEBUG
    CSTL_VectorCRef owner;
#endif
} CSTL_VectorIter;

/**
 * Initializes the vector pointed to by `new_instance`, but does not allocate any memory.
 * 
 * An initialized vector can be trivially destroyed without leaks as long
 * as no functions that allocate (push, insert, reserve, etc.) have been called on it. 
 * 
 * Re-initializing a vector with a backing memory allocation will leak the old
 * memory allocation.
 * 
 */
void CSTL_vector_construct(CSTL_VectorVal* new_instance);

/**
 * Destroys the vector pointed to by `instance`, destroying elements
 * and freeing the backing storage.
 * 
 */
void CSTL_vector_destroy(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc);
/**
 * Replaces the contents of `instance` with the contents of `other_instance`.
 * 
 * If `propagate_alloc == true && alloc != other_alloc` then storage 
 * is freed with `alloc` and allocated again with `other_alloc` before elements
 * are copied. Then, `instance` uses `other_alloc` as its allocator. If
 * the allocation fails, returns `false`, otherwise always returns `true`.
 * 
 * If `propagate_alloc == false` `instance` keeps using `alloc` as its allocator,
 * potentially reusing its storage.
 * 
 * You are responsible for replacing the allocator outside of `CSTL_VectorVal` if applicable.
 * 
 */
bool CSTL_vector_copy_assign(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_VectorCRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Moves the contents of `other_instance` to the contents of `instance`.
 * 
 * If `propagate_alloc == true` storage is replaced with storage of `other_instance`.
 * Then, `instance` uses `other_alloc` as its allocator.
 * 
 * If `propagate_alloc == false && alloc != other_alloc` then storage is reused
 * and individual elements of `other_instance` are moved in. If the allocation fails,
 * returns `false`, otherwise always returns `true`. Then, `instance` uses
 * `alloc` as its allocator.
 * 
 * You are responsible for replacing the allocator outside of `CSTL_VectorVal` if applicable.
 * 
 */
bool CSTL_vector_move_assign(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_VectorRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Destroys vector contents, replacing them with a copy of the range `[first, last)`.
 * 
 * If `new_size > CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_copy_assign_range(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* first, const void* last, CSTL_Alloc* alloc);

/**
 * Destroys vector contents, replacing them with a copy of the range `[first, last)`.
 * 
 * If `new_size > CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_move_assign_range(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, void* first, void* last, CSTL_Alloc* alloc);

/**
 * Destroys vector contents, replacing them with `new_size` copies of `value`.
 * 
 * If `new_size > CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_assign_n(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc);

/**
 * Swaps vector contents.
 * 
 * You are responsible for swapping the allocators.
 * 
 */
void CSTL_vector_swap(CSTL_VectorRef instance, CSTL_VectorRef other_instance);

/**
 * Returns a pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(instance, type)` the behavior is undefined.
 * 
 */
void* CSTL_vector_index(CSTL_VectorRef instance, CSTL_Type type, size_t pos);

/**
 * Returns a const pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(instance, type)` the behavior is undefined.
 * 
 */
const void* CSTL_vector_const_index(CSTL_VectorCRef instance, CSTL_Type type, size_t pos);

/**
 * Returns a pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(instance, type)` a null pointer is returned.
 * 
 */
void* CSTL_vector_at(CSTL_VectorRef instance, CSTL_Type type, size_t pos);

/**
 * Returns a const pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(instance, type)` a null pointer is returned.
 * 
 */
const void* CSTL_vector_const_at(CSTL_VectorCRef instance, CSTL_Type type, size_t pos);

/**
 * Returns a pointer to the first element in the vector.
 * 
 * If `CSTL_vector_empty(instance) == true` the behavior is undefined.
 * 
 */
void* CSTL_vector_front(CSTL_VectorRef instance);

/**
 * Returns a const pointer to the first element in the vector.
 * 
 * If `CSTL_vector_empty(instance) == true` the behavior is undefined.
 * 
 */
const void* CSTL_vector_const_front(CSTL_VectorCRef instance);

/**
 * Returns a pointer to the last element in the vector.
 * 
 * If `CSTL_vector_empty(instance) == true` the behavior is undefined.
 * 
 */
void* CSTL_vector_back(CSTL_VectorRef instance, CSTL_Type type);

/**
 * Returns a const pointer to the last element in the vector.
 * 
 * If `CSTL_vector_empty(instance) == true` the behavior is undefined.
 * 
 */
const void* CSTL_vector_const_back(CSTL_VectorCRef instance, CSTL_Type type);

/**
 * Returns a pointer to the underlying storage.
 * 
 * The returned pointer is valid even if the vector is empty,
 * in which case it is not dereferenceable.
 * 
 */
void* CSTL_vector_data(CSTL_VectorRef instance);

/**
 * Returns a pointer to the underlying storage.
 * 
 * The returned pointer is valid even if the vector is empty,
 * in which case it is not dereferenceable.
 * 
 */
const void* CSTL_vector_const_data(CSTL_VectorCRef instance);

/**
 * Construct an iterator to the first element of the vector.
 * 
 * If the vector is empty: `CSTL_vector_iterator_eq(begin, end) == true`.
 * 
 */
CSTL_VectorIter CSTL_vector_begin(CSTL_VectorCRef instance, CSTL_Type type);

/**
 * Construct an iterator past the last element of the vector.
 * 
 * If the vector is empty: `CSTL_vector_iterator_eq(begin, end) == true`.
 * 
 */
CSTL_VectorIter CSTL_vector_end(CSTL_VectorCRef instance, CSTL_Type type);

/**
 * Seeks the iterator forwards by `n` elements.
 * 
 * Returns a new iterator at the resulting iterator position.
 * 
 */
CSTL_VectorIter CSTL_vector_iterator_add(CSTL_VectorIter iterator, ptrdiff_t n);

/**
 * Seeks the iterator backwards by `n` elements.
 * 
 * Returns a new iterator at the resulting iterator position.
 * 
 */
CSTL_VectorIter CSTL_vector_iterator_sub(CSTL_VectorIter iterator, ptrdiff_t n);

/**
 * Dereferences the iterator at the element it's pointing to.
 * 
 * Returns a pointer to the element.
 * 
 * `iterator` must be dereferenceable, not value initialized
 * and pointing to a valid element inside the vector.
 * 
 * It is not valid to mutate an element derived from an iterator obtained with
 * a const vector pointer.
 * 
 */
void* CSTL_vector_iterator_deref(CSTL_VectorIter iterator);

/**
 * Dereferences the iterator at an element offset `n`.
 * 
 * Returns a pointer to the element.
 * 
 * `iterator` must be dereferenceable at its new position, not value initialized
 * and pointing to a valid element inside the vector.
 * 
 * It is not valid to mutate an element derived from an iterator obtained with
 * a const vector pointer.
 * 
 */
void* CSTL_vector_iterator_index(CSTL_VectorIter iterator, ptrdiff_t n);

/**
 * Subtracts two iterators and returns the distance measured in elements.
 * 
 * Returns the signed number of elements between two iterators.
 * 
 * They must belong to the same vector.
 * 
 */
ptrdiff_t CSTL_vector_iterator_distance(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

/**
 * Compares iterators for equality.
 * 
 * They must belong to the same vector.
 * 
 */
bool CSTL_vector_iterator_eq(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

/**
 * Compares iterators for less than.
 * 
 * They must belong to the same vector.
 * 
 */
bool CSTL_vector_iterator_lt(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

/**
 * Returns `true` if the vector is empty or `false` otherwise.
 * 
 */
bool CSTL_vector_empty(CSTL_VectorCRef instance);

/**
 * Returns the number of elements in the vector.
 * 
 */
size_t CSTL_vector_size(CSTL_VectorCRef instance, CSTL_Type type);

/**
 * Returns the total element capacity of the vector.
 * 
 */
size_t CSTL_vector_capacity(CSTL_VectorCRef instance, CSTL_Type type);

/**
 * Returns the maximum possible number of elements in the vector.
 * 
 * As if by `(PTRDIFF_MAX - 1) / CSTL_sizeof_type(type)`.
 * 
 */
size_t CSTL_vector_max_size(CSTL_Type type);

/**
 * Resizes the vector to contain `new_size` elements.
 * 
 * If `new_size > CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_resize(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc);

/**
 * Truncates the vector to contain `new_size` elements, removing any excess.
 * 
 * Has no effect if `new_size >= CSTL_vector_size(instance, type)`.
 * 
 * If `new_size > CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
void CSTL_vector_truncate(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, size_t new_size);

/**
 * If `new_capacity > CSTL_vector_capacity(instance, type)`, reallocates and expands
 * the vector storage.
 * 
 * If `new_capacity` exceeds `CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_reserve(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, size_t new_capacity, CSTL_Alloc* alloc);

/**
 * Request removal of unused capacity.
 * 
 * If a reallocation occurs and fails returns `false`, otherwise
 * always returns `true`.
 * 
 */
bool CSTL_vector_shrink_to_fit(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_Alloc* alloc);

/**
 * Erase all elements from the vector without affecting capacity.
 * 
 */
void CSTL_vector_clear(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop);

/**
 * Appends a copy of `value` to the end of the vector.
 * 
 * If `CSTL_vector_size(instance, type) == CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_copy_push_back(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* value, CSTL_Alloc* alloc);

/**
 * Appends `value` to the end of the vector by moving it.
 * 
 * If `CSTL_vector_size(instance, type) == CSTL_vector_max_size(type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_move_push_back(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, void* value, CSTL_Alloc* alloc);

/**
 * Removes the last element from the vector.
 * 
 * If `CSTL_vector_empty(instance) == true` the behavior is undefined.
 * 
 */
void CSTL_vector_pop_back(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop);

/**
 * Inserts a copy of `value` into the vector before `where` and returns
 * an iterator to the newly inserted element.
 * 
 * If `CSTL_vector_size(instance, ...) == CSTL_vector_max_size(...)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(instance, ...)`.
 * 
 */
CSTL_VectorIter CSTL_vector_copy_insert(CSTL_VectorRef instance, CSTL_CopyTypeCRef copy, CSTL_VectorIter where, const void* value, CSTL_Alloc* alloc);

/**
 * Inserts `value` into the vector before `where` by moving it and returns
 * an iterator to the newly inserted element.
 * 
 * If `CSTL_vector_size(instance, ...) == CSTL_vector_max_size(...)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(instance, ...)`.
 * 
 */
CSTL_VectorIter CSTL_vector_move_insert(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter where, void* value, CSTL_Alloc* alloc);

/**
 * Inserts a copy of the range `[range_first, range_last)` into the vector before `where` and returns
 * an iterator to the newly inserted elements.
 * 
 * Exceeding the maximum possible vector size has the same effect as `CSTL_vector_copy_insert`,
 * returning `CSTL_vector_end(instance, ...)`.
 * 
 */
CSTL_VectorIter CSTL_vector_copy_insert_range(CSTL_VectorRef instance, CSTL_CopyTypeCRef copy, CSTL_VectorIter where, const void* range_first, const void* range_last, CSTL_Alloc* alloc);

/**
 * Inserts the range `[range_first, range_last)` into the vector before `where` by moving it and returns
 * an iterator to the newly inserted elements.
 * 
 * Exceeding the maximum possible vector size has the same effect as `CSTL_vector_move_insert`,
 * returning `CSTL_vector_end(instance, ...)`.
 * 
 */
CSTL_VectorIter CSTL_vector_move_insert_range(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter where, void* range_first, void* range_last, CSTL_Alloc* alloc);

/**
 * Inserts `count` copies of `value` into the vector before `where` and returns
 * an iterator to the first newly inserted element.
 * 
 * If `count > CSTL_vector_max_size(...) - CSTL_vector_size(instance, ...)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(instance, ...)`.
 * 
 */
CSTL_VectorIter CSTL_vector_insert_n(CSTL_VectorRef instance, CSTL_CopyTypeCRef copy, CSTL_VectorIter where, size_t count, const void* value, CSTL_Alloc* alloc);

/**
 * Removes the element at `where` and returns an iterator following the
 * removed element.
 * 
 * The iterator `where` must be valid and dereferenceable.
 * 
 */
CSTL_VectorIter CSTL_vector_erase(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter where);

/**
 * Removes elements in the range `[first, last)` and returns an iterator following the
 * removed elements.
 * 
 * If `first == last`, no operation is performed.
 * 
 */
CSTL_VectorIter CSTL_vector_erase_range(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter first, CSTL_VectorIter last);

#if defined(__cplusplus)
}
#endif

#endif
