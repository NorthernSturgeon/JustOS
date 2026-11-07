# README {#mainpage}
Operating system for x86-64. 

### About
The core principles of JustOS are portability and extensibility. Achieving this, however, requires implementing a variety of alternative engineering tricks and finding optimal solutions under complex conditions...

See the history and follow the development progress in real time in the [changelog](https://github.com/NorthernSturgeon/JustOS/blob/develop/CHANGELOG.md)@develop

### Key feautures
- UEFI bootloader
- PIE for whole OS
- Dynamic linking for kernel
- Human-readable configuration files
- Lock-free global memory allocator
- Per-CPU allocator
- Much more in the future...

<a href="https://northernsturgeon.github.io/JustOS/">**Documentation**</a>

### System requirements:
- CPU: Any processor with x86-64 architecture support
- UEFI support
- At least 128 MB of RAM

### Dependecies
- GNU-EFI for bootloader
- Doxygen for generating documentation

### Deployment on virtual drive
1. Create .vhd file that contains EFI partition (MUST be 1st partition) with size at least 32 MiB
2. Create /EFI/Boot/ and /JUSTOS folders on the partition
3. Fill **VHDINSTALLPATH** @ *`local.env`* with absolute path to the drive
4. Run `make build` and `sudo make install` in both `bootldr` and `kernel` folders
5. The drive is ready to use!

### Deployment on removable media
1. Fill **TARGETMEDIAPATH** @ *`local.env`* with absolute path to the root folder of the media
2. Make sure that media has /EFI/Boot and /JUSTOS folders
3. Run `deploy.sh` in `utils` folder
4. Profit!

### License
JustOS is licensed under the Mozilla Public License, v. 2.0.

The implementation of the XXHash64 algorithm (bootldr/xxhash64.c) is based on code by [Stephan Brumme](https://github.com/stbrumme/xxhash), which was originally published under the MIT license. The full MIT copyright and permission notice is preserved in the header of that file.