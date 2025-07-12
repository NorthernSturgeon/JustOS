# JustOS
My operating system for x86-64

## About
An OS from an amateur developer with very serious intentions

## Project timeline
+ \[🌱 Origin] 09.02.2022 — Birth of concept  
+ \[🚀 Launch] 12.07.2025 — First release: bootloader works!  
+ ...

## Deployment on virtual drive
1. Create .vhd file that contains EFI partition (MUST be 1st partition) with size at least 128 MiB
2. Create directories EFI/Boot/ on the partition
3. Write the absolute path to the file in the local.env file
4. Run `make build` and `sudo make install` in both `bootldr` and `kernel` folders
5. The drive is ready to use!