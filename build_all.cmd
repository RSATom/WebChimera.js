rd /S /Q build
rd /S /Q build_nw_ia32
rd /S /Q build_electron_ia32

call cmake-js rebuild --runtime=nw --arch=ia32 --runtime-version=0.12.0
rename build build_nw_ia32

call cmake-js rebuild --runtime=electron --arch=ia32 --runtime-version=0.27.0
rename build build_electron_ia32
