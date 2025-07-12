OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

SECTIONS{
	.text ALIGN(4096):{ *(.text) *(.rodata.*)}
	.data ALIGN(4096):{ *(.data) *(.got.plt) *(.dynamic) *(.bss) *(*.hash)}
}
