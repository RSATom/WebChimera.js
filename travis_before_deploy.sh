#!/bin/sh

VLC_APP=./deps/VLC.app
OUT_DIR=$BUILD_DIR/webchimera.js

if [ ! $TRAVIS ]; then
  exit 1
fi

zip -j $WCJS_ARCHIVE_PATH $BUILD_DIR/WebChimera.js.node

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  mkdir $OUT_DIR

  cp -f $BUILD_DIR/WebChimera.js.node $OUT_DIR
  echo "module.exports = require('./WebChimera.js.node')" > $OUT_DIR/index.js

  mkdir $OUT_DIR/lib
  cp -R $VLC_APP/Contents/MacOS/lib/*.dylib $OUT_DIR/lib

  mkdir -p $OUT_DIR/lib/vlc
  cp -R $VLC_APP/Contents/MacOS/plugins $OUT_DIR/lib/vlc

  mkdir -p $OUT_DIR/lib/vlc/share/lua
  cp -R $VLC_APP/Contents/MacOS/share/lua/extensions $OUT_DIR/lib/vlc/share/lua
  cp -R $VLC_APP/Contents/MacOS/share/lua/modules $OUT_DIR/lib/vlc/share/lua
  cp -R $VLC_APP/Contents/MacOS/share/lua/playlist $OUT_DIR/lib/vlc/share/lua

  mkdir -p $OUT_DIR/lib/vlc/lib
  ln -sf ../../libvlccore.9.dylib $OUT_DIR/lib/vlc/lib/libvlccore.9.dylib

  tar -cvzf $WCJS_FULL_ARCHIVE_PATH -C $BUILD_DIR webchimera.js

  rm -rf $OUT_DIR
fi
