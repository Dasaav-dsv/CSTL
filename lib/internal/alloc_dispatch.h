#pragma once

#ifndef CSTL_ALLOC_DISPATCH_H
#define CSTL_ALLOC_DISPATCH_H

#include "../alloc.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct CSTL_SmallAllocFrame {
    void* pointer;
    char _buf[512];
    uintptr_t cookie;
} CSTL_SmallAllocFrame;

static inline void* CSTL_allocate(size_t size, size_t alignment, CSTL_Alloc* alloc) {
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

static inline void CSTL_free(void* memory, size_t size, size_t alignment, CSTL_Alloc* alloc) {
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

static inline void* CSTL_small_alloc(CSTL_SmallAllocFrame* frame, size_t size, size_t alignment, CSTL_Alloc* alloc, uintptr_t cookie) {
    size_t align_val  = alignment - 1;

    uintptr_t frame_base = (uintptr_t)&frame->_buf;
    uintptr_t alloc_base = (frame_base + align_val) & ~align_val;

#ifndef NDEBUG
    frame->cookie = frame_base ^ (uintptr_t)cookie;
#else
    (void)cookie;
#endif

    if (frame_base + sizeof(frame->_buf) >= alloc_base + size
        && frame_base >= alloc_base) {
        frame->pointer = (void*)alloc_base;
    } else {
        frame->pointer = CSTL_allocate(size, alignment, alloc);
    }

    return frame->pointer;
}

static inline void CSTL_small_free(CSTL_SmallAllocFrame* frame, size_t size, size_t alignment, CSTL_Alloc* alloc, uintptr_t cookie) {
    uintptr_t alloc_base  = (uintptr_t)frame->pointer;
    uintptr_t frame_base = (uintptr_t)&frame->_buf;

#ifndef NDEBUG
    assert(frame->cookie == (frame_base ^ cookie));
#else
    (void)cookie;
#endif

    if (frame_base + sizeof(frame->_buf) < alloc_base + size
        || frame_base < alloc_base) {
        CSTL_free(frame->pointer, size, alignment, alloc);
    }

    frame->pointer = NULL;
}

#endif
