#pragma once

#ifndef CSTL_TYPE_H
#define CSTL_TYPE_H

#if defined(__cplusplus)
#include <cstddef>
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#endif

/**
 * Opaque pseudohandle to the size and alignment of a C type.
 * 
 * Defined with `CSTL_define_type`, it is valid if not equal to `NULL`.
 * 
 * The size of the type must be less than or equal to `INTPTR_MAX`.
 * 
 */
typedef struct CSTL_SType* CSTL_Type;

/**
 * Destroy range.
 * 
 * Required to destroy objects in the range `[first, last)`.
 * 
 * After this call, all objects in the range `[first, last)` are
 * treated as uninitialized memory.
 * 
 */
typedef void (*CSTL_Drop)(void* first, void* last);

/**
 * Move range.
 * 
 * Required to write `last - first` objects to uninitialized memory at
 * `dest` so that each of them is equal to the corresponding object in
 * the range `[first, last)` before the operation.
 * 
 * After this call, all objects in the range `[first, last)` are
 * treated as valid objects in an unspecified state.
 * 
 */
typedef void (*CSTL_Move)(void* first, void* last, void* dest);

/**
 * Copy range.
 * 
 * Required to write `(last - first) / size` objects to uninitialized
 * memory at `dest` so that each of them is equal to the corresponding
 * object in the range `[first, last)`.
 * 
 * Note that a `CSTL_Copy` function satisfies the requirements of
 * `CSTL_Move`.
 * 
 */
typedef void (*CSTL_Copy)(const void* first, const void* last, void* dest);

/**
 * Fill range.
 * 
 * Required to write `(last - first) / size` copies of `instance`
 * to uninitialized memory at `[first, last)` so that each of them
 * is equal to the corresponding `instance`.
 * 
 */
typedef void (*CSTL_Fill)(void* first, void* last, const void* value);

/**
 * Function table for a type that can be destroyed by calling `drop`
 * on a range of objects of that type.
 * 
 */
typedef struct CSTL_DropType {
    CSTL_Drop drop;
} CSTL_DropType;

/**
 * Function table for a type that can be moved by calling `move`
 * on a range of objects of that type to another range, or destroyed.
 * 
 */
typedef struct CSTL_MoveType {
    CSTL_DropType drop_type;
    CSTL_Move move;
} CSTL_MoveType;

/**
 * Function table for a type that can be copied by calling `copy`
 * on a range of objects of that type to another range, or moved, or destroyed.
 * 
 * It is valid for `copy` to bind the same function as `move_type.move`.
 * 
 */
typedef struct CSTL_CopyType {
    CSTL_MoveType move_type;
    CSTL_Copy copy;
    CSTL_Fill fill;
} CSTL_CopyType;

/**
 * Reference to a const `CSTL_DropType`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_DropType* CSTL_DropTypeCRef;

/**
 * Reference to a const `CSTL_MoveType`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_MoveType* CSTL_MoveTypeCRef;

/**
 * Reference to a const `CSTL_CopyType`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_CopyType* CSTL_CopyTypeCRef;

/**
 * Compare two objects for equality.
 * 
 * The function must establish an equivalence relation:
 * `a == b` => `b == a`, `a == b` && `b == c` => `a == c`
 * 
 */
typedef bool (*CSTL_IsEq)(const void* lhs, const void* rhs);

/**
 * Compare two objects for less than.
 * 
 * The function must establish at least a strict weak ordering:
 * `a < b` => `b < a == false`
 * `a < b` && `b < c` => `a < c`
 * 
 */
typedef bool (*CSTL_IsLt)(const void* lhs, const void* rhs);

/**
 * Function table for a type instances of which can be ordered
 * with respect to others in a string weak ordering.
 * 
 */
typedef struct CSTL_CompType {
    CSTL_IsEq is_eq;
    CSTL_IsLt is_lt;
} CSTL_CompType;

/**
 * Obtain a hash for an instance of an object.
 * 
 * The relation `a == b` => `hash(a) == hash(b)` must hold.
 * 
 */
typedef size_t (*CSTL_Hash)(const void* instance);

/**
 * Function table for a type instances of which can be hashed
 * and compared for equality.
 * 
 */
typedef struct CSTL_HashType {
    CSTL_IsEq is_eq;
    CSTL_Hash hash;
} CSTL_HashType;

/**
 * Obtain a pseudohandle to the size and alignment of a C type.
 * 
 * If the below requirements are broken, the function returns
 * an invalid handle equal to `NULL`:
 * 1. `size` must be a non-zero multiple of `alignment`.
 * 2. `alignment` must be a power of 2.
 * 3. `size` must be less than or equal to `INTPTR_MAX`.
 * 
 */
CSTL_Type CSTL_define_type(size_t size, size_t alignment);

/**
 * Get the size of a C type back from a packed pseudohandle representation.
 * 
 */
size_t CSTL_sizeof_type(CSTL_Type type);

/**
 * Get the alignment of a C type back from a packed pseudohandle representation.
 * 
 */
size_t CSTL_alignof_type(CSTL_Type type);

#if defined(__cplusplus)
}
#endif

#endif
