mkdir Debug
cd Debug
cmake -G "MinGW Makefiles"  \sack -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
make install
cd ..

mkdir Release
cd Release
cmake -G "MinGW Makefiles"  \sack -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
make install
cd ..

mkdir RelWithDebInfo
cd RelWithDebInfo
cmake -G "MinGW Makefiles"  \sack -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
make install
cd ..

mkdir MinSizeRel
cd MinSizeRel
cmake -G "MinGW Makefiles"  \sack -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
make install
cd ..


cmd /c "M:\build\mingw_sack_core\bin\Debug\sack_deploy.exe"
mkdir binary_Debug
cd binary_Debug
cmake -G "MinGW Makefiles"  \sack\binary -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_binaries_debug
make install

cd ..

cmd /c "M:\build\mingw_sack_core\bin\Release\sack_deploy.exe"
mkdir binary_Release
cd binary_Release
cmake -G "MinGW Makefiles"  \sack\binary -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_binaries_release
make install
cd ..

cmd /c "M:\build\mingw_sack_core\bin\Debug\sack_deploy.exe"
mkdir intershell_Debug
cd intershell_Debug
cmake -G "MinGW Makefiles"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_intershell_debug -DVLC_PATH="e:/tools/vlc-1.1.4"
make install
cd ..

cmd /c "M:\build\mingw_sack_core\bin\Release\sack_deploy.exe"
mkdir intershell_Release
cd intershell_Release
cmake -G "MinGW Makefiles"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_intershell_release -DVLC_PATH="e:/tools/vlc-1.1.4"
make install
cd ..

cmd /c "M:\build\mingw_sack_core\bin\Debug\sack_deploy.exe"
cmd /c "M:\build\mingw_sack_intershell_debug\intershell.deploy.exe"
mkdir video_link_Debug
cd video_link_Debug
cmake -G "MinGW Makefiles"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_video_link_debug
make install
cd ..

cmd /c "M:\build\mingw_sack_core\bin\Release\sack_deploy.exe"
cmd /c "M:\build\mingw_sack_intershell_Release\intershell.deploy.exe"
mkdir video_link_Release
cd video_link_Release
cmake -G "MinGW Makefiles"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/mingw_sack_video_link_release
make install
cd ..

:end
