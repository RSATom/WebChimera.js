#!/bin/sh

rm -rf ./build
rm -rf ./Release
mkdir ./Release

./build_nwjs.sh
zip -j ./Release/WebChimera.js_nwjs_mac.zip ./build/Release/WebChimera.js.node
rm -rf ./build

./build_electron.sh
zip -j ./Release/WebChimera.js_electron_mac.zip ./build/Release/WebChimera.js.node
rm -rf ./build
