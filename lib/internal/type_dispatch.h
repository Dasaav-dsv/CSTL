#pragma once

#ifndef CSTL_TYPE_DISPATCH_H
#define CSTL_TYPE_DISPATCH_H

#include "../alloc.h"
#include "../type.h"

#include "type_ext.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct CSTL_SmallAllocFrame {
    void* pointer;
    char _buf[512];
    uintptr_t cookie;
} CSTL_SmallAllocFrame;

static inline void* CSTL_alloc(size_t count, CSTL_TypeCRef type, CSTL_Alloc* alloc) {
    size_t size      = count * type->size;
    size_t alignment = CSTL_type_alignment(type);

    if (alloc == NULL) {
#ifdef _CRTALLOCATOR
        return _aligned_malloc(size, alignment);
#else
        return aligned_alloc(alignment, size);
#endif
    }

    assert(alloc->aligned_alloc != NULL);

    return alloc->aligned_alloc(alloc->opaque, size, alignment);
}

static inline void CSTL_free(void* memory, size_t count, CSTL_TypeCRef type, CSTL_Alloc* alloc) {
    size_t size      = count * type->size;
    size_t alignment = CSTL_type_alignment(type);

    if (alloc == NULL) {
#ifdef _CRTALLOCATOR
        _aligned_free(memory);
#else
        free(memory);
#endif
        return;
    }

    assert(alloc->aligned_free != NULL);

    alloc->aligned_free(alloc->opaque, memory, size, alignment);
}

static inline void* CSTL_small_alloc(CSTL_SmallAllocFrame* frame, size_t count, CSTL_TypeCRef type, CSTL_Alloc* alloc) {
    size_t align_val  = CSTL_type_alignment(type) - 1;
    size_t align_size = type->size * count;

    uintptr_t frame_base = (uintptr_t)&frame->_buf;
    uintptr_t alloc_base = (frame_base + align_val) & ~align_val;

#ifndef NDEBUG
    frame->cookie = frame_base ^ (uintptr_t)type->size_rcp;
#endif

    if (frame_base + sizeof(frame->_buf) >= alloc_base + align_size
        && frame_base >= alloc_base) {
        frame->pointer = (void*)alloc_base;
    } else {
        frame->pointer = CSTL_alloc(count, type, alloc);
    }

    return frame->pointer;
}

static inline void CSTL_small_free(CSTL_SmallAllocFrame* frame, size_t count, CSTL_TypeCRef type, CSTL_Alloc* alloc) {
    size_t align_size = type->size * count;

    uintptr_t alloc_base  = (uintptr_t)frame->pointer;
    uintptr_t frame_base = (uintptr_t)&frame->_buf;

    assert(frame->cookie == (frame_base ^ (uintptr_t)type->size_rcp));

    if (frame_base + sizeof(frame->_buf) < alloc_base + align_size
        || frame_base < alloc_base) {
        CSTL_free(frame->pointer, count, type, alloc);
    }

    frame->pointer = NULL;
}

static inline void CSTL_move_from(void* new_instance, void* src, CSTL_TypeCRef type) {
    assert(new_instance != NULL);
    assert(src != NULL);

    if (type->move_from != NULL) {
        type->move_from(new_instance, src);
    } else {
        memmove(new_instance, src, type->size);
    }
}

static inline void CSTL_copy_from(void* new_instance, const void* src, CSTL_TypeCRef type) {
    assert(new_instance != NULL);
    assert(src != NULL);

    if (type->copy_from != NULL) {
        type->copy_from(new_instance, src);
    } else {
        memcpy(new_instance, src, type->size);
    }
}

static inline void CSTL_destroy_at(void* instance, CSTL_TypeCRef type) {
    assert(instance != NULL);

    if (type->destroy != NULL) {
        type->destroy(instance);
    } else {
        memset(instance, 0xDE, type->size);
    }
}

static inline bool CSTL_is_eq(const void* lhs, const void* rhs, CSTL_EqTypeCRef type) {
    assert(lhs != NULL);
    assert(rhs != NULL);

    if (type->is_eq != NULL) {
        return type->is_eq(lhs, rhs);
    } else {
        return memcmp(lhs, rhs, type->base.size) == 0;
    }
}

static inline bool CSTL_is_lt(const void* lhs, const void* rhs, CSTL_CompTypeCRef type) {
    assert(lhs != NULL);
    assert(rhs != NULL);

    if (type->is_lt != NULL) {
        return type->is_lt(lhs, rhs);
    } else {
        return memcmp(lhs, rhs, type->base.base.size) < 0;
    }
}

static inline size_t CSTL_fnv1a_append_bytes(size_t val, const unsigned char* const first,
    const size_t count) { // accumulate range [first, first + count) into partial FNV-1a hash val

    #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
    const size_t fnv_prime        = 1099511628211ULL;
    #else // ^^^ defined(_WIN64) / !defined(_WIN64) vvv
    const size_t fnv_prime        = 16777619U;
    #endif // ^^^ !defined(_WIN64) ^^^

    for (size_t idx = 0; idx < count; ++idx) {
        val ^= (size_t)first[idx];
        val *= fnv_prime;
    }

    return val;
}

static inline size_t CSTL_hash(const void* instance, CSTL_HashTypeCRef type) {
    assert(instance != NULL);

    if (type->hash != NULL) {
        return type->hash(instance);
    } else {
        #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
        const size_t fnv_offset_basis = 14695981039346656037ULL;
        #else // ^^^ defined(_WIN64) / !defined(_WIN64) vvv
        const size_t fnv_offset_basis = 2166136261U;
        #endif // ^^^ !defined(_WIN64) ^^^

        // FNV-1a based on STL <type_traits> `_Hash_representation`:    
        return CSTL_fnv1a_append_bytes(fnv_offset_basis,
            (const unsigned char*)instance, type->base.base.size);
    }
}

#endif
