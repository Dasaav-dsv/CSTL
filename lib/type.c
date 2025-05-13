#include "type.h"
#include "internal/type_ext.h"

#include <stdint.h>
#include <limits.h>

CSTL_Type CSTL_define_type(size_t size, size_t alignment) {
    if (size == 0 || size > (uintptr_t)INTPTR_MAX) {
        // `size` is zero or too big.
        return NULL;
    }

    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        // `alignment` is zero or not a power of 2.
        return NULL;
    }

    if ((size & (alignment - 1)) != 0) {
        // `size` isn't a multiple of `alignment`.
        return NULL;
    }

    intptr_t isize  = (intptr_t)size;
    intptr_t ialign = (intptr_t)alignment;

    if (isize & ialign) {
        return (CSTL_Type)isize;
    } else {
        return (CSTL_Type)-(isize | ialign);
    }
}

size_t CSTL_sizeof_type(CSTL_Type type) {
    return CSTL_type_size(type);
}

size_t CSTL_alignof_type(CSTL_Type type) {
    return CSTL_type_alignment(type);
}
