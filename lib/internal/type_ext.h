#pragma once

#ifndef CSTL_TYPE_EXT_H
#define CSTL_TYPE_EXT_H

#include "../type.h"

#include <stddef.h>
#include <stdint.h>

inline size_t CSTL_alignof_type(CSTL_Type type) {
    intptr_t packed  = (intptr_t)type;
    intptr_t npacked = -packed;

    return (size_t)(packed & npacked);
}

inline size_t CSTL_sizeof_type(CSTL_Type type) {
    size_t alignment = CSTL_alignof_type(type);
    intptr_t packed  = (intptr_t)type;

    if (packed < 0) {
        return (size_t)packed ^ alignment;
    } else {
        return (size_t)packed;
    }
}

#endif
