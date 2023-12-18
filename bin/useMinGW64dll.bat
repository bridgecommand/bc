cd /D "%~dp0"
del Irrlicht.dll
copy Irrlicht_mingw64.dll Irrlicht.dll
del libenet.dll
copy libenet64.dll libenet.dll