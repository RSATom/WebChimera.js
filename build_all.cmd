rd /S /Q build
rd /S /Q build_nw
rd /S /Q build_electron

call build_nw.cmd
rename build build_nw

call build_electron.cmd
rename build build_electron
