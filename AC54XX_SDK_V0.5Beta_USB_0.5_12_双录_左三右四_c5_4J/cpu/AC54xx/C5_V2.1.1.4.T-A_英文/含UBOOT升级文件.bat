@echo off

dv16_isd_sdr.exe isd_tools_flash-new.cfg -f uboot.boot ui.apu sdram.apu -resource res audlogo 32 -aline 4096 -bfumode5 

del upgrade\JL_AC*.bfu upgrade\*.bin

file_package.exe -file sdram.bin -dir upgrade -o package
copy /B JL_AC51.bfu+package upgrade\JL_AC54.bfu

del JL_AC51.bfu package
echo.
echo 升级文件在upgrade目录下，将upgrade目录下的所有文件copy到SD卡的根目录，插卡上电即可升级
echo.
pause