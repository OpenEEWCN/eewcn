::@echo off
setlocal ENABLEDELAYEDEXPANSION
set LUPDATE=D:\Qt\5.15.2\mingw81_32\bin\lupdate.exe

set LST=
for %%i in (*.ui *.qs) do set LST=!LST! %%i
"%LUPDATE%" !LST! -target-language ja_JP -ts ja.ts
"%LUPDATE%" !LST! -target-language zh_CN -ts zh_CN.ts
"%LUPDATE%" !LST! -target-language zh_TW -ts zh_TW.ts
