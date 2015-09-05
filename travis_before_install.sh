#!/bin/sh

if [ ! $TRAVIS ]
then
	exit 1
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  curl -L -o ./deps/vlc-$VLC_VER.dmg http://get.videolan.org/vlc/$VLC_VER/macosx/vlc-$VLC_VER.dmg
  hdiutil mount ./deps/vlc-$VLC_VER.dmg
  cp -R "/Volumes/vlc-$VLC_VER/VLC.app" ./deps
fi
