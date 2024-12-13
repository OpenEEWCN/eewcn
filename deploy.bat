@echo off
set QTDIR=C:\Qt\5.15.2\mingw81_32
set QTQIF=C:\Qt\QtIFW-4.3.0

pushd .
call "%QTDIR%\bin\qtenv2.bat"
popd
set INSTALLERDIR=%cd%-deploy
set DEPLOYDIR=%cd%-deploy\packages\io.github.openeewcn.eewcn\data
md "%DEPLOYDIR%"
if not exist build\Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\*.exe goto:eof
copy build\Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\*.exe "%DEPLOYDIR%"
xcopy /e /y Fonts "%DEPLOYDIR%\Fonts\"
xcopy /e /y Media "%DEPLOYDIR%\Media\"
xcopy /e /y geodata "%DEPLOYDIR%\geodata\"
copy /y eewcn_*.qm "%DEPLOYDIR%"
copy /y opensslbin\*.dll "%DEPLOYDIR%"
windeployqt --qmldir . "%DEPLOYDIR%"
copy /y qt_zh_CN.qm "%DEPLOYDIR%\translations"

md "%INSTALLERDIR%\config"
md "%INSTALLERDIR%\packages\io.github.openeewcn.eewcn\meta"

set CONFIGXML=%INSTALLERDIR%\config\config.xml
set PACKAGEXML=%INSTALLERDIR%\packages\io.github.openeewcn.eewcn\meta\package.xml

for /f "tokens=2,3* delims= " %%i in ('type value.h^|find "#define VERSION"') do @@set %%i=%%j
set VERSION=%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_PATCH%

echo ^<?xml version="1.0" encoding="UTF-8"?^>>"%CONFIGXML%"
echo ^<Installer^>>>"%CONFIGXML%"
echo     ^<Name^>EEW CN^</Name^>>>"%CONFIGXML%"
echo     ^<Version^>%VERSION%^</Version^>>>"%CONFIGXML%"
echo     ^<Title^>EEW CN^</Title^>>>"%CONFIGXML%"
echo     ^<Publisher^>OpenEEWCN^</Publisher^>>>"%CONFIGXML%"
echo     ^<StartMenuDir^>EEW CN^</StartMenuDir^>>>"%CONFIGXML%"
echo     ^<TargetDir^>@ApplicationsDirX86@/eewcn^</TargetDir^>>>"%CONFIGXML%"
echo     ^<InstallerApplicationIcon^>eewcn^</InstallerApplicationIcon^>>>"%CONFIGXML%"
echo     ^<InstallerWindowIcon^>eewcn.ico^</InstallerWindowIcon^>>>"%CONFIGXML%"
echo     ^<SupportsModify^>false^</SupportsModify^>>>"%CONFIGXML%"
echo     ^<ControlScript^>control.qs^</ControlScript^>>>"%CONFIGXML%"
echo ^</Installer^>>>"%CONFIGXML%"

echo ^<?xml version="1.0" encoding="UTF-8"?^>>"%PACKAGEXML%"
echo ^<Package^>>>"%PACKAGEXML%"
echo     ^<DisplayName^>EEW CN^</DisplayName^>>>"%PACKAGEXML%"
echo     ^<Description^>EEW CN^</Description^>>>"%PACKAGEXML%"
echo     ^<Version^>1.1.33^</Version^>>>"%PACKAGEXML%"
echo     ^<ReleaseDate^>%DATE:~0,4%-%DATE:~5,2%-%DATE:~8,2%^</ReleaseDate^>>>"%PACKAGEXML%"
echo     ^<Licenses^>>>"%PACKAGEXML%"
echo         ^<License name="EEW CN License" file="license.txt" /^>>>"%PACKAGEXML%"
echo     ^</Licenses^>>>"%PACKAGEXML%"
echo     ^<Default^>script^</Default^>>>"%PACKAGEXML%"
echo     ^<Script^>install.qs^</Script^>>>"%PACKAGEXML%"
echo     ^<UserInterfaces^>>>"%PACKAGEXML%"
echo         ^<UserInterface^>page.ui^</UserInterface^>>>"%PACKAGEXML%"
echo         ^<UserInterface^>checkfinish.ui^</UserInterface^>>>"%PACKAGEXML%"
echo     ^</UserInterfaces^>>>"%PACKAGEXML%"
::echo     ^<Translations^>>>"%PACKAGEXML%"
::echo         ^<Translation^>ja.qm^</Translation^>>>"%PACKAGEXML%"
::echo         ^<Translation^>zh_CN.qm^</Translation^>>>"%PACKAGEXML%"
::echo     ^</Translations^>>>"%PACKAGEXML%"
echo     ^<Operations^>>>"%PACKAGEXML%"
echo         ^<Operation name="CreateShortcut"^>>>"%PACKAGEXML%"
echo             ^<Argument^>@TargetDir@/eewcn.exe^</Argument^>>>"%PACKAGEXML%"
echo             ^<Argument^>@StartMenuDir@/EEW CN.lnk^</Argument^>>>"%PACKAGEXML%"
echo         ^</Operation^>>>"%PACKAGEXML%"
echo         ^<Operation name="CreateShortcut"^>>>"%PACKAGEXML%"
echo             ^<Argument^>@TargetDir@/maintenancetool.exe^</Argument^>>>"%PACKAGEXML%"
echo             ^<Argument^>@StartMenuDir@/^&#21368;^&#36733;.lnk^</Argument^>>>"%PACKAGEXML%"
echo         ^</Operation^>>>"%PACKAGEXML%"
echo         ^<Operation name="CreateShortcut"^>>>"%PACKAGEXML%"
echo             ^<Argument^>@TargetDir@/eewcn.exe^</Argument^>>>"%PACKAGEXML%"
echo             ^<Argument^>@DesktopDir@/EEW CN.lnk^</Argument^>>>"%PACKAGEXML%"
echo         ^</Operation^>>>"%PACKAGEXML%"
echo     ^</Operations^>>>"%PACKAGEXML%"
echo ^</Package^>>>"%PACKAGEXML%"

copy /y Installer\data\* "%DEPLOYDIR%"
copy /y Media\eewcn.ico "%INSTALLERDIR%\config"
copy /y Installer\config\* "%INSTALLERDIR%\config"
copy /y Installer\meta\* "%DEPLOYDIR%\..\meta"

set BINARYCREATOR=%QTQIF%\bin\binarycreator.exe
if not exist %BINARYCREATOR% goto:eof
%BINARYCREATOR% -f -c "%INSTALLERDIR%\config\config.xml" -p "%INSTALLERDIR%\packages" --ac 7 "%INSTALLERDIR%\eewcn.exe"
