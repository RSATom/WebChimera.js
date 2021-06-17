# <img alt="WebChimera.js" src="https://raw.githubusercontent.com/jaruba/wcjs-logos/master/logos/small/webchimera.png">
libvlc binding for Electron

[![Join the chat at https://gitter.im/RSATom/WebChimera](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/RSATom/WebChimera?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Join the chat at https://discord.gg/3h3K3JF](https://img.shields.io/discord/716939396464508958?label=discord)](https://discord.gg/3h3K3JF)

## Demos
* [WebChimera.js Renderer Demo](https://github.com/RSATom/wcjs-ugly-demo)
* [WebChimera.js Player - Single Video Demo](https://github.com/jaruba/node-vlcPlayer-demo)
* [WebChimera.js Player - Multiscreen Demo](https://github.com/jaruba/node-vlc-multiscreen)

## Docs
* [JavaScript API](https://github.com/RSATom/WebChimera.js/wiki/JavaScript-API)

## Known issues and workarounds
* [latest `Electron` versions is not compatible with `WebChimera.js` on Linux](https://github.com/RSATom/WebChimera.js/issues/69)
* [libvlc 2.2.x is broken on OS X](https://github.com/RSATom/WebChimera.js/wiki/Due-to-bug-libvlc-2.2.x-could-not-be-used-as-is-outside-VLC.app-on-Mac-OS-X)
* [libvlc has compatibility issue with latest `Electron`/`NW.js` versions on Windows](https://github.com/RSATom/WebChimera.js/wiki/Latest-Electron-and-NW.js-versions-has-compatibility-issue-on-Windows)
* [libvlc 2.2.x x64 has issue with subtitles](https://github.com/RSATom/WebChimera.js/issues/65)
* [Webpack](https://github.com/RSATom/WebChimera.js/wiki/WebChimera.js-with-Webpack)


## Prebuilt binaries
* https://github.com/RSATom/WebChimera.js/releases

### Using prebuilt on Windows
* download `WebChimera.js_*_VLC_*_win.zip` corresponding to your engine and extract to `node_modules`

### Using prebuilt on Mac OS X
* download `WebChimera.js_*_osx.tar.gz` corresponding to your engine and extract to `node_modules`

### Using prebuilt on Linux
* install `VLC` (for apt based distros: `sudo apt-get install vlc`)
* `npm install webchimera.js --ignore-scripts`
* download `WebChimera.js_*_linux.zip` and extract to `webchimera.js\Release`

## Build Prerequisites
### Windows
* [Visual Studio Community 2019](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx);
* [VLC Player](http://www.videolan.org/vlc/download-windows.html) in your PATH environment variable;
* [CMake](http://www.cmake.org/);
* [Electron](https://www.electronjs.org/);

### Mac OS X
* [Apple XCode](https://developer.apple.com/xcode/);
* [VLC Player](http://www.videolan.org/vlc/download-macosx.html);
* [CMake](http://www.cmake.org/);
* [Node.js](https://nodejs.org);
* [Electron](https://www.electronjs.org/);

### Linux
for apt based distros:
* `$ sudo apt-get install build-essential cmake libvlc-dev`

## Build from sources
### Windows
* `git clone --recursive https://github.com/RSATom/WebChimera.js.git`
* `cd WebChimera.js`
* `build_electron.cmd`

### Mac OS X & Linux
* `git clone --recursive https://github.com/RSATom/WebChimera.js.git`
* `cd WebChimera.js`
* `./build_electron.sh`
