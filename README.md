# CSTL
This library aims to target the stable C++14 MSVC STL ABI without depending on MSVC, STL or C++.

It substitutes static typing of C++ templates, which cannot be instantialized accross FFI boundaries, with dynamic typing.

Tests for implemented types can be found at `./tests/`.

### Types implemented so far:
- [x] `std::vector`    -> `CSTL_VectorVal`
- [x] `std::string`    -> `CSTL_StringVal`
- [x] `std::wstring`   -> `CSTL_WideStringVal`
- [x] `std::u8string`  -> `CSTL_UTF8StringVal`
- [x] `std::u16string` -> `CSTL_UTF16StringVal`
- [x] `std::u32string` -> `CSTL_UTF32StringVal`
- [ ] `std::map`
- [ ] `std::set`
- [ ] `std::list`
- [ ] `std::unordered_map`
- [ ] `std::unordered_set`
- [ ] `std::deque`
- [ ] `std::basic_string` (unspecialized)

## License
This work is licensed under the Apache License v2.0 with LLVM Exceptions.

The Microsoft C++ Standard Library is under the Apache License v2.0 with LLVM Exceptions, a copy of which can be found at `./STL/LICENSE.txt`.
