OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kentry)

SECTIONS{
	.text ALIGN(4096):{ *(.text) *(.rodata.*)}
	.data ALIGN(4096):{ *(.data) *(.bss) *(.got.plt) *(.dynamic) *(*.hash)}
}
