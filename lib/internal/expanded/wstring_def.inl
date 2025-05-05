#include "wstring_decl.inl"
#include "../alloc_dispatch.h"

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

void CSTL_wstring_char_copy(wchar_t* dst, const wchar_t* src, size_t n) {
    memcpy((void*)dst, (const void*)src, n * sizeof(wchar_t));
}

void CSTL_wstring_char_move(wchar_t* dst, const wchar_t* src, size_t n) {
    memmove((void*)dst, (const void*)src, n * sizeof(wchar_t));
}

size_t CSTL_wstring_char_len(const wchar_t* ptr) {
    size_t result = CSTL_string_npos;
    do {
        ++result;
    } while (ptr[result] != 0);
    return result;
}

void CSTL_wstring_char_set(wchar_t* dst, wchar_t ch, size_t n) {
    while (n-- != 0) {
        *(dst++) = ch;
    }
}

const wchar_t* CSTL_wstring_char_memchr(const wchar_t* first, const wchar_t* last, wchar_t ch) {
    for (; first != last; ++first) {
        if (*first == ch) {
            return first;
        }
    }
    return NULL;
}

int CSTL_wstring_char_memcmp(const wchar_t* first1, const wchar_t* first2, size_t count)  {
    for (size_t i = 0; i < count; ++i) {
        if (first1[i] != first2[i]) {
            return first1[i] < first2[i] ? -1 : 1;
        }
    }
    return 0;
}

size_t CSTL_wstring_char_find_ch(const wchar_t* haystack, size_t hay_size, size_t start_at, wchar_t ch) {
    if (start_at < hay_size) {
        const wchar_t* found_at = CSTL_wstring_char_memchr(haystack + start_at,
            haystack + start_at + hay_size, ch);
            
        if (found_at != NULL) {
            return (size_t)(found_at - haystack);
        }
    }

    return CSTL_string_npos;
}

size_t CSTL_wstring_char_find_str(const wchar_t* haystack, size_t hay_size, size_t start_at, const wchar_t* needle, size_t needle_size) {
    if (needle_size > hay_size || start_at > hay_size - needle_size) {
        return CSTL_string_npos;
    }

    if (needle_size == 0) {
        return start_at;
    }

    const wchar_t* possible_matches_end = haystack + (hay_size - needle_size) + 1;

    for (const wchar_t* match_try = haystack + start_at;; ++match_try) {
        match_try = CSTL_wstring_char_memchr(match_try, possible_matches_end, *needle);

        if (match_try == NULL) {
            return CSTL_string_npos;
        }

        if (CSTL_wstring_char_memcmp(match_try + 1, needle + 1, needle_size - 1) == 0) {
            return (size_t)(match_try - haystack);
        }
    }
}

size_t CSTL_wstring_char_rfind_ch(const wchar_t* haystack, size_t hay_size, size_t start_at, wchar_t ch) {
    if (hay_size != 0) {
        size_t start_pos = start_at < hay_size - 1 ? start_at : hay_size - 1;
        for (const wchar_t* match_try = haystack + start_pos;; --match_try) {
            if (*match_try == ch) {
                return (size_t)(match_try - haystack);
            }

            if (match_try == haystack) {
                break; // at beginning, no more chance for match
            }
        }
    }


    return CSTL_string_npos;
}

size_t CSTL_wstring_char_rfind_str(const wchar_t* haystack, size_t hay_size, size_t start_at, const wchar_t* needle, size_t needle_size) {
    if (needle_size == 0) {
        return start_at < hay_size ? start_at : hay_size;
    }

    if (needle_size <= hay_size) {
        size_t start_pos = start_at < hay_size - needle_size ? start_at : hay_size - needle_size;
        for (const wchar_t* match_try = haystack + start_pos;; --match_try) {
            if (*match_try == *needle && CSTL_wstring_char_memcmp(match_try + 1, needle + 1, needle_size - 1) == 0) {
                return (size_t)(match_try - haystack);
            }

            if (match_try == haystack) {
                break; // at beginning, no more chance for match
            }
        }
    }

    return CSTL_string_npos;
}

bool CSTL_wstring_large_mode_engaged(CSTL_WideStringCRef instance) {
    return instance->res > CSTL_string_small_capacity;
}

wchar_t* CSTL_wstring_ptr(CSTL_WideStringRef instance) {
    wchar_t* result = instance->bx.buf;
    if (CSTL_wstring_large_mode_engaged(instance)) {
        result = instance->bx.ptr;
    }

    return result;
}

const wchar_t* CSTL_wstring_const_ptr(CSTL_WideStringCRef instance) {
    const wchar_t* result = instance->bx.buf;
    if (CSTL_wstring_large_mode_engaged(instance)) {
        result = instance->bx.ptr;
    }

    return result;
}

void CSTL_wstring_eos(CSTL_WideStringRef instance, size_t new_size) {
    instance->size = new_size;
    CSTL_wstring_ptr(instance)[new_size] = 0;
}

