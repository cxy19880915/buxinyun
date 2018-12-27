

REM dv10_app_make.exe -addr 0x4000000 -infile sdram.bin  -compression
apu_make.exe -addr 0x4000000 -infile sdram.bin  -ofile sdram.apu

dv16_isd_sdr.exe isd_tools_flash-new.cfg -f uboot.boot ui.apu sdram.apu -resource res audlogo 32 -tonorflash -dev dv16 -boot 0x3f02000 -div2 -runaddr 0x04000000  -aline 4096 -bfumode3 -reboot

