#pragma once

#ifndef CSTL_VECTOR_DBG_H
#define CSTL_VECTOR_DBG_H

#include "type_ext.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../vector.h"

#ifndef NDEBUG
static inline bool CSTL_is_vector_aligned(CSTL_VectorCtx context) {
    CSTL_VectorCRef instance = (CSTL_VectorCRef)context.instance;
    CSTL_TypeCRef type          = (CSTL_TypeCRef)context.type;

    uintptr_t first_address = (uintptr_t)instance->first;
    uintptr_t last_address  = (uintptr_t)instance->last;
    uintptr_t end_address   = (uintptr_t)instance->end;

    uintptr_t size     = last_address - first_address;
    uintptr_t capacity = end_address - first_address;

    return CSTL_is_address_aligned(first_address, type)
        && CSTL_is_address_aligned(last_address, type)
        && CSTL_is_address_aligned(end_address, type)
        && CSTL_type_remainder_by_rcp(size, type) == 0
        && CSTL_type_remainder_by_rcp(capacity, type) == 0;
}

static inline bool CSTL_verify_address_in_vector(CSTL_VectorCRef instance, uintptr_t address) {
    uintptr_t low_bound  = (uintptr_t)instance->first;
    uintptr_t high_bound = (uintptr_t)instance->last;

    return low_bound <= address && address <= high_bound;
}

#define CSTL_verify_iterator(iterator) \
    CSTL_verify_address_in_vector((iterator)->owner, (uintptr_t)(iterator)->pointer)

#define CSTL_set_iterator_owner(iterator, owned_by) \
    (iterator)->owner = (owned_by)
#else
#define CSTL_verify_iterator(iterator) (void)(iterator)
#define CSTL_set_iterator_owner(iterator, owned_by) (void)(iterator); (void)(owned_by)
#endif

size_t ndbg_vector_size(CSTL_VectorCtx context);
size_t ndbg_vector_capacity(CSTL_VectorCtx context);
size_t ndbg_vector_max_size(CSTL_TypeCRef type);

CSTL_VectorIter ndbg_make_iterator(CSTL_VectorCtx context, const void* pointer);
CSTL_VectorIter ndbg_vector_begin(CSTL_VectorCtx context);
CSTL_VectorIter ndbg_vector_end(CSTL_VectorCtx context);

CSTL_VectorIter ndbg_vector_iterator_add(CSTL_VectorIter iterator, ptrdiff_t n);
CSTL_VectorIter ndbg_vector_iterator_sub(CSTL_VectorIter iterator, ptrdiff_t n);

void* ndbg_vector_iterator_deref(CSTL_VectorIter iterator);
void* ndbg_vector_iterator_index(CSTL_VectorIter iterator, ptrdiff_t n);

ptrdiff_t ndbg_vector_iterator_distance(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

bool ndbg_vector_iterator_eq(CSTL_VectorIter lhs, CSTL_VectorIter rhs);
bool ndbg_vector_iterator_lt(CSTL_VectorIter lhs, CSTL_VectorIter rhs);

#endif
