# Bridge Command

This is the source code for Bridge Command, developed in C++ using the
Irrlicht 3d library.

For more information, see https://www.bridgecommand.co.uk

## To compile Irrlicht on Windows
----------
1) Get Irrlicht from SVN repo here : svn://svn.code.sf.net/p/irrlicht/code/trunk
2) Put it in bin/libs/irrlicht
3) Download last DirectX SDK here : https://www.microsoft.com/en-us/download/details.aspx?id=6812
4) Add Irrlicht project to BC project into VS : bin/libs/irrlicht/source/Irrlicht/*.sln
5) Update Irrlicht properties to add include and libraries folder from DirectX SDK

## To compile Irrlicht on Linux/MacOs
----------
1) Get Irrlicht from SVN repo here : svn://svn.code.sf.net/p/irrlicht/code/trunk
2) Put it in bin/libs/irrlicht
3) Go to bin/libs/irrlicht/source/Irrlicht/ 
4) Run "make sharedlib; make install"
5) Duplicate generated so files : cp /usr/local/lib64/libIrrlicht.so* /usr/local/lib/

## Add last version of Enet library
----------
1) Download tarball here : http://enet.bespin.org/Downloads.html
2) Untar into :  bin/libs/enet/

## To compile BC on Windows
----------
1) Visual Studio 2022 (or greater) are used
2) Add BC project -> Generate

## To compile BC on Linux
----------
1) Run "sudo apt-get install cmake mesa-common-dev libxxf86vm-dev freeglut3-dev libxext-dev libxcursor-dev portaudio19-dev libsndfile1-dev libopenxr-dev"
2) Go to bc/bin directory
3) Run cmake ../src 
4) Run make

## To compile BC on Linux
----------
1) Ensure you have XCode installed (required to compile programs on Mac). This can be obtained from the App Store. You will also need the Command line tools, which can be installed with 'sudo xcode-select --install'
2) Build the required sound packages:

## To compile BC on MacOs
----------
1) Ensure you have XCode installed (required to compile programs on Mac). This can be obtained from the App Store. You will also need the Command line tools, which can be installed with 'sudo xcode-select --install'
2) Build the required sound packages:

## Libsndfile (not mandatory to run BC):
-----------
Download and uncompress libsndfile-1.0.28.tar.gz from http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz
Create a folder for the output, referred to as <Somewhere> (Absolute path)
Change directory to where you've uncompressed libsndfile, then run the following in the terminal window:

(If building to be compatible with older macOS versions, run export MACOSX_DEPLOYMENT_TARGET=10.7 before building libsndfile and portaudio, and then open a new terminal before running the main Bridge Command build)

./configure --disable-shared --prefix=<Somewhere>
make
make install
cd <Somewhere>
cp -a lib <BridgeCommandSourceLocation>/libs/libsndfile/

## Portaudio (not mandatory to run BC):
----------
Download and uncompress pa_stable_v190600_20161030.tgz from http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
(For macOS Big Sur (11.0) and onwards, a more recent version is needed. 
I have successfully compiled with http://files.portaudio.com/archives/pa_snapshot.tgz created Sunday, 07-Mar-2021 01:23:04 UTC)
Change directory to where you've uncompressed Portaudio, then run the following in the terminal window:

./configure --disable-mac-universal --disable-shared --enable-static
make
cd lib/.libs/
cp * <BridgeCommandSourceLocation>/libs/portaudio/lib/

3) Download and install CMake using the OSX package from https://cmake.org/download/
4) Open a terminal, and change directory to the 'bc/bin' directory, and run './makeAndBuildApp'
If successful, this will build into the BridgeCommand.app, then run it
This assumes that the CMake binary exists at /Applications/CMake.app/Contents/bin/cmake


## Licence
-------

Bridge Command is Copyright (C) 2025 by James Packer. The model files
distributed in this release are copyright by their authors. In this
release, models have been provided by Ragnar, Juergen Klemp, Simon D 
Richardson, Jason Simpson, Thierry Videlaine, NETC (Naval Education 
and Training Command) and James Packer.

Bridge Command is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License Version 2
as published by the Free Software Foundation.

The models provided by Ragnar, Jason Simpson and James Packer are 
free software; you can redistribute them and/or modify them under the
terms of the GNU General Public License Version 2 as published by the
Free Software Foundation.

Other models distributed with Bridge Command may be used with Bridge 
Command. For any other use, you must obtain permission from the 
relevant author. 

Each model is credited to its author in the documentation distributed
with Bridge Command.

Bridge Command uses the Irrlicht Engine 
(http://irrlicht.sourceforge.net), the ENet networking library 
(http://enet.bespin.org), ASIO, PortAudio, water based on Keith Lantz
FFT water implementation and the RealisticWaterSceneNode by elvman
(https://github.com/elvman/RealisticWaterSceneNode), AIS Parser by 
Brian C. Lane, and the Serial library by William Woodall. Bridge 
Command depends on libsndfile, which is released under the GNU Lesser
General Public License version 2.1 or 3.

The Irrlicht Engine is based in part on the work of the Independent 
JPEG Group, the zlib, and libpng.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
