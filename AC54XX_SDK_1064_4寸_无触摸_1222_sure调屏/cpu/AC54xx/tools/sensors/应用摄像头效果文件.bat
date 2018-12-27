


::！！！！！更改摄像头效果文件请改此处, 注意路径信息！！！！

..\dv16_isp_cfg.exe -i gc2023\JL_JT_NIGHT.J3L -o ..\res\isp_cfg_0.bin
..\dv16_isp_cfg.exe -i gc2023\JL_JT_INDOOR.J3L -o 	..\res\isp_cfg_1.bin
..\dv16_isp_cfg.exe -i gc2023\JL_JT_OUTCLUDY.J3L -o ..\res\isp_cfg_2.bin
..\dv16_isp_cfg.exe -i gc2023\JL_JT_OUTDOOR.J3L -o ..\res\isp_cfg_3.bin

set /p  run="是否启动下载？(y / n)
echo %run%
if %run%==y  (
    cd ..
    call 下载代码.bat
)