size_t CSTL_wstring_clamp_suffix_size(CSTL_WideStringCRef instance, size_t off, size_t size) {
    size_t suffix_size = instance->size - off;
    return size < suffix_size ? size : suffix_size;
}

size_t CSTL_wstring_calculate_growth(size_t requested, size_t old) {
    const size_t max    = CSTL_wstring_max_size();
    const size_t masked = requested | CSTL_string_alloc_mask;

    if (masked > max) {
        return max;
    }

    if (old > max - old / 2) {
        return max;
    }

    size_t geometric = old + old / 2;

    return geometric > masked ? geometric : masked;
}

wchar_t* CSTL_wstring_allocate_for_capacity(size_t capacity, CSTL_Alloc* alloc) {
    size_t size = (capacity + 1) * sizeof(wchar_t); // +1 for null terminator
    return (wchar_t*)CSTL_allocate(size, alignof(wchar_t), alloc);
}

void CSTL_wstring_deallocate_for_capacity(wchar_t* old_ptr, size_t capacity, CSTL_Alloc* alloc) {
    CSTL_free((void*)old_ptr, capacity + 1, alignof(wchar_t), alloc); // +1 for null terminator
}

void CSTL_wstring_tidy_deallocate(CSTL_WideStringRef instance, CSTL_Alloc* alloc) {
    if (CSTL_wstring_large_mode_engaged(instance)) {
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, instance->res, alloc);
        instance->bx.ptr = NULL;
    }

    instance->size = 0;
    instance->res  = CSTL_string_small_capacity;

    instance->bx.buf[0] = 0;
}

void CSTL_wstring_take_contents(CSTL_WideStringRef instance, CSTL_WideStringRef other_instance) {
    memcpy((void*)instance, (const void*)other_instance, sizeof(CSTL_WideStringVal));

    other_instance->size = 0;
    other_instance->res  = CSTL_string_small_capacity;

    other_instance->bx.buf[0] = 0;
}

void CSTL_wstring_become_small(CSTL_WideStringRef instance, CSTL_Alloc* alloc) {
    wchar_t* old_ptr = CSTL_wstring_ptr(instance);

    CSTL_wstring_char_copy(instance->bx.buf, old_ptr, instance->size + 1);
    CSTL_wstring_deallocate_for_capacity(old_ptr, instance->res, alloc);

    instance->res = CSTL_string_small_capacity;
}

void CSTL_wstring_construct(CSTL_WideStringVal* new_instance) {
    if (new_instance == NULL) {
        return;
    }

    new_instance->size = 0;
    new_instance->res  = CSTL_string_small_capacity;

    new_instance->bx.buf[0] = 0;
}

void CSTL_wstring_destroy(CSTL_WideStringRef instance, CSTL_Alloc* alloc) {
    CSTL_wstring_tidy_deallocate(instance, alloc);
}

bool CSTL_wstring_substr(CSTL_WideStringVal* new_instance, CSTL_WideStringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc) {
    if (new_instance == NULL) {
        return false;
    }

    if (new_instance != other) {
        CSTL_wstring_construct(new_instance);
    }

    return CSTL_wstring_assign_substr(new_instance, other, other_off, count, alloc);
}

bool CSTL_wstring_assign(CSTL_WideStringRef instance, const wchar_t* ptr, CSTL_Alloc* alloc) {
    return CSTL_wstring_assign_n(instance, ptr, CSTL_wstring_char_len(ptr), alloc);
}

bool CSTL_wstring_assign_n(CSTL_WideStringRef instance, const wchar_t* ptr, size_t count, CSTL_Alloc* alloc) {
    if (count <= instance->res) {
        instance->size = count;

        wchar_t* old_ptr = CSTL_wstring_ptr(instance);
        CSTL_wstring_char_move(old_ptr, ptr, count);
        old_ptr[count] = 0;

        return true;
    }

    if (count > CSTL_wstring_max_size()) {
        return false;
    }

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(count, old_capacity);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    instance->size = count;
    instance->res  = new_capacity;

    CSTL_wstring_char_copy(new_ptr, ptr, count);
    new_ptr[count] = 0;

    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    }

    instance->bx.ptr = new_ptr;

    return true;
}

bool CSTL_wstring_assign_char(CSTL_WideStringRef instance, size_t count, wchar_t ch, CSTL_Alloc* alloc) {
    if (count <= instance->res) {
        instance->size = count;
        
        wchar_t* old_ptr = CSTL_wstring_ptr(instance);
        CSTL_wstring_char_set(old_ptr, ch, count);
        old_ptr[count] = 0;

        return true;
    }

    if (count > CSTL_wstring_max_size()) {
        return false;
    }

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(count, old_capacity);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    instance->size = count;
    instance->res  = new_capacity;

    CSTL_wstring_char_set(new_ptr, ch, count);
    new_ptr[count] = 0;

    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    }

    instance->bx.ptr = new_ptr;

    return true;
}

bool CSTL_wstring_assign_substr(CSTL_WideStringRef instance, CSTL_WideStringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc) {
    if (other->size < other_off) {
        return false;
    }

    count = CSTL_wstring_clamp_suffix_size(other, other_off, count);
    return CSTL_wstring_assign_n(instance, CSTL_wstring_const_ptr(other) + other_off, count, alloc);
}

