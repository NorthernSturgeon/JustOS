include ../local.env

CFLAGS:=-c -I /usr/local/include/efi \
	-ffreestanding -mabi=sysv -Os \
	-I /usr/local/include/efi/x86_64 \
	-I /usr/local/include/efi/protocol \
	-fno-stack-protector -fshort-wchar \
	-mno-red-zone -Wall -Wextra -Werror \
	-fpic -Wno-maybe-uninitialized -Wno-unused-function -DEFI_FUNCTION_WRAPPER

LDFLAGS:=/usr/local/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc \
	-T /usr/local/lib/elf_x86_64_efi.lds \
	-shared -Bsymbolic -L /usr/local/lib \
	-l:libgnuefi.a -l:libefi.a

rebuild: clear build

vhdinstall:
	losetup --partscan /dev/loop0 ${VHDINSTALLPATH}
	mount /dev/loop0p1 /mnt
	cp out/bootx64.efi /mnt/EFI/Boot
	cp justboot.tsv /mnt/JUSTOS
	umount /dev/loop0p1
	losetup -d /dev/loop0

clear:
	rm -R out
	mkdir out
	touch ./out/.gitkeep

build:
	@as final.S -64 -o out/final.o 
	@as mem.S -64 -o out/mem.o 
	@gcc video.c $(CFLAGS) -o out/video.o
	@gcc console.c $(CFLAGS) -o out/console.o
	@gcc font.c $(CFLAGS) -o out/font.o
	@gcc tsv.c $(CFLAGS) -o out/tsv.o
	@gcc bootloader.c $(CFLAGS) -o out/bootldr.o
	@ld out/bootldr.o out/tsv.o out/console.o out/video.o out/font.o out/mem.o out/final.o $(LDFLAGS) -o out/bootldr.so
	@objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
		-j .rel -j .rela -j .reloc -O efi-app-x86_64 \
		out/bootldr.so out/bootx64.efi
	@echo Build successful
