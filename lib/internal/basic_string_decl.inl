#include "basic_string.h"

#include "alloc.h"

#if defined(__cplusplus)
#include <cstddef>
#include <cstdint>
#include <cuchar>
#ifndef __cpp_char8_t
typedef unsigned char char8_t;
#endif
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uchar.h>
typedef unsigned char char8_t;
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

#undef CSTL_string_bufsize
#undef CSTL_string_alloc_mask
#undef CSTL_string_small_capacity
#undef CSTL_string_npos

// length of internal buffer, [1, 16]
#define CSTL_string_bufsize (16 / sizeof(CSTL_char_t) < 1 ? 1 : 16 / sizeof(CSTL_char_t))
// roundup mask for allocated buffers, [0, 15]
#define CSTL_string_alloc_mask (sizeof(CSTL_char_t) <= 1 ? 15 \
                              : sizeof(CSTL_char_t) <= 2 ? 7  \
                              : sizeof(CSTL_char_t) <= 4 ? 3  \
                              : sizeof(CSTL_char_t) <= 8 ? 1  \
                                                         : 0)
// capacity in small mode
#define CSTL_string_small_capacity (CSTL_string_bufsize - 1)
// `std::basic_string` npos constant
#define CSTL_string_npos (size_t)-1;

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

/**
 * Initializes the string with the substring at offset `other_off`
 * in `other` with the length given by `count`.
 * 
 * If `new_instance == NULL` or if `other_off` is outside of the range
 * `[other, other + CSTL_*string_size(other)]` returns `false` and does nothing,
 * otherwise it returns `true`.
 * 
 * If `new_instance == other` the substring operation is performed in-place without
 * de-initializing `other`.
 * 
 * Remember that re-initializing a different string with a backing memory allocation
 * will leak the old memory allocation.
 * 
 */
