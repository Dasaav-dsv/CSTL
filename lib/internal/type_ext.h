#pragma once

#ifndef CSTL_TYPE_EXT_H
#define CSTL_TYPE_EXT_H

#include "../type.h"
#include "rcp_division.h"

#define CSTL_TYPE_INVALID 0b0000000000000001

#if defined(__cplusplus)
#include <cstddef>
#include <cstdint>
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

static inline size_t CSTL_type_alignment(CSTL_TypeCRef type) {
    return (size_t)1 << type->align_lg;
}

static inline bool CSTL_is_address_aligned(uintptr_t address, CSTL_TypeCRef type) {
    return (address & (CSTL_type_alignment(type) - 1)) == 0;
}

static inline size_t CSTL_type_divide_by_rcp(size_t dividend, CSTL_TypeCRef type) {
    return CSTL_divide_by_rcp(dividend, type->size_rcp, type->size_rcp_sh);
}

static inline ptrdiff_t CSTL_type_divide_by_rcp_signed(ptrdiff_t dividend, CSTL_TypeCRef type) {
    return CSTL_divide_by_rcp_signed(dividend, type->size_rcp, type->size_rcp_sh);
}

static inline size_t CSTL_type_remainder_by_rcp(size_t dividend, CSTL_TypeCRef type) {
    return CSTL_remainder_by_rcp(dividend, type->size_rcp, type->size_rcp_sh, type->size);
}

static inline void* CSTL_pointer_add(const void* pointer, ptrdiff_t n, CSTL_TypeCRef type) {
    return (void*)((intptr_t)pointer + (intptr_t)type->size * n);
}

static inline void* CSTL_pointer_sub(const void* pointer, ptrdiff_t n, CSTL_TypeCRef type) {
    return CSTL_pointer_add(pointer, -n, type);
}

static inline ptrdiff_t CSTL_pointer_distance(const void* lhs, const void* rhs, CSTL_TypeCRef type) {
    return CSTL_type_divide_by_rcp_signed((intptr_t)rhs - (intptr_t)lhs, type);
}

#if defined(__cplusplus)
}
#endif

#endif
