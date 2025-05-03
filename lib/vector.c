#include "vector.h"
#include "internal/vector_dbg.h"

#include "internal/type_dispatch.h"
#include "internal/type_ext.h"

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

void* CSTL_fill(void* first, void* last, const void* value, CSTL_TypeCRef type) {
    while ((uintptr_t)first < (uintptr_t)last) {
        CSTL_type_destroy_at(first, type);
        CSTL_type_copy_from(first, value, type);
        first = CSTL_pointer_add(first, 1, type);
    }
    return first;
}

void* CSTL_fill_n(void* first, size_t count, const void* value, CSTL_TypeCRef type) {
    for (; count != 0; --count) {
        CSTL_type_destroy_at(first, type);
        CSTL_type_copy_from(first, value, type);
        first = CSTL_pointer_add(first, 1, type);
    }
    return first;
}

void* CSTL_uninitialized_fill(void* first, void* last, const void* value, CSTL_TypeCRef type) {
    while ((uintptr_t)first < (uintptr_t)last) {
        CSTL_type_copy_from(first, value, type);
        first = CSTL_pointer_add(first, 1, type);
    }
    return first;
}

void* CSTL_uninitialized_fill_n(void* first, size_t count, const void* value, CSTL_TypeCRef type) {
    for (; count != 0; --count) {
        CSTL_type_copy_from(first, value, type);
        first = CSTL_pointer_add(first, 1, type);
    }
    return first;
}

void* CSTL_copy(const void* first, const void* last, void* dest, CSTL_TypeCRef type) {
    while ((uintptr_t)first < (uintptr_t)last) {
        CSTL_type_destroy_at(dest, type);
        CSTL_type_copy_from(dest, first, type);
        first = CSTL_pointer_add(first, 1, type);
        dest  = CSTL_pointer_add(dest, 1, type);
    }
    return dest;
}

void* CSTL_copy_n(const void* first, size_t count, void* dest, CSTL_TypeCRef type) {
    for (; count != 0; --count) {
        CSTL_type_destroy_at(dest, type);
        CSTL_type_copy_from(dest, first, type);
        first = CSTL_pointer_add(first, 1, type);
        dest  = CSTL_pointer_add(dest, 1, type);
    }
    return dest;
}

void* CSTL_uninitialized_copy_n(const void* first, size_t count, void* dest, CSTL_TypeCRef type) {
    for (; count != 0; --count) {
        CSTL_type_copy_from(dest, first, type);
        first = CSTL_pointer_add(first, 1, type);
        dest  = CSTL_pointer_add(dest, 1, type);
    }
    return dest;
}

void* CSTL_copy_backwards(const void* first, const void* last, void* dest, CSTL_TypeCRef type) {
    while ((uintptr_t)first < (uintptr_t)last) {
        last = CSTL_pointer_sub(last, 1, type);
        dest = CSTL_pointer_sub(dest, 1, type);
        CSTL_type_destroy_at(dest, type);
        CSTL_type_copy_from(dest, last, type);
    }
    return dest;
}

void* CSTL_move(void* first, void* last, void* dest, CSTL_TypeCRef type) {
    while ((uintptr_t)first < (uintptr_t)last) {
        CSTL_type_destroy_at(dest, type);
        CSTL_type_move_from(dest, first, type);
        first = CSTL_pointer_add(first, 1, type);
        dest  = CSTL_pointer_add(dest, 1, type);
    }
    return dest;
}

void* CSTL_move_n(void* first, size_t count, void* dest, CSTL_TypeCRef type) {
    for (; count != 0; --count) {
        CSTL_type_destroy_at(dest, type);
        CSTL_type_move_from(dest, first, type);
        first = CSTL_pointer_add(first, 1, type);
        dest  = CSTL_pointer_add(dest, 1, type);
    }
    return dest;
}

void* CSTL_uninitialized_move_n(void* first, size_t count, void* dest, CSTL_TypeCRef type) {
    for (; count != 0; --count) {
        CSTL_type_move_from(dest, first, type);
        first = CSTL_pointer_add(first, 1, type);
        dest  = CSTL_pointer_add(dest, 1, type);
    }
    return dest;
}

void* CSTL_move_backwards(void* first, void* last, void* dest, CSTL_TypeCRef type) {
    while ((uintptr_t)first < (uintptr_t)last) {
        last = CSTL_pointer_sub(last, 1, type);
        dest = CSTL_pointer_sub(dest, 1, type);
        CSTL_type_destroy_at(dest, type);
        CSTL_type_move_from(dest, last, type);
    }
    return dest;
}

void* CSTL_destroy(void* first, void* last, CSTL_TypeCRef type) {
    while ((uintptr_t)first < (uintptr_t)last) {
        CSTL_type_destroy_at(first, type);
        first = CSTL_pointer_add(first, 1, type);
    }
    return first;
}

