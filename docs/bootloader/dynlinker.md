# Dynamic linker {#bootdl}
JustOS uses position-independent code, which requires dynamic linking before launching...

### Overview
Dynamic linker of the JustOS bootloader uses hash tables for symbol processing. The one named `loaded_libs` contains a list of all files loaded into memory, while `global_exports` stores the symbols of the global namespace (it is possible to use a separate local namespace for each module).

### Dependency search
The ELF format keeps a list of dependencies in the *DT_NEEDED* entries of the dynamic section. During the kernel building, EFI-compliant paths to the libraries are recorded into those entries. To achieve this, the *DT_SONAME* entry of each library contains the path where the library will reside on disk. The dynamic linker parses these entries and recursively loads the files into memory using the `get_file` function. The recursion continues as long as the number of symbols increases after each subsequent `get_file` call - this prevents infinite loops in the presence of circular dependencies.

### File loading
Let's consider the `get_file` function. It returns a pointer to the file entry within `loaded_libs`. If the file is not yet present in memory, the following algorithm is executed:

1. The file is loaded into memory.
2. The ELF structure is parsed.
3. Memory for TLS (Thread-Local Storage) is initialized, if present.
4. The file entry is added to the `loaded_libs` table.
5. **Exported symbols are added to the namespace.**

### Symbol resolution
The linker recursively iterates through the .rela.dyn section of each file and applies relocations according to the ELF specification, relying on the previously constructed namespace. The following relocation types are supported: R_X86_64_RELATIVE, R_X86_64_GLOB_DAT, R_X86_64_JUMP_SLOT (with forced eager binding), and R_X86_64_DTPMOD. Notably, for R_X86_64_DTPMOD relocations, the DTV base address is written in accordance with the TLS model adopted by JustOS.