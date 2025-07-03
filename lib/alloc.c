#include "alloc.h"

#if defined(__cplusplus)
#include <cstddef>
extern "C" {
#else
#include <stdbool.h>
#endif

bool CSTL_alloc_is_equal(const CSTL_Alloc* lhs, const CSTL_Alloc* rhs) {
    if (!lhs && !rhs) {
        return true;
    }

    if (!lhs || !rhs) {
        return false;
    }

    return lhs->opaque == rhs->opaque && lhs->aligned_alloc == rhs->aligned_alloc && lhs->aligned_free == rhs->aligned_free;
}
