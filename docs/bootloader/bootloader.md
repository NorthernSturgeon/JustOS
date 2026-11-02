# JustOS Bootloader {#bootldr}
Dynamic linking, identity mapping into the higher half, early environment setup - all of this leads JustOS to build a custom bootloader instead of porting existing solutions.

## Boot process
### 0. Start
The bootloader's life revolves around the [`boot_info`](structboot__info__t.html) structure, to which it passes all the information the kernel may need. First, the pointer to the ACPI structures is filled in. As the code runs, all *19 fields* get their values.

### 1. Disk access
`OpenRoot` gets from EFI a handle to the filesystem where the bootloader is stored, and `ReadFile` is a wrapper for loading files into memory. Currently, partitions other than ESP are not supported. `ReadFile`, like the bootloader as a whole, always uses **absolute paths**.

### 2. Reading the config file
The bootloader looks for the config file at <i>`\JUSTOS\justboot.tsv`</i>, loads it using the *\subpage tsvboot "TSV parser"*, and applies the settings:

	video    width    height    #Requested screen resolution (in pixels)
	stack    size               #Kernel stack size (bytes)
	splash   path               #Path to splash (.bmp)

### 3. Kernel loading and dynamic linking
Config parsing continues. The bootloader **does not load the kernel into memory directly**: that is delegated to the *\subpage bootdl "dynamic linker"*. After the last one is initialized, the image path is passed to `dl_process`, which loads and processes the kernel and all its dependencies itself; once modules appear, it will be called for each of them. The dynamic linker finishes the work and returns *a list of all files loaded into memory* for later use.

### 4. ExitBootServices
Preparation for taking full control of the system begins. `allocate_ptzone` calculates the memory needed for *page tables* and requests it from the firmware. If the allocation succeeds, the bootloader **zeroes the region**.

`BS->ExitBootServices()` is given 4 attempts because the memory map is volatile. If it fails, the bootloader is forced to terminate.

### 5. Environment setup
A standalone console is initialized. Then the paging setup function is called, which, based on the EFI memory map, creates an identity mapping in the higher half of the address space. After that, the memory map is converted to the E820 format used by the kernel.

### 6. Transferring control

\snippet bootldr/bootloader.c start_kernel

The remaining `boot_info` fields are filled in, and `start_kernel`, the last bootloader function, is called. It is written in assembly and does the following:

1. Switch page tables to the configured ones
2. Enable SSE extensions
3. Fill the GDT
4. Reload segment registers
5. **Far jump to the kernel entry point**