bool CSTL_string_(substr)(CSTL_String(Val)* new_instance, CSTL_String(CRef) other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the null-terminated string at `ptr`.
 * 
 * If the length of the string at `ptr` is greater than `CSTL_*string_max_size()` this function
 * has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(assign)(CSTL_String(Ref) instance, const CSTL_char_t* ptr, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the first `count` characters of the string at `ptr`.
 * 
 * If `n` is greater than `CSTL_*string_max_size()` this function has no effect
 * and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(assign_n)(CSTL_String(Ref) instance, const CSTL_char_t* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the `count` copies of the character `ch`.
 * 
 * If `count` is greater than `CSTL_*string_max_size()` this function has no effect
 * and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(assign_char)(CSTL_String(Ref) instance, size_t count, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the substring at offset `other_off` in `other`
 * with the length given by `count`.
 * 
 * If `other_off` is outside of the range `[other, other + CSTL_*string_size(other)]` returns
 * `false` and does nothing, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(assign_substr)(CSTL_String(Ref) instance, CSTL_String(CRef) other, size_t other_off, size_t count, CSTL_Alloc* alloc);

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
CSTL_char_t* CSTL_string_(at)(CSTL_String(Ref) instance, size_t pos);

/**
 * Returns a const pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_*string_length(instance)` a null pointer is returned.
 * 
 */
const CSTL_char_t* CSTL_string_(const_at)(CSTL_String(CRef) instance, size_t pos);

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
 * Returns the number of characters in the string.
 * 
 */
size_t CSTL_string_(size)(CSTL_String(CRef) instance);

/**
 * Returns the number of characters in the string.
 * 
 */
size_t CSTL_string_(length)(CSTL_String(CRef) instance);

/**
 * Returns the total characters capacity of the string.
 * 
 */
size_t CSTL_string_(capacity)(CSTL_String(CRef) instance);

/**
 * Returns the maximum possible number of characters in the string.
 * 
 */
size_t CSTL_string_(max_size)();

/**
 * If `new_capacity > CSTL_*string_capacity(instance)`, reallocates and expands
 * the underlying array storage.
 * 
 * If `new_capacity` exceeds `CSTL_*string_max_size()` this function has no effect
 * and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(reserve)(CSTL_String(Ref) instance, size_t new_capacity, CSTL_Alloc* alloc);

/**
 * Request removal of unused capacity.
 * 
 */
void CSTL_string_(shrink_to_fit)(CSTL_String(Ref) instance, CSTL_Alloc* alloc);

/**
 * Erase all characters from the string without affecting capacity.
 * 
 */
void CSTL_string_(clear)(CSTL_String(Ref) instance);

/**
 * Inserts the null-terminated string at `ptr` at the pointer `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
CSTL_char_t* CSTL_string_(insert)(CSTL_String(Ref) instance, const CSTL_char_t* where, const CSTL_char_t* ptr, CSTL_Alloc* alloc);

/**
 * Inserts the first `count` characters of the string at `ptr` at the pointer  `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
CSTL_char_t* CSTL_string_(insert_n)(CSTL_String(Ref) instance, const CSTL_char_t* where, const CSTL_char_t* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Inserts `count` copies of the character `ch` at the pointer `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
CSTL_char_t* CSTL_string_(insert_char)(CSTL_String(Ref) instance, const CSTL_char_t* where, size_t count, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Inserts the string `other` at the pointer `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
CSTL_char_t* CSTL_string_(insert_str)(CSTL_String(Ref) instance, const CSTL_char_t* where, CSTL_String(CRef) other, CSTL_Alloc* alloc);

/**
 * Inserts the substring at offset `other_off` in `other` with the length
 * given by `count` at the pointer `where` in `instance`.
 * 
 * If `other_off` is outside of the range `[other, other + CSTL_*string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
CSTL_char_t* CSTL_string_(insert_substr)(CSTL_String(Ref) instance, const CSTL_char_t* where, CSTL_String(CRef) other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Inserts the null-terminated string at `ptr` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(insert_at)(CSTL_String(Ref) instance, size_t off, const CSTL_char_t* ptr, CSTL_Alloc* alloc);

/**
 * Inserts the first `count` characters of the string at `ptr` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(insert_n_at)(CSTL_String(Ref) instance, size_t off, const CSTL_char_t* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Inserts `count` copies of the character `ch` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(insert_char_at)(CSTL_String(Ref) instance, size_t off, size_t count, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Inserts the string `other` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(insert_str_at)(CSTL_String(Ref) instance, size_t off, CSTL_String(CRef) other, CSTL_Alloc* alloc);

/**
 * Inserts the substring at offset `other_off` in `other` with the length
 * given by `count` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if `other_off` is outside of the range `[other, other + CSTL_*string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(insert_substr_at)(CSTL_String(Ref) instance, size_t off, CSTL_String(CRef) other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Removes the character at `where` and returns a pointer following the
 * removed character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
CSTL_char_t* CSTL_string_(erase)(CSTL_String(Ref) instance, const CSTL_char_t* where);

/**
 * Removes the characters in the range `[first, last)` and returns a pointer following the
 * removed character.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
CSTL_char_t* CSTL_string_(erase_substr)(CSTL_String(Ref) instance, const CSTL_char_t* first, const CSTL_char_t* last);

/**
 * Removes the character at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(erase_at)(CSTL_String(Ref) instance, size_t off);

/**
 * Removes the substring at offset `off` in `instance` with the length given by `count`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(erase_substr_at)(CSTL_String(Ref) instance, size_t off, size_t count);

/**
 * Appends the character `ch` to the end of the string.
 * 
 * If `CSTL_*string_size(instance) == CSTL_*string_max_size()` this function
 * has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(push_back)(CSTL_String(Ref) instance, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Appends the character `ch` to the end of the string.
 * 
 * If `CSTL_*string_size(instance) == CSTL_*string_max_size()` this function
 * has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
void CSTL_string_(pop_back)(CSTL_String(Ref) instance);

/**
 * Appends the null-terminated string at `ptr` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(append)(CSTL_String(Ref) instance, const CSTL_char_t* ptr, CSTL_Alloc* alloc);

/**
 * Appends the first `count` characters of the string at `ptr` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(append_n)(CSTL_String(Ref) instance, const CSTL_char_t* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Appends `count` copies of the character `ch` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(append_char)(CSTL_String(Ref) instance, size_t count, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Appends the string `other` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(append_str)(CSTL_String(Ref) instance, CSTL_String(CRef) other, CSTL_Alloc* alloc);

/**
 * Appends the substring at offset `other_off` in `other` with the length
 * given by `count` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(append_substr)(CSTL_String(Ref) instance, CSTL_String(CRef) other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the null-terminated
 * string at `ptr`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_(replace)(CSTL_String(Ref) instance, const CSTL_char_t* first, const CSTL_char_t* last, const CSTL_char_t* ptr, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the first `count`
 * characters of the string at `ptr`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_(replace_n)(CSTL_String(Ref) instance, const CSTL_char_t* first, const CSTL_char_t* last, const CSTL_char_t* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with `count` copies
 * of the character `ch`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_(replace_char)(CSTL_String(Ref) instance, const CSTL_char_t* first, const CSTL_char_t* last, size_t count, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the string `other`.
 * 
 * If the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_(replace_str)(CSTL_String(Ref) instance, const CSTL_char_t* first, const CSTL_char_t* last, CSTL_String(CRef) other, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the substring
 * at offset `other_off` in `other` with the length given by `count`.
 * 
 * If `other_off` is outside of the range `[other, other + CSTL_*string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_(replace_substr)(CSTL_String(Ref) instance, const CSTL_char_t* first, const CSTL_char_t* last, CSTL_String(CRef) other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the null-terminated string at `ptr`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(replace_at)(CSTL_String(Ref) instance, size_t off, size_t count, const CSTL_char_t* ptr, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the first `count2` characters of the string at `ptr`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(replace_n_at)(CSTL_String(Ref) instance, size_t off, size_t count, const CSTL_char_t* ptr, size_t count2, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with `count` copies of the character `ch`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(replace_char_at)(CSTL_String(Ref) instance, size_t off, size_t count, size_t count2, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the string `other`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(replace_str_at)(CSTL_String(Ref) instance, size_t off, size_t count, CSTL_String(CRef) other, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the substring at offset `other_off` in `other` with the length
 * given by `count2`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_*string_size(instance)]`
 * or if `other_off` is outside of the range `[other, other + CSTL_*string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_*string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(replace_substr_at)(CSTL_String(Ref) instance, size_t off, size_t count, CSTL_String(CRef) other, size_t other_off, size_t count2, CSTL_Alloc* alloc);

/**
 * Copies a substring `[off, off + count)` to character string pointed to by
 * `dest`. The resulting character string is not null terminated.
 * 
 * Returns the number of characters copied or `CSTL_string_npos` if
 * `off > CSTL_*string_size(instance)` (out of range).
 * 
 */
size_t CSTL_string_(copy)(CSTL_String(CRef) instance, CSTL_char_t* dest, size_t count, size_t off);

/**
 * Resizes the string to contain `new_size` characters, appending `ch`
 * if current size is less than `new_size`.
 * 
 * If `new_size` is greater than `CSTL_*string_max_size()` this
 * function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_(resize)(CSTL_String(Ref) instance, size_t new_size, CSTL_char_t ch, CSTL_Alloc* alloc);

/**
 * Find the first from offset `off` substring equal to the null-terminated string `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(find)(CSTL_String(CRef) instance, const CSTL_char_t* ptr, size_t off);

/**
 * Find the first from offset `off` substring equal to the first `count` characters at `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(find_n)(CSTL_String(CRef) instance, const CSTL_char_t* ptr, size_t off, size_t count);

/**
 * Find the first from offset `off` occurence of the character `ch`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(find_char)(CSTL_String(CRef) instance, CSTL_char_t ch, size_t off);

/**
 * Find the first from offset `off` substring equal to the string `other`
 * and return its position from the start of the original string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(find_str)(CSTL_String(CRef) instance, CSTL_String(CRef) other, size_t off);

/**
 * Find the last before offset `off` substring equal to the null-terminated string `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(rfind)(CSTL_String(CRef) instance, const CSTL_char_t* ptr, size_t off);

/**
 * Find the last before offset `off` substring equal to the first `count` characters at `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(rfind_n)(CSTL_String(CRef) instance, const CSTL_char_t* ptr, size_t off, size_t count);

/**
 * Find the last before offset `off` occurence of the character `ch`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(rfind_char)(CSTL_String(CRef) instance, CSTL_char_t ch, size_t off);

/**
 * Find the last before offset `off` substring equal to the string `other`
 * and return its position from the start of the original string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_(rfind_str)(CSTL_String(CRef) instance, CSTL_String(CRef) other, size_t off);

/**
 * Compare two null-terminated character sequences.
 * 
 * The return value is negative if `left` compares less than `right`,
 * positive if it compares greater and zero if `left` and `right`
 * compare equal.
 * 
 * There is no `CSTL_*String` version of this function as the return
 * value does not allow for reporting out of bounds errors.
 * 
 */
int CSTL_string_(compare)(const CSTL_char_t* left, const CSTL_char_t* right);

/**
 * Compare an explicit length character sequence with a null-terminated one.
 * 
 * The return value is negative if `left` compares less than `right`,
 * positive if it compares greater and zero if `left` and `right`
 * compare equal.
 * 
 * To compare an explicit length `left` and a null-terminated `right`,
 * swap them and negate the result: `-CSTL_*string_compare_n(right, left, left_count)`.
 * 
 * There is no `CSTL_*String` version of this function as the return
 * value does not allow for reporting out of bounds errors.
 * 
 */
int CSTL_string_(compare_n)(const CSTL_char_t* left, const CSTL_char_t* right, size_t right_count);

/**
 * Compare two explicit length character sequences.
 * 
 * The return value is negative if `left` compares less than `right`,
 * positive if it compares greater and zero if `left` and `right`
 * compare equal.
 * 
 * There is no `CSTL_*String` version of this function as the return
 * value does not allow for reporting out of bounds errors.
 * 
 */
int CSTL_string_(compare_nn)(const CSTL_char_t* left, size_t left_count, const CSTL_char_t* right, size_t right_count);
