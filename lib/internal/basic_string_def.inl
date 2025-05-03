#include "basic_string_decl.inl"
#include "alloc_dispatch.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

bool CSTL_string_(large_mode_engaged)(CSTL_String(CRef) instance) {
    return instance->res > CSTL_string_small_res;
}

CSTL_char_t* CSTL_string_(ptr)(CSTL_String(Ref) instance) {
    CSTL_char_t* result = instance->bx.buf;
    if (CSTL_string_(large_mode_engaged)(instance)) {
        result = instance->bx.ptr;
    }

    return result;
}

const CSTL_char_t* CSTL_string_(const_ptr)(CSTL_String(CRef) instance) {
    const CSTL_char_t* result = instance->bx.buf;
    if (CSTL_string_(large_mode_engaged)(instance)) {
        result = instance->bx.ptr;
    }

    return result;
}

void CSTL_string_(deallocate_for_capacity)(CSTL_char_t* old_ptr, size_t capacity, CSTL_Alloc* alloc) {
    CSTL_free((void*)old_ptr, capacity + 1, alignof(CSTL_char_t), alloc); // +1 for null terminator
}

void CSTL_string_(tidy_deallocate)(CSTL_String(Ref) instance, CSTL_Alloc* alloc) {
    if (CSTL_string_(large_mode_engaged)(instance)) {
        CSTL_string_(deallocate_for_capacity)(instance->bx.ptr, instance->res, alloc);
        instance->bx.ptr = NULL;
    }

    instance->size = 0;
    instance->res  = CSTL_string_small_res;

    instance->bx.buf[0] = 0;
}

void CSTL_string_(construct)(CSTL_String(Val)* new_instance) {
    if (new_instance == NULL) {
        return;
    }

    new_instance->size = 0;
    new_instance->res  = CSTL_string_small_res;

    new_instance->bx.buf[0] = 0;
}

void CSTL_string_(destroy)(CSTL_String(Ref) instance, CSTL_Alloc* alloc) {
    CSTL_string_(tidy_deallocate)(instance, alloc);
}

void CSTL_string_(assign)(CSTL_String(Ref) instance, CSTL_char_t* str, CSTL_Alloc* alloc);

void CSTL_string_(assign_n)(CSTL_String(Ref) instance, CSTL_char_t* str, size_t count, CSTL_Alloc* alloc);

bool CSTL_string_(assign_char)(CSTL_String(Ref) instance, size_t new_size, CSTL_char_t value, CSTL_Alloc* alloc);

bool CSTL_string_(assign_substr)(CSTL_String(Ref) instance, CSTL_String(Ref) other, size_t pos, size_t count, CSTL_Alloc* alloc);

void CSTL_string_(copy_assign)(CSTL_String(Ref) instance, CSTL_Alloc* alloc, CSTL_String(CRef) other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc);

void CSTL_string_(move_assign)(CSTL_String(Ref) instance, CSTL_Alloc* alloc, CSTL_String(Ref) other_instance, CSTL_Alloc* other_alloc, bool propagate_alloc);

void CSTL_string_(swap)(CSTL_String(Ref) instance, CSTL_String(Ref) other_instance) {
    if (instance != other_instance) {
        unsigned char temp_mem[sizeof(CSTL_String(Val))];
        memcpy((void*)temp_mem, (const void*)instance, sizeof(CSTL_String(Val)));
        memcpy((void*)instance, (const void*)other_instance, sizeof(CSTL_String(Val)));
        memcpy((void*)other_instance, (const void*)temp_mem, sizeof(CSTL_String(Val)));
    }
}

CSTL_char_t* CSTL_string_(index)(CSTL_String(Ref) instance, size_t pos) {
    assert(pos < instance->size);
    return &CSTL_string_(ptr)(instance)[pos];
}

const CSTL_char_t* CSTL_string_(const_index)(CSTL_String(CRef) instance, size_t pos) {
    assert(pos < instance->size);
    return &CSTL_string_(const_ptr)(instance)[pos];
}

CSTL_char_t* CSTL_string_(get_at)(CSTL_String(Ref) instance, size_t pos)  {
    if (instance->size <= pos) {
        return NULL;
    }

    return &CSTL_string_(ptr)(instance)[pos];
}

const CSTL_char_t* CSTL_string_(const_get_at)(CSTL_String(CRef) instance, size_t pos) {
    if (instance->size <= pos) {
        return NULL;
    }

    return &CSTL_string_(const_ptr)(instance)[pos];
}

CSTL_char_t* CSTL_string_(front)(CSTL_String(Ref) instance) {
    assert(instance->size != 0);
    return CSTL_string_(ptr)(instance);
}

const CSTL_char_t* CSTL_string_(const_front)(CSTL_String(CRef) instance) {
    assert(instance->size != 0);
    return CSTL_string_(const_ptr)(instance);
}

CSTL_char_t* CSTL_string_(back)(CSTL_String(Ref) instance) {
    assert(instance->size != 0);
    return &CSTL_string_(ptr)(instance)[instance->size - 1];
}

const CSTL_char_t* CSTL_string_(const_back)(CSTL_String(CRef) instance) {
    assert(instance->size != 0);
    return &CSTL_string_(const_ptr)(instance)[instance->size - 1];
}

CSTL_char_t* CSTL_string_(data)(CSTL_String(Ref) instance) {
    return CSTL_string_(ptr)(instance);
}

const CSTL_char_t* CSTL_string_(c_str)(CSTL_String(CRef) instance) {
    return CSTL_string_(const_ptr)(instance);
}

CSTL_char_t* CSTL_string_(begin)(CSTL_String(Ref) instance) {
    return CSTL_string_(ptr)(instance);
}

const CSTL_char_t* CSTL_string_(const_begin)(CSTL_String(CRef) instance) {
    return CSTL_string_(const_ptr)(instance);
}

CSTL_char_t* CSTL_string_(end)(CSTL_String(Ref) instance) {
    return &CSTL_string_(ptr)(instance)[instance->size];
}

const CSTL_char_t* CSTL_string_(const_end)(CSTL_String(CRef) instance) {
    return &CSTL_string_(const_ptr)(instance)[instance->size];
}

bool CSTL_string_(empty)(CSTL_String(CRef) instance) {
    return instance->size == 0;
}

size_t CSTL_string_(size)(CSTL_String(CRef) instance) {
    return instance->size;
}

size_t CSTL_string_(length)(CSTL_String(CRef) instance) {
    return instance->size;
}

size_t CSTL_string_(capacity)(CSTL_String(CRef) instance) {
    return instance->res;
}

size_t CSTL_string_(max_size)() {
    return (size_t)PTRDIFF_MAX / sizeof(CSTL_char_t); 
}
