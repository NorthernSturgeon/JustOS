include ../local.env

CFLAGS=-c -I include -nostdlib -ffreestanding \
	-fomit-frame-pointer -fno-stack-protector \
	-Wall -Wextra -Werror -static -fPIE \
	-fno-asynchronous-unwind-tables -include lib/types.h \
	-march=x86-64 -std=c17 -fno-tree-vrp -mstackrealign \
	-O1 -mcmodel=medium -mno-red-zone -o

LDFLAGS=-nostdlib -znocombreloc \
	-T script.lds -o out/kernel.elf

clear:
	rm -R ./out
	mkdir ./out
	touch ./out/.gitkeep

rebuild: clear build

build: build-all link-all

vhdinstall:
	losetup --partscan /dev/loop0 ${VHDINSTALLPATH}
	mount /dev/loop0p1 /mnt
	cp out/kernel.elf /mnt/JUSTOS
	umount /dev/loop0p1
	losetup -d /dev/loop0

build-all:
	@as lib/mem.S -64 -o out/lib.mem.s.o
	@as lib/atomic.S -64 -o out/lib.atomic.s.o
	@gcc lib/physmem.c $(CFLAGS) out/lib.physmem.c.o
	@gcc lib/memory.c $(CFLAGS) out/lib.memory.c.o
	@gcc lib/console.c $(CFLAGS) out/lib.console.c.o
	@#gcc lib/crc32.c $(CFLAGS) out/lib.crc32.c.o
	@gcc lib/font.c $(CFLAGS) out/lib.font.c.o
	@#gcc lib/string.c $(CFLAGS) out/lib.string.c.o
	@echo kernel/lib compiled successfully!

	@gcc video.c $(CFLAGS) out/video.c.o
	@as header.S -64 -o out/header.s.o
	@gcc kernel.c $(CFLAGS) out/kernel.c.o
	@echo kernel compiled successfully!

link-all:
	@ld $(LDFLAGS) out/header.s.o out/kernel.c.o \
		out/lib.mem.s.o out/lib.atomic.s.o out/lib.physmem.c.o out/lib.memory.c.o \
		out/lib.font.c.o out/video.c.o out/lib.console.c.o 
	@echo all built successfully!
