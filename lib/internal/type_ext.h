#pragma once

#ifndef CSTL_TYPE_EXT_H
#define CSTL_TYPE_EXT_H

#include "../type.h"

#include <stddef.h>
#include <stdint.h>

static inline size_t CSTL_type_alignment(CSTL_Type type) {
    intptr_t packed  = (intptr_t)type;
    intptr_t npacked = -packed;

    return (size_t)(packed & npacked);
}

static inline size_t CSTL_type_size(CSTL_Type type) {
    size_t alignment = CSTL_type_alignment(type);
    intptr_t packed  = (intptr_t)type;

    if (packed < 0) {
        return ((size_t)-packed) ^ alignment;
    } else {
        return (size_t)packed;
    }
}

#endif
