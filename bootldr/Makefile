include ../local.env

C_SOURCES := video.c console.c font.c tsv.c bootloader.c
ASM_SOURCES := final.S mem.S
OBJECTS := $(C_SOURCES:.c=.c.o) $(ASM_SOURCES:.S=.S.o)
OUT_DIR := out

CFLAGS := -c -I /usr/local/include/efi \
	-ffreestanding -mabi=sysv -Os \
	-I /usr/local/include/efi/x86_64 \
	-I /usr/local/include/efi/protocol \
	-fno-stack-protector -fshort-wchar \
	-mno-red-zone -Wall -Wextra -Werror \
	-fpic -Wno-maybe-uninitialized -Wno-unused-function -DEFI_FUNCTION_WRAPPER

LDFLAGS := /usr/local/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc \
	-T /usr/local/lib/elf_x86_64_efi.lds \
	-shared -Bsymbolic -L /usr/local/lib \
	-l:libgnuefi.a -l:libefi.a

$(OUT_DIR)/%.c.o: %.c
	@gcc $< $(CFLAGS) -o $@

$(OUT_DIR)/%.S.o: %.S
	@as $< -64 -o $@

.PHONY: rebuild build clean vhdinstall

rebuild: clean build

vhdinstall:
	losetup --partscan /dev/loop0 ${VHDINSTALLPATH}
	mount /dev/loop0p1 /mnt
	cp $(OUT_DIR)/bootx64.efi /mnt/EFI/Boot
	cp justboot.tsv /mnt/JUSTOS
	umount /dev/loop0p1
	losetup -d /dev/loop0

clean:
	rm -R $(OUT_DIR)
	mkdir $(OUT_DIR)
	touch ./$(OUT_DIR)/.gitkeep

build: $(addprefix $(OUT_DIR)/,$(OBJECTS))
	@ld $^ $(LDFLAGS) -o $(OUT_DIR)/bootldr.so
	@objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
		-j .rel -j .rela -j .reloc -O efi-app-x86_64 \
		$(OUT_DIR)/bootldr.so $(OUT_DIR)/bootx64.efi
	@echo Build successful
