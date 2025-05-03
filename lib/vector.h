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
 * A representation of the parts making up a vector.
 * 
 * Contains a references to `CSTL_VectorVal` and `CSTL_Type` which must all outlive it.
 * 
 * This struct is *not* `std::vector`, it simply contains references
 * to the parts necessary for its manipulation.
 * 
 */
typedef struct CSTL_VectorCtx {
    CSTL_VectorCRef instance;
    CSTL_TypeCRef type;
} CSTL_VectorCtx;

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
    CSTL_TypeCRef type;
#ifndef NDEBUG
    CSTL_VectorCRef owner;
#endif
} CSTL_VectorIter;

/**
 * Initializes the vector pointed to by `context->instance`, but does not allocate any memory.
 * 
 * An initialized vector can be trivially destroyed without leaks as long
 * as no functions that allocate (push, insert, reserve, etc.) have been called on it. 
 * 
 * Re-initializing a vector with a backing memory allocation will leak the old
 * memory allocation.
 * 
 */
void CSTL_vector_construct(CSTL_VectorCtx context);

/**
 * Destroys the vector pointed to by `context->instance`, destroying elements
 * and freeing the backing storage.
 * 
 */
void CSTL_vector_destroy(CSTL_VectorCtx context, CSTL_Alloc* alloc);

/**
 * Destroys vector contents, replacing them with `new_size` copies of `value`.
 * 
 * If `new_size > CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * `value` cannot point to an element inside the vector.
 * 
 */
bool CSTL_vector_assign(CSTL_VectorCtx context, size_t new_size, const void* value, CSTL_Alloc* alloc);

/**
 * Destroys vector contents, replacing them with a copy of the range `[first, first + new_size)`.
 * 
 * If `new_size > CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * `first` cannot point inside the vector.
 * 
 */