void CSTL_wstring_copy_assign(CSTL_WideStringRef instance, CSTL_Alloc* alloc, CSTL_WideStringCRef other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    if (instance == other_instance) {
        return;
    }

    if (propagate_alloc) {
        if (alloc != other_alloc) {
            size_t other_size            = other_instance->size;
            const wchar_t* other_ptr = CSTL_wstring_const_ptr(other_instance);

            if (other_size > CSTL_string_small_capacity) {
                size_t new_capacity  = CSTL_wstring_calculate_growth(other_size, CSTL_string_small_capacity);
                wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, other_alloc);

                CSTL_wstring_char_copy(new_ptr, other_ptr, other_size + 1);

                CSTL_wstring_tidy_deallocate(instance, alloc);

                instance->size = other_size;
                instance->res  = new_capacity;

                instance->bx.ptr = new_ptr;
            } else {
                CSTL_wstring_tidy_deallocate(instance, alloc);
                CSTL_wstring_char_copy(instance->bx.buf, other_ptr, other_size + 1);

                instance->size = other_size;
                instance->res  = CSTL_string_small_capacity;
            }

            return;
        }
    }

    CSTL_wstring_assign_n(instance, CSTL_wstring_const_ptr(other_instance),
        other_instance->size, alloc);
}

void CSTL_wstring_move_assign(CSTL_WideStringRef instance, CSTL_Alloc* alloc, CSTL_WideStringRef other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    if (instance == other_instance) {
        return;
    }

    if (!propagate_alloc) {
        if (alloc != other_alloc) {
            CSTL_wstring_assign_n(instance, CSTL_wstring_const_ptr(other_instance),
                other_instance->size, alloc);
            return;
        }
    }

    CSTL_wstring_tidy_deallocate(instance, alloc);
    CSTL_wstring_take_contents(instance, other_instance);
}

void CSTL_wstring_swap(CSTL_WideStringRef instance, CSTL_WideStringRef other_instance) {
    if (instance != other_instance) {
        unsigned char temp_mem[sizeof(CSTL_WideStringVal)];
        memcpy((void*)temp_mem, (const void*)instance, sizeof(CSTL_WideStringVal));
        memcpy((void*)instance, (const void*)other_instance, sizeof(CSTL_WideStringVal));
        memcpy((void*)other_instance, (const void*)temp_mem, sizeof(CSTL_WideStringVal));
    }
}

wchar_t* CSTL_wstring_index(CSTL_WideStringRef instance, size_t pos) {
    assert(pos < instance->size);
    return &CSTL_wstring_ptr(instance)[pos];
}

const wchar_t* CSTL_wstring_const_index(CSTL_WideStringCRef instance, size_t pos) {
    assert(pos < instance->size);
    return &CSTL_wstring_const_ptr(instance)[pos];
}

wchar_t* CSTL_wstring_at(CSTL_WideStringRef instance, size_t pos)  {
    if (instance->size <= pos) {
        return NULL;
    }

    return &CSTL_wstring_ptr(instance)[pos];
}

const wchar_t* CSTL_wstring_const_at(CSTL_WideStringCRef instance, size_t pos) {
    if (instance->size <= pos) {
        return NULL;
    }

    return &CSTL_wstring_const_ptr(instance)[pos];
}

wchar_t* CSTL_wstring_front(CSTL_WideStringRef instance) {
    assert(instance->size != 0);
    return CSTL_wstring_ptr(instance);
}

const wchar_t* CSTL_wstring_const_front(CSTL_WideStringCRef instance) {
    assert(instance->size != 0);
    return CSTL_wstring_const_ptr(instance);
}

wchar_t* CSTL_wstring_back(CSTL_WideStringRef instance) {
    assert(instance->size != 0);
    return &CSTL_wstring_ptr(instance)[instance->size - 1];
}

const wchar_t* CSTL_wstring_const_back(CSTL_WideStringCRef instance) {
    assert(instance->size != 0);
    return &CSTL_wstring_const_ptr(instance)[instance->size - 1];
}

wchar_t* CSTL_wstring_data(CSTL_WideStringRef instance) {
    return CSTL_wstring_ptr(instance);
}

const wchar_t* CSTL_wstring_c_str(CSTL_WideStringCRef instance) {
    return CSTL_wstring_const_ptr(instance);
}

wchar_t* CSTL_wstring_begin(CSTL_WideStringRef instance) {
    return CSTL_wstring_ptr(instance);
}

const wchar_t* CSTL_wstring_const_begin(CSTL_WideStringCRef instance) {
    return CSTL_wstring_const_ptr(instance);
}

wchar_t* CSTL_wstring_end(CSTL_WideStringRef instance) {
    return &CSTL_wstring_ptr(instance)[instance->size];
}

const wchar_t* CSTL_wstring_const_end(CSTL_WideStringCRef instance) {
    return &CSTL_wstring_const_ptr(instance)[instance->size];
}

