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
* [latest `Electron` versions is not compatible with `WebChimera.js` on Linux](https://github.com/RSATom/WebChimera.js/issues/69)
* [libvlc 2.2.x is broken on OS X](https://github.com/RSATom/WebChimera.js/wiki/Due-to-bug-libvlc-2.2.x-could-not-be-used-as-is-outside-VLC.app-on-Mac-OS-X)
* [libvlc has compatibility issue with latest `Electron`/`NW.js` versions on Windows](https://github.com/RSATom/WebChimera.js/wiki/Latest-Electron-and-NW.js-versions-has-compatibility-issue-on-Windows)
* [libvlc 2.2.x x64 has issue with subtitles](https://github.com/RSATom/WebChimera.js/issues/65)
* [Webpack](https://github.com/RSATom/WebChimera.js/wiki/WebChimera.js-with-Webpack)


## Known Projects using WebChimera.js
* [Stremio](http://www.strem.io/)
* [Powder Player](http://powder.media/)
* [Butter](http://butterproject.org/)
* [PagalPlayer](https://hlgkb.github.io/pagal-player/)

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



## Limitations 
### Read this for Android / Raspberry Pi / performance issues

WebChimera.js is a node.js module that utilizes libvlc and receives decoded frames in UIntArray objects. This is very efficient by itself. In pure node.js, we can receive the frames with absolutely **no overhead** because we directly give libvlc a memory address to decode in, to our pre-allocated raw memory of the UIntArray. 

However, in a real-world scenario, we'd need something to draw the frames in. If we use WebChimera.js with Electron/NW.js (most common use case), then we can draw the frames in WebGL by sending them to the GPU through texImage2D. That sounds good, and there should be no issues.

But Chromium uses a **multi-process architecture**, which means the GPU process and the renderer process, where our node.js context lies, are isolated between each other. When we call WebGL texImage2D, we transfer our frame buffer to the GPU process, which is a memcpy operation. Since texImage2D is a memcpy on it's own, we end up with two memcpy operations per frame. OK for a moderately powerful desktop machine, unless we render 4k or full HD 60fps, but obviously inefficient.

Because of that limitation, we cannot even begin to think about anything ARM based, where we don't have that clock speed or memory bandwidth. That means that Raspberry Pi or Android is **out of the question** with that architecture.
That doesn't mean WebChimera.js is incompatible with ARM by design - but **the combo of WebChimera.js + NW/Electron is terrible for mobile/embedded devices**.

Is there a way we can do it? Perhaps, if we have a low-level node.js binding to a graphical layer and avoid Chromium runtime somehow. For now, however, using a different solution is advised. 
