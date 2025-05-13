#include "vector.h"

#ifndef NDEBUG
static inline bool CSTL_verify_address(CSTL_VectorCRef instance, const char* address) {
    return address >= (const char*)instance->first && address <= (const char*)instance->last;
}

#define CSTL_verify_iterator(iterator) \
    assert(CSTL_verify_address((iterator)->owner, (iterator)->pointer))

#define CSTL_set_iterator_owner(iterator, owned_by) \
    (iterator)->owner = (owned_by)
#else
#define CSTL_verify_iterator(iterator) (void)(iterator)
#define CSTL_set_iterator_owner(iterator, owned_by) (void)(iterator)
#endif

// #include "internal/type_dispatch.h"
#include "internal/alloc_dispatch.h"
#include "internal/type_ext.h"

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146) // dumb non-standard diagnostic
#endif

static inline size_t CSTL_vector_size_bytes(CSTL_VectorCRef instance) {
    return (size_t)((char*)instance->last - (char*)instance->first);
}

static inline size_t CSTL_vector_capacity_bytes(CSTL_VectorCRef instance) {
    return (size_t)((char*)instance->end - (char*)instance->first);
}

static inline size_t CSTL_vector_bytes_max(size_t size) {
    return ((size_t)PTRDIFF_MAX - 1) / size * size;
}

static inline bool CSTL_vector_checked_mul(size_t* bytes, size_t type_size, size_t n) {
    *bytes = n * type_size;

    if ((*bytes / type_size) != n || *bytes >= CSTL_vector_bytes_max(type_size)) {
        return false;
    }

    return true;
}

static inline size_t CSTL_vector_growth_bytes(CSTL_VectorCRef instance, size_t type_size, size_t new_bytes) {
    size_t old_bytes = (size_t)((char*)instance->end - (char*)instance->first);
    size_t max_bytes = CSTL_vector_bytes_max(type_size);

    if (old_bytes > max_bytes - old_bytes) {
        return max_bytes; // geometric growth would overflow
    }

    size_t geometric = old_bytes + old_bytes;

    if (geometric < new_bytes) {
        return new_bytes; // geometric growth would be insufficient
    }

    return geometric; // geometric growth is sufficient
}

static inline CSTL_VectorVal CSTL_vector_new_with_bytes(size_t bytes, size_t alignment, CSTL_Alloc* alloc) {
    char* first = CSTL_allocate(bytes, alignment, alloc);
    char* last  = first;
    char* end   = first != NULL ? first + bytes : NULL;

    CSTL_VectorVal tmp = {first, last, end};

    return tmp;
}

static inline void CSTL_vector_tidy(CSTL_VectorRef instance, size_t alignment, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc) {
    if (instance->first) {
        drop->drop(instance->first, instance->last);
        
        CSTL_free(instance->first, CSTL_vector_capacity_bytes(instance),
            alignment, alloc);

        instance->first = NULL;
        instance->last  = NULL;
        instance->end   = NULL;
    }
}

static inline void CSTL_vector_replace(CSTL_VectorRef instance, size_t alignment, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc, CSTL_VectorVal val) {
    CSTL_vector_tidy(instance, alignment, drop, alloc);
    *instance = val;
}

static inline bool CSTL_vector_reallocate_bytes(CSTL_VectorRef instance, size_t alignment, CSTL_MoveTypeCRef move, size_t new_bytes, CSTL_Alloc* old_alloc, CSTL_Alloc* new_alloc) {   
    CSTL_VectorVal tmp = CSTL_vector_new_with_bytes(new_bytes, alignment, new_alloc);

    if (tmp.first == NULL) {
        return false;
    }

    tmp.last = (char*)tmp.first + CSTL_vector_size_bytes(instance);
    move->move(instance->first, instance->last, tmp.first);

    CSTL_vector_replace(instance, alignment, &move->drop_type, old_alloc, tmp);

    return true;
}

static inline void* CSTL_vector_sized_move(size_t type_size, CSTL_MoveTypeCRef move, void* first, void* last, void* dest) {
    while ((char*)first < (char*)last) {
        void* dest_next = (char*)dest + type_size;
        move->drop_type.drop(dest, dest_next);
        void* first_next = (char*)first + type_size;
        move->move(first, first_next, dest);
        dest = dest_next;
        first = first_next;
    }
    return dest;
}

static inline void* CSTL_vector_sized_move_backwards(size_t type_size, CSTL_MoveTypeCRef move, void* first, void* last, void* dest) {
    while ((char*)first < last) {
        void* dest_prev = dest;
        dest = (char*)dest - type_size;
        void* last_prev = last;
        last = (char*)last - type_size;
        move->drop_type.drop(dest, dest_prev);
        move->move(last, last_prev, dest);
    }
    return dest;
}