bool CSTL_wstring_empty(CSTL_WideStringCRef instance) {
    return instance->size == 0;
}

size_t CSTL_wstring_size(CSTL_WideStringCRef instance) {
    return instance->size;
}

size_t CSTL_wstring_length(CSTL_WideStringCRef instance) {
    return instance->size;
}

size_t CSTL_wstring_capacity(CSTL_WideStringCRef instance) {
    return instance->res;
}

size_t CSTL_wstring_max_size() {
    return sizeof(wchar_t) == 1 ? (size_t)PTRDIFF_MAX - 1
        : (size_t)PTRDIFF_MAX / sizeof(wchar_t); 
}

bool CSTL_wstring_reserve(CSTL_WideStringRef instance, size_t new_capacity, CSTL_Alloc* alloc) {
    if (instance->res >= new_capacity) {
        return true; // nothing to do
    }

    if (new_capacity > CSTL_wstring_max_size()) {
        return false;
    }

    size_t old_capacity  = instance->res;
    new_capacity         = CSTL_wstring_calculate_growth(new_capacity, instance->res);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    instance->res = new_capacity;

    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, instance->size + 1);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    } else {
        CSTL_wstring_char_copy(new_ptr, instance->bx.buf, instance->size + 1);
    }

    instance->bx.ptr = new_ptr;

    return true;
}

void CSTL_wstring_shrink_to_fit(CSTL_WideStringRef instance, CSTL_Alloc* alloc) {
    if (!CSTL_wstring_large_mode_engaged(instance)) {
        return;
    }

    if (instance->size < CSTL_string_small_capacity) {
        CSTL_wstring_become_small(instance, alloc);
        return;
    }

    size_t max_size        = CSTL_wstring_max_size();
    size_t masked_size     = instance->size | CSTL_string_alloc_mask;
    size_t target_capacity = masked_size < max_size ? masked_size : max_size;

    if (target_capacity < instance->res) {
        wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(target_capacity, alloc);

        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, instance->size + 1);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, instance->res, alloc);

        instance->bx.ptr = new_ptr;
        instance->res    = target_capacity;
    }
}

void CSTL_wstring_clear(CSTL_WideStringRef instance) {
    CSTL_wstring_eos(instance, 0);
}

bool CSTL_wstring_insert_at(CSTL_WideStringRef instance, size_t off, const wchar_t* ptr, CSTL_Alloc* alloc) {
    return CSTL_wstring_insert_n_at(instance, off, ptr, CSTL_wstring_char_len(ptr), alloc);
}

bool CSTL_wstring_insert_n_at(CSTL_WideStringRef instance, size_t off, const wchar_t* ptr, size_t count, CSTL_Alloc* alloc) {
    size_t old_size = instance->size;

    if (old_size < off) {
        return false;
    }

    if (count <= instance->res - old_size) {
        instance->size = old_size + count;

        wchar_t* old_ptr   = CSTL_wstring_ptr(instance);
        wchar_t* insert_at = old_ptr + off;

        size_t ptr_shifted_after;
        if (ptr + count <= insert_at || ptr > old_ptr + old_size) {
            ptr_shifted_after = count;
        } else if (insert_at <= ptr) {
            ptr_shifted_after = 0;
        } else {
            ptr_shifted_after = (size_t)(insert_at - ptr);
        }

        CSTL_wstring_char_move(insert_at + count, insert_at, old_size - off + 1); // move suffix + null down
        CSTL_wstring_char_copy(insert_at, ptr, ptr_shifted_after);
        CSTL_wstring_char_copy(
            insert_at + ptr_shifted_after, ptr + count + ptr_shifted_after, count - ptr_shifted_after);

        return true;
    }

    if (CSTL_wstring_max_size() - old_size < count) {
        return false;
    }

    size_t new_size = old_size + count;

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(new_size, instance->res);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    instance->size = new_size;
    instance->res  = new_capacity;

    CSTL_wstring_char_copy(new_ptr + off, ptr, count);
    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, off);
        CSTL_wstring_char_copy(new_ptr + off + count, instance->bx.ptr + off, old_size - off + 1);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    } else {
        CSTL_wstring_char_copy(new_ptr, instance->bx.buf, off);
        CSTL_wstring_char_copy(new_ptr + off + count, instance->bx.buf + off, old_size - off + 1);
    }

    instance->bx.ptr = new_ptr;

    return true;
}

