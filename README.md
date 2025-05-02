# CSTL
This library aims to target the stable C++14 MSVC STL ABI without depending on MSVC, STL or C++.

It substitutes static typing of C++ templates, which cannot be instantialized accross FFI boundaries, with dynamic typing.

### Types implemented so far:
- [x] `std::vector` -> `CSTL_VectorVal`
- [ ] `std::basic_string`
- [ ] `std::map`
- [ ] `std::set`
- [ ] `std::list`
- [ ] `std::unordered_map`
- [ ] `std::unordered_set`
- [ ] `std::deque`

## License
This work is licensed under the Apache License v2.0 with LLVM Exceptions.

The Microsoft C++ Standard Library is under the Apache License v2.0 with LLVM Exceptions, a copy of which can be found at `STL/LICENSE.txt`.
