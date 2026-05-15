#!/usr/bin/env bash
set -e

APP=animedl
APPDIR=animedl.AppDir

# Qt from installer
export QMAKE=/home/ifan/Qt/6.11.1/gcc_64/bin/qmake
export PATH="/home/ifan/Qt/6.11.1/gcc_64/bin:$PATH"

# clean
rm -rf "$APPDIR"

# basic structure
mkdir -p "$APPDIR/usr/bin"

# binaries
cp ../build/animedl "$APPDIR/usr/bin/"
cp -r ../build/backend "$APPDIR/usr/bin/"
cp ../build/yt-dlp "$APPDIR/usr/bin/"

chmod +x "$APPDIR/usr/bin/"*
chmod +x "$APPDIR/usr/bin/backend/"* || true

# desktop/icon
cp animedl.desktop "$APPDIR/"
cp ../icons/icon.png "$APPDIR/$APP.png"

# AppRun
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/sh

HERE="$(dirname "$(readlink -f "$0")")"

export QT_PLUGIN_PATH="$HERE/usr/plugins"
export QML2_IMPORT_PATH="$HERE/usr/qml"

exec "$HERE/usr/bin/animedl" "$@"
EOF

chmod +x "$APPDIR/AppRun"

# linuxdeploy tools
chmod +x linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage

export NO_STRIP=1

# build AppImage
./linuxdeploy-x86_64.AppImage \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/animedl" \
    --desktop-file "$APPDIR/animedl.desktop" \
    --icon-file "$APPDIR/$APP.png" \
    --plugin qt \
    --output appimage

mv Anime_Downloader-x86_64.AppImage "$APP.AppImage"
