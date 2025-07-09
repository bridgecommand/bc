#!/bin/sh
rm -rf ./bridgecommand
rm ./bridgecommand.deb

mkdir ./bridgecommand
cd ./bridgecommand
mkdir DEBIAN
mkdir usr
cd usr
mkdir bin
mkdir share
cd bin
cp ../../../../../bin/linux/bridgecommand* .
cd ..
cd share
mkdir applications
mkdir bridgecommand
mkdir doc
mkdir doc/bridgecommand
mkdir icons
mkdir icons/hicolor
mkdir icons/hicolor/48x48
mkdir icons/hicolor/48x48/apps
cp -ar ../../../../../resources/* ./bridgecommand/
cp -a ../../../../../LICENSE ./bridgecommand/
cp -a ../../../../../readme.md ./bridgecommand/
cp -a ../../../../../doc/* ./doc/bridgecommand/
cp -a ../../../bridgecommand.desktop ./applications/
cp -a ../../../bridgecommand.png ./icons/hicolor/48x48/apps/

cd ../../DEBIAN
file="control"
arch=$(dpkg --print-architecture)
versionNo=$(../usr/bin/bridgecommand --version)
echo "Package: bridgecommand-SOMOS-version" > $file
echo "Version: $versionNo" >> $file
echo "Maintainer: Florent Richard <florent.richard@supmaritime.fr>" >> $file
echo "Priority: optional" >> $file
echo "Architecture: $arch" >> $file
echo "Depends: libc6 (>= 2.34), libgcc-s1 (>= 3.0), libgl1, libportaudio2 (>= 19+svn20101113), libsndfile1 (>= 1.0.20), libstdc++6 (>= 11), libx11-6, libxxf86vm1, libopenxr-loader1" >> $file
echo "Description: A ship simulation program" >> $file
echo " For use in navigation and bridge training" >> $file

cat control

cd ../..

find bridgecommand/usr -type d -exec chmod 755 {} \;
find bridgecommand/usr -type f -exec chmod 644 {} \;

chmod 755 bridgecommand/usr/bin/*
chmod +x bridgecommand/usr/share/bridgecommand/scripts/linux/*

strip --strip-unneeded --remove-section=.comment --remove-section=.note bridgecommand/usr/bin/*

fakeroot dpkg-deb --build bridgecommand

