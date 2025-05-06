#include "type.h"

#include "internal/type_ext.h"
#include "internal/rcp_division.h"

CSTL_TypeErr CSTL_define_type(
    CSTL_Type* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance)
) {
    if (new_instance == NULL) {
        return CSTL_TypeErr_NullPointer;
    }

    // Output an object before checks.
    {
        CSTL_Type instance = {
            .size = 1,
            .size_rcp = SIZE_MAX,
            .size_rcp_sh = 0,
            .align_lg = 0,
            .use_move_from = (uint8_t)(move_from != NULL),
            .internal_flags = CSTL_TYPE_INVALID,
            .copy_from = copy_from,
            .move_from = move_from,
            .destroy = destroy
        };

        *new_instance = instance;
    }

    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return CSTL_TypeErr_Misaligned;
    }

    if (size == 0 || size > SIZE_MAX / 2 || (size & (alignment - 1))) {
        return CSTL_TypeErr_BadSize;
    }

    new_instance->size        = size;
    uint8_t size_rcp_sh       = 0;
    new_instance->size_rcp    = CSTL_fixed_point_euclid_rcp(size, &size_rcp_sh);
    new_instance->size_rcp_sh = size_rcp_sh;

    new_instance->internal_flags &= ~CSTL_TYPE_INVALID;

    return CSTL_TypeErr_Ok;
}

CSTL_TypeErr CSTL_define_eq_type(
    CSTL_EqType* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance),
    bool (*is_eq)(const void* lhs, const void* rhs)
) {
    if (new_instance == NULL) {
        return CSTL_TypeErr_NullPointer;
    }

    new_instance->is_eq = is_eq;

    return CSTL_define_type(
        &new_instance->base,
        size,
        alignment,
        copy_from,
        move_from,
        destroy
    );
}

CSTL_TypeErr CSTL_define_comp_type(
    CSTL_CompType* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance),
    bool (*is_eq)(const void* lhs, const void* rhs),
    bool (*is_lt)(const void* lhs, const void* rhs)
) {
    if (new_instance == NULL) {
        return CSTL_TypeErr_NullPointer;
    }

    new_instance->is_lt = is_lt;

    return CSTL_define_eq_type(
        &new_instance->base,
        size,
        alignment,
        copy_from,
        move_from,
        destroy,
        is_eq
    );
}

CSTL_TypeErr CSTL_define_hash_type(
    CSTL_HashType* new_instance,
    size_t size,
    size_t alignment,
    void (*copy_from)(void* new_instance, const void* src),
    void (*move_from)(void* new_instance, void* src),
    void (*destroy)(void* instance),
    bool (*is_eq)(const void* lhs, const void* rhs),
    size_t (*hash)(const void* instance)
) {
    if (new_instance == NULL) {
        return CSTL_TypeErr_NullPointer;
    }

    new_instance->hash = hash;

    return CSTL_define_eq_type(
        &new_instance->base,
        size,
        alignment,
        copy_from,
        move_from,
        destroy,
        is_eq
    );
}