bool CSTL_wstring_insert_char_at(CSTL_WideStringRef instance, size_t off, size_t count, wchar_t ch, CSTL_Alloc* alloc) {    
    size_t old_size = instance->size;

    if (old_size < off) {
        return false;
    }

    if (count <= instance->res - old_size) {
        instance->size = old_size + count;

        wchar_t* old_ptr   = CSTL_wstring_ptr(instance);
        wchar_t* insert_at = old_ptr + off;

        CSTL_wstring_char_move(insert_at + count, insert_at, old_size - off + 1); // move suffix + null down
        CSTL_wstring_char_set(insert_at, ch, count); // fill hole

        return true;
    }

    if (CSTL_wstring_max_size() - old_size < count) {
        return false;
    }

    size_t new_size = old_size + count;

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(new_size, instance->res);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    instance->size = new_size;
    instance->res  = new_capacity;

    CSTL_wstring_char_set(new_ptr + off, ch, count);
    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, off);
        CSTL_wstring_char_copy(new_ptr + off + count, instance->bx.ptr + off, old_size - off + 1);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    } else {
        CSTL_wstring_char_copy(new_ptr, instance->bx.buf, off);
        CSTL_wstring_char_copy(new_ptr + off + count, instance->bx.buf + off, old_size - off + 1);
    }

    instance->bx.ptr = new_ptr;

    return true;
}

bool CSTL_wstring_insert_str_at(CSTL_WideStringRef instance, size_t off, CSTL_WideStringCRef other, CSTL_Alloc* alloc) {
    return CSTL_wstring_insert_n_at(instance, off, CSTL_wstring_const_ptr(other), other->size, alloc);
}

bool CSTL_wstring_insert_substr_at(CSTL_WideStringRef instance, size_t off, CSTL_WideStringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc) {
    if (other->size < other_off) {
        return false;
    }

    count = CSTL_wstring_clamp_suffix_size(other, other_off, count);
    return CSTL_wstring_insert_n_at(instance, off, CSTL_wstring_const_ptr(other) + other_off, count, alloc);
}

bool CSTL_wstring_erase_at(CSTL_WideStringRef instance, size_t off) {
    if (instance->size < off) {
        return false;
    }

    size_t old_size = instance->size;
    size_t new_size = old_size - 1;

    instance->size = new_size;

    wchar_t* ptr      = CSTL_wstring_ptr(instance);
    wchar_t* erase_at = ptr + off;

    CSTL_wstring_char_move(erase_at, erase_at + 1, old_size - off);

    return true;
}

bool CSTL_wstring_erase_substr_at(CSTL_WideStringRef instance, size_t off, size_t count) {
    if (instance->size < off) {
        return false;
    }

    count = CSTL_wstring_clamp_suffix_size(instance, off, count);

    size_t new_size = instance->size - count;

    instance->size = new_size;

    wchar_t* ptr      = CSTL_wstring_ptr(instance);
    wchar_t* erase_at = ptr + off;

    CSTL_wstring_char_move(erase_at, erase_at + count, new_size - off + 1);

    return true;
}

bool CSTL_wstring_push_back(CSTL_WideStringRef instance, wchar_t ch, CSTL_Alloc* alloc) {
    size_t old_size = instance->size;

    if (old_size < instance->res) {
        instance->size   = old_size + 1;
        wchar_t* ptr = CSTL_wstring_ptr(instance);

        ptr[old_size]     = ch;
        ptr[old_size + 1] = 0;

        return true;
    }

    if (old_size == CSTL_wstring_max_size()) {
        return false;
    }

    size_t new_size = old_size + 1;

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(new_size, instance->res);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    instance->size = new_size;
    instance->res  = new_capacity;

    new_ptr[old_size] = ch;
    new_ptr[new_size] = 0;

    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, old_size);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    } else {
        CSTL_wstring_char_copy(new_ptr, instance->bx.buf, old_size);
    }

    instance->bx.ptr = new_ptr;

    return true;
}

void CSTL_wstring_pop_back(CSTL_WideStringRef instance) {
    size_t old_size = instance->size;
    assert(old_size != 0);
    CSTL_wstring_eos(instance, old_size - 1);
}

bool CSTL_wstring_append(CSTL_WideStringRef instance, const wchar_t* ptr, CSTL_Alloc* alloc) {
    return CSTL_wstring_append_n(instance, ptr, CSTL_wstring_char_len(ptr), alloc);
}

bool CSTL_wstring_append_n(CSTL_WideStringRef instance, const wchar_t* ptr, size_t count, CSTL_Alloc* alloc) {
    size_t old_size = instance->size;
    size_t new_size = old_size + count;

    if (count <= instance->res - old_size) {
        instance->size = new_size;

        wchar_t* old_ptr = CSTL_wstring_ptr(instance);

        CSTL_wstring_char_move(old_ptr + old_size, ptr, count);
        old_ptr[new_size] = 0;

        return true;
    }

    if (CSTL_wstring_max_size() - old_size < count) {
        return false;
    }

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(new_size, instance->res);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    instance->size = new_size;
    instance->res  = new_capacity;

    CSTL_wstring_char_copy(new_ptr + old_size, ptr, count);
    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, old_size);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    } else {
        CSTL_wstring_char_copy(new_ptr, instance->bx.buf, old_size);
    }

    new_ptr[new_size] = 0;
    
    instance->bx.ptr = new_ptr;

    return true;
}

bool CSTL_wstring_append_char(CSTL_WideStringRef instance, size_t count, wchar_t ch, CSTL_Alloc* alloc) {
    return CSTL_wstring_insert_char_at(instance, instance->size, count, ch, alloc);
}

