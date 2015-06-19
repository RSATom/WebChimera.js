WebChimera.js
---
libvlc binding for node.js/io.js/NW.js/Electron

[![Join the chat at https://gitter.im/RSATom/WebChimera](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/RSATom/WebChimera?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Demos
* [WebChimera.js Player - Single Video Demo](https://github.com/jaruba/node-vlcPlayer-demo)
* [WebChimera.js Player - Multiscreen Demo](https://github.com/jaruba/node-vlc-multiscreen)
* [WebChimera.js Renderer Demo](https://github.com/RSATom/wcjs-ugly-demo)

## Docs
* [JavaScript API](https://github.com/RSATom/WebChimera.js/wiki/JavaScript-API)

## Known Projects using WebChimera.js
* [Stremio](http://blog.strem.io/post/121596387436/alpha-of-stremio-2-0)

## Build Prerequisites
### Windows
* [Visual Studio Community 2013](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx);
* [VLC Player](http://www.videolan.org/vlc/download-windows.html) in your PATH environment variable;
* [CMake](http://www.cmake.org/);
* [io.js](https://iojs.org) or [Node.js](https://nodejs.org);

### Mac OS X
* [Apple XCode](https://developer.apple.com/xcode/);
* [VLC Player](http://www.videolan.org/vlc/download-macosx.html);
* [CMake](http://www.cmake.org/);
* [io.js](https://iojs.org) or [Node.js](https://nodejs.org);

### Linux
for apt based distros:
* `$ sudo apt-get install build-essential cmake libvlc-dev`

## Build from sources
### Windows
* `git clone --recursive https://github.com/RSATom/WebChimera.js.git`
* `cd WebChimera.js`
* `build_nwjs.cmd` or `build_electron.cmd`

### Mac OS X & Linux
* `git clone --recursive https://github.com/RSATom/WebChimera.js.git`
* `cd WebChimera.js`
* `./build_nwjs.sh` or `./build_electron.sh`
