# THE CHANGELOG (and prospect)
    of the JUSTOS
### Current stable version: [v0.0.6](#v006--2026-07-31)

## [Unversioned] / NET: N/A
- Optimization of ***strlen***
- Implementation of ***aligned_malloc***
- Implementaion of ***realloc***
- С++ support
- Approximate videomode
- Correct processing of weak bindings

## [v0.0.7] / NET: 2026-10-24
- [x] License!
- [ ] Fast heap verification
- [ ] Documentation
- [x] Logo rendering
- [x] Deployment on removable media
- [x] Renaming: *`libc.so`* -> *`xlibc.so`*

## [v0.0.7-alpha.1] / 2026-10-23
- Prototype of fast heap for small data
- Basic interrupt handling
- Implemented ***strlen***
- Optimized operations with *gorl* @ *`physmem.c`*
- Refactored ***printf***
- Fixed incorrect implementation of ***memmove***
- Using *<stdbool.h>* instead of defining `bool` manually

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
- Enforced C17 standard
- Bugfixes...

## [v0.0.2]() / 2025-07-17
- Implemented ***printf***
- Bugfixes...

## [v0.0.1]() / 2025-07-12
- Initial...