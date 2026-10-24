#!/bin/sh

source '../local.env'

wants="bootldr/out/bootx64.efi bootldr/justboot.tsv bootldr/testicon.bmp kernel/out/kernel.elf kernel/out/libc.so"

data=($wants)

cp ../bootldr/out/bootx64.efi $TARGETMEDIAPATH/EFI/Boot

for word in "${data[@]}"; do
    cp ../$word $TARGETMEDIAPATH/JUSTOS
done