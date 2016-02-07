setlocal EnableDelayedExpansion

if "%npm_config_wcjs_arch%"=="x64" (
    set VLC_ARCHIVE=vlc-%VLC_VER%-win64.7z
    set VLC_ARCHIVE_URL="http://get.videolan.org/vlc/%VLC_VER%/win64/!VLC_ARCHIVE!"
) else (
    set VLC_ARCHIVE=vlc-%VLC_VER%-win32.7z
    set VLC_ARCHIVE_URL="http://get.videolan.org/vlc/%VLC_VER%/win32/!VLC_ARCHIVE!"
)

set FULL_PACKAGE_DIR=%BUILD_DIR%\webchimera.js
mkdir %FULL_PACKAGE_DIR%

appveyor DownloadFile -Url %VLC_ARCHIVE_URL%
7z x %VLC_ARCHIVE% -o%BUILD_DIR%
set VLC_DIR=%BUILD_DIR%\vlc-%VLC_VER%

copy /Y %BUILD_DIR%\WebChimera.js.node %FULL_PACKAGE_DIR%
echo module.exports = require('./WebChimera.js.node')> %FULL_PACKAGE_DIR%\index.js

echo plugins\access_output > exclude_vlc_plugins.txt
echo plugins\control >> exclude_vlc_plugins.txt
echo plugins\gui >> exclude_vlc_plugins.txt
echo plugins\video_output >> exclude_vlc_plugins.txt
echo plugins\visualization >> exclude_vlc_plugins.txt

xcopy /E /I /Q /Y %VLC_DIR%\plugins %FULL_PACKAGE_DIR%\plugins /EXCLUDE:exclude_vlc_plugins.txt
xcopy /Q /Y %VLC_DIR%\plugins\video_output\libvmem_plugin.dll %FULL_PACKAGE_DIR%\plugins\video_output\
copy /Y %VLC_DIR%\libvlc.dll %FULL_PACKAGE_DIR%
copy /Y %VLC_DIR%\libvlccore.dll %FULL_PACKAGE_DIR%

7z a %BUILD_DIR%\%WCJS_FULL_ARCHIVE% %FULL_PACKAGE_DIR%