void* CSTL_destroy_n(void* first, size_t count, CSTL_TypeCRef type) {
    for (; 0 < count; --count) {
        CSTL_type_destroy_at(first, type);
        first = CSTL_pointer_add(first, 1, type);
    }
    return first;
}

static inline size_t CSTL_vector_calculate_growth(CSTL_VectorCtx context, size_t new_size) {
    CSTL_TypeCRef type = (CSTL_TypeCRef)context.type;

    size_t old_capacity = ndbg_vector_capacity(context);
    size_t max          = ndbg_vector_max_size(type);

    if (old_capacity > max - old_capacity / 2) {
        return max; // geometric growth would overflow
    }

    size_t geometric = old_capacity + old_capacity / 2;

    if (geometric < new_size) {
        return new_size; // geometric growth would be insufficient
    }

    return geometric; // geometric growth is sufficient
}

CSTL_VectorVal CSTL_vector_new_with_capacity(size_t capacity, CSTL_TypeCRef type, CSTL_Alloc* alloc) {
    void* first = CSTL_type_allocate(capacity, type, alloc);

    CSTL_VectorVal tmp = {
        .first = first,
        .last  = first,
        .end   = CSTL_pointer_add(first, (ptrdiff_t)capacity, type)
    };

    return tmp;
}

void CSTL_vector_tidy(CSTL_VectorCtx context, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    if (instance->first) {
        size_t size     = ndbg_vector_size(context);
        size_t capacity = ndbg_vector_capacity(context);

        CSTL_destroy_n(instance->first, size, type);
        CSTL_type_free(instance->first, capacity, type, alloc);

        instance->first = NULL;
        instance->last  = NULL;
        instance->end   = NULL;
    }
}

void CSTL_vector_replace(CSTL_VectorCtx context, CSTL_Alloc* alloc, CSTL_VectorVal new) {
    CSTL_vector_tidy(context, alloc);
    *(CSTL_VectorRef)context.instance = new;
}

bool CSTL_vector_clear_and_reserve_geometric(CSTL_VectorCtx context, size_t new_size, CSTL_Alloc* alloc) {
    CSTL_TypeCRef type = (CSTL_TypeCRef)context.type;

    if (new_size > ndbg_vector_max_size(type)) {
        return false;
    }

    size_t new_capacity = CSTL_vector_calculate_growth(context, new_size);

    CSTL_VectorVal tmp = CSTL_vector_new_with_capacity(new_capacity, type, alloc);
    CSTL_vector_replace(context, alloc, tmp);

    return true;
}

void CSTL_vector_construct(CSTL_VectorCtx context) {
    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    
    CSTL_VectorVal* instance = (CSTL_VectorVal*)context.instance;

    if (instance == NULL) {
        return;
    }

    instance->first = NULL;
    instance->last = NULL;
    instance->end = NULL;
}

void CSTL_vector_destroy(CSTL_VectorCtx context, CSTL_Alloc* alloc) {
    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    CSTL_vector_tidy(context, alloc);
}

bool CSTL_vector_assign(CSTL_VectorCtx context, size_t new_size, const void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(!CSTL_verify_address_in_vector(instance, (uintptr_t)value));

    size_t old_capacity = ndbg_vector_capacity(context);
    if (new_size > old_capacity) { // reallocate
        if (!CSTL_vector_clear_and_reserve_geometric(context, new_size, alloc)) {
            return false;
        }

        instance->last = CSTL_uninitialized_fill_n(instance->first, new_size, value, type);

        return true;
    }

    void* old_first = (void*)instance->first;
    void* old_last  = (void*)instance->last;
    void* new_last  = CSTL_pointer_add(old_first, (ptrdiff_t)new_size, type);

    if ((uintptr_t)old_last < (uintptr_t)new_last) {
        CSTL_fill(old_first, old_last, value, type);

        instance->last = CSTL_uninitialized_fill(old_last, new_last, value, type);
    } else {
        instance->last = CSTL_fill(old_first, new_last, value, type);

        CSTL_destroy(instance->last, old_last, type);
    }

    return true;
}

void CSTL_vector_copy_assign_counted_range(CSTL_VectorCtx context, const void* first, size_t new_size, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    // assign elements from counted range [first, first + new_size)
    size_t old_capacity = ndbg_vector_capacity(context);
    if (new_size > old_capacity) {
        CSTL_vector_clear_and_reserve_geometric(context, new_size, alloc);

        instance->last = CSTL_uninitialized_copy_n(first, new_size, instance->first, type);

        return;
    }

    size_t old_size = ndbg_vector_size(context);
    if (new_size > old_size) {
        CSTL_copy_n(first, old_size, instance->first, type);
        first = CSTL_pointer_add(first, (ptrdiff_t)old_size, type);

        instance->last = CSTL_uninitialized_copy_n(first, new_size - old_size, instance->last, type);
    } else {
        instance->last = CSTL_copy_n(first, new_size, instance->first, type);

        CSTL_destroy_n(instance->last, old_size - new_size, type);
    }
}