bool CSTL_vector_assign_continuous_range(CSTL_VectorCtx context, size_t new_size, const void* first, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `context->instance` with the contents of `other_context->instance`.
 * 
 * If `propagate_alloc == true && alloc != other_alloc` then storage 
 * is freed with `alloc` and allocated again with `context->other_alloc` before elements
 * are copied. Then, `context->instance` uses `context->other_alloc` as its allocator.
 * 
 * If `propagate_alloc == false` `context->instance` keeps using `alloc` as its allocator,
 * potentially reusing its storage.
 * 
 * You are responsible for replacing the allocator outside of `CSTL_VectorVal` if applicable.
 * 
 */
void CSTL_vector_copy_assign(CSTL_VectorCtx context, CSTL_Alloc* alloc, CSTL_VectorCtx other_context, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Moves the contents of `other_context->instance` to the contents of `context->instance`.
 * 
 * If `propagate_alloc == true` storage is replaced with storage of `other_context->instance`.
 * Then, `context->instance` uses `context->other_alloc` as its allocator.
 * 
 * If `propagate_alloc == false && alloc != other_alloc` then storage is reused
 * and individual elements of `context->other` are moved in.
 * Then, `context->instance` uses `alloc` as its allocator.
 * 
 * You are responsible for replacing the allocator outside of `CSTL_VectorVal` if applicable.
 * 
 */
void CSTL_vector_move_assign(CSTL_VectorCtx context, CSTL_Alloc* alloc, CSTL_VectorCtx other_context, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Swaps vector contents.
 * 
 * You are responsible for swapping the allocators.
 * 
 */
void CSTL_vector_swap(CSTL_VectorCtx context, CSTL_VectorCtx other_context);

/**
 * Returns a pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(context)` the behavior is undefined.
 * 
 */
void* CSTL_vector_index(CSTL_VectorCtx context, size_t pos);

/**
 * Returns a const pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(context)` the behavior is undefined.
 * 
 */
const void* CSTL_vector_const_index(CSTL_VectorCtx context, size_t pos);

/**
 * Returns a pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(context)` a null pointer is returned.
 * 
 */
void* CSTL_vector_get_at(CSTL_VectorCtx context, size_t pos);

/**
 * Returns a const pointer to the element at `pos`.
 * 
 * If `pos >= CSTL_vector_size(context)` a null pointer is returned.
 * 
 */
const void* CSTL_vector_const_get_at(CSTL_VectorCtx context, size_t pos);

/**
 * Returns a pointer to the first element in the vector.
 * 
 * If `CSTL_vector_empty(context) == true` or if an incompatible
 * `CSTL_Type` is used the behavior is undefined.
 * 
 */
void* CSTL_vector_front(CSTL_VectorCtx context);

/**
 * Returns a const pointer to the first element in the vector.
 * 
 * If `CSTL_vector_empty(context) == true` or if an incompatible
 * `CSTL_Type` is used the behavior is undefined.
 * 
 */
const void* CSTL_vector_const_front(CSTL_VectorCtx context);

/**
 * Returns a pointer to the last element in the vector.
 * 
 * If `CSTL_vector_empty(context) == true` or if an incompatible
 * `CSTL_Type` is used the behavior is undefined.
 * 
 */
void* CSTL_vector_back(CSTL_VectorCtx context);

/**
 * Returns a const pointer to the last element in the vector.
 * 
 * If `CSTL_vector_empty(context) == true` or if an incompatible
 * `CSTL_Type` is used the behavior is undefined.
 * 
 */
const void* CSTL_vector_const_back(CSTL_VectorCtx context);

/**
 * Returns a pointer to the underlying storage.
 * 
 * The returned pointer is valid even if the vector is empty,
 * in which case it is not dereferenceable.
 * 
 */
void* CSTL_vector_data(CSTL_VectorCtx context);

/**
 * Returns a pointer to the underlying storage.
 * 
 * The returned pointer is valid even if the vector is empty,
 * in which case it is not dereferenceable.
 * 
 */
const void* CSTL_vector_const_data(CSTL_VectorCtx context);

/**
 * Construct an iterator to the first element of the vector.
 * 
 * If the vector is empty: `CSTL_vector_iterator_eq(begin, end) == true`.
 * 
 */
CSTL_VectorIter CSTL_vector_begin(CSTL_VectorCtx context);

/**
 * Construct a const iterator to the first element of the vector.
 * 
 * If the vector is empty: `CSTL_vector_const_iterator_eq(begin, end) == true`.
 * 
 */
CSTL_VectorIter CSTL_vector_end(CSTL_VectorCtx context);

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
 * They must belong to the same vector and have compatible `CSTL_Type`s.
 * 
 */
ptrdiff_t CSTL_vector_iterator_distance(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

/**
 * Compares iterators for equality.
 * 
 * They must belong to the same vector and have compatible `CSTL_Type`s.
 * 
 */
bool CSTL_vector_iterator_eq(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

/**
 * Compares iterators for less than.
 * 
 * They must belong to the same vector and have compatible `CSTL_Type`s.
 * 
 */
bool CSTL_vector_iterator_lt(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

/**
 * Returns `true` if the vector is empty or `false` otherwise.
 * 
 */
bool CSTL_vector_empty(CSTL_VectorCtx context);

/**
 * Returns the number of elements in the vector.
 * 
 */
size_t CSTL_vector_size(CSTL_VectorCtx context);

/**
 * Returns the total element capacity of the vector.
 * 
 */
size_t CSTL_vector_capacity(CSTL_VectorCtx context);

/**
 * Returns the maximum possible number of elements in the vector.
 * 
 * As if by `PTRDIFF_MAX / type->size`.
 * 
 */
size_t CSTL_vector_max_size(CSTL_TypeCRef type);

/**
 * If `new_capacity > CSTL_vector_capacity(context)`, reallocates and expands
 * the vector storage.
 * 
 * If `new_capacity` exceeds `CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_reserve(CSTL_VectorCtx context, size_t new_capacity, CSTL_Alloc* alloc);

/**
 * Request removal of unused capacity.
 * 
 */
void CSTL_vector_shrink_to_fit(CSTL_VectorCtx context, CSTL_Alloc* alloc);

/**
 * Erase all elements from the vector without affecting capacity.
 * 
 */
void CSTL_vector_clear(CSTL_VectorCtx context);

/**
 * Inserts `count` copies of `value` into the vector before `where` and returns
 * an iterator to the first newly inserted element.
 * 
 * If `count > CSTL_vector_max_size(context.type) - CSTL_vector_size(context)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(context)`.
 * 
 */
CSTL_VectorIter CSTL_vector_insert(CSTL_VectorCtx context, CSTL_VectorIter where, size_t count, const void* value, CSTL_Alloc* alloc);

/**
 * Inserts a copy of `value` into the vector before `where` and returns
 * an iterator to the newly inserted element.
 * 
 * If `CSTL_vector_size(context) == CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(context)`.
 * 
 */
CSTL_VectorIter CSTL_vector_emplace(CSTL_VectorCtx context, CSTL_VectorIter where, void* value, CSTL_Alloc* alloc);

/**
 * Inserts a copy of `value` into the vector before `where` and returns
 * an iterator to the newly inserted element.
 * 
 * If `CSTL_vector_size(context) == CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(context)`.
 * 
 */
CSTL_VectorIter CSTL_vector_emplace_const(CSTL_VectorCtx context, CSTL_VectorIter where, const void* value, CSTL_Alloc* alloc);

/**
 * Removes the element at `where` and returns an iterator following the
 * removed element.
 * 
 * The iterator `where` must be valid and dereferenceable.
 * 
 */
CSTL_VectorIter CSTL_vector_erase(CSTL_VectorCtx context, CSTL_VectorIter where);

/**
 * Removes the element in the range `[first, last)` and returns an iterator following the
 * removed elements.
 * 
 * If `first == last`, no operation is performed.
 * 
 */
CSTL_VectorIter CSTL_vector_erase_range(CSTL_VectorCtx context, CSTL_VectorIter first, CSTL_VectorIter last);

/**
 * Appends a copy of `value` to the end of the vector and returns
 * an iterator to the newly inserted element.
 * 
 * If `CSTL_vector_size(context) == CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(instance, type)`.
 * 
 */
CSTL_VectorIter CSTL_vector_emplace_back(CSTL_VectorCtx context, void* value, CSTL_Alloc* alloc);

/**
 * Appends a copy of `value` to the end of the vector and returns
 * an iterator to the newly inserted element.
 * 
 * If `CSTL_vector_size(context) == CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `CSTL_vector_end(instance, type)`.
 * 
 */
CSTL_VectorIter CSTL_vector_emplace_back_const(CSTL_VectorCtx context, const void* value, CSTL_Alloc* alloc);

/**
 * Appends a copy of `value` to the end of the vector.
 * 
 * If `CSTL_vector_size(context) == CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_push(CSTL_VectorCtx context, void* value, CSTL_Alloc* alloc);

/**
 * Appends a copy of `value` to the end of the vector.
 * 
 * If `CSTL_vector_size(context) == CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_push_const(CSTL_VectorCtx context, const void* value, CSTL_Alloc* alloc);

/**
 * Removes the last element from the vector.
 * 
 * If `CSTL_vector_empty(context) == true` or if an incompatible
 * `CSTL_Type` is used the behavior is undefined.
 * 
 */
void CSTL_vector_pop(CSTL_VectorCtx context);

/**
 * Resizes the vector to contain `new_size` elements.
 * 
 * If `new_size > CSTL_vector_max_size(context.type)` (vector too long)
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_vector_resize(CSTL_VectorCtx context, size_t new_size, const void* value, CSTL_Alloc* alloc);

#if defined(__cplusplus)
}
#endif

#endif
