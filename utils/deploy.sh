#!/bin/sh

source '../local.env'

wants="bootldr/out/bootx64.efi assets/testicon.bmp kernel/out/kernel.elf kernel/out/xlibc.so"

data=($wants)

cp ../bootldr/out/bootx64.efi $TARGETMEDIAPATH/EFI/Boot

for word in "${data[@]}"; do
    cp ../$word $TARGETMEDIAPATH/JUSTOS
done