void CSTL_vector_construct(CSTL_VectorVal* new_instance) {
    if (new_instance == NULL) {
        return;
    }

    new_instance->first = NULL;
    new_instance->last = NULL;
    new_instance->end = NULL;
}

void CSTL_vector_destroy(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc) {
    CSTL_vector_tidy(instance, CSTL_alignof_type(type), drop, alloc);
}

bool CSTL_vector_copy_assign(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_VectorCRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    if (instance == other_instance) {
        return true;
    }

    if (propagate_alloc) {
        if (alloc != other_alloc) {
            size_t new_bytes = (size_t)((char*)other_instance->last - (char*)other_instance->first);

            if (!CSTL_vector_reallocate_bytes(instance, CSTL_alignof_type(type),
                &copy->move_type, new_bytes, alloc, other_alloc)) {
                return false;
            }

            alloc = other_alloc;
        }
    }
    
    return CSTL_vector_copy_assign_range(instance, type, copy, other_instance->first, other_instance->last, alloc);
}

bool CSTL_vector_move_assign(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_VectorRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    size_t alignment = CSTL_alignof_type(type);

    if (instance == other_instance) {
        return true;
    }

    if (!propagate_alloc) {
        if (alloc != other_alloc) {
            size_t new_bytes = (size_t)((char*)other_instance->last - (char*)other_instance->first);

            if (!CSTL_vector_reallocate_bytes(instance, alignment, move, new_bytes, alloc, other_alloc)) {
                return false;
            }

            move->drop_type.drop(instance->first, instance->last);
            move->move(other_instance->first, other_instance->last, instance->first);

            instance->last = (char*)instance->first + new_bytes;

            return true;
        }
    }

    CSTL_vector_replace(instance, alignment, &move->drop_type, alloc, *other_instance);

    other_instance->first = NULL;
    other_instance->last  = NULL;
    other_instance->end   = NULL;

    return true;
}

bool CSTL_vector_assign_n(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc) {
    CSTL_SmallAllocFrame frame;

    size_t alignment = CSTL_alignof_type(type);
    size_t type_size = CSTL_sizeof_type(type);
    size_t new_bytes = 0;

    if (!CSTL_vector_checked_mul(&new_bytes, type_size, new_size)) {
        return false;
    }

    char* first = instance->first;
    char* last  = instance->first;
    char* end   = instance->first;

    if (new_bytes > (size_t)(end - first)) { // reallocate
        size_t new_capacity = CSTL_vector_growth_bytes(instance, type_size, new_bytes);
        CSTL_VectorVal tmp  = CSTL_vector_new_with_bytes(new_bytes, alignment, alloc);

        if (tmp.first == NULL) {
            return false;
        }

        tmp.last = (char*)tmp.first + new_bytes;
        copy->fill(tmp.first, tmp.last, value);

        CSTL_vector_replace(instance, alignment, &copy->move_type.drop_type, alloc, tmp);

        return true;
    }

    bool is_aliased  = (char*)value >= first && (char*)value < last;
    uintptr_t cookie = (uintptr_t)&CSTL_vector_assign_n ^ (uintptr_t)&is_aliased;
    void* tmp        = (void*)value;

    if (is_aliased) {
        tmp = CSTL_small_alloc(&frame, type_size, alignment, alloc, cookie);

        if (tmp == NULL) {
            return false;
        }

        copy->copy(value, (char*)value + type_size, tmp);
    }

    copy->move_type.drop_type.drop(first, last);

    instance->last = (char*)instance->first + new_bytes;
    copy->fill(first, instance->last, tmp);

    if (is_aliased) {
        copy->move_type.drop_type.drop(tmp, (char*)tmp + type_size);
        CSTL_small_free(&frame, new_bytes, alignment, alloc, cookie);
    }

    return true;
}

