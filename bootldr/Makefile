include ../local.env

CFLAGS:=-c -I /usr/local/include/efi \
	-ffreestanding -mabi=sysv \
	-I /usr/local/include/efi/x86_64 \
	-I /usr/local/include/efi/protocol \
	-fno-stack-protector -fshort-wchar \
	-mno-red-zone -Wall -Wextra -Werror \
	-fpic -DEFI_FUNCTION_WRAPPER -ggdb

LDFLAGS:=-nostdlib -znocombreloc \
	-T /usr/local/lib/elf_x86_64_efi.lds \
	-shared -Bsymbolic -L /usr/local/lib \
	/usr/local/lib/crt0-efi-x86_64.o

rebuild: clear build

vhdinstall:
	losetup --partscan /dev/loop0 ${VHDINSTALLPATH}
	mount /dev/loop0p1 /mnt
	cp out/bootx64.efi /mnt/EFI/Boot
	sudo umount /dev/loop0p1
	losetup -d /dev/loop0

clear:
	rm -R out
	mkdir out

build:
	@as final.S -64 -o out/final.o 
	@as mem.S -64 -o out/mem.o 
	@gcc video.c $(CFLAGS) -o out/video.o
	@gcc console.c $(CFLAGS) -o out/console.o
	@gcc font.c $(CFLAGS) -o out/font.o
	@gcc bootloader.c $(CFLAGS) -o out/bootldr.o
	@ld out/bootldr.o out/console.o out/video.o out/font.o out/mem.o out/final.o $(LDFLAGS) -o out/bootldr.so -lefi -lgnuefi
	@objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
		-j .rel -j .rela -j .reloc --target=efi-app-x86_64 \
		out/bootldr.so out/bootx64.efi
	@echo Build successful
