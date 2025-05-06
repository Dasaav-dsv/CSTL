#pragma once

#ifndef CSTL_TYPE_H
#define CSTL_TYPE_H

#if defined(__cplusplus)
#include <cstddef>
#include <cstdint>
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

/**
 * Basic type info used in place of a type template parameter.
 * 
 * Two `CSTL_Type` instances are compatible iff their size and alignment
 * are equal and the bound functions can operate interchangeably.
 * 
 * It is a logic error to use incompatible `CSTL_Type` instances one
 * after another when manipulating an object.
 * 
 */
typedef struct CSTL_Type {
    /**
     * Size of the type in bytes, including any padding bytes.
     * 
     * Must be a non-zero multiple of `1 << align_lg`.
     * 
     */
    size_t size;

    /**
     * Fixed point reciprocal of the type's size, used for
     * fast division and remainder operations.
     * 
     * Calculated automatically by `CSTL_define_*_type`. 
     * 
     */
    size_t size_rcp;

    /**
     * Right shift of the fixed point reciprocal of the type's size,
     * used for fast division and remainder operations.
     * 
     * Calculated automatically by `CSTL_define_*_type`. 
     * 
     */
    uint8_t size_rcp_sh;

    /**
     * Natural alignment of the type in bytes (log2).
     * 
     * Calculated automatically by `CSTL_define_*_type`.
     * 
     */
    uint8_t align_lg;

    /**
     * Determines whether `move_from` can be called on a `void*` pointer
     * to an object of this type.
     * 
     * Set by `CSTL_define_*_type` or manually.
     * 
     */
    uint8_t use_move_from;

    /**
     * Opaque bitfield with unstable ABI.
     * 
     */
    uint8_t internal_flags;

    /**
     * Bound copy constructor function.
     * 
     * It is NOT permitted to mutate the `source`.
     * 
     * If null a `memmove(new_instance, src, size)` will be used in its stead.
     * 
     */
    void (*copy_from)(void* new_instance, const void* src);

    /**
     * Bound move constructor function.
     * 
     * It is permitted to mutate the `source`, therefore it is suitable
     * for a move constructor.
     * 
     * Remember that the moved-from object must stay in a valid state
     * and be able to be destroyed after this call.
     * 
     * If null a `memmove(new_instance, src, size)` will be used in its stead.
     * 
     */
    void (*move_from)(void* new_instance, void* src);

    /**
     * Bound destructor function.
     * 
     * If null a `memset(instance, 0xDE, size)` will be used in its stead.
     * 
     */
    void (*destroy)(void* instance);
} CSTL_Type;

/**
 * Reference to a mutable `CSTL_TypeRef`.
 * 
 * Must not be null.
 * 
 */
typedef CSTL_Type* CSTL_TypeRef;

/**
 * Reference to a const `CSTL_TypeRef`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_Type* CSTL_TypeCRef;

/**
 * Type info used in place of a template parameter for a type that can be compared.
 * 
 * The type must establish an equivalence relation:
 * `is_eq(a, b) == true` => `is_eq(b, a) == true`,
 * `is_eq(a, b) == true` && `is_eq(b, c) == true` => `is_eq(a, c) == true`,
 * 
 * Base `CSTL_Type` behavior and size and alignment requirements are unchanged.
 * 
 */
typedef struct CSTL_EqType {
    /**
     * The underlying type info.
     * 
     */
    CSTL_Type base;

    /**
     * Bound "equality" binary function.
     * 
     * Must establish an equivalence relation.
     * 
     * If null a `memcmp(lhs, rhs, size) == 0` will be used in its stead.
     * 
     */
    bool (*is_eq)(const void* lhs, const void* rhs);
} CSTL_EqType;

/**
 * Reference to a mutable `CSTL_EqType`.
 * 
 * Must not be null.
 * 
 */
typedef CSTL_EqType* CSTL_EqTypeRef;

/**
 * Reference to a const `CSTL_EqType`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_EqType* CSTL_EqTypeCRef;

/**
 * Type info used in place of a template parameter for a type that can be compared.
 * 
 * The type must establish at least a strict weak ordering:
 * `is_lt(a, b) == true` => `is_lt(b, a) == false`
 * `is_lt(a, b) == true` && `is_lt(b, c) == true` => `is_lt(a, c) == true`
 * 
 * Base `CSTL_EqType` behavior and size and alignment requirements are unchanged.
 * 
 */
typedef struct CSTL_CompType {
    /**
     * The underlying type info.
     * 
     */
    CSTL_EqType base;

    /**
     * Bound "less than" binary function.
     * 
     * Must establish a strict weak ordering relation.
     * 
     * If null a `memcmp(lhs, rhs, size) < 0` will be used in its stead.
     * 
     */
    bool (*is_lt)(const void* lhs, const void* rhs);
} CSTL_CompType;