bool CSTL_vector_copy_assign_range(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* range_first, const void* range_last, CSTL_Alloc* alloc) {
    CSTL_SmallAllocFrame frame;

    size_t alignment = CSTL_alignof_type(type);
    size_t type_size = CSTL_sizeof_type(type);
    size_t new_bytes = (size_t)((char*)range_last - (char*)range_first);

    char* first = instance->first;
    char* last  = instance->first;
    char* end   = instance->first;

    if (new_bytes > CSTL_vector_capacity_bytes(instance)) { // reallocate
        size_t new_capacity = CSTL_vector_growth_bytes(instance, type_size, new_bytes);
        CSTL_VectorVal tmp  = CSTL_vector_new_with_bytes(new_bytes, alignment, alloc);

        if (tmp.first == NULL) {
            return false;
        }

        tmp.last = (char*)tmp.first + new_bytes;
        copy->copy(range_first, range_last, tmp.first);

        CSTL_vector_replace(instance, alignment, &copy->move_type.drop_type, alloc, tmp);

        return true;
    }

    bool is_aliased  = (char*)range_last >= first && (char*)range_first < last;
    uintptr_t cookie = (uintptr_t)&CSTL_vector_copy_assign_range ^ (uintptr_t)&is_aliased;
    void* tmp_first  = (void*)range_first;
    void* tmp_last   = (void*)range_last;

    if (is_aliased) {
        tmp_first = CSTL_small_alloc(&frame, new_bytes, alignment, alloc, cookie);
        tmp_last  = (char*)tmp_first + new_bytes;

        if (tmp_first == NULL) {
            return false;
        }

        copy->copy(range_first, range_last, tmp_first);
    }

    copy->move_type.drop_type.drop(first, last); 
    copy->copy(tmp_first, tmp_last, first);
    
    instance->last = first + new_bytes;

    if (is_aliased) {
        copy->move_type.drop_type.drop(tmp_first, tmp_last);
        CSTL_small_free(&frame, new_bytes, alignment, alloc, cookie);
    }

    return true;
}

bool CSTL_vector_move_assign_range(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, void* range_first, void* range_last, CSTL_Alloc* alloc) {
    CSTL_SmallAllocFrame frame;

    size_t alignment = CSTL_alignof_type(type);
    size_t type_size = CSTL_sizeof_type(type);
    size_t new_bytes = (size_t)((char*)range_last - (char*)range_first);

    char* first = instance->first;
    char* last  = instance->first;
    char* end   = instance->first;

    if (new_bytes > CSTL_vector_capacity_bytes(instance)) { // reallocate
        size_t new_capacity = CSTL_vector_growth_bytes(instance, type_size, new_bytes);
        CSTL_VectorVal tmp  = CSTL_vector_new_with_bytes(new_bytes, alignment, alloc);

        if (tmp.first == NULL) {
            return false;
        }

        tmp.last = (char*)tmp.first + new_bytes;
        move->move(range_first, range_last, tmp.first);

        CSTL_vector_replace(instance, alignment, &move->drop_type, alloc, tmp);

        return true;
    }

    bool is_aliased  = (char*)range_last >= first && (char*)range_first < last;
    uintptr_t cookie = (uintptr_t)&CSTL_vector_copy_assign_range ^ (uintptr_t)&is_aliased;
    void* tmp_first  = (void*)range_first;
    void* tmp_last   = (void*)range_last;

    if (is_aliased) {
        tmp_first = CSTL_small_alloc(&frame, new_bytes, alignment, alloc, cookie);
        tmp_last  = (char*)tmp_first + new_bytes;

        if (tmp_first == NULL) {
            return false;
        }

        move->move(range_first, range_last, tmp_first);
    }

    move->drop_type.drop(first, last); 
    move->move(tmp_first, tmp_last, first);
    
    instance->last = first + new_bytes;

    if (is_aliased) {
        move->drop_type.drop(tmp_first, tmp_last);
        CSTL_small_free(&frame, new_bytes, alignment, alloc, cookie);
    }

    return true;
}

