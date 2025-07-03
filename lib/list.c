#include "list.h"
#include "alloc.h"

#include "internal/alloc_dispatch.h"
#include "internal/type_ext.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma warning(push)
#endif

static inline void* CSTL_node_value(CSTL_ListNode* node) {
    return (void*)(node + 1);
}

static inline const void* CSTL_const_node_value(const CSTL_ListNode* node) {
    return (const void*)(node + 1);
}

static void CSTL_list_link_node(CSTL_ListNode* where, CSTL_ListNode* new_node) {
    new_node->next    = where;
    new_node->prev    = where->prev;
    where->prev->next = new_node;
    where->prev       = new_node;
}

static void CSTL_list_unlink_node(CSTL_ListNode* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

static CSTL_ListNode* CSTL_list_create_node(CSTL_Type type, const void* value, CSTL_CopyTypeCRef copy, CSTL_Alloc* alloc) {
    size_t type_size    = CSTL_type_size(type);
    size_t node_size    = sizeof(CSTL_ListNode) + type_size;
    CSTL_ListNode* node = (CSTL_ListNode*)CSTL_allocate(node_size, CSTL_type_alignment(type), alloc);
    if (!node)
        return NULL;

    copy->fill(CSTL_node_value(node), (char*)CSTL_node_value(node) + type_size, value);
    return node;
}

static void CSTL_list_destroy_node(CSTL_ListNode* node, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc) {
    if (drop) {
        void* first = CSTL_node_value(node);
        drop->drop(first, (char*)first + CSTL_type_size(type));
    }
    size_t node_size = sizeof(CSTL_ListNode) + CSTL_type_size(type);
    CSTL_free(node, node_size, CSTL_type_alignment(type), alloc);
}

static CSTL_ListNode* CSTL_list_create_node_move(CSTL_Type type, void* value, CSTL_MoveTypeCRef move, CSTL_Alloc* alloc) {
    size_t type_size    = CSTL_type_size(type);
    size_t node_size    = sizeof(CSTL_ListNode) + type_size;
    CSTL_ListNode* node = (CSTL_ListNode*)CSTL_allocate(node_size, CSTL_type_alignment(type), alloc);
    if (!node)
        return NULL;

    move->move(value, (char*)value + type_size, CSTL_node_value(node));
    return node;
}

void CSTL_list_construct(CSTL_ListRef new_instance, CSTL_Alloc* alloc) {
    new_instance->sentinel = (CSTL_ListNode*)CSTL_allocate(sizeof(CSTL_ListNode), _Alignof(CSTL_ListNode), alloc);
    if (!new_instance->sentinel) {
        return;
    }
    new_instance->sentinel->next = new_instance->sentinel;
    new_instance->sentinel->prev = new_instance->sentinel;
    new_instance->size           = 0;
}

void CSTL_list_clear(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc) {
    CSTL_ListNode* sentinel = instance->sentinel;
    CSTL_ListNode* current  = sentinel->next;
    while (current != sentinel) {
        CSTL_ListNode* next = current->next;
        CSTL_list_destroy_node(current, type, drop, alloc);
        current = next;
    }
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    instance->size = 0;
}

void CSTL_list_destroy(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc) {
    CSTL_list_clear(instance, type, drop, alloc);
    CSTL_free(instance->sentinel, sizeof(CSTL_ListNode), _Alignof(CSTL_ListNode), alloc);
}

bool CSTL_list_empty(CSTL_ListCRef instance) {
    return instance->size == 0;
}

size_t CSTL_list_size(CSTL_ListCRef instance) {
    return instance->size;
}

size_t CSTL_list_max_size(CSTL_Type type) {
    size_t type_size = CSTL_type_size(type);
    size_t node_size = sizeof(CSTL_ListNode) + type_size;
    if (node_size == 0)
        return (size_t)-1;
    return (size_t)-1 / node_size;
}

void* CSTL_list_front(CSTL_ListRef instance) {
    assert(!CSTL_list_empty(instance));
    return CSTL_node_value(instance->sentinel->next);
}

const void* CSTL_list_const_front(CSTL_ListCRef instance) {
    assert(!CSTL_list_empty(instance));
    return CSTL_const_node_value(instance->sentinel->next);
}

void* CSTL_list_back(CSTL_ListRef instance) {
    assert(!CSTL_list_empty(instance));
    return CSTL_node_value(instance->sentinel->prev);
}

const void* CSTL_list_const_back(CSTL_ListCRef instance) {
    assert(!CSTL_list_empty(instance));
    return CSTL_const_node_value(instance->sentinel->prev);
}

CSTL_ListIter CSTL_list_begin(CSTL_ListCRef instance) {
    return (CSTL_ListIter){.owner = instance, .pointer = instance->sentinel->next};
}

CSTL_ListIter CSTL_list_end(CSTL_ListCRef instance) {
    return (CSTL_ListIter){.owner = instance, .pointer = instance->sentinel};
}

bool CSTL_list_copy_push_back(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* value, CSTL_Alloc* alloc) {
    CSTL_ListIter end_iter = CSTL_list_end(instance);
    CSTL_ListIter result   = CSTL_list_copy_insert(instance, type, copy, end_iter, value, alloc);
    return !CSTL_list_iterator_eq(result, end_iter);
}

void CSTL_list_pop_back(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc) {
    assert(!CSTL_list_empty(instance));
    CSTL_ListIter back_iter = {.owner = instance, .pointer = instance->sentinel->prev};
    CSTL_list_erase(instance, type, drop, alloc, back_iter);
}

bool CSTL_list_copy_push_front(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, const void* value, CSTL_Alloc* alloc) {
    CSTL_ListIter begin_iter = CSTL_list_begin(instance);
    CSTL_ListIter result     = CSTL_list_copy_insert(instance, type, copy, begin_iter, value, alloc);
    return !CSTL_list_iterator_eq(result, begin_iter);
}

void CSTL_list_pop_front(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc) {
    assert(!CSTL_list_empty(instance));
    CSTL_list_erase(instance, type, drop, alloc, CSTL_list_begin(instance));
}

CSTL_ListIter CSTL_list_copy_insert(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_ListIter where, const void* value, CSTL_Alloc* alloc) {
    assert(where.owner == instance);
    CSTL_ListNode* new_node = CSTL_list_create_node(type, value, copy, alloc);
    if (!new_node) {
        return CSTL_list_end(instance);
    }
    CSTL_list_link_node((CSTL_ListNode*)where.pointer, new_node);
    instance->size++;
    return (CSTL_ListIter){.owner = instance, .pointer = new_node};
}

CSTL_ListIter CSTL_list_erase(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc, CSTL_ListIter where) {
    assert(where.owner == instance && where.pointer != instance->sentinel);
    CSTL_ListNode* node_to_erase = (CSTL_ListNode*)where.pointer;
    CSTL_ListNode* next_node     = node_to_erase->next;

    CSTL_list_unlink_node(node_to_erase);
    CSTL_list_destroy_node(node_to_erase, type, drop, alloc);
    instance->size--;

    return (CSTL_ListIter){.owner = instance, .pointer = next_node};
}

CSTL_ListIter CSTL_list_erase_range(CSTL_ListRef instance, CSTL_Type type, CSTL_DropTypeCRef drop, CSTL_Alloc* alloc, CSTL_ListIter first, CSTL_ListIter last) {
    assert(first.owner == instance && last.owner == instance);
    if (CSTL_list_iterator_eq(first, last)) {
        return last;
    }

    CSTL_ListNode* node_before_first = ((CSTL_ListNode*)first.pointer)->prev;
    CSTL_ListNode* current           = (CSTL_ListNode*)first.pointer;
    while (current != last.pointer) {
        CSTL_ListNode* to_erase = current;
        current                 = current->next;
        CSTL_list_destroy_node(to_erase, type, drop, alloc);
        instance->size--;
    }

    node_before_first->next              = (CSTL_ListNode*)last.pointer;
    ((CSTL_ListNode*)last.pointer)->prev = node_before_first;

    return last;
}

bool CSTL_list_resize(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc) {
    size_t current_size = instance->size;
    if (current_size < new_size) {
        size_t diff            = new_size - current_size;
        CSTL_ListIter end_iter = CSTL_list_end(instance);
        if (CSTL_list_iterator_eq(CSTL_list_insert_n(instance, type, copy, end_iter, diff, value, alloc), end_iter) && diff > 0) {
            return false;
        }
    } else {
        while (instance->size > new_size) {
            CSTL_list_pop_back(instance, type, &copy->move_type.drop_type, alloc);
        }
    }
    return true;
}

bool CSTL_list_assign_n(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, size_t new_size, const void* value, CSTL_Alloc* alloc) {
    if (new_size > CSTL_list_max_size(type))
        return false;
    CSTL_list_clear(instance, type, &copy->move_type.drop_type, alloc);
    CSTL_ListIter end_iter = CSTL_list_end(instance);
    if (CSTL_list_iterator_eq(CSTL_list_insert_n(instance, type, copy, end_iter, new_size, value, alloc), end_iter) && new_size > 0) {
        return false;
    }
    return true;
}

CSTL_ListIter CSTL_list_insert_n(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_ListIter where, size_t count, const void* value, CSTL_Alloc* alloc) {
    assert(where.owner == instance);
    if (count == 0)
        return where;
    if (instance->size + count > CSTL_list_max_size(type))
        return CSTL_list_end(instance);

    CSTL_ListNode* first_new_node   = NULL;
    CSTL_ListNode* current_new_node = NULL;

    for (size_t i = 0; i < count; ++i) {
        CSTL_ListNode* new_node = CSTL_list_create_node(type, value, copy, alloc);
        if (!new_node) {
            while (first_new_node) {
                CSTL_ListNode* to_free = first_new_node;
                first_new_node         = first_new_node->next;
                CSTL_list_destroy_node(to_free, type, &copy->move_type.drop_type, alloc);
            }
            return CSTL_list_end(instance);
        }
        if (!first_new_node) {
            first_new_node   = new_node;
            current_new_node = new_node;
        } else {
            current_new_node->next = new_node;
            new_node->prev         = current_new_node;
            current_new_node       = new_node;
        }
    }

    CSTL_ListNode* where_node  = (CSTL_ListNode*)where.pointer;
    CSTL_ListNode* node_before = where_node->prev;

    node_before->next    = first_new_node;
    first_new_node->prev = node_before;

    where_node->prev       = current_new_node;
    current_new_node->next = where_node;

    instance->size += count;
    return (CSTL_ListIter){.owner = instance, .pointer = first_new_node};
}

void CSTL_list_swap(CSTL_ListRef instance, CSTL_ListRef other_instance) {
    if (instance == other_instance) {
        return;
    }

    CSTL_ListNode* temp_sentinel = instance->sentinel;
    instance->sentinel           = other_instance->sentinel;
    other_instance->sentinel     = temp_sentinel;

    size_t temp_size     = instance->size;
    instance->size       = other_instance->size;
    other_instance->size = temp_size;
}

CSTL_ListIter CSTL_list_iterator_add(CSTL_ListIter iterator, ptrdiff_t n) {
    CSTL_ListNode* node           = (CSTL_ListNode*)iterator.pointer;
    const CSTL_ListNode* sentinel = iterator.owner->sentinel;
    if (n > 0) {
        for (ptrdiff_t i = 0; i < n && node != sentinel; ++i) {
            node = node->next;
        }
    } else if (n < 0) {
        for (ptrdiff_t i = n; i < 0; ++i) {
            node = node->prev;
            if (node == sentinel && i < -1) {
                break;
            }
        }
    }
    iterator.pointer = node;
    return iterator;
}

CSTL_ListIter CSTL_list_iterator_sub(CSTL_ListIter iterator, ptrdiff_t n) {
    return CSTL_list_iterator_add(iterator, -n);
}

void* CSTL_list_iterator_deref(CSTL_ListIter iterator) {
    assert(iterator.pointer != iterator.owner->sentinel);
    return CSTL_node_value((CSTL_ListNode*)iterator.pointer);
}

ptrdiff_t CSTL_list_iterator_distance(CSTL_ListIter lhs, CSTL_ListIter rhs) {
    assert(lhs.owner == rhs.owner);
    ptrdiff_t dist               = 0;
    const CSTL_ListNode* current = lhs.pointer;
    while (current != rhs.pointer) {
        current = current->next;
        dist++;
    }
    return dist;
}

bool CSTL_list_iterator_eq(CSTL_ListIter lhs, CSTL_ListIter rhs) {
    assert(lhs.owner == rhs.owner);
    return lhs.pointer == rhs.pointer;
}

bool CSTL_list_move_push_back(CSTL_ListRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, void* value, CSTL_Alloc* alloc) {
    CSTL_ListIter end_iter = CSTL_list_end(instance);
    CSTL_ListIter result   = CSTL_list_move_insert(instance, type, move, end_iter, value, alloc);
    return !CSTL_list_iterator_eq(result, end_iter);
}

CSTL_ListIter CSTL_list_move_insert(CSTL_ListRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_ListIter where, void* value, CSTL_Alloc* alloc) {
    assert(where.owner == instance);
    CSTL_ListNode* new_node = CSTL_list_create_node_move(type, value, move, alloc);
    if (!new_node) {
        return CSTL_list_end(instance);
    }
    CSTL_list_link_node((CSTL_ListNode*)where.pointer, new_node);
    instance->size++;
    return (CSTL_ListIter){.owner = instance, .pointer = new_node};
}

bool CSTL_list_copy_assign(CSTL_ListRef instance, CSTL_Type type, CSTL_CopyTypeCRef copy, CSTL_ListCRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    if (instance == other_instance) {
        return true;
    }

    if (propagate_alloc && !CSTL_alloc_is_equal(alloc, other_alloc)) {
        // Allocators are different and we must propagate.
        // Destroy all elements and free the sentinel with the old allocator.
        CSTL_list_destroy(instance, type, &copy->move_type.drop_type, alloc);
        // Propagate the new allocator.
        *alloc = *other_alloc;
        // Construct a new sentinel with the new allocator.
        CSTL_list_construct(instance, alloc);
    } else {
        // Allocators are the same or we don't propagate.
        // Just clear the elements.
        CSTL_list_clear(instance, type, &copy->move_type.drop_type, alloc);
    }

    // Copy elements from the other list.
    for (CSTL_ListIter it = CSTL_list_begin(other_instance); !CSTL_list_iterator_eq(it, CSTL_list_end(other_instance)); it = CSTL_list_iterator_add(it, 1)) {
        if (!CSTL_list_copy_push_back(instance, type, copy, CSTL_const_node_value(it.pointer), alloc)) {
            // In case of allocation failure during copy, the list is left in a valid but partially copied state.
            return false;
        }
    }

    return true;
}

bool CSTL_list_move_assign(CSTL_ListRef instance, CSTL_Type type, CSTL_MoveTypeCRef move, CSTL_ListRef other_instance, CSTL_Alloc* alloc, CSTL_Alloc* other_alloc, bool propagate_alloc) {
    if (instance == other_instance) {
        return true;
    }

    if (propagate_alloc || CSTL_alloc_is_equal(alloc, other_alloc)) {
        CSTL_list_clear(instance, type, &move->drop_type, alloc);

        if (propagate_alloc && !CSTL_alloc_is_equal(alloc, other_alloc)) {
            // Free the old sentinel with the old allocator
            CSTL_free(instance->sentinel, sizeof(CSTL_ListNode), _Alignof(CSTL_ListNode), alloc);
            // Propagate the new allocator
            *alloc = *other_alloc;
            // Take ownership of the other's resources
            instance->sentinel = other_instance->sentinel;
            instance->size     = other_instance->size;
            // Leave the other list in a valid, empty state using its own allocator
            CSTL_list_construct(other_instance, other_alloc);
        } else {
            // Allocators are compatible, just swap the guts. O(1)
            CSTL_list_swap(instance, other_instance);
        }
    } else {
        // Allocators are not compatible and we can't propagate.
        // Must move elements one by one. O(N)
        CSTL_list_clear(instance, type, &move->drop_type, alloc);
        for (CSTL_ListIter it = CSTL_list_begin(other_instance); !CSTL_list_iterator_eq(it, CSTL_list_end(other_instance)); it = CSTL_list_iterator_add(it, 1)) {
            if (!CSTL_list_move_push_back(instance, type, move, CSTL_list_iterator_deref(it), alloc)) {
                return false;
            }
        }
        CSTL_list_clear(other_instance, type, &move->drop_type, other_alloc);
    }

    return true;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