void CSTL_vector_move_assign_counted_range(CSTL_VectorCtx context, void* first, size_t new_size, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    size_t old_capacity = ndbg_vector_capacity(context);
    if (new_size > old_capacity) {
        CSTL_vector_clear_and_reserve_geometric(context, new_size, alloc);

        if (type->use_move_from) {
            instance->last = CSTL_uninitialized_move_n(first, new_size, instance->first, type);
        } else {
            instance->last = CSTL_uninitialized_copy_n(first, new_size, instance->first, type);
        }

        return;
    }

    size_t old_size = ndbg_vector_size(context);
    if (new_size > old_size) {
        if (type->use_move_from) {
            CSTL_move_n(first, old_size, instance->first, type);
            first = CSTL_pointer_add(first, (ptrdiff_t)old_size, type);
    
            instance->last = CSTL_uninitialized_move_n(first, new_size - old_size, instance->last, type);
        } else {
            CSTL_copy_n(first, old_size, instance->first, type);
            first = CSTL_pointer_add(first, (ptrdiff_t)old_size, type);
    
            instance->last = CSTL_uninitialized_copy_n(first, new_size - old_size, instance->last, type);
        }
    } else {
        if (type->use_move_from) {
            instance->last = CSTL_move_n(first, new_size, instance->first, type);
        } else {
            instance->last = CSTL_copy_n(first, new_size, instance->first, type);
        }

        CSTL_destroy_n(instance->last, old_size - new_size, type);
    }
}

bool CSTL_vector_assign_continuous_range(CSTL_VectorCtx context, size_t new_size, const void* first, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance; (void)instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(!CSTL_verify_address_in_vector(instance, (uintptr_t)first));
    assert(!CSTL_verify_address_in_vector(instance, (uintptr_t)first + new_size * type->size));

    if (new_size > CSTL_vector_max_size(type)) {
        return false;
    }

    CSTL_vector_copy_assign_counted_range(context, first, new_size, alloc);

    return true;
}

void CSTL_vector_copy(CSTL_VectorCtx context, CSTL_Alloc* alloc, CSTL_VectorCtx other_context, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    CSTL_VectorRef instance       = (CSTL_VectorRef)context.instance;
    CSTL_VectorRef other_instance = (CSTL_VectorRef)other_context.instance;

    if (instance == other_instance) {
        return;
    }

    if (propagate_alloc) {
        if (alloc != other_alloc) {
            CSTL_vector_tidy(context, alloc);
            alloc = other_alloc;
        }
    }

    size_t new_size = ndbg_vector_size(other_context);
    CSTL_vector_copy_assign_counted_range(context, other_instance->first, new_size, alloc);
}

void CSTL_vector_move(CSTL_VectorCtx context, CSTL_Alloc* alloc, CSTL_VectorCtx other_context, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    CSTL_VectorRef instance       = (CSTL_VectorRef)context.instance;
    CSTL_VectorRef other_instance = (CSTL_VectorRef)other_context.instance;

    if (instance == other_instance) {
        return;
    }

    if (!propagate_alloc) {
        if (alloc != other_alloc) {
            size_t new_size = ndbg_vector_size(other_context);
            CSTL_vector_move_assign_counted_range(context, other_instance->first, new_size, alloc);

            return;
        }
    }

    CSTL_vector_replace(context, alloc, *other_instance);

    other_instance->first = NULL;
    other_instance->last  = NULL;
    other_instance->end   = NULL;
}

void CSTL_vector_swap(CSTL_VectorCtx context, CSTL_VectorCtx other_context) {
    CSTL_VectorRef instance       = (CSTL_VectorRef)context.instance;
    CSTL_VectorRef other_instance = (CSTL_VectorRef)other_context.instance;

    void* tmp_first = instance->first;
    void* tmp_last  = instance->last;
    void* tmp_end   = instance->end;

    instance->first = other_instance->first;
    instance->last  = other_instance->last;
    instance->end   = other_instance->end;

    other_instance->first = tmp_first;
    other_instance->last  = tmp_last;
    other_instance->end   = tmp_end;
}

void* CSTL_vector_index(CSTL_VectorCtx context, size_t pos) {
    CSTL_VectorCRef instance = (CSTL_VectorCRef)context.instance;
    CSTL_TypeCRef type       = (CSTL_TypeCRef)context.type;

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));
    assert(pos < ndbg_vector_size(context));

    return CSTL_pointer_add(instance->first, (ptrdiff_t)pos, type);
}

const void* CSTL_vector_const_index(CSTL_VectorCtx context, size_t pos) {
    return CSTL_vector_index(context, pos);
}