void CSTL_vector_swap(CSTL_VectorRef instance, CSTL_VectorRef other_instance) {
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

void* CSTL_vector_index(CSTL_VectorRef instance, CSTL_Type type, size_t pos) {
    size_t type_size = CSTL_sizeof_type(type);
    size_t pos_bytes = pos * type_size;

    assert(pos_bytes / type_size == pos);
    assert(pos_bytes < CSTL_vector_size_bytes(instance));

    return (char*)instance->first + pos_bytes;
}

const void* CSTL_vector_const_index(CSTL_VectorCRef instance, CSTL_Type type, size_t pos) {
    return CSTL_vector_index((CSTL_VectorRef)instance, type, pos);
}

void* CSTL_vector_at(CSTL_VectorRef instance, CSTL_Type type, size_t pos) {
    size_t type_size = CSTL_sizeof_type(type);
    size_t pos_bytes = 0;

    if (!CSTL_vector_checked_mul(&pos_bytes, type_size, pos)) {
        return NULL;
    }

    char* pos_at = (char*)instance->first + pos_bytes;

    if (pos_at >= (char*)instance->last) {
        return NULL;
    }

    return pos_at;
}

const void* CSTL_vector_const_at(CSTL_VectorCRef instance, CSTL_Type type, size_t pos) {
    return CSTL_vector_at((CSTL_VectorRef)instance, type, pos);
}

void* CSTL_vector_front(CSTL_VectorRef instance) {
    assert(!CSTL_vector_empty(instance));
    return instance->first;
}

const void* CSTL_vector_const_front(CSTL_VectorCRef instance) {
    return CSTL_vector_front((CSTL_VectorRef)instance);
}

void* CSTL_vector_back(CSTL_VectorRef instance, CSTL_Type type) {
    assert(!CSTL_vector_empty(instance));
    return (char*)instance->last - CSTL_sizeof_type(type);
}

const void* CSTL_vector_const_back(CSTL_VectorCRef instance, CSTL_Type type) {
    return CSTL_vector_back((CSTL_VectorRef)instance, type);
}

void* CSTL_vector_data(CSTL_VectorRef instance) {
    return instance->first;
}

const void* CSTL_vector_const_data(CSTL_VectorCRef instance) {
    return instance->first;
}

CSTL_VectorIter CSTL_vector_begin(CSTL_VectorCRef instance, CSTL_Type type) {
    CSTL_VectorIter iterator = {
        .pointer = instance->first,
        .size = CSTL_sizeof_type(type),
    };

    CSTL_set_iterator_owner(&iterator, instance);

    return iterator;
}

CSTL_VectorIter CSTL_vector_end(CSTL_VectorCRef instance, CSTL_Type type) {
    CSTL_VectorIter iterator = {
        .pointer = instance->last,
        .size = CSTL_sizeof_type(type),
    };

    CSTL_set_iterator_owner(&iterator, instance);

    return iterator;
}

CSTL_VectorIter CSTL_vector_iterator_add(CSTL_VectorIter iterator, ptrdiff_t n) {
    assert(iterator.pointer != NULL);
    CSTL_verify_iterator(&iterator);

    const void* new_pointer = (const char*)iterator.pointer
        + n * (ptrdiff_t)iterator.size;

    CSTL_VectorIter tmp = {
        .pointer = new_pointer,
        .size = iterator.size,
    };

    CSTL_set_iterator_owner(&tmp, iterator.owner);
    CSTL_verify_iterator(&tmp);

    return tmp;
}

CSTL_VectorIter CSTL_vector_iterator_sub(CSTL_VectorIter iterator, ptrdiff_t n) {
    return CSTL_vector_iterator_add(iterator, -n);
}

void* CSTL_vector_iterator_deref(CSTL_VectorIter iterator) {
    assert(iterator.pointer != NULL);
    CSTL_verify_iterator(&iterator);
    return (void*)iterator.pointer;
}

void* CSTL_vector_iterator_index(CSTL_VectorIter iterator, ptrdiff_t n) {
    assert(iterator.pointer != NULL);
    CSTL_verify_iterator(&iterator);

    const void* new_pointer = (const char*)iterator.pointer
        + n * (ptrdiff_t)iterator.size;

    return (void*)new_pointer;
}

ptrdiff_t CSTL_vector_iterator_distance(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    CSTL_verify_iterator(&lhs);
    CSTL_verify_iterator(&rhs);
    assert(lhs.owner == rhs.owner);
    return ((const char*)rhs.pointer - (const char*)lhs.pointer) / (ptrdiff_t)lhs.size;
}

bool CSTL_vector_iterator_eq(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    CSTL_verify_iterator(&lhs);
    CSTL_verify_iterator(&rhs);
    assert(lhs.owner == rhs.owner);
    return lhs.pointer == rhs.pointer;
}

bool CSTL_vector_iterator_lt(CSTL_VectorIter lhs, CSTL_VectorIter rhs) {
    CSTL_verify_iterator(&lhs);
    CSTL_verify_iterator(&rhs);
    assert(lhs.owner == rhs.owner);
    return (const char*)lhs.pointer < (const char*)rhs.pointer;
}

bool CSTL_vector_empty(CSTL_VectorCRef instance) {
    return instance->first == instance->last;
}

size_t CSTL_vector_size(CSTL_VectorCRef instance, CSTL_Type type) {
    return CSTL_vector_size_bytes(instance) / CSTL_sizeof_type(type);
}

size_t CSTL_vector_capacity(CSTL_VectorCRef instance, CSTL_Type type) {
    return CSTL_vector_capacity_bytes(instance) / CSTL_sizeof_type(type);
}

size_t CSTL_vector_max_size(CSTL_Type type) {
    return (size_t)(PTRDIFF_MAX - 1) / CSTL_sizeof_type(type);
}

bool CSTL_vector_resize(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc) {
    size_t alignment = CSTL_alignof_type(type);
    size_t type_size = CSTL_sizeof_type(type);
    size_t new_bytes = 0;

    if (!CSTL_vector_checked_mul(&new_bytes, type_size, new_size)) {
        return false;
    }

    size_t old_bytes = CSTL_vector_size_bytes(instance);

    if (new_bytes == old_bytes) {
        return true;
    }

    void* old_last = instance->last;
    void* new_last = (char*)instance->first + new_bytes;

    if (new_bytes > old_bytes) {
        if (new_bytes > CSTL_vector_capacity_bytes(instance)) {
            size_t new_capacity = CSTL_vector_growth_bytes(instance, type_size, new_bytes);
            CSTL_VectorVal tmp  = CSTL_vector_new_with_bytes(new_capacity, alignment, alloc);

            if (tmp.first == NULL) {
                return false;
            }

            old_last = (char*)tmp.first + old_bytes;
            new_last = (char*)tmp.first + new_bytes;

            copy->copy(instance->first, instance->last, tmp.first);

            CSTL_vector_replace(instance, alignment, &copy->move_type.drop_type, alloc, tmp);
        }

        copy->fill(old_last, new_last, value);
    } else {
        copy->move_type.drop_type.drop(new_last, old_last);
    }

    instance->last = new_last;
        
    return true;
}

void CSTL_vector_truncate(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, size_t new_size) {
    size_t type_size = CSTL_sizeof_type(type);
    size_t new_bytes = 0;

    if (!CSTL_vector_checked_mul(&new_bytes, type_size, new_size)) {
        return;
    }

    size_t old_bytes = CSTL_vector_size_bytes(instance);

    if (new_bytes >= old_bytes) {
        return;
    }

    void* old_last = instance->last;
    void* new_last = (char*)instance->first + new_bytes;
    instance->last = new_last;

    drop->drop(new_last, old_last);
}

bool CSTL_vector_reserve(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, size_t new_capacity, CSTL_Alloc* alloc) {
    // increase capacity to new_capacity (without geometric growth)
    size_t alignment = CSTL_alignof_type(type);
    size_t type_size = CSTL_sizeof_type(type);
    size_t new_bytes = 0;

    if (!CSTL_vector_checked_mul(&new_bytes, type_size, new_capacity)) {
        return false;
    }

    if (new_bytes > CSTL_vector_capacity_bytes(instance)) { // something to do (reserve() never shrinks)
        return CSTL_vector_reallocate_bytes(instance, alignment, move, new_bytes, alloc, alloc);
    }

    return true;
}

bool CSTL_vector_shrink_to_fit(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_Alloc* alloc) {
    size_t alignment = CSTL_alignof_type(type);

    void* old_last = instance->last;

    if (old_last != instance->end) { // something to do
        void* old_first = instance->first;

        if (old_first == old_last) {
            CSTL_vector_tidy(instance, alignment, &move->drop_type, alloc);
        } else {
            size_t new_capacity = CSTL_vector_size_bytes(instance);

            return CSTL_vector_reallocate_bytes(instance, alignment, move, new_capacity, alloc, alloc);
        }
    }

    return true;
}

void CSTL_vector_clear(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop) {
    void* first = instance->first;
    void* last  = instance->last;

    if (first == last) {
        return;
    }
    
    drop->drop(first, last);

    instance->last = first;
}

CSTL_VectorIter CSTL_vector_insert_n(CSTL_VectorRef instance, CSTL_CopyTypeCRef copy, CSTL_VectorIter where, size_t count, const void* value, CSTL_Alloc* alloc) {
    CSTL_verify_iterator(&where);
    assert(where.owner == instance);

    if (count == 0) {
        return where;
    }

    size_t type_size = where.size;
    size_t alignment = type_size & -type_size;
    size_t new_bytes = 0;

    if (!CSTL_vector_checked_mul(&new_bytes, type_size, count)) {
        where.pointer = instance->last;
        return where;
    }

    void* where_pointer = (void*)where.pointer;

    size_t where_bytes  = (size_t)((char*)where_pointer - (char*)instance->first);
    size_t unused_bytes = (size_t)((char*)instance->end - (char*)instance->last);

    if (new_bytes > unused_bytes) {
        size_t old_bytes = CSTL_vector_size_bytes(instance);

        if (new_bytes > CSTL_vector_bytes_max(type_size) - old_bytes) {
            where.pointer = instance->last;
            return where;
        }

        size_t new_capacity = CSTL_vector_growth_bytes(instance, type_size, old_bytes + new_bytes);

        CSTL_VectorVal tmp = CSTL_vector_new_with_bytes(new_capacity, alignment, alloc);

        if (tmp.first == NULL) {
            where.pointer = instance->last;
            return where;
        }

        tmp.last = (char*)tmp.first + old_bytes + new_bytes;

        void* constructed_first = (char*)tmp.first + where_bytes;
        void* constructed_last  = (char*)constructed_first + new_bytes;

        copy->fill(constructed_first, constructed_last, value);
    
        if (where_pointer != instance->last) {
            copy->move_type.move(where_pointer, instance->last, constructed_last);
        }

        copy->move_type.move(instance->first, where_pointer, tmp.first);

        CSTL_vector_replace(instance, alignment, &copy->move_type.drop_type, alloc, tmp);

        where_pointer = constructed_first;
    } else {
        void* old_last    = instance->last;
        void* where_last  = (char*)where_pointer + new_bytes;

        size_t affected_bytes = (size_t)((char*)instance->last - (char*)where_pointer);

        if (new_bytes > affected_bytes) {
            void* new_mid  = (char*)old_last + new_bytes - affected_bytes;

            copy->move_type.move(where_pointer, old_last, new_mid);
            copy->move_type.drop_type.drop(where_pointer, old_last);
        } else {
            void* new_mid  = (char*)old_last - new_bytes;

            copy->move_type.move(new_mid, old_last, old_last);
            CSTL_vector_sized_move_backwards(type_size, &copy->move_type, where_pointer, new_mid, old_last);
            copy->move_type.drop_type.drop(where_pointer, where_last);
        }

        copy->fill(where_pointer, where_last, value);
        instance->last = (char*)old_last + new_bytes;
    }

    where.pointer = where_pointer;
    return where;
}

void* CSTL_vector_copy_insert_reallocate(CSTL_VectorRef instance, CSTL_CopyTypeCRef copy, size_t type_size, void* where, const void* value, CSTL_Alloc* alloc) {
    size_t fake_alignment = type_size & -type_size;

    size_t where_bytes = (size_t)((char*)where - (char*)instance->first);

    size_t old_bytes = CSTL_vector_size_bytes(instance);
    size_t new_bytes = CSTL_vector_growth_bytes(instance, type_size, old_bytes + type_size);

    CSTL_VectorVal tmp = CSTL_vector_new_with_bytes(new_bytes, fake_alignment, alloc);

    if (tmp.first == NULL) {
        return instance->last;
    }

    tmp.last = (char*)tmp.first + old_bytes + type_size;

    void* constructed_first = (char*)tmp.first + where_bytes;
    void* constructed_last  = (char*)constructed_first + type_size;

    copy->copy(value, (const char*)value + type_size, constructed_first);

    if (where != instance->last) {
        copy->move_type.move(where, instance->last, constructed_last);
    }

    copy->move_type.move(instance->first, where, tmp.first);

    CSTL_vector_replace(instance, fake_alignment, &copy->move_type.drop_type, alloc, tmp);

    return constructed_first;
}

void* CSTL_vector_move_insert_reallocate(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, size_t type_size, void* where, void* value, CSTL_Alloc* alloc) {
    size_t fake_alignment = type_size & -type_size;

    size_t where_bytes = (size_t)((char*)where - (char*)instance->first);

    size_t old_bytes = CSTL_vector_size_bytes(instance);
    size_t new_bytes = CSTL_vector_growth_bytes(instance, type_size, old_bytes + type_size);

    CSTL_VectorVal tmp = CSTL_vector_new_with_bytes(new_bytes, fake_alignment, alloc);

    if (tmp.first == NULL) {
        return instance->last;
    }

    tmp.last = (char*)tmp.first + old_bytes + type_size;

    void* constructed_first = (char*)tmp.first + where_bytes;
    void* constructed_last  = (char*)constructed_first + type_size;

    move->move(value, (char*)value + type_size, constructed_first);

    if (where != instance->last) {
        move->move(where, instance->last, constructed_last);
    }

    move->move(instance->first, where, tmp.first);

    CSTL_vector_replace(instance, fake_alignment, &move->drop_type, alloc, tmp);

    return constructed_first;
}

CSTL_VectorIter CSTL_vector_copy_insert(CSTL_VectorRef instance, CSTL_CopyTypeCRef copy, CSTL_VectorIter where, const void* value, CSTL_Alloc* alloc) {
    CSTL_verify_iterator(&where);
    assert(where.owner == instance);

    size_t type_size = where.size;

    void* where_pointer = (void*)where.pointer;
    void* old_last      = instance->last;

    if (CSTL_vector_size_bytes(instance) >= CSTL_vector_bytes_max(type_size)) {
        where.pointer = old_last;
        return where;
    }

    if (old_last != instance->end) {
        instance->last = (char*)old_last + type_size;

        if (where_pointer != old_last) {
            void* old_back = (char*)old_last - type_size;

            CSTL_vector_sized_move_backwards(type_size, &copy->move_type, where_pointer, old_last, old_last);
            copy->move_type.drop_type.drop(where_pointer, (char*)where_pointer + type_size);
        }

        copy->copy(value, (const char*)value + type_size, where_pointer);
    } else {
        where.pointer = CSTL_vector_copy_insert_reallocate(instance, copy, type_size, where_pointer, value, alloc);
    }

    return where;
}

CSTL_VectorIter CSTL_vector_move_insert(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter where, void* value, CSTL_Alloc* alloc) {
    CSTL_verify_iterator(&where);
    assert(where.owner == instance);

    size_t type_size = where.size;

    void* where_pointer = (void*)where.pointer;
    void* old_last      = instance->last;

    if (CSTL_vector_size_bytes(instance) >= CSTL_vector_bytes_max(type_size)) {
        where.pointer = old_last;
        return where;
    }

    if (old_last != instance->end) {
        instance->last = (char*)old_last + type_size;

        if (where_pointer != old_last) {
            void* old_back = (char*)old_last - type_size;

            move->move(old_back, old_last, old_last);
            CSTL_vector_sized_move_backwards(type_size, move, where_pointer, old_back, old_last);
            move->drop_type.drop(where_pointer, (char*)where_pointer + type_size);
        }

        move->move(value, (char*)value + type_size, where_pointer);
    } else {
        where.pointer = CSTL_vector_move_insert_reallocate(instance, move, type_size, where_pointer, value, alloc);
    }

    return where;
}

CSTL_VectorIter CSTL_vector_copy_insert_range(CSTL_VectorRef instance, CSTL_CopyTypeCRef copy, CSTL_VectorIter where, const void* range_first, const void* range_last, CSTL_Alloc* alloc) {
    CSTL_verify_iterator(&where);
    assert(where.owner == instance);

    if (range_first == range_last) {
        return where;
    }

    size_t type_size = where.size;
    size_t alignment = type_size & -type_size;
    size_t new_bytes = (size_t)((char*)range_last - (char*)range_first);

    void* where_pointer = (void*)where.pointer;

    size_t where_bytes  = (size_t)((char*)where_pointer - (char*)instance->first);
    size_t unused_bytes = (size_t)((char*)instance->end - (char*)instance->last);

    if (new_bytes > unused_bytes) {
        size_t old_bytes = CSTL_vector_size_bytes(instance);

        if (new_bytes > CSTL_vector_bytes_max(type_size) - old_bytes) {
            where.pointer = instance->last;
            return where;
        }

        size_t new_capacity = CSTL_vector_growth_bytes(instance, type_size, old_bytes + new_bytes);

        CSTL_VectorVal tmp = CSTL_vector_new_with_bytes(new_capacity, alignment, alloc);

        if (tmp.first == NULL) {
            where.pointer = instance->last;
            return where;
        }

        tmp.last = (char*)tmp.first + old_bytes + new_bytes;

        void* constructed_first = (char*)tmp.first + where_bytes;
        void* constructed_last  = (char*)constructed_first + new_bytes;

        copy->copy(range_first, range_last, constructed_first);
    
        if (where_pointer != instance->last) {
            copy->move_type.move(where_pointer, instance->last, constructed_last);
        }

        copy->move_type.move(instance->first, where_pointer, tmp.first);

        CSTL_vector_replace(instance, alignment, &copy->move_type.drop_type, alloc, tmp);

        where_pointer = constructed_first;
    } else {
        void* old_last = instance->last;

        size_t affected_bytes = (size_t)((char*)instance->last - (char*)where_pointer);

        if (new_bytes > affected_bytes) {
            void* new_mid  = (char*)old_last + new_bytes - affected_bytes;

            copy->move_type.move(where_pointer, old_last, new_mid);
            copy->move_type.drop_type.drop(where_pointer, old_last);
        } else {
            void* new_mid  = (char*)old_last - new_bytes;

            copy->move_type.move(new_mid, old_last, old_last);
            CSTL_vector_sized_move_backwards(type_size, &copy->move_type, where_pointer, new_mid, old_last);
            copy->move_type.drop_type.drop(where_pointer, (char*)where_pointer + new_bytes);
        }

        copy->copy(range_first, range_last, where_pointer);
        instance->last = (char*)old_last + new_bytes;
    }

    where.pointer = where_pointer;
    return where;
}

CSTL_VectorIter CSTL_vector_move_insert_range(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter where, void* range_first, void* range_last, CSTL_Alloc* alloc) {
    CSTL_verify_iterator(&where);
    assert(where.owner == instance);

    if (range_first == range_last) {
        return where;
    }

    size_t type_size = where.size;
    size_t alignment = type_size & -type_size;
    size_t new_bytes = (size_t)((char*)range_last - (char*)range_first);

    void* where_pointer = (void*)where.pointer;

    size_t where_bytes  = (size_t)((char*)where_pointer - (char*)instance->first);
    size_t unused_bytes = (size_t)((char*)instance->end - (char*)instance->last);

    if (new_bytes > unused_bytes) {
        size_t old_bytes = CSTL_vector_size_bytes(instance);

        if (new_bytes > CSTL_vector_bytes_max(type_size) - old_bytes) {
            where.pointer = instance->last;
            return where;
        }

        size_t new_capacity = CSTL_vector_growth_bytes(instance, type_size, old_bytes + new_bytes);

        CSTL_VectorVal tmp = CSTL_vector_new_with_bytes(new_capacity, alignment, alloc);

        if (tmp.first == NULL) {
            where.pointer = instance->last;
            return where;
        }

        tmp.last = (char*)tmp.first + old_bytes + new_bytes;

        void* constructed_first = (char*)tmp.first + where_bytes;
        void* constructed_last  = (char*)constructed_first + new_bytes;

        move->move(range_first, range_last, constructed_first);
    
        if (where_pointer != instance->last) {
            move->move(where_pointer, instance->last, constructed_last);
        }

        move->move(instance->first, where_pointer, tmp.first);

        CSTL_vector_replace(instance, alignment, &move->drop_type, alloc, tmp);

        where_pointer = constructed_first;
    } else {
        void* old_last = instance->last;

        size_t affected_bytes = (size_t)((char*)instance->last - (char*)where_pointer);

        if (new_bytes > affected_bytes) {
            void* new_mid  = (char*)old_last + new_bytes - affected_bytes;

            move->move(where_pointer, old_last, new_mid);
            move->drop_type.drop(where_pointer, old_last);
        } else {
            void* new_mid  = (char*)old_last - new_bytes;

            move->move(new_mid, old_last, old_last);
            CSTL_vector_sized_move_backwards(type_size, move, where_pointer, new_mid, old_last);
            move->drop_type.drop(where_pointer, (char*)where_pointer + new_bytes);
        }

        move->move(range_first, range_last, where_pointer);
        instance->last = (char*)old_last + new_bytes;
    }

    where.pointer = where_pointer;
    return where;
}

bool CSTL_vector_copy_push_back(CSTL_VectorRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* value, CSTL_Alloc* alloc) {
    CSTL_VectorIter inserted = CSTL_vector_copy_insert(instance, copy,
        CSTL_vector_end(instance, type), value, alloc);
    return inserted.pointer != instance->last;
}

bool CSTL_vector_move_push_back(CSTL_VectorRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, void* value, CSTL_Alloc* alloc) {
    CSTL_VectorIter inserted = CSTL_vector_move_insert(instance, move,
        CSTL_vector_end(instance, type), value, alloc);
    return inserted.pointer != instance->last;
}

void CSTL_vector_pop_back(CSTL_VectorRef instance, CSTL_Type type, CSTL_DropTypeCRef drop) {
    void* new_last = CSTL_vector_back(instance, type);
    void* old_last = instance->last;

    drop->drop(new_last, old_last);

    instance->last = new_last;
}

CSTL_VectorIter CSTL_vector_erase(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter where) {
    return CSTL_vector_erase_range(instance, move, where, CSTL_vector_iterator_add(where, 1));
}

CSTL_VectorIter CSTL_vector_erase_range(CSTL_VectorRef instance, CSTL_MoveTypeCRef move, CSTL_VectorIter first, CSTL_VectorIter last) {
    CSTL_verify_iterator(&first);
    CSTL_verify_iterator(&last);
    assert(first.owner == instance && first.owner == last.owner);

    if (first.pointer != last.pointer) { // something to do
        void* new_last = CSTL_vector_sized_move(last.size, move, (void*)last.pointer,
            instance->last, (void*)first.pointer);

        move->drop_type.drop(new_last, instance->last);

        instance->last = new_last;
    }

    return first;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
