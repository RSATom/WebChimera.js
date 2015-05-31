#!/bin/sh

rm -rf ./build
rm -rf ./Release
mkdir ./Release

cmake-js rebuild --runtime=nw --runtime-version=0.12.0
zip -j ./Release/WebChimera.js_nwjs_mac.zip ./build/Release/WebChimera.js.node
rm -rf ./build

cmake-js rebuild --runtime=electron --runtime-version=0.27.0
zip -j ./Release/WebChimera.js_electron_mac.zip ./build/Release/WebChimera.js.node
rm -rf ./build