void* CSTL_vector_get_at(CSTL_VectorCtx context, size_t pos) {
    CSTL_VectorCRef instance = (CSTL_VectorCRef)context.instance;
    CSTL_TypeCRef type       = (CSTL_TypeCRef)context.type;

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    uintptr_t address        = (uintptr_t)instance->first;
    uintptr_t address_offset = (uintptr_t)pos * type->size;
    uintptr_t new_address    = address + address_offset;

    if (new_address >= (uintptr_t)instance->last) {
        return NULL;
    }

    return (void*)new_address;
}

const void* CSTL_vector_const_get_at(CSTL_VectorCtx context, size_t pos) {
    return (const void*)CSTL_vector_get_at(context, pos);
}

void* CSTL_vector_front(CSTL_VectorCtx context) {
    assert(!CSTL_vector_empty(context));
    return context.instance->first;
}

const void* CSTL_vector_const_front(CSTL_VectorCtx context)  {
    assert(!CSTL_vector_empty(context));
    return context.instance->first;
}

void* CSTL_vector_back(CSTL_VectorCtx context)  {
    assert(!CSTL_vector_empty(context));
    return CSTL_pointer_sub(context.instance->last, 1, context.type);
}

const void* CSTL_vector_const_back(CSTL_VectorCtx context) {
    assert(!CSTL_vector_empty(context));
    return CSTL_pointer_sub(context.instance->last, 1, context.type);
}

void* CSTL_vector_data(CSTL_VectorCtx context) {
    return context.instance->first;
}

const void* CSTL_vector_const_data(CSTL_VectorCtx context) {
    return context.instance->first;
}

CSTL_VectorIter CSTL_vector_begin(CSTL_VectorCtx context) {
    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    return ndbg_vector_begin(context);
}

CSTL_VectorIter CSTL_vector_end(CSTL_VectorCtx context) {
    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    return ndbg_vector_end(context);
}

CSTL_VectorIter CSTL_vector_iterator_add(CSTL_VectorIter iterator, ptrdiff_t n) {
    assert(iterator.pointer != NULL);
    CSTL_verify_iterator(&iterator);
    CSTL_VectorIter tmp = ndbg_vector_iterator_add(iterator, n);
    CSTL_verify_iterator(&tmp);
    return tmp;
}

CSTL_VectorIter CSTL_vector_iterator_sub(CSTL_VectorIter iterator, ptrdiff_t n) {
    return CSTL_vector_iterator_add(iterator, -n);
}

void* CSTL_vector_iterator_deref(CSTL_VectorIter iterator) {
    assert(iterator.pointer != NULL);
    CSTL_verify_iterator(&iterator);
    return ndbg_vector_iterator_deref(iterator);
}

void* CSTL_vector_iterator_index(CSTL_VectorIter iterator, ptrdiff_t n) {
    assert(iterator.pointer != NULL);
    CSTL_verify_iterator(&iterator);
    return ndbg_vector_iterator_index(iterator, n);
}

ptrdiff_t CSTL_vector_iterator_distance(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    CSTL_verify_iterator(&lhs);
    CSTL_verify_iterator(&rhs);
    assert(lhs.owner == rhs.owner);
    return ndbg_vector_iterator_distance(lhs, rhs);
}

bool CSTL_vector_iterator_eq(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    CSTL_verify_iterator(&lhs);
    CSTL_verify_iterator(&rhs);
    assert(lhs.owner == rhs.owner);
    return ndbg_vector_iterator_eq(lhs, rhs);
}

bool CSTL_vector_iterator_lt(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    CSTL_verify_iterator(&lhs);
    CSTL_verify_iterator(&rhs);
    assert(lhs.owner == rhs.owner);
    return ndbg_vector_iterator_lt(lhs, rhs);
}

bool CSTL_vector_empty(CSTL_VectorCtx context) {
    return context.instance->first == context.instance->last;
}

size_t CSTL_vector_size(CSTL_VectorCtx context) {
    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    return ndbg_vector_size(context);
}

size_t CSTL_vector_capacity(CSTL_VectorCtx context) {
    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    return ndbg_vector_capacity(context);
}

size_t CSTL_vector_max_size(CSTL_TypeCRef type) {
    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    return ndbg_vector_max_size(type);
}

void CSTL_vector_reallocate(CSTL_VectorCtx context, size_t new_capacity, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    CSTL_VectorVal tmp = CSTL_vector_new_with_capacity(new_capacity, context.type, alloc);

    size_t size = ndbg_vector_size(context);
    
    if (context.type->use_move_from) {
        tmp.last = CSTL_uninitialized_move_n(instance->first, size, tmp.first, type);
    } else {
        tmp.last = CSTL_uninitialized_copy_n(instance->first, size, tmp.first, type);
    }

    CSTL_vector_replace(context, alloc, tmp);
}

