#pragma once

#ifndef CSTL_XSTRING_H
#define CSTL_XSTRING_H

#if defined(__cplusplus)
extern "C" {
#endif

// CSTL_String <-> std::string
#include "internal/expanded/string_decl.inl"
// CSTL_WideString <-> std::wstring
#include "internal/expanded/wstring_decl.inl"
// CSTL_UTF8String <-> std::u8string
#include "internal/expanded/u8string_decl.inl"
// CSTL_UTF16String <-> std::u16string
#include "internal/expanded/u16string_decl.inl"
// CSTL_UTF32String <-> std::u32string
#include "internal/expanded/u32string_decl.inl"

#if defined(__cplusplus)
}
#endif

#endif
