.global _inb
.global _outb
.text
    _inb:
        mov %di, %dx
        in %dx, %al
        ret

    _outb:
        mov %di, %dx
        mov %sil, %al
        out %al, %dx
        ret
