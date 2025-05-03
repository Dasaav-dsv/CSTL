#pragma once

#ifndef CSTL_XSTRING_H
#define CSTL_XSTRING_H

#if defined(__cplusplus)
#include <cuchar>
#ifndef __cpp_char8_t
typedef unsigned char char8_t;
#endif
extern "C" {
#else
#include "uchar.h"
typedef unsigned char char8_t;
#endif

#define CSTL_string_type
#define CSTL_string_prefix
#define CSTL_char_t char
// CSTL_String <-> std::string
#include "internal/basic_string_decl.inl"
#undef CSTL_string_type
#undef CSTL_string_prefix
#undef CSTL_char_t

#define CSTL_string_type Wide
#define CSTL_string_prefix w
#define CSTL_char_t wchar_t
// CSTL_WideString <-> std::wstring
#include "internal/basic_string_decl.inl"
#undef CSTL_string_type
#undef CSTL_string_prefix
#undef CSTL_char_t

#define CSTL_string_type UTF8
#define CSTL_string_prefix u8
#define CSTL_char_t char8_t
// CSTL_UTF8String <-> std::u8string
#include "internal/basic_string_decl.inl"
#undef CSTL_string_type
#undef CSTL_string_prefix
#undef CSTL_char_t

#define CSTL_string_type UTF16
#define CSTL_string_prefix u16
#define CSTL_char_t char16_t
// CSTL_UTF16String <-> std::u16string
#include "internal/basic_string_decl.inl"
#undef CSTL_string_type
#undef CSTL_string_prefix
#undef CSTL_char_t

#define CSTL_string_type UTF32
#define CSTL_string_prefix u32
#define CSTL_char_t char32_t
// CSTL_UTF32String <-> std::u32string
#include "internal/basic_string_decl.inl"
#undef CSTL_string_type
#undef CSTL_string_prefix
#undef CSTL_char_t

#if defined(__cplusplus)
}
#endif

#endif
