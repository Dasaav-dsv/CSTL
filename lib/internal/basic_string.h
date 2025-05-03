#pragma once

#ifndef CSTL_BASIC_STRING_H
#define CSTL_BASIC_STRING_H

#define CSTL_String__(type, name)  CSTL_ ## type ## String ## name
#define CSTL_String_(type, name) CSTL_String__(type, name)

#define CSTL_string____(type, name)  CSTL_ ## type ## string_ ## name
#define CSTL_string___(type, name) CSTL_string____(type, name)

/**
 * Macro that expands to an identifier like `CSTL_[*]String[name]`,
 * where `*` is the value of `CSTL_string_type`.
 * 
 * `#define CSTL_string_type`
 * `CSTL_String(Iter) -> CSTL_StringIter`
 * `#define CSTL_string_type Wide`
 * `CSTL_String(Iter) -> CSTL_WideStringIter`
 * 
 */
#define CSTL_String(name) CSTL_String_(CSTL_string_type, name)

/**
 * Macro that expands to an identifier like `CSTL_[*]string_[name]`,
 * where `*` is the value of `CSTL_string_prefix`.
 * 
 * `#define CSTL_string_prefix`
 * `CSTL_string_(size) -> CSTL_string_size`
 * `#define CSTL_string_prefix w`
 * `CSTL_string_(size) -> CSTL_wstring_size`
 * 
 */
#define CSTL_string_(name) CSTL_string___(CSTL_string_prefix, name)

#endif
