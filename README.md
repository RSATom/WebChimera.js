# <img alt="WebChimera.js" src="https://raw.githubusercontent.com/jaruba/wcjs-logos/master/logos/small/webchimera.png">
libvlc binding for node.js/io.js/NW.js/Electron

[![Join the chat at https://gitter.im/RSATom/WebChimera](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/RSATom/WebChimera?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Demos
* [WebChimera.js Player - Single Video Demo](https://github.com/jaruba/node-vlcPlayer-demo)
* [WebChimera.js Player - Multiscreen Demo](https://github.com/jaruba/node-vlc-multiscreen)
* [WebChimera.js Renderer Demo](https://github.com/RSATom/wcjs-ugly-demo)

## Docs
* [JavaScript API](https://github.com/RSATom/WebChimera.js/wiki/JavaScript-API)

## Known Projects using WebChimera.js
* [Stremio](http://www.strem.io/)
* [Powder Player](http://powder.media/)

## Prebuilt
To install WebChimera.js without building it, use [wcjs-prebuilt](http://github.com/Ivshti/wcjs-prebuilt) which supports Electron and NW.js environments.
```bash
npm install wcjs-prebuilt
```

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
