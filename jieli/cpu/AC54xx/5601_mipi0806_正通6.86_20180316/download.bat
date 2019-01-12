

C:\JL\pi32/bin/llvm-objcopy -O binary -j .rom_code main.or32 sdram_text.bin

C:\JL\pi32\bin/llvm-objcopy -O binary -j .ram0_data main.or32 ram0_data.bin

C:\JL\pi32\bin/llvm-objcopy -O binary -j .ram2_data main.or32 ram2_data.bin

copy  sdram_text.bin /b + ram0_data.bin /b + ram2_data.bin /b  sdram.bin

C:\JL\pi32\bin/llvm-objdump -h  main.or32

call ÏÂÔØ´úÂë.bat
