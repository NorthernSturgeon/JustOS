OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kentry)

SECTIONS{
	. = 0x1000;
	.text ALIGN(4096):{
		*(.text)
	}
	.data ALIGN(4096):{ *(.data) *(.bss)}

	.percpu : {
		PROVIDE_HIDDEN(_percpu_start_ = .);
		*(.percpu)
	}

	/DISCARD/ :{
		*(.comment)
		*(.note*)
		*(.eh_frame)
	}
}