bool CSTL_vector_reserve(CSTL_VectorCtx context, size_t new_capacity, CSTL_Alloc* alloc) {
    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    // increase capacity to new_capacity (without geometric growth)
    if (new_capacity > ndbg_vector_capacity(context)) { // something to do (reserve() never shrinks)
        if (new_capacity > ndbg_vector_max_size(context.type)) {
            return false;
        }

        CSTL_vector_reallocate(context, new_capacity, alloc);
    }

    return true;
}

void CSTL_vector_shrink_to_fit(CSTL_VectorCtx context, CSTL_Alloc* alloc) { // reduce capacity to size
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;

    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    const void* old_last = instance->last;
    if (old_last != instance->end) { // something to do
        const void* old_first = instance->first;
        if (old_first == old_last) {
            CSTL_vector_tidy(context, alloc);
        } else {
            size_t new_capacity = ndbg_vector_size(context);
            CSTL_vector_reallocate(context, new_capacity, alloc);
        }
    }
}

void CSTL_vector_clear(CSTL_VectorCtx context) { // erase all
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    void* first = (void*)instance->first;
    void* last  = (void*)instance->last;

    if (first == last) {
        return;
    }

    size_t size = ndbg_vector_size(context);
    CSTL_destroy_n(first, size, type);

    instance->last = first;
}

CSTL_VectorIter CSTL_vector_insert(CSTL_VectorCtx context, CSTL_VectorIter where, size_t count, const void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    CSTL_verify_iterator(&where);
    assert(where.owner == context.instance);

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    void* where_pointer    = (void*)where.pointer;
    ptrdiff_t where_off    = CSTL_pointer_distance(instance->first, where_pointer, type);
    size_t unused_capacity = CSTL_pointer_distance(instance->end, instance->last, type);

    if (count == 0) { // nothing to do, avoid invalidating iterators
    } else if (count > unused_capacity) {
        size_t old_size = CSTL_vector_size(context);

        if (count > CSTL_vector_max_size(type) - old_size) {
            return ndbg_vector_end(context);
        }

        size_t new_size     = old_size + count;
        size_t new_capacity = CSTL_vector_calculate_growth(context, new_size);

        CSTL_VectorVal tmp = CSTL_vector_new_with_capacity(new_capacity, type, alloc);
        tmp.last = CSTL_pointer_add(tmp.first, (ptrdiff_t)new_size, type);

        void* constructed_first = CSTL_pointer_add(tmp.first, where_off, type);
        void* constructed_last  = CSTL_pointer_add(constructed_first, (ptrdiff_t)count, type);

        CSTL_uninitialized_fill_n(constructed_first, count, value, type);
    
        if (where_pointer == instance->last) {
            if (type->use_move_from) {
                CSTL_uninitialized_move_n(instance->first, old_size, tmp.first, type);
            } else {
                CSTL_uninitialized_copy_n(instance->first, old_size, tmp.first, type);
            }
        } else {
            if (type->use_move_from) {
                CSTL_uninitialized_move_n(instance->first, (size_t)where_off, tmp.first, type);
                CSTL_uninitialized_move_n(where_pointer, old_size - (size_t)where_off, constructed_last, type);
            } else {
                CSTL_uninitialized_copy_n(instance->first, (size_t)where_off, tmp.first, type);
                CSTL_uninitialized_copy_n(where_pointer, old_size - (size_t)where_off, constructed_last, type);
            }
        }

        CSTL_vector_replace(context, alloc, tmp);

        where_pointer = constructed_first;
    } else {
        void* old_last = (void*)instance->last;

        size_t affected_elements = (size_t)CSTL_pointer_distance(where_pointer, old_last, type);

        if (count > affected_elements) {
            instance->last = CSTL_uninitialized_fill_n(old_last, count - affected_elements, value, type);

            if (type->use_move_from) {
                instance->last = CSTL_uninitialized_move_n(where_pointer, affected_elements, instance->last, type);
            } else {
                instance->last = CSTL_uninitialized_copy_n(where_pointer, affected_elements, instance->last, type);
            }

            CSTL_fill_n(where_pointer, affected_elements, value, type);
        } else {
            void* new_mid = CSTL_pointer_sub(instance->last, count, type);

            if (type->use_move_from) {
                instance->last = CSTL_uninitialized_move_n(new_mid, count, old_last, type);
                CSTL_move_backwards(where_pointer, new_mid, old_last, type);
            } else {
                instance->last = CSTL_uninitialized_copy_n(new_mid, count, old_last, type);
                CSTL_copy_backwards(where_pointer, new_mid, old_last, type);
            }

            CSTL_fill_n(where_pointer, count, value, type);
        }
    }

    return ndbg_make_iterator(context, where_pointer);
}

