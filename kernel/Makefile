include ../local.env

CFLAGS=-c -I include -nostdlib -ffreestanding \
	-fomit-frame-pointer -fno-stack-protector \
	-Wall -Wextra -Werror -static -fPIE \
	-fno-asynchronous-unwind-tables -m64 \
	-mno-sse -mno-sse2 \
	-O0 -mcmodel=small -mno-red-zone -o

LDFLAGS=-nostdlib -znocombreloc --no-dynamic-linker \
	-T script.lds -o out/kernel.elf

clear:
	rm -R ./out
	mkdir ./out

rebuild: clear build

build: build-all link-all

vhdinstall:
	losetup --partscan /dev/loop0 ${VHDINSTALLPATH}
	mount /dev/loop0p1 /mnt
	cp out/kernel.elf /mnt/EFI/Boot
	sudo umount /dev/loop0p1
	losetup -d /dev/loop0

build-all:
	@as lib/mem.S -64 -o out/lib.mem.s.o
	@as lib/byteorder.S -64 -o out/lib.byteorder.s.o
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
		out/lib.mem.s.o out/lib.font.c.o \
		out/video.c.o out/lib.console.c.o \
		out/lib.byteorder.s.o
	@echo all built successfully!