bool CSTL_wstring_append_str(CSTL_WideStringRef instance, CSTL_WideStringCRef other, CSTL_Alloc* alloc) {
    return CSTL_wstring_append_n(instance, CSTL_wstring_const_ptr(other), other->size, alloc);
}

bool CSTL_wstring_append_substr(CSTL_WideStringRef instance, CSTL_WideStringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc) {
    if (other->size < other_off) {
        return false;
    }

    count = CSTL_wstring_clamp_suffix_size(other, other_off, count);
    return CSTL_wstring_append_n(instance, CSTL_wstring_const_ptr(other) + other_off, count, alloc);
}

bool CSTL_wstring_replace_at(CSTL_WideStringRef instance, size_t off, size_t count, const wchar_t* ptr, CSTL_Alloc* alloc) {
    return CSTL_wstring_replace_n_at(instance, off, count, ptr, CSTL_wstring_char_len(ptr), alloc);
}

bool CSTL_wstring_replace_n_at(CSTL_WideStringRef instance, size_t off, size_t count, const wchar_t* ptr, size_t count2, CSTL_Alloc* alloc) {
    if (instance->size < off) {
        return false;
    }

    count = CSTL_wstring_clamp_suffix_size(instance, off, count);

    if (count == count2) { // size doesn't change
        CSTL_wstring_char_move(CSTL_wstring_ptr(instance) + off, ptr, count2);
        return true;
    }

    size_t old_size    = instance->size;
    size_t suffix_size = old_size - count - off + 1;

    if (count2 < count) { // suffix shifts backwards
        wchar_t* old_ptr   = CSTL_wstring_ptr(instance);
        wchar_t* insert_at = old_ptr + off;

        CSTL_wstring_char_move(insert_at, ptr, count2);
        CSTL_wstring_char_move(insert_at + count2, insert_at + count, suffix_size);

        size_t new_size = old_size - (count - count2);
        instance->size  = new_size;

        return true;
    }

    size_t growth = count2 - count;

    if (growth <= instance->res - old_size) {
        instance->size = old_size + growth;

        wchar_t* old_ptr   = CSTL_wstring_ptr(instance);
        wchar_t* insert_at = old_ptr + off;
        wchar_t* suffix_at = insert_at + count;

        size_t pre_shifted_after;
        if (ptr + count2 <= insert_at || ptr > old_ptr + old_size) {
            pre_shifted_after = count2;
        } else if (suffix_at <= ptr) {
            pre_shifted_after = 0;
        } else {
            pre_shifted_after = (size_t)(suffix_at - ptr);
        }

        CSTL_wstring_char_move(suffix_at + growth, suffix_at, suffix_size);
        CSTL_wstring_char_move(insert_at, ptr, pre_shifted_after);
        CSTL_wstring_char_copy(
            insert_at + pre_shifted_after, ptr + growth + pre_shifted_after, count2 - pre_shifted_after);

        return true;
    }

    if (CSTL_wstring_max_size() - old_size < growth) {
        return false;
    }

    size_t new_size = old_size + growth;

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(new_size, instance->res);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    CSTL_wstring_char_copy(new_ptr + off, ptr, count2);
    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, off);
        CSTL_wstring_char_copy(
            new_ptr + off + count2, instance->bx.ptr + off + count, old_size - count - off + 1);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    } else {
        CSTL_wstring_char_copy(new_ptr, instance->bx.buf, off);
        CSTL_wstring_char_copy(
            new_ptr + off + count2, instance->bx.buf + off + count, old_size - count - off + 1);
    }

    instance->size = new_size;
    instance->res  = new_capacity;

    instance->bx.ptr = new_ptr;

    return true;
}

bool CSTL_wstring_replace_char_at(CSTL_WideStringRef instance, size_t off, size_t count, size_t count2, wchar_t ch, CSTL_Alloc* alloc) {
    if (instance->size < off) {
        return false;
    }

    count = CSTL_wstring_clamp_suffix_size(instance, off, count);

    if (count == count2) { // size doesn't change
        CSTL_wstring_char_set(CSTL_wstring_ptr(instance) + off, ch, count2);
        return true;
    }

    size_t old_size = instance->size;
    
    if (count2 < count || count2 - count <= instance->res - old_size) {
        size_t new_size = old_size + count2 - count;
        instance->size  = new_size;

        wchar_t* old_ptr   = CSTL_wstring_ptr(instance);
        wchar_t* insert_at = old_ptr + off;

        CSTL_wstring_char_move(insert_at + count2, insert_at + count, old_size - count - off + 1);
        CSTL_wstring_char_set(insert_at, ch, count2);

        return true;
    }

    size_t growth = count2 - count;

    if (CSTL_wstring_max_size() - old_size < growth) {
        return false;
    }

    size_t new_size = old_size + growth;

    size_t old_capacity  = instance->res;
    size_t new_capacity  = CSTL_wstring_calculate_growth(new_size, instance->res);
    wchar_t* new_ptr = CSTL_wstring_allocate_for_capacity(new_capacity, alloc);

    CSTL_wstring_char_set(new_ptr + off, ch, count2);
    if (old_capacity > CSTL_string_small_capacity) {
        CSTL_wstring_char_copy(new_ptr, instance->bx.ptr, off);
        CSTL_wstring_char_copy(
            new_ptr + off + count2, instance->bx.ptr + off + count, old_size - count - off + 1);
        CSTL_wstring_deallocate_for_capacity(instance->bx.ptr, old_capacity, alloc);
    } else {
        CSTL_wstring_char_copy(new_ptr, instance->bx.buf, off);
        CSTL_wstring_char_copy(
            new_ptr + off + count2, instance->bx.buf + off + count, old_size - count - off + 1);
    }

    instance->size = new_size;
    instance->res  = new_capacity;

    instance->bx.ptr = new_ptr;

    return true;
}

