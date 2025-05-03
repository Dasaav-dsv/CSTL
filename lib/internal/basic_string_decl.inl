#include "basic_string.h"

#include "alloc.h"

#if defined(__cplusplus)
#include <cstddef>
#include <cstdint>
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

#ifndef CSTL_string_type
#define CSTL_string_type
#endif

#ifndef CSTL_string_prefix
#define CSTL_string_prefix
#endif

#ifndef CSTL_char_t
#define CSTL_char_t char
#endif

// length of internal buffer, [1, 16]
#define CSTL_string_bufsize (16 / sizeof(CSTL_char_t) < 1 ? 1 : 16 / sizeof(CSTL_char_t))
// roundup mask for allocated buffers, [0, 15]
#define CSTL_string_alloc_mask (sizeof(CSTL_char_t) <= 1 ? 15 \
                              : sizeof(CSTL_char_t) <= 2 ? 7  \
                              : sizeof(CSTL_char_t) <= 4 ? 3  \
                              : sizeof(CSTL_char_t) <= 8 ? 1  \
                                                         : 0)
// capacity in small mode
#define CSTL_string_small_res (CSTL_string_bufsize - 1)

/**
 * STL ABI `std::basic_string` layout.
 * 
 * Does not include the allocator, which nonetheless is a part of the `std::basic_string`
 * structure! You are responsible for including it, since it can take on any form.
 * 
 * Do not manipulate the members directly, use the associated functions!
 * 
 */
typedef struct CSTL_String(Val) {
    union {
        CSTL_char_t buf[CSTL_string_bufsize];
        CSTL_char_t* ptr;
    } bx; // why is it called `_Bx`?
    size_t size;
    size_t res;
} CSTL_String(Val);

/**
 * Reference to a mutable `CSTL_*StringVal`.
 * 
 * Must not be null.
 * 
 */
typedef CSTL_String(Val)* CSTL_String(Ref);

/**
 * Reference to a const `CSTL_*StringVal`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_String(Val)* CSTL_String(CRef);

/**
 * Initializes the string, but does not allocate any memory.
 * 
 * An initialized string can be trivially destroyed without leaks as long
 * as its owned string is small enough to be inline (smaller than CSTL_string_alloc_mask). 
 * 
 * Re-initializing a string with a backing memory allocation will leak the old
 * memory allocation.
 * 
 */
void CSTL_string_(construct)(CSTL_String(Val)* new_instance);

/**
 * Destroys the string, freeing the backing storage if necessary.
 * 
 */
void CSTL_string_(destroy)(CSTL_String(Ref) instance, CSTL_Alloc* alloc);

void CSTL_string_(assign)(CSTL_String(Ref) instance, CSTL_char_t* str, CSTL_Alloc* alloc);

void CSTL_string_(assign_n)(CSTL_String(Ref) instance, CSTL_char_t* str, size_t count, CSTL_Alloc* alloc);

bool CSTL_string_(assign_char)(CSTL_String(Ref) instance, size_t new_size, CSTL_char_t value, CSTL_Alloc* alloc);

