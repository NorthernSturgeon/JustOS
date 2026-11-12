# JustOS kernel {#kernel}
> The heart of the OS...

## Environment
At the moment control is transferred, the kernel receives these things from the bootloader in a ready-to-use form:

- Virtual address space
- Global Descriptor Table
- E820 memory map
- Video mode and framebuffer
- All specifed modules and libraries with resolved relocations and initialized TLS
- Stack space
- RSDP of ACPI

## System initialization
The kernel executes an initialization sequence:

1. Clear the screen, initialize the console
2. Initialize the physical memory allocator:

    a. Sort and minimize the memory map

	b. Create the `gorl`

	c. Mark all memory used by the OS as busy

	d. Initialize `malloc` of *klibc*
3. Create and apply the IDT
4. Enable the \subpage int "interrupts"
5. Test the access to TLS data
6. Render the splash screen