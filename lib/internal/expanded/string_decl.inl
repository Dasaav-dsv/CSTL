#include "../../alloc.h"

#if defined(__cplusplus)
#include <cstddef>
#include <cstdint>
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

#undef CSTL_string_bufsize
#undef CSTL_string_alloc_mask
#undef CSTL_string_small_capacity
#undef CSTL_string_npos

// length of internal buffer, [1, 16]
#define CSTL_string_bufsize (16 / sizeof(char) < 1 ? 1 : 16 / sizeof(char))
// roundup mask for allocated buffers, [0, 15]
#define CSTL_string_alloc_mask (sizeof(char) <= 1 ? 15 \
                              : sizeof(char) <= 2 ? 7  \
                              : sizeof(char) <= 4 ? 3  \
                              : sizeof(char) <= 8 ? 1  \
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
typedef struct CSTL_StringVal {
    union CSTL_StringUnion {
        char buf[CSTL_string_bufsize];
        char* ptr;
    } bx; // why is it called `_Bx`?
    size_t size;
    size_t res;
} CSTL_StringVal;

/**
 * Reference to a mutable `CSTL_StringVal`.
 * 
 * Must not be null.
 * 
 */
typedef CSTL_StringVal* CSTL_StringRef;

/**
 * Reference to a const `CSTL_StringVal`.
 * 
 * Must not be null.
 * 
 */
typedef const CSTL_StringVal* CSTL_StringCRef;

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
void CSTL_string_construct(CSTL_StringVal* new_instance);

/**
 * Destroys the string, freeing the backing storage if necessary.
 * 
 */
void CSTL_string_destroy(CSTL_StringRef instance, CSTL_Alloc* alloc);

/**
 * Initializes the string with the substring at offset `other_off`
 * in `other` with the length given by `count`.
 * 
 * If `new_instance == NULL` or if `other_off` is outside of the range
 * `[other, other + CSTL_string_size(other)]` returns `false` and does nothing,
 * otherwise it returns `true`.
 * 
 * If `new_instance == other` the substring operation is performed in-place without
 * de-initializing `other`.
 * 
 * Remember that re-initializing a different string with a backing memory allocation
 * will leak the old memory allocation.
 * 
 */
