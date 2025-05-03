#include "basic_string.h"

#ifndef CSTL_string_type
#define CSTL_string_type
#endif

#ifndef CSTL_string_prefix
#define CSTL_string_prefix
#endif

#ifndef CSTL_char_t
#define CSTL_char_t char
#endif

// length of internal buffer, [1, 16]
#define CSTL_string_bufsize (16 / sizeof(CSTL_char_t) < 1 ? 1 : 16 / sizeof(CSTL_char_t))
// roundup mask for allocated buffers, [0, 15]
#define CSTL_string_alloc_mask (sizeof(CSTL_char_t) <= 1 ? 15 \
                              : sizeof(CSTL_char_t) <= 2 ? 7  \
                              : sizeof(CSTL_char_t) <= 4 ? 3  \
                              : sizeof(CSTL_char_t) <= 8 ? 1  \
                                                         : 0)
// capacity in small mode
#define CSTL_string_small_res (CSTL_string_bufsize - 1)

typedef struct CSTL_String(Val) {
    union {
        CSTL_char_t buf[CSTL_string_bufsize];
        CSTL_char_t* ptr;
    };
    size_t size;
    size_t res;
} CSTL_String(Val);
