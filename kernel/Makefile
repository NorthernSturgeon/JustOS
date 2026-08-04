include ../local.env

LIBC_SOURCES := lib/mem.S lib/atomic.S lib/memory.c \
				lib/console.c lib/font.c
#				lib/crc32.c lib/string.c

KERNEL_SOURCES := header.S kernel.c isr.S video.c \
				physmem.c isr_id.S interrupts.c

CFLAGS := -c -I include -nostdlib -ffreestanding -fPIC \
	-fomit-frame-pointer -fno-stack-protector \
	-Wall -Wextra -Werror -fno-plt -ftree-vectorize \
	-fno-asynchronous-unwind-tables -include lib/types.h \
	-march=x86-64 -std=c17 -fno-tree-vrp -mstackrealign \
	-O2 -mcmodel=medium -mno-red-zone -fno-semantic-interposition

KERNEL_CFLAGS := $(CFLAGS) -fvisibility=hidden

LIBC_CFLAGS := $(CFLAGS)

KERNEL_LDFLAGS := -nostdlib -z relro -z now --warn-unresolved-symbols -h '\JUSTOS\kernel.elf' -I JUSTOS/bootx64.efi -pic --export-dynamic -T script.lds

LIBC_LDFLAGS := -nostdlib -z relro -z now -h '\JUSTOS\libc.so' -shared -T libc.lds

OUT_DIR := out

LIBC_OBJS := $(addprefix $(OUT_DIR)/,$(subst /,.,$(LIBC_SOURCES:.c=.c.o)))
LIBC_OBJS := $(LIBC_OBJS:.S=.S.o)

KERNEL_OBJS := $(addprefix $(OUT_DIR)/,$(KERNEL_SOURCES:.c=.c.o))
KERNEL_OBJS := $(KERNEL_OBJS:.S=.S.o)

$(OUT_DIR)/lib.%.c.o: lib/%.c
	@gcc $< $(LIBC_CFLAGS) -o $@

$(OUT_DIR)/lib.%.S.o: lib/%.S
	@as $< -64 -o $@

$(OUT_DIR)/%.c.o: %.c
	@gcc $< $(KERNEL_CFLAGS) -o $@

$(OUT_DIR)/%.S.o: %.S
	@as $< -64 -o $@

.PHONY: all rebuild clear vhdinstall lib kernel

all: kernel

rebuild: clear all

clear:
	rm -Rf $(OUT_DIR)
	mkdir $(OUT_DIR)
	touch ./$(OUT_DIR)/.gitkeep

lib: $(LIBC_OBJS)
	@echo $(LIBC_OBJS)
	@ld $(LIBC_OBJS) $(LIBC_LDFLAGS) -o $(OUT_DIR)/libc.so
	@echo libc.so compiled successfully!

kernel: lib $(KERNEL_OBJS)
	@echo $(KERNEL_OBJS)
	@ld $(KERNEL_OBJS) $(KERNEL_LDFLAGS) -o $(OUT_DIR)/kernel.elf $(OUT_DIR)/libc.so
	@echo kernel.elf compiled successfully!

vhdinstall:
	losetup --partscan /dev/loop0 ${VHDINSTALLPATH}
	mount /dev/loop0p1 /mnt
	cp $(OUT_DIR)/kernel.elf /mnt/JUSTOS
	cp $(OUT_DIR)/libc.so /mnt/JUSTOS
	umount /dev/loop0p1
	losetup -d /dev/loop0