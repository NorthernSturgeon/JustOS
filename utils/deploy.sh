#!/bin/sh

source '../local.env'

wants="bootldr/out/bootx64.efi kernel/out/kernel.elf kernel/out/xlibc.so"

data=($wants)

cp ../bootldr/out/bootx64.efi $TARGETMEDIAPATH/EFI/Boot
cp ../$SPLASHPATH $TARGETMEDIAPATH/JUSTOS/splash.bmp

for word in "${data[@]}"; do
    cp ../$word $TARGETMEDIAPATH/JUSTOS
done