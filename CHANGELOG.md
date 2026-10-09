# THE CHANGELOG (and prospect)
    of the JUSTOS
### Current stable version: [v0.0.6](#v006--2026-07-31)

## [Unversioned] / NET: N/A
- Optimization of ***strlen***
- Implementation of ***aligned_malloc***

## [v0.0.7] / NET: N/A
- License!
- Improved project structure
- C++ support

## [v0.0.7-alpha.2] / NET: N/A
- GC and arena priority in fast heap

## [v0.0.7-alpha.1] / NET: 2026-09
- [ ] Prototype of fast heap for small data
- [x] Basic interrupt handling
- [x] Implemented ***strlen***
- [x] Optimized operations with *gorl* @ *`physmem.c`*
- [x] Refactored ***printf***
- [x] Fixed incorrect implementation of ***memmove***

## [v0.0.6]() / 2026-07-31
- Beautiful CHANGELOG
- TLS correct support
- Using standard header files for integer types
- *`physmem.c`* is now a part of kernel, not libc
- Cosmetic changes...

## [v0.0.6-alpha]() / 2026-07-30
- TLS support prototype
- Now all loaded libraries will reserve memory

## [v0.0.5]() / 2026-07-29
- Dynamic linking!
- Fixed incorrect implementation of min-heap for page allocator
- Optimizations and bugfixes...

## [v0.0.5-alpha]() / 2026-07-28
- Size of stack is now configurable in BCF
- Dynamic linker works in VM
- Enabled SSE/SSE2 instructions
- Video framebuffer is no longer declared as `volatile`

## [v0.0.4]() / 2026-06-02
- Improved building
- Preparations for dynamic linking support

## [v0.0.3]() / 2026-05-23
- Developed memory manager
- Introduced bootloader configuration file in TSV format
- Separated kernel and it's stack
- Bugfixes...

## [v0.0.2]() / 2025-07-17
- Implemented ***printf***
- Bugfixes...

## [v0.0.1]() / 2025-07-12
- Initial...