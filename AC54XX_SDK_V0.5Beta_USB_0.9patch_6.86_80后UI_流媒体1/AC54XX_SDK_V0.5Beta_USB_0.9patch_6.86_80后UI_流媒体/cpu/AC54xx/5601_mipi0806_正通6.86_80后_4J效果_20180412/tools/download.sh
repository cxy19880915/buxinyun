#!/bin/sh

/opt/pi32v2/bin/pi32v2-uclinux-objcopy -O binary -j .rom_code cpu/AC54xx/tools/main.or32 cpu/AC54xx/tools/sdram_text.bin
/opt/pi32v2/bin/pi32v2-uclinux-objcopy -O binary -j .ram0_data cpu/AC54xx/tools/main.or32 cpu/AC54xx/tools/ram0_data.bin
/opt/pi32v2/bin/pi32v2-uclinux-objcopy -O binary -j .ram2_data cpu/AC54xx/tools/main.or32 cpu/AC54xx/tools/ram2_data.bin


cat cpu/AC54xx/tools/sdram_text.bin cpu/AC54xx/tools/ram0_data.bin cpu/AC54xx/tools/ram2_data.bin > cpu/AC54xx/tools/sdram.bin
#rm cpu/AC54xx/tools/sdram_text.bin cpu/AC54xx/tools/main.or32 cpu/AC54xx/tools/ram0_data.bin cpu/AC54xx/tools/ram2_data.bin

#/opt/pi32v2/bin/pi32v2-uclinux-objdump -h cpu/AC54xx/tools/main.or32

host-client -f cpu/AC54xx/tools/sdram.bin