bool CSTL_string_substr(CSTL_StringVal* new_instance, CSTL_StringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the null-terminated string at `ptr`.
 * 
 * If the length of the string at `ptr` is greater than `CSTL_string_max_size()` this function
 * has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_assign(CSTL_StringRef instance, const char* ptr, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the first `count` characters of the string at `ptr`.
 * 
 * If `n` is greater than `CSTL_string_max_size()` this function has no effect
 * and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_assign_n(CSTL_StringRef instance, const char* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the `count` copies of the character `ch`.
 * 
 * If `count` is greater than `CSTL_string_max_size()` this function has no effect
 * and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_assign_char(CSTL_StringRef instance, size_t count, char ch, CSTL_Alloc* alloc);

/**
 * Replaces the contents of `instance` with the substring at offset `other_off` in `other`
 * with the length given by `count`.
 * 
 * If `other_off` is outside of the range `[other, other + CSTL_string_size(other)]` returns
 * `false` and does nothing, otherwise it returns `true`.
 * 
 */
bool CSTL_string_assign_substr(CSTL_StringRef instance, CSTL_StringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc);

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
 * You are responsible for replacing the allocator outside of `CSTL_StringVal` if applicable.
 * 
 */
void CSTL_string_copy_assign(CSTL_StringRef instance, CSTL_Alloc* alloc, CSTL_StringCRef other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Moves the contents of `other_instance` to the contents of `instance`.
 * 
 * If `propagate_alloc == true` storage is replaced with storage of `other_instance`.
 * Then, `instance` uses `other_alloc` as its allocator.
 * 
 * If `propagate_alloc == false && alloc != other_alloc` then storage is reused
 * and individual characters of `other` are moved in. Then, `instance` uses `alloc` as its allocator.
 * 
 * You are responsible for replacing the allocator outside of `CSTL_StringVal` if applicable.
 * 
 */
void CSTL_string_move_assign(CSTL_StringRef instance, CSTL_Alloc* alloc, CSTL_StringRef other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc);

/**
 * Swaps string contents.
 * 
 * You are responsible for swapping the allocators.
 * 
 */
void CSTL_string_swap(CSTL_StringRef instance, CSTL_StringRef other_instance);

/**
 * Returns a pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_string_length(instance)` the behavior is undefined.
 * 
 */
char* CSTL_string_index(CSTL_StringRef instance, size_t pos);

/**
 * Returns a const pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_string_length(instance)` the behavior is undefined.
 * 
 */
const char* CSTL_string_const_index(CSTL_StringCRef instance, size_t pos);

/**
 * Returns a pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_string_length(instance)` a null pointer is returned.
 * 
 */
char* CSTL_string_at(CSTL_StringRef instance, size_t pos);

/**
 * Returns a const pointer to the character at `pos`.
 * 
 * If `pos >= CSTL_string_length(instance)` a null pointer is returned.
 * 
 */
const char* CSTL_string_const_at(CSTL_StringCRef instance, size_t pos);

/**
 * Returns a pointer to the first character.
 * 
 * If `CSTL_string_empty(instance) == true` the behavior is undefined.
 * 
 */
char* CSTL_string_front(CSTL_StringRef instance);

/**
 * Returns a const pointer to the first character.
 * 
 * If `CSTL_string_empty(instance) == true` the behavior is undefined.
 * 
 */
const char* CSTL_string_const_front(CSTL_StringCRef instance);

/**
 * Returns a pointer to the last character.
 * 
 * If `CSTL_string_empty(instance) == true` the behavior is undefined.
 * 
 */
char* CSTL_string_back(CSTL_StringRef instance);

/**
 * Returns a const pointer to the last character.
 * 
 * If `CSTL_string_empty(instance) == true` the behavior is undefined.
 * 
 */
const char* CSTL_string_const_back(CSTL_StringCRef instance);

/**
 * Returns a pointer to the underlying null-terminated array
 * serving as character storage.
 * 
 * If `CSTL_string_empty(instance) == true` the pointer is still
 * valid and points to a single null character.
 * 
 * The range `[CSTL_string_data(instance), CSTL_string_data(instance) + size]`
 * is always valid.
 * 
 * The array may be mutated through the returned pointer excluding
 * the past-the-end null terminator.
 * 
 */
char* CSTL_string_data(CSTL_StringRef instance);

/**
 * Returns a const pointer to the underlying null-terminated array
 * serving as character storage.
 * 
 * If `CSTL_string_empty(instance) == true` the pointer is still
 * valid and points to a single null character.
 * 
 * The range `[CSTL_string_c_str(instance), CSTL_string_c_str(instance) + size]`
 * is always valid.
 * 
 */
const char* CSTL_string_c_str(CSTL_StringCRef instance);

/**
 * Returns an iterator (pointer) to the first character of the string.
 * 
 * If `CSTL_string_empty(instance) == true` then
 * `CSTL_string_begin(instance) == CSTL_string_end(instance)`.
 * 
 */
char* CSTL_string_begin(CSTL_StringRef instance);

/**
 * Returns an iterator (pointer) past the last character of the string.
 * 
 * If `CSTL_string_empty(instance) == true` then
 * `CSTL_string_begin(instance) == CSTL_string_end(instance)`.
 * 
 */
const char* CSTL_string_const_begin(CSTL_StringCRef instance);

/**
 * Returns an iterator (pointer) past the last character of the string.
 * 
 * If `CSTL_string_empty(instance) == true` then
 * `CSTL_string_begin(instance) == CSTL_string_end(instance)`.
 * 
 */
char* CSTL_string_end(CSTL_StringRef instance);

/**
 * Returns a const iterator (pointer) past the last character of the string.
 * 
 * If `CSTL_string_empty(instance) == true` then
 * `CSTL_string_const_begin(instance) == CSTL_string_const_end(instance)`.
 * 
 */
const char* CSTL_string_const_end(CSTL_StringCRef instance);

/**
 * Returns `true` if the string is empty or `false` otherwise.
 * 
 */
bool CSTL_string_empty(CSTL_StringCRef instance);

/**
 * Returns the number of characters in the string.
 * 
 */
size_t CSTL_string_size(CSTL_StringCRef instance);

/**
 * Returns the number of characters in the string.
 * 
 */
size_t CSTL_string_length(CSTL_StringCRef instance);

/**
 * Returns the total characters capacity of the string.
 * 
 */
size_t CSTL_string_capacity(CSTL_StringCRef instance);

/**
 * Returns the maximum possible number of characters in the string.
 * 
 */
size_t CSTL_string_max_size();

/**
 * If `new_capacity > CSTL_string_capacity(instance)`, reallocates and expands
 * the underlying array storage.
 * 
 * If `new_capacity` exceeds `CSTL_string_max_size()` this function has no effect
 * and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_reserve(CSTL_StringRef instance, size_t new_capacity, CSTL_Alloc* alloc);

/**
 * Request removal of unused capacity.
 * 
 */
void CSTL_string_shrink_to_fit(CSTL_StringRef instance, CSTL_Alloc* alloc);

/**
 * Erase all characters from the string without affecting capacity.
 * 
 */
void CSTL_string_clear(CSTL_StringRef instance);

/**
 * Inserts the null-terminated string at `ptr` at the pointer `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
char* CSTL_string_insert(CSTL_StringRef instance, const char* where, const char* ptr, CSTL_Alloc* alloc);

/**
 * Inserts the first `count` characters of the string at `ptr` at the pointer  `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
char* CSTL_string_insert_n(CSTL_StringRef instance, const char* where, const char* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Inserts `count` copies of the character `ch` at the pointer `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
char* CSTL_string_insert_char(CSTL_StringRef instance, const char* where, size_t count, char ch, CSTL_Alloc* alloc);

/**
 * Inserts the string `other` at the pointer `where` in `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
char* CSTL_string_insert_str(CSTL_StringRef instance, const char* where, CSTL_StringCRef other, CSTL_Alloc* alloc);

/**
 * Inserts the substring at offset `other_off` in `other` with the length
 * given by `count` at the pointer `where` in `instance`.
 * 
 * If `other_off` is outside of the range `[other, other + CSTL_string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `NULL`, otherwise it returns a pointer
 * to the first inserted character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
char* CSTL_string_insert_substr(CSTL_StringRef instance, const char* where, CSTL_StringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Inserts the null-terminated string at `ptr` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_insert_at(CSTL_StringRef instance, size_t off, const char* ptr, CSTL_Alloc* alloc);

/**
 * Inserts the first `count` characters of the string at `ptr` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_insert_n_at(CSTL_StringRef instance, size_t off, const char* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Inserts `count` copies of the character `ch` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_insert_char_at(CSTL_StringRef instance, size_t off, size_t count, char ch, CSTL_Alloc* alloc);

/**
 * Inserts the string `other` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_insert_str_at(CSTL_StringRef instance, size_t off, CSTL_StringCRef other, CSTL_Alloc* alloc);

/**
 * Inserts the substring at offset `other_off` in `other` with the length
 * given by `count` at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if `other_off` is outside of the range `[other, other + CSTL_string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_insert_substr_at(CSTL_StringRef instance, size_t off, CSTL_StringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Removes the character at `where` and returns a pointer following the
 * removed character.
 * 
 * The pointer `where` must be valid and dereferenceable on `instance`.
 * 
 */
char* CSTL_string_erase(CSTL_StringRef instance, const char* where);

/**
 * Removes the characters in the range `[first, last)` and returns a pointer following the
 * removed character.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
char* CSTL_string_erase_substr(CSTL_StringRef instance, const char* first, const char* last);

/**
 * Removes the character at offset `off` in `instance`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_erase_at(CSTL_StringRef instance, size_t off);

/**
 * Removes the substring at offset `off` in `instance` with the length given by `count`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_erase_substr_at(CSTL_StringRef instance, size_t off, size_t count);

/**
 * Appends the character `ch` to the end of the string.
 * 
 * If `CSTL_string_size(instance) == CSTL_string_max_size()` this function
 * has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_push_back(CSTL_StringRef instance, char ch, CSTL_Alloc* alloc);

/**
 * Appends the character `ch` to the end of the string.
 * 
 * If `CSTL_string_size(instance) == CSTL_string_max_size()` this function
 * has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
void CSTL_string_pop_back(CSTL_StringRef instance);

/**
 * Appends the null-terminated string at `ptr` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_append(CSTL_StringRef instance, const char* ptr, CSTL_Alloc* alloc);

/**
 * Appends the first `count` characters of the string at `ptr` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_append_n(CSTL_StringRef instance, const char* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Appends `count` copies of the character `ch` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_append_char(CSTL_StringRef instance, size_t count, char ch, CSTL_Alloc* alloc);

/**
 * Appends the string `other` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_append_str(CSTL_StringRef instance, CSTL_StringCRef other, CSTL_Alloc* alloc);

/**
 * Appends the substring at offset `other_off` in `other` with the length
 * given by `count` to `instance`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_append_substr(CSTL_StringRef instance, CSTL_StringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the null-terminated
 * string at `ptr`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_replace(CSTL_StringRef instance, const char* first, const char* last, const char* ptr, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the first `count`
 * characters of the string at `ptr`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_replace_n(CSTL_StringRef instance, const char* first, const char* last, const char* ptr, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with `count` copies
 * of the character `ch`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_replace_char(CSTL_StringRef instance, const char* first, const char* last, size_t count, char ch, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the string `other`.
 * 
 * If the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_replace_str(CSTL_StringRef instance, const char* first, const char* last, CSTL_StringCRef other, CSTL_Alloc* alloc);

/**
 * Replaces the characters in the range `[first, last)` with the substring
 * at offset `other_off` in `other` with the length given by `count`.
 * 
 * If `other_off` is outside of the range `[other, other + CSTL_string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 * If `first == last`, no operation is performed.
 * 
 * The range `[first, last)` must be valid and dereferenceable on `instance`.
 * 
 */
bool CSTL_string_replace_substr(CSTL_StringRef instance, const char* first, const char* last, CSTL_StringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the null-terminated string at `ptr`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_replace_at(CSTL_StringRef instance, size_t off, size_t count, const char* ptr, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the first `count2` characters of the string at `ptr`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_replace_n_at(CSTL_StringRef instance, size_t off, size_t count, const char* ptr, size_t count2, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with `count` copies of the character `ch`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_replace_char_at(CSTL_StringRef instance, size_t off, size_t count, size_t count2, char ch, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the string `other`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_replace_str_at(CSTL_StringRef instance, size_t off, size_t count, CSTL_StringCRef other, CSTL_Alloc* alloc);

/**
 * Replaces the substring at offset `off` in `instance` with the length given by `count`
 * with the substring at offset `other_off` in `other` with the length
 * given by `count2`.
 * 
 * If `off` is outside of the range `[instance, instance + CSTL_string_size(instance)]`
 * or if `other_off` is outside of the range `[other, other + CSTL_string_size(other)]`
 * or if the length of the resulting string is greater than `CSTL_string_max_size()`
 * this function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_replace_substr_at(CSTL_StringRef instance, size_t off, size_t count, CSTL_StringCRef other, size_t other_off, size_t count2, CSTL_Alloc* alloc);

/**
 * Copies a substring `[off, off + count)` to character string pointed to by
 * `dest`. The resulting character string is not null terminated.
 * 
 * Returns the number of characters copied or `CSTL_string_npos` if
 * `off > CSTL_string_size(instance)` (out of range).
 * 
 */
size_t CSTL_string_copy(CSTL_StringCRef instance, char* dest, size_t count, size_t off);

/**
 * Resizes the string to contain `new_size` characters, appending `ch`
 * if current size is less than `new_size`.
 * 
 * If `new_size` is greater than `CSTL_string_max_size()` this
 * function has no effect and returns `false`, otherwise it returns `true`.
 * 
 */
bool CSTL_string_resize(CSTL_StringRef instance, size_t new_size, char ch, CSTL_Alloc* alloc);

/**
 * Find the first from offset `off` substring equal to the null-terminated string `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_find(CSTL_StringCRef instance, const char* ptr, size_t off);

/**
 * Find the first from offset `off` substring equal to the first `count` characters at `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_find_n(CSTL_StringCRef instance, const char* ptr, size_t off, size_t count);

/**
 * Find the first from offset `off` occurence of the character `ch`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_find_char(CSTL_StringCRef instance, char ch, size_t off);

/**
 * Find the first from offset `off` substring equal to the string `other`
 * and return its position from the start of the original string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_find_str(CSTL_StringCRef instance, CSTL_StringCRef other, size_t off);

/**
 * Find the last before offset `off` substring equal to the null-terminated string `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_rfind(CSTL_StringCRef instance, const char* ptr, size_t off);

/**
 * Find the last before offset `off` substring equal to the first `count` characters at `ptr`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_rfind_n(CSTL_StringCRef instance, const char* ptr, size_t off, size_t count);

/**
 * Find the last before offset `off` occurence of the character `ch`
 * and return its position from the start of the string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_rfind_char(CSTL_StringCRef instance, char ch, size_t off);

/**
 * Find the last before offset `off` substring equal to the string `other`
 * and return its position from the start of the original string.
 * 
 * If no match is found `CSTL_string_npos` is returned.
 * 
 */
size_t CSTL_string_rfind_str(CSTL_StringCRef instance, CSTL_StringCRef other, size_t off);

/**
 * Compare two null-terminated character sequences.
 * 
 * The return value is negative if `left` compares less than `right`,
 * positive if it compares greater and zero if `left` and `right`
 * compare equal.
 * 
 * There is no `CSTL_String` version of this function as the return
 * value does not allow for reporting out of bounds errors.
 * 
 */
int CSTL_string_compare(const char* left, const char* right);

/**
 * Compare an explicit length character sequence with a null-terminated one.
 * 
 * The return value is negative if `left` compares less than `right`,
 * positive if it compares greater and zero if `left` and `right`
 * compare equal.
 * 
 * To compare an explicit length `left` and a null-terminated `right`,
 * swap them and negate the result: `-CSTL_string_compare_n(right, left, left_count)`.
 * 
 * There is no `CSTL_String` version of this function as the return
 * value does not allow for reporting out of bounds errors.
 * 
 */
int CSTL_string_compare_n(const char* left, const char* right, size_t right_count);

/**
 * Compare two explicit length character sequences.
 * 
 * The return value is negative if `left` compares less than `right`,
 * positive if it compares greater and zero if `left` and `right`
 * compare equal.
 * 
 * There is no `CSTL_String` version of this function as the return
 * value does not allow for reporting out of bounds errors.
 * 
 */
int CSTL_string_compare_nn(const char* left, size_t left_count, const char* right, size_t right_count);