bool CSTL_string_(assign_substr)(CSTL_String(Ref) instance, CSTL_String(Ref) other, size_t pos, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the contents of `other_instance`.
 * 
 * If `propagate_alloc == true && alloc != other_alloc` then storage 
 * is freed with `alloc` and allocated again with `other_alloc` before contents
 * are copied. Then, `instance` uses `other_alloc` as its allocator.
 * 
 * If `propagate_alloc == false` `instance` keeps using `alloc` as its allocator,
 * potentially reusing its storage.
 * 
 * You are responsible for replacing the allocator outside of `CSTL_*StringVal` if applicable.
 * 
 */
void CSTL_string_(copy_assign)(CSTL_String(Ref) instance, CSTL_Alloc* alloc, CSTL_String(CRef) other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Moves the contents of `other_instance` to the contents of `instance`.
 * 
 * If `propagate_alloc == true` storage is replaced with storage of `other_instance`.
 * Then, `instance` uses `other_alloc` as its allocator.
 * 
 * If `propagate_alloc == false && alloc != other_alloc` then storage is reused
 * and individual characters of `other` are moved in. Then, `instance` uses `alloc` as its allocator.
 * 
 * You are responsible for replacing the allocator outside of `CSTL_*StringVal` if applicable.
 * 
 */
void CSTL_string_(move_assign)(CSTL_String(Ref) instance, CSTL_Alloc* alloc, CSTL_String(Ref) other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Swaps string contents.
 * 
 * You are responsible for swapping the allocators.
 * 
 */
void CSTL_string_(swap)(CSTL_String(Ref) instance, CSTL_String(Ref) other_instance);

/**
 * Returns a pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_*string_length(instance)` the behavior is undefined.
 * 
 */
CSTL_char_t* CSTL_string_(index)(CSTL_String(Ref) instance, size_t pos);

/**
 * Returns a const pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_*string_length(instance)` the behavior is undefined.
 * 
 */
const CSTL_char_t* CSTL_string_(const_index)(CSTL_String(CRef) instance, size_t pos);

/**
 * Returns a pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_*string_length(instance)` a null pointer is returned.
 * 
 */
CSTL_char_t* CSTL_string_(get_at)(CSTL_String(Ref) instance, size_t pos);

/**
 * Returns a const pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_*string_length(instance)` a null pointer is returned.
 * 
 */
const CSTL_char_t* CSTL_string_(const_get_at)(CSTL_String(CRef) instance, size_t pos);

/**
 * Returns a pointer to the first character.
 * 
 * If `CSTL_*string_empty(instance) == true` the behavior is undefined.
 * 
 */
CSTL_char_t* CSTL_string_(front)(CSTL_String(Ref) instance);

/**
 * Returns a const pointer to the first character.
 * 
 * If `CSTL_*string_empty(instance) == true` the behavior is undefined.
 * 
 */
const CSTL_char_t* CSTL_string_(const_front)(CSTL_String(CRef) instance);

/**
 * Returns a pointer to the last character.
 * 
 * If `CSTL_*string_empty(instance) == true` the behavior is undefined.
 * 
 */
CSTL_char_t* CSTL_string_(back)(CSTL_String(Ref) instance);

/**
 * Returns a const pointer to the last character.
 * 
 * If `CSTL_*string_empty(instance) == true` the behavior is undefined.
 * 
 */
const CSTL_char_t* CSTL_string_(const_back)(CSTL_String(CRef) instance);

/**
 * Returns a pointer to the underlying null-terminated array
 * serving as character storage.
 * 
 * If `CSTL_*string_empty(instance) == true` the pointer is still
 * valid and points to a single null character.
 * 
 * The range `[CSTL_*string_data(instance), CSTL_*string_data(instance) + size]`
 * is always valid.
 * 
 * The array may be mutated through the returned pointer excluding
 * the past-the-end null terminator.
 * 
 */
CSTL_char_t* CSTL_string_(data)(CSTL_String(Ref) instance);

/**
 * Returns a const pointer to the underlying null-terminated array
 * serving as character storage.
 * 
 * If `CSTL_*string_empty(instance) == true` the pointer is still
 * valid and points to a single null character.
 * 
 * The range `[CSTL_*string_c_str(instance), CSTL_*string_c_str(instance) + size]`
 * is always valid.
 * 
 */
const CSTL_char_t* CSTL_string_(c_str)(CSTL_String(CRef) instance);

/**
 * Returns an iterator (pointer) to the first character of the string.
 * 
 * If `CSTL_*string_empty(instance) == true` then
 * `CSTL_*string_begin(instance) == CSTL_*string_end(instance)`.
 * 
 */
CSTL_char_t* CSTL_string_(begin)(CSTL_String(Ref) instance);

/**
 * Returns an iterator (pointer) past the last character of the string.
 * 
 * If `CSTL_*string_empty(instance) == true` then
 * `CSTL_*string_begin(instance) == CSTL_*string_end(instance)`.
 * 
 */
const CSTL_char_t* CSTL_string_(const_begin)(CSTL_String(CRef) instance);

/**
 * Returns an iterator (pointer) past the last character of the string.
 * 
 * If `CSTL_*string_empty(instance) == true` then
 * `CSTL_*string_begin(instance) == CSTL_*string_end(instance)`.
 * 
 */
CSTL_char_t* CSTL_string_(end)(CSTL_String(Ref) instance);

/**
 * Returns a const iterator (pointer) past the last character of the string.
 * 
 * If `CSTL_*string_empty(instance) == true` then
 * `CSTL_*string_const_begin(instance) == CSTL_*string_const_end(instance)`.
 * 
 */
const CSTL_char_t* CSTL_string_(const_end)(CSTL_String(CRef) instance);

/**
 * Returns `true` if the string is empty or `false` otherwise.
 * 
 */
bool CSTL_string_(empty)(CSTL_String(CRef) instance);

/**
 * Returns the number of elements in the string.
 * 
 * Elements are not the same as characters in multibyte strings.
 * 
 */
size_t CSTL_string_(size)(CSTL_String(CRef) instance);

/**
 * Returns the number of elements in the string.
 * 
 * Elements are not the same as characters in multibyte strings.
 * 
 */
size_t CSTL_string_(length)(CSTL_String(CRef) instance);

/**
 * Returns the total element capacity of the string.
 * 
 * Elements are not the same as characters in multibyte strings.
 * 
 */
size_t CSTL_string_(capacity)(CSTL_String(CRef) instance);

/**
 * Returns the maximum possible number of elements in the string.
 * 
 * Elements are not the same as characters in multibyte strings.
 * 
 */
size_t CSTL_string_(max_size)();