/**
 * Reference to a mutable `CSTL_CompType`.
 * 
 * Must not be null.
 * 
 */
typedef CSTL_CompType* CSTL_CompTypeRef;

/**
 * Reference to a const `CSTL_CompType`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_CompType* CSTL_CompTypeCRef;

/**
 * Type info used in place of a template parameter for a type that can be hashed.
 * 
 * The relation `is_eq(a, b)` => `hash(a) == hash(b)` must hold.
 * 
 * Base `CSTL_EqType` behavior and size and alignment requirements are unchanged.
 * 
 */
typedef struct CSTL_HashType {
    /**
     * The underlying type info.
     * 
     */
    CSTL_EqType base;
    
    /**
     * Bound hash function.
     * 
     * If `is_eq(a, b)` then `hash(a) == hash(b)`.
     * 
     * If null an STL `std::hash` conforming implementation will be used in its stead.
     * 
     */
    size_t (*hash)(const void* instance);
} CSTL_HashType;

/**
 * Reference to a mutable `CSTL_HashType`.
 * 
 * Must not be null.
 * 
 */
typedef CSTL_HashType* CSTL_HashTypeRef;

/**
 * Reference to a const `CSTL_HashType`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_HashType* CSTL_HashTypeCRef;

/**
 * All possible return values of `CSTL_define_*_type`.
 * 
 */
typedef enum CSTL_TypeErr {
    /**
     * Other error, zero initialized state of `CSTL_TypeErr`.
     * 
     */
    CSTL_TypeErr_Other,

    /**
     * No error.
     * 
     */
    CSTL_TypeErr_Ok,

    /**
     * Improper size of type.
     * 
     * The size of a type must be a non-zero multiple of its alignment.
     * 
     * Cstl also imposes a maximum size `SIZE_MAX / 2` for optimization reasons.
     * 
     * It follows from the alignment requirement that an alignment of `SIZE_MAX / 2 + 1`
     * is impossible, and `size + alignment` cannot overflow.
     * 
     */
    CSTL_TypeErr_BadSize,

    /**
     * Improper alignment of type.
     * 
     * Types must have a non-zero power of 2 alignment.
     * 
     */
    CSTL_TypeErr_Misaligned,

    /**
     * Pointer passed as `new_instance` was null.
     * 
     */
    CSTL_TypeErr_NullPointer,
} CSTL_TypeErr;

/**
 * Define a type, writing it to `new_instance` and validating parameters.
 * 
 * `copy_from`, `move_from`, `destroy` may be null, in which case their default
 * behavior is invoked. See descriptions of these fields in `CSTL_Type`.
 * 
 * If `use_copy_from == false` and `use_move_from == false` most container
 * relocation functions will fail.
 * 
 */
CSTL_TypeErr CSTL_define_type(
    CSTL_Type* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance)
);

/**
 * Define a type, writing it to `new_instance` and validating parameters.
 * 
 * `copy_from`, `move_from`, `destroy`, `is_eq` may be null, in which case their default
 * behavior is invoked. See descriptions of these fields in `CSTL_Type`.
 * 
 * If `use_copy_from == false` and `use_move_from == false` most container
 * relocation functions will fail.
 * 
 */
CSTL_TypeErr CSTL_define_eq_type(
    CSTL_EqType* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance),
    bool (*is_eq)(const void* lhs, const void* rhs)
);

/**
 * Define a type, writing it to `new_instance` and validating parameters.
 * 
 * `copy_from`, `move_from`, `destroy`, `is_eq`, `is_lt` may be null,
 * in which case their default behavior is invoked. See descriptions
 * of these fields in `CSTL_Type`.
 * 
 */
CSTL_TypeErr CSTL_define_comp_type(
    CSTL_CompType* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance),
    bool (*is_eq)(const void* lhs, const void* rhs),
    bool (*is_lt)(const void* lhs, const void* rhs)
);

/**
 * Define a type, writing it to `new_instance` and validating parameters.
 * 
 * `copy_from`, `move_from`, `destroy`, `is_eq`, `hash` may be null,
 * in which case their default behavior is invoked. See descriptions
 * of these fields in `CSTL_Type`.
 * 
 */
CSTL_TypeErr CSTL_define_hash_type(
    CSTL_HashType* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance),
    bool (*is_eq)(const void* lhs, const void* rhs),
    size_t (*hash)(const void* instance)
);

#if defined(__cplusplus)
}
#endif

#endif
