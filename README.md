# <img alt="WebChimera.js" src="https://raw.githubusercontent.com/jaruba/wcjs-logos/master/logos/small/webchimera.png">
libvlc binding for node.js/io.js/NW.js/Electron

[![Join the chat at https://gitter.im/RSATom/WebChimera](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/RSATom/WebChimera?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Demos
* [WebChimera.js Renderer Demo](https://github.com/RSATom/wcjs-ugly-demo)
* [WebChimera.js Player - Single Video Demo](https://github.com/jaruba/node-vlcPlayer-demo)
* [WebChimera.js Player - Multiscreen Demo](https://github.com/jaruba/node-vlc-multiscreen)

## Docs
* [JavaScript API](https://github.com/RSATom/WebChimera.js/wiki/JavaScript-API)

## Known issues and workarounds
* [libvlc 2.2.1 is broken on OS X ](https://github.com/RSATom/WebChimera.js/wiki/Due-to-bug-libvlc-2.2.1-could-not-be-used-as-is-outside-VLC.app-on-Mac-OS-X)
* [libvlc has compatibility issue with Electron v0.36.x on Windows](https://github.com/RSATom/WebChimera.js/wiki/Electron-v0.36.x-compatibility-issue-on-Windows)
* [libvlc 3.0 has broken vmem plugin](https://github.com/RSATom/WebChimera.js/wiki/Vmem-plugin-in-VLC-3.0-is-broken)

## Known Projects using WebChimera.js
* [Stremio](http://www.strem.io/)
* [Powder Player](http://powder.media/)
* [Butter](http://butterproject.org/)

## Prebuilt binaries
* https://github.com/RSATom/WebChimera.js/releases

### Using prebuilt on Windows
* `npm install WebChimera.js --ignore-scripts`
* download `WebChimera.js_*_win.zip` and extract to `webchimera.js\Release`
* download `VLC` archive from [http://www.videolan.org](http://www.videolan.org/vlc/download-windows.html) and extract root folder contents from archive to `webchimera.js\Release`

### Using prebuilt on Mac OS X
* download `WebChimera.js_*_osx.tar.gz` and extract to `node_modules`

### Using prebuilt on Linux
* install `VLC` (for apt based distros: `sudo apt-get install vlc`)
* `npm install WebChimera.js --ignore-scripts`
* download `WebChimera.js_*_linux.zip` and extract to `webchimera.js\Release`

## Build Prerequisites
### Windows
* [Visual Studio Community 2013](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx);
* [VLC Player](http://www.videolan.org/vlc/download-windows.html) in your PATH environment variable;
* [CMake](http://www.cmake.org/);
* [io.js](https://iojs.org) or [Node.js](https://nodejs.org);
* [NW.js ia32](http://nwjs.io/) or [Electron ia32](http://electron.atom.io/);

### Mac OS X
* [Apple XCode](https://developer.apple.com/xcode/);
* [VLC Player](http://www.videolan.org/vlc/download-macosx.html);
* [CMake](http://www.cmake.org/);
* [io.js](https://iojs.org) or [Node.js](https://nodejs.org);
* [NW.js](http://nwjs.io/) or [Electron](http://electron.atom.io/);

### Linux
for apt based distros:
* `$ sudo apt-get install build-essential cmake libvlc-dev`

## Build from sources
### Windows
* `git clone --recursive https://github.com/RSATom/WebChimera.js.git`
* `cd WebChimera.js`
* `build_nwjs.cmd` or `build_electron.cmd` or `build_node.cmd` or `build_iojs.cmd`

### Mac OS X & Linux
* `git clone --recursive https://github.com/RSATom/WebChimera.js.git`
* `cd WebChimera.js`
* `./build_nwjs.sh` or `./build_electron.sh` or `./build_node.sh` or `./build_iojs.sh`