void* CSTL_vector_move_back_with_unused_capacity(CSTL_VectorCtx context, void* value) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    void* last = instance->last;
    instance->last = CSTL_pointer_add(last, 1, type);
    if (type->use_move_from) {
        CSTL_type_move_from(last, value, type);
    } else {
        CSTL_type_copy_from(last, value, type);
    }
    return last;
}

void* CSTL_vector_move_emplace_reallocate(CSTL_VectorCtx context, void* where_pointer, void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    ptrdiff_t where_off = CSTL_pointer_distance(instance->first, where_pointer, type);
    size_t old_size  = ndbg_vector_size(context);

    if (old_size == ndbg_vector_max_size(type)) {
        return instance->last;
    }

    size_t new_size     = old_size + 1;
    size_t new_capacity = CSTL_vector_calculate_growth(context, new_size);

    CSTL_VectorVal tmp = CSTL_vector_new_with_capacity(new_capacity, type, alloc);
    tmp.last = CSTL_pointer_add(tmp.first, (ptrdiff_t)new_size, type);

    void* constructed_first = CSTL_pointer_add(tmp.first, where_off, type);
    void* constructed_last  = CSTL_pointer_add(constructed_first, 1, type);

    if (type->use_move_from) {
        CSTL_type_move_from(constructed_first, value, type);
    } else {
        CSTL_type_copy_from(constructed_first, value, type);
    }

    void* old_first = (void*)instance->last;
    void* old_mid   = CSTL_pointer_add(old_first, where_off, type);

    if (where_pointer == instance->last) {
        if (type->use_move_from) {
            CSTL_uninitialized_move_n(old_first, old_size, tmp.first, type);
        } else {
            CSTL_uninitialized_copy_n(old_first, old_size, tmp.first, type);
        }
    } else {
        if (type->use_move_from) {
            CSTL_uninitialized_move_n(old_first, (size_t)where_off, tmp.first, type);
            CSTL_uninitialized_move_n(old_mid, old_size - (size_t)where_off, constructed_last, type);
        } else {
            CSTL_uninitialized_copy_n(old_first, (size_t)where_off, tmp.first, type);
            CSTL_uninitialized_copy_n(old_mid, old_size - (size_t)where_off, constructed_last, type);
        }
    }

    CSTL_vector_replace(context, alloc, tmp);

    return constructed_first;
}

CSTL_VectorIter CSTL_vector_emplace(CSTL_VectorCtx context, CSTL_VectorIter where, void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    CSTL_verify_iterator(&where);
    assert(where.owner == context.instance);

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    void* where_pointer = (void*)where.pointer;
    void* old_last      = instance->last;

    if (old_last != instance->end) {
        if (where_pointer == old_last) {
            CSTL_vector_move_back_with_unused_capacity(context, value);
        } else {
            CSTL_SmallAllocFrame frame;
            if (type->use_move_from) {
                void* tmp = CSTL_type_small_alloc(&frame, 1, type, alloc);
                CSTL_type_move_from(tmp, value, type); // handle aliasing
                value = tmp;
            }

            instance->last = CSTL_pointer_add(old_last, 1, type);
            void* old_back = CSTL_pointer_sub(old_last, 1, type);

            if (type->use_move_from) {
                CSTL_type_move_from(old_last, old_back, type);
                CSTL_move_backwards(where_pointer, old_back, old_last, type);
                CSTL_type_move_from(where_pointer, value, type);
            } else {
                CSTL_type_copy_from(old_last, old_back, type);
                CSTL_copy_backwards(where_pointer, old_back, old_last, type);
                CSTL_type_copy_from(where_pointer, value, type);
            }

            if (type->use_move_from) {
                CSTL_type_destroy_at(value, type);
                CSTL_type_small_free(&frame, 1, type, alloc);
            }
        }
    } else {
        where_pointer = CSTL_vector_move_emplace_reallocate(context, where_pointer, value, alloc);
    }

    return ndbg_make_iterator(context, where_pointer);
}

void* CSTL_vector_copy_back_with_unused_capacity(CSTL_VectorCtx context, const void* value) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    void* last = instance->last;
    instance->last = CSTL_pointer_add(last, 1, type);
    CSTL_type_copy_from(last, value, type);
    return last;
}

