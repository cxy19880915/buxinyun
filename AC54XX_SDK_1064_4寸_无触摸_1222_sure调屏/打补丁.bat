@echo off
setlocal enabledelayedexpansion

if exist cpu\AC54xx\tools\audlogo\ANI.sty.pic rename cpu\AC54xx\tools\audlogo\ANI.sty.pic ANI.pic 
if exist cpu\AC54xx\tools\audlogo\ANI.sty.str rename cpu\AC54xx\tools\audlogo\ANI.sty.str ANI.str 

for /f "tokens=2,3,4 delims=_."  %%i in (patch_tools/patch_list.txt) do (
		set sdk_version_0=%%i
		set sdk_version_1=%%j
		set /a patch_version=%%k
)

echo 当前已打补丁:patch_!sdk_version_0!.!sdk_version_1!_!patch_version!
set /p input="请选择: 打补丁(y)/撤销上次补丁(n):"

if %input%a==ya (
	set /a v = !patch_version!+1
	set file_name=patch_!sdk_version_0!.!sdk_version_1!_!v!
) else (
	set file_name=patch_!sdk_version_0!.!sdk_version_1!_!patch_version!
)

set patch_file=patch_tools\!file_name!

if exist !patch_file! (
	if %input%a==ya (
		patch_tools\patch.exe -p1 < patch_tools/!file_name!
		echo !file_name!>> patch_tools\patch_list.txt 
	) else (
		patch_tools\patch.exe -R -p1 < patch_tools/!file_name!
		goto mf
	)
) else (
	echo no_avaliable_patch_file: !patch_file!
)
pause
exit

:mf

echo. > patch_tools\new_list.txt

for /f "tokens=2,3,4 delims=_."  %%i in (patch_tools\patch_list.txt) do (
		set /a b=%%k
		if !sdk_version_0! == %%i (
			if !sdk_version_1! == %%j (
				if !b! == !patch_version! (
					del patch_tools\patch_list.txt
					rename patch_tools\new_list.txt patch_list.txt
					echo 已撤销
					pause
					exit
				)
			)
		)
		echo patch_%%i.%%j_%%k>> patch_tools\new_list.txt
)

pause