bool CSTL_wstring_replace_str_at(CSTL_WideStringRef instance, size_t off, size_t count, CSTL_WideStringCRef other, CSTL_Alloc* alloc) {
    return CSTL_wstring_replace_n_at(instance, off, count, CSTL_wstring_const_ptr(other), other->size, alloc);
}

bool CSTL_wstring_replace_substr_at(CSTL_WideStringRef instance, size_t off, size_t count, CSTL_WideStringCRef other, size_t other_off, size_t count2, CSTL_Alloc* alloc) {
    if (other->size < other_off) {
        return false;
    }

    count2 = CSTL_wstring_clamp_suffix_size(other, other_off, count2);
    return CSTL_wstring_replace_n_at(instance, off, count, CSTL_wstring_const_ptr(other) + other_off, count2, alloc);
}

size_t CSTL_wstring_copy(CSTL_WideStringCRef instance, wchar_t* dest, size_t count, size_t off) {
    if (instance->size < off) {
        return CSTL_string_npos;
    }

    count = CSTL_wstring_clamp_suffix_size(instance, off, count);
    CSTL_wstring_char_copy(dest, CSTL_wstring_const_ptr(instance) + off, count);

    return count;
}

bool CSTL_wstring_resize(CSTL_WideStringRef instance, size_t new_size, wchar_t ch, CSTL_Alloc* alloc) {
    size_t old_size = CSTL_wstring_size(instance);

    if (new_size > old_size) {
        return CSTL_wstring_append_char(instance, new_size - old_size, ch, alloc);
    } else {
        CSTL_wstring_eos(instance, new_size);
    }

    return true;
}

size_t CSTL_wstring_find(CSTL_WideStringCRef instance, const wchar_t* ptr, size_t off) {
    return CSTL_wstring_find_n(instance, ptr, off, CSTL_wstring_char_len(ptr));
}

size_t CSTL_wstring_find_n(CSTL_WideStringCRef instance, const wchar_t* ptr, size_t off, size_t count) {
    return CSTL_wstring_char_find_str(CSTL_wstring_const_ptr(instance), instance->size, off, ptr, count);
}

size_t CSTL_wstring_find_char(CSTL_WideStringCRef instance, wchar_t ch, size_t off) {
    return CSTL_wstring_char_find_ch(CSTL_wstring_const_ptr(instance), instance->size, off, ch);
}

size_t CSTL_wstring_find_str(CSTL_WideStringCRef instance, CSTL_WideStringCRef other, size_t off) {
    return CSTL_wstring_char_find_str(CSTL_wstring_const_ptr(instance), instance->size, off,
        CSTL_wstring_const_ptr(other), other->size);
}

size_t CSTL_wstring_rfind(CSTL_WideStringCRef instance, const wchar_t* ptr, size_t off) {
    return CSTL_wstring_rfind_n(instance, ptr, off, CSTL_wstring_char_len(ptr));
}

size_t CSTL_wstring_rfind_n(CSTL_WideStringCRef instance, const wchar_t* ptr, size_t off, size_t count) {
    return CSTL_wstring_char_rfind_str(CSTL_wstring_const_ptr(instance), instance->size, off, ptr, count);
}

size_t CSTL_wstring_rfind_char(CSTL_WideStringCRef instance, wchar_t ch, size_t off) {
    return CSTL_wstring_char_rfind_ch(CSTL_wstring_const_ptr(instance), instance->size, off, ch);
}

size_t CSTL_wstring_rfind_str(CSTL_WideStringCRef instance, CSTL_WideStringCRef other, size_t off) {
    return CSTL_wstring_char_rfind_str(CSTL_wstring_const_ptr(instance), instance->size, off,
        CSTL_wstring_const_ptr(other), other->size);
}

int CSTL_wstring_compare(const wchar_t* left, const wchar_t* right) {
    return CSTL_wstring_compare_nn(left, CSTL_wstring_char_len(left), right, CSTL_wstring_char_len(right));
}

int CSTL_wstring_compare_n(const wchar_t* left, const wchar_t* right, size_t right_count) {
    return CSTL_wstring_compare_nn(left, CSTL_wstring_char_len(left), right, right_count);
}