void* CSTL_vector_copy_emplace_reallocate(CSTL_VectorCtx context, void* where_pointer, const void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    ptrdiff_t where_off = CSTL_pointer_distance(instance->first, where_pointer, type);
    size_t old_size  = ndbg_vector_size(context);

    if (old_size == ndbg_vector_max_size(type)) {
        return instance->last;
    }

    size_t new_size     = old_size + 1;
    size_t new_capacity = CSTL_vector_calculate_growth(context, new_size);

    CSTL_VectorVal tmp = CSTL_vector_new_with_capacity(new_capacity, type, alloc);
    tmp.last = CSTL_pointer_add(tmp.first, (ptrdiff_t)new_size, type);

    void* constructed_first = CSTL_pointer_add(tmp.first, where_off, type);
    void* constructed_last  = CSTL_pointer_add(constructed_first, 1, type);

    CSTL_type_copy_from(constructed_first, value, type);

    void* old_first = (void*)instance->first;
    void* old_mid   = CSTL_pointer_add(old_first, where_off, type);

    if (where_pointer == instance->last) {
        if (type->use_move_from) {
            CSTL_uninitialized_move_n(old_first, old_size, tmp.first, type);
        } else {
            CSTL_uninitialized_copy_n(old_first, old_size, tmp.first, type);
        }
    } else {
        if (type->use_move_from) {
            CSTL_uninitialized_move_n(old_first, (size_t)where_off, tmp.first, type);
            CSTL_uninitialized_move_n(old_mid, old_size - (size_t)where_off, constructed_last, type);
        } else {
            CSTL_uninitialized_copy_n(old_first, (size_t)where_off, tmp.first, type);
            CSTL_uninitialized_copy_n(old_mid, old_size - (size_t)where_off, constructed_last, type);
        }
    }

    CSTL_vector_replace(context, alloc, tmp);

    return constructed_first;
}

CSTL_VectorIter CSTL_vector_emplace_const(CSTL_VectorCtx context, CSTL_VectorIter where, const void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    CSTL_verify_iterator(&where);
    assert(where.owner == context.instance);

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    void* where_pointer = (void*)where.pointer;
    void* old_last      = (void*)instance->last;

    if (old_last != instance->end) {
        if (where_pointer == old_last) {
            CSTL_vector_copy_back_with_unused_capacity(context, value);
        } else {
            instance->last = CSTL_pointer_add(old_last, 1, type);
            void* old_back = CSTL_pointer_sub(old_last, 1, type);

            if (type->use_move_from) {
                CSTL_type_move_from(old_last, old_back, type);
                CSTL_move_backwards(where_pointer, old_back, old_last, type);
                CSTL_type_copy_from(where_pointer, value, type);
            } else {
                CSTL_type_copy_from(old_last, old_back, type);
                CSTL_copy_backwards(where_pointer, old_back, old_last, type);
                CSTL_type_copy_from(where_pointer, value, type);
            }
        }
    } else {
        where_pointer = CSTL_vector_copy_emplace_reallocate(context, where_pointer, value, alloc);
    }

    return ndbg_make_iterator(context, where_pointer);
}

CSTL_VectorIter CSTL_vector_erase(CSTL_VectorCtx context, CSTL_VectorIter where) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    void* where_pointer = (void*)where.pointer;

    void* first = CSTL_pointer_add(where_pointer, 1, type);
    void* last  = (void*)instance->last;

    if (type->use_move_from) {
        CSTL_move(first, last, where_pointer, type);
    } else {
        CSTL_copy(first, last, where_pointer, type);
    }

    instance->last = CSTL_pointer_sub(instance->last, 1, type);
    CSTL_type_destroy_at(instance->last, type);

    return where;
}

CSTL_VectorIter CSTL_vector_erase_range(CSTL_VectorCtx context, CSTL_VectorIter first, CSTL_VectorIter last) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    CSTL_verify_iterator(&first);
    CSTL_verify_iterator(&last);
    assert(first.owner == context.instance && first.owner == last.owner);

    assert((context.type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    if (!CSTL_vector_iterator_eq(first, last)) { // something to do
        void* old_last = (void*)instance->last;

        if (type->use_move_from) {
            instance->last = CSTL_move((void*)last.pointer, old_last, (void*)first.pointer, type);
        } else {
            instance->last = CSTL_copy((void*)last.pointer, old_last, (void*)first.pointer, type);
        }

        CSTL_destroy((void*)instance->last, old_last, type);
    }

    return first;
}

CSTL_VectorIter CSTL_vector_emplace_back(CSTL_VectorCtx context, void* value, CSTL_Alloc* alloc) {
    return CSTL_vector_emplace(context, ndbg_vector_end(context), value, alloc);
}

CSTL_VectorIter CSTL_vector_emplace_back_const(CSTL_VectorCtx context, const void* value, CSTL_Alloc* alloc) {
    return CSTL_vector_emplace_const(context, ndbg_vector_end(context), value, alloc);
}

bool CSTL_vector_push(CSTL_VectorCtx context, void* value, CSTL_Alloc* alloc) {
    return CSTL_vector_emplace_back(context, value, alloc).pointer != context.instance->last;
}

bool CSTL_vector_push_const(CSTL_VectorCtx context, const void* value, CSTL_Alloc* alloc) {
    return CSTL_vector_emplace_back_const(context, value, alloc).pointer != context.instance->last;
}

void CSTL_vector_pop(CSTL_VectorCtx context) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));
    assert(!CSTL_vector_empty(context));

    instance->last = CSTL_vector_back(context);
    CSTL_type_destroy_at(instance->last, type);
}

