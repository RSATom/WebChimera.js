rd /S /Q build
rd /S /Q Release

call build_nwjs.cmd
"%ProgramFiles%\7-Zip\7z" a ./Release/WebChimera.js_nwjs_win.zip ./build/Release/WebChimera.js.node
rd /S /Q build

call build_electron.cmd
"%ProgramFiles%\7-Zip\7z" a ./Release/WebChimera.js_electron_win.zip ./build/Release/WebChimera.js.node
rd /S /Q build