int CSTL_wstring_compare_nn(const wchar_t* left, size_t left_count, const wchar_t* right, size_t right_count) {
    bool left_lt_right = left_count < right_count;
    size_t count       = left_lt_right ? left_count : right_count;

    int result = CSTL_wstring_char_memcmp(left, right, count);
    if (result == 0) {
        return left_lt_right ? -1 : 1;
    } else {
        return result;
    }
}

wchar_t* CSTL_wstring_insert(CSTL_WideStringRef instance, const wchar_t* where, const wchar_t* ptr, CSTL_Alloc* alloc) {
    wchar_t* ptr1 = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(where - ptr1);
    if (CSTL_wstring_insert_at(instance, off, ptr, alloc)) {
        return ptr1 + (ptrdiff_t)off;
    }
    return NULL;
}

wchar_t* CSTL_wstring_insert_n(CSTL_WideStringRef instance, const wchar_t* where, const wchar_t* ptr, size_t count, CSTL_Alloc* alloc) {
    wchar_t* ptr1 = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(where - ptr1);
    if (CSTL_wstring_insert_n_at(instance, off, ptr, count, alloc)) {
        return ptr1 + (ptrdiff_t)off;
    }
    return NULL;
}

wchar_t* CSTL_wstring_insert_char(CSTL_WideStringRef instance, const wchar_t* where, size_t count, wchar_t ch, CSTL_Alloc* alloc) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(where - ptr);
    if (CSTL_wstring_insert_char_at(instance, off, count, ch, alloc)) {
        return ptr + (ptrdiff_t)off;
    }
    return NULL;
}

wchar_t* CSTL_wstring_insert_str(CSTL_WideStringRef instance, const wchar_t* where, CSTL_WideStringCRef other, CSTL_Alloc* alloc) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(where - ptr);
    if (CSTL_wstring_insert_str_at(instance, off, other, alloc)) {
        return ptr + (ptrdiff_t)off;
    }
    return NULL;
}

wchar_t* CSTL_wstring_insert_substr(CSTL_WideStringRef instance, const wchar_t* where, CSTL_WideStringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(where - ptr);
    if (CSTL_wstring_insert_substr_at(instance, off, other, other_off, count, alloc)) {
        return ptr + (ptrdiff_t)off;
    }
    return NULL;
}

wchar_t* CSTL_wstring_erase(CSTL_WideStringRef instance, const wchar_t* where) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(where - ptr);
    if (CSTL_wstring_erase_at(instance, off)) {
        return ptr + (ptrdiff_t)off;
    }
    return NULL;
}

wchar_t* CSTL_wstring_erase_substr(CSTL_WideStringRef instance, const wchar_t* first, const wchar_t* last) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(first - ptr);
    if (first == last) {
        return ptr + (ptrdiff_t)off;
    }
    size_t count = (size_t)(last - first);
    if (count < instance->size && CSTL_wstring_erase_substr_at(instance, off, count)) {
        return ptr + (ptrdiff_t)off;
    }
    return NULL;
}

bool CSTL_wstring_replace(CSTL_WideStringRef instance, const wchar_t* first, const wchar_t* last, const wchar_t* ptr, CSTL_Alloc* alloc) {
    wchar_t* ptr1 = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(first - ptr1);
    size_t count = (size_t)(last - first);
    return (count == 0 || count < instance->size)
        && CSTL_wstring_replace_at(instance, off, count, ptr, alloc);
}

bool CSTL_wstring_replace_n(CSTL_WideStringRef instance, const wchar_t* first, const wchar_t* last, const wchar_t* ptr, size_t count, CSTL_Alloc* alloc) {
    wchar_t* ptr1 = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(first - ptr1);
    size_t count1 = (size_t)(last - first);
    return (count1 == 0 || count1 < instance->size)
        && CSTL_wstring_replace_n_at(instance, off, count1, ptr, count, alloc);
}

bool CSTL_wstring_replace_char(CSTL_WideStringRef instance, const wchar_t* first, const wchar_t* last, size_t count, wchar_t ch, CSTL_Alloc* alloc) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(first - ptr);
    size_t count1 = (size_t)(last - first);
    return (count1 == 0 || count1 < instance->size)
        && CSTL_wstring_replace_char_at(instance, off, count1, count, ch, alloc);
}

bool CSTL_wstring_replace_str(CSTL_WideStringRef instance, const wchar_t* first, const wchar_t* last, CSTL_WideStringCRef other, CSTL_Alloc* alloc) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(first - ptr);
    size_t count = (size_t)(last - first);
    return (count == 0 || count < instance->size)
        && CSTL_wstring_replace_str_at(instance, off, count, other, alloc);
}

bool CSTL_wstring_replace_substr(CSTL_WideStringRef instance, const wchar_t* first, const wchar_t* last, CSTL_WideStringCRef other, size_t other_off, size_t count, CSTL_Alloc* alloc) {
    wchar_t* ptr = CSTL_wstring_ptr(instance);
    size_t off = (size_t)(first - ptr);
    size_t count1 = (size_t)(last - first);
    return (count1 == 0 || count1 < instance->size)
        && CSTL_wstring_replace_substr_at(instance, off, count1, other, other_off, count, alloc);
}