bool CSTL_vector_resize_reallocate(CSTL_VectorCtx context, size_t new_size, const void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    if (new_size > ndbg_vector_max_size(type)) {
        return false;
    }

    size_t old_size     = ndbg_vector_size(context);
    size_t new_capacity = CSTL_vector_calculate_growth(context, new_size);

    CSTL_VectorVal tmp = CSTL_vector_new_with_capacity(new_capacity, type, alloc);

    if (type->use_move_from) {
        tmp.last = CSTL_uninitialized_move_n((void*)instance->first, old_size, tmp.first, type);
    } else {
        tmp.last = CSTL_uninitialized_copy_n((void*)instance->first, old_size, tmp.first, type);
    }

    tmp.last = CSTL_uninitialized_fill_n(tmp.last, new_size - old_size, value, type);

    CSTL_vector_replace(context, alloc, tmp);

    return true;
}

bool CSTL_vector_resize(CSTL_VectorCtx context, size_t new_size, const void* value, CSTL_Alloc* alloc) {
    CSTL_VectorRef instance = (CSTL_VectorRef)context.instance;
    CSTL_TypeCRef type      = (CSTL_TypeCRef)context.type;

    assert((type->internal_flags & CSTL_TYPE_INVALID) == 0);
    assert(CSTL_is_vector_aligned(context));

    size_t old_size = ndbg_vector_size(context);
    if (new_size < old_size) {
        void* new_last = CSTL_pointer_add(instance->first, (ptrdiff_t)new_size, type);
        CSTL_destroy_n(new_last, old_size - new_size, type);
        instance->last = new_last;
        
        return true;
    }

    if (new_size > old_size) {
        size_t old_capacity = ndbg_vector_capacity(context);

        if (new_size > old_capacity) {
            return CSTL_vector_resize_reallocate(context, new_size, value, alloc);
        }

        instance->last = CSTL_uninitialized_fill_n(instance->last,
            new_size - old_size, value, type);
    }
        
    return true;
}

size_t ndbg_vector_size(CSTL_VectorCtx context) {
    uintptr_t first_address = (uintptr_t)context.instance->first;
    uintptr_t last_address  = (uintptr_t)context.instance->last;

    return CSTL_type_divide_by_rcp(last_address - first_address, context.type);
}

size_t ndbg_vector_capacity(CSTL_VectorCtx context) {
    uintptr_t first_address = (uintptr_t)context.instance->first;
    uintptr_t end_address   = (uintptr_t)context.instance->end;

    return CSTL_type_divide_by_rcp(end_address - first_address, context.type);
}

size_t ndbg_vector_max_size(CSTL_TypeCRef type) {
    return CSTL_type_divide_by_rcp(SIZE_MAX, type);
}

CSTL_VectorIter ndbg_make_iterator(CSTL_VectorCtx context, const void* pointer) {
    CSTL_VectorIter iterator = {
        .pointer = pointer,
        .type = context.type,
    };

    CSTL_set_iterator_owner(&iterator, context.instance);

    return iterator;
}

CSTL_VectorIter ndbg_vector_begin(CSTL_VectorCtx context) {
    CSTL_VectorIter iterator = {
        .pointer = context.instance->first,
        .type = context.type,
    };

    CSTL_set_iterator_owner(&iterator, context.instance);

    return iterator;
}

CSTL_VectorIter ndbg_vector_end(CSTL_VectorCtx context) {
    CSTL_VectorIter iterator = {
        .pointer = context.instance->last,
        .type = context.type,
    };

    CSTL_set_iterator_owner(&iterator, context.instance);

    return iterator;
}

CSTL_VectorIter ndbg_vector_iterator_add(CSTL_VectorIter iterator, ptrdiff_t n) {
    iterator.pointer = CSTL_pointer_add(iterator.pointer, n, iterator.type);
    return iterator;
}

CSTL_VectorIter ndbg_vector_iterator_sub(CSTL_VectorIter iterator, ptrdiff_t n) {
    return ndbg_vector_iterator_add(iterator, -n);
}

void* ndbg_vector_iterator_deref(CSTL_VectorIter iterator) {
    return (void*)iterator.pointer;
}

void* ndbg_vector_iterator_index(CSTL_VectorIter iterator, ptrdiff_t n) {
    CSTL_VectorIter tmp = ndbg_vector_iterator_add(iterator, n);
    return (void*)tmp.pointer;
}

ptrdiff_t ndbg_vector_iterator_distance(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    return CSTL_pointer_distance(lhs.pointer, rhs.pointer, lhs.type);
}

bool ndbg_vector_iterator_eq(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    return lhs.pointer == rhs.pointer;
}

bool ndbg_vector_iterator_lt(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    return lhs.pointer < rhs.pointer;
}
