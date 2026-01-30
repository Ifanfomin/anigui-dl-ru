#/usr/bin/sh
rm -rf animedl.AppDir
mkdir animedl.AppDir
mkdir -p animedl.AppDir/usr/lib/qt6/plugins/
mkdir animedl.AppDir/usr/bin/

cp -r /usr/lib/qt6/plugins/platforms animedl.AppDir/usr/lib/qt6/plugins/
cp -r /usr/lib/qt6/plugins/imageformats animedl.AppDir/usr/lib/qt6/plugins/
cp -r /usr/lib/qt6/plugins/iconengines animedl.AppDir/usr/lib/qt6/plugins/
cp -r /usr/lib/qt6/plugins/styles animedl.AppDir/usr/lib/qt6/plugins/

cp ../build/Desktop-Release/animedl animedl.AppDir/usr/bin/
cp -r ../build/Desktop-Release/backend animedl.AppDir/usr/bin/backend
cp ../build/Desktop-Release/yt-dlp animedl.AppDir/usr/bin/
chmod +x animedl.AppDir/usr/bin/*

cp ../icons/icon.png animedl.AppDir/animedl.png
cp animedl.desktop animedl.AppDir/
cp AppRun animedl.AppDir/

# wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage 
# wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
# chmod +x linuxdeploy*.AppImage

./linuxdeploy-x86_64.AppImage \
  --appdir animedl.AppDir \
  --executable animedl.AppDir/usr/bin/animedl \
  --desktop-file animedl.AppDir/animedl.desktop \
  --icon-file animedl.AppDir/animedl.png

# проверка до сборки
# animedl.AppDir/AppRun

# сборка
./linuxdeploy-x86_64.AppImage --appdir animedl.AppDir --output appimage

mv Anime_Downloader-x86_64.AppImage animedl.AppImage
