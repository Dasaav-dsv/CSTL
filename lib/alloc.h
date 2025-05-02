#pragma once

#ifndef CSTL_ALLOC_H
#define CSTL_ALLOC_H

#if defined(__cplusplus)
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

/**
 * Opaque memory allocator interface.
 * 
 * The caller is responsible for making sure an allocator owns and is compatible
 * for freeing memory of a given container.
 * 
 * `aligned_alloc` and `aligned_free` must not be null, while `allocator` can
 * be used to pass any opaque data (if any) directly to the bound functions.
 * 
 * A strict alignment requirement may be imposed by allocated types.
 * 
 */
typedef struct CSTL_Alloc {
    /**
     * Pointer to opaque data, passed to bound memory allocation functions.
     * 
     */
    void* opaque;

    /**
     * Must return a memory block of at least `size` bytes that is at least
     * as aligned as `alignment`
     * 
     */
    void* (*aligned_alloc)(void* opaque, size_t size, size_t alignment);

    /**
     * Must be able to free previously allocated memory.
     * 
     * The caller is responsible for allocator compatibility.
     * 
     */
    void (*aligned_free)(void* opaque, void* memory, size_t size, size_t alignment);
} CSTL_Alloc;

#if defined(__cplusplus)
}
#endif

#endif