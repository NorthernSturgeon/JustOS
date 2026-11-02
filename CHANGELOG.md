# THE CHANGELOG (and prospect)
    of the JUSTOS
### Current stable version: [v0.0.7-rc1](#v007rc1)

## [Unversioned] / NET: N/A
- Optimization of ***strlen***
- Implementation of ***aligned_malloc***
- Implementaion of ***realloc***
- С++ support
- Approximate videomode setup
- Correct processing of weak bindings
- New place for the *boot_info*

***

## [v0.0.7] / NET: N/A
- More documentation
## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.7-rc1">v0.0.7-rc1</a> / 2026-11-02 {#v007rc1}
### Common
- License!
- Fast heap verification
- Basic documentation
- Logo rendering
- Deployment on removable media
- Fixed UB in dynamic linker
- Stock TLS mechanism
- Fixed unalignment access and strict-aliasing issues
### API
- Renaming: *`libc.so`* -> *`klibc.so`*
- Changed return type for *rwlock_\*_crit* from `uint8_t` to `bool`
- Macro *rgb(r,g,b)* for color @ *`video.h`*

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.7-alpha.1">v0.0.7-alpha.1</a> / 2026-10-23
- Prototype of fast heap for small data
- Basic interrupt handling
- Implemented ***strlen***
- Optimized operations with *gorl* @ *`physmem.c`*
- Refactored ***printf***
- Fixed incorrect implementation of ***memmove***
- Using *<stdbool.h>* instead of defining `bool` manually

***

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.6">v0.0.6</a> / 2026-07-31
- Beautiful CHANGELOG
- TLS correct support
- Using standard header files for integer types
- *`physmem.c`* is now a part of kernel, not libc
- Cosmetic changes...

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.6-alpha">v0.0.6-alpha</a> / 2026-07-30
- TLS support prototype
- Now all loaded libraries will reserve memory

***

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.5">v0.0.5</a> / 2026-07-29
- Dynamic linking!
- Fixed incorrect implementation of min-heap for page allocator
- Optimizations and bugfixes...

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.5-alpha">v0.0.5-alpha</a> / 2026-07-28
- Size of stack is now configurable in BCF
- Dynamic linker works in VM
- Enabled SSE/SSE2 instructions
- Video framebuffer is no longer declared as `volatile`

***

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.4">v0.0.4</a> / 2026-06-02
- Improved building
- Preparations for dynamic linking support

***

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.3">v0.0.3</a> / 2026-05-23
- Developed memory manager
- Introduced bootloader configuration file in TSV format
- Separated kernel and it's stack
- Enforced C17 standard
- Bugfixes...

***

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.2">v0.0.2</a> / 2025-07-17
- Implemented ***printf***
- Bugfixes...

***

## <a href="https://github.com/NorthernSturgeon/JustOS/releases/tag/v0.0.1">v0.0.1</a> / 2025-07-12
- Initial...