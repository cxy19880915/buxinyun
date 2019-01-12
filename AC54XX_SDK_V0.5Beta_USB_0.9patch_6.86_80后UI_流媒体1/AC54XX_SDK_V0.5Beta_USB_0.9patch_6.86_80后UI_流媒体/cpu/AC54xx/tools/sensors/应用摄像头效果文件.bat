


::！！！！！更改摄像头效果文件请改此处, 注意路径信息！！！！

..\dv16_isp_cfg.exe -i bg0806\JL_JT_0.J3L -o ..\res\isp_cfg_0.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_1.J3L -o ..\res\isp_cfg_1.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_2.J3L -o ..\res\isp_cfg_2.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_3.J3L -o ..\res\isp_cfg_3.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_4.J3L -o ..\res\isp_cfg_4.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_5.J3L -o ..\res\isp_cfg_5.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_5_1.J3L -o ..\res\isp_cfg_5_1.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_5_2.J3L -o ..\res\isp_cfg_5_2.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_6.J3L -o ..\res\isp_cfg_6.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_7.J3L -o ..\res\isp_cfg_7.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_8.J3L -o ..\res\isp_cfg_8.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_9.J3L -o ..\res\isp_cfg_9.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_10.J3L -o ..\res\isp_cfg_10.bin
..\dv16_isp_cfg.exe -i bg0806\JL_JT_11.J3L -o ..\res\isp_cfg_11.bin


set /p  run="是否启动下载？(y / n)
echo %run%
if %run%==y  (
    cd ..
    call 下载代码.bat
)





