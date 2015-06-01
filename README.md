WebChimera.js
---

libvlc binding for node.js/io.js/NW.js/Electron

## Windows

### Build requirements
* [Visual Studio Community 2013](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx);
* [VLC Player](http://www.videolan.org/vlc/download-windows.html) in your PATH environment variable;
* [CMake](http://www.cmake.org/);
* [io.js](https://iojs.org) or [Node.js](https://nodejs.org);

### Install & build
* `git clone --recursive https://github.com/RSATom/WebChimera.js.git`
* `cd WebChimera.js`
* `npm install`

## Linux

Clone this repository

```bash
$ git clone --recursive https://github.com/mallendeo/WebChimera.js && cd WebChimera.js
```
Install dev dependencies and cmake

For apt based distros
```bash
$ sudo apt-get install build-essentials cmake libvlc-dev
```

Run npm install script
```bash
$ npm install
```
  

Test demo in nw.js
```bash
$ cd demos/ugly
$ npm install
$ sudo npm install nw -g
$ nw
```
