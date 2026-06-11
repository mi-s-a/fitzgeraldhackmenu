@echo off

"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
fitzgeraldhackmenu.vcxproj ^
/p:Configuration=Release ^
/p:Platform=x64 ^
/p:PlatformToolset=v143 ^
/p:TargetName=fitzgeraldhackmenu_utility ^
/m

pause