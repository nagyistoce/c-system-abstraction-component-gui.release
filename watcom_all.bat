mkdir Debug
cd Debug
cmake -G "Watcom WMake"  \sack -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
wmake install
cd ..
mkdir Release
cd Release
cmake -G "Watcom WMake"  \sack -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
wmake install
cd ..
mkdir RelWithDebInfo
cd RelWithDebInfo
cmake -G "Watcom WMake"  \sack -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
wmake install
cd ..
mkdir MinSizeRel
cd MinSizeRel
cmake -G "Watcom WMake"  \sack -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_core -DNEED_FREETYPE=1 -DNEED_JPEG=1 -DNEED_PNG=1 -DNEED_ZLIB=1
wmake install
cd ..

goto END

cmd /c "M:\build\vs10_sack_core\Debug\sack_deploy.exe"
mkdir binary_Debug
cd binary_Debug
cmake -G "MinGW Makefiles"  \sack\binary -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_core_binaries_debug
devenv SackBinaries.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Release\sack_deploy.exe"
mkdir binary_Release
cd binary_Release
cmake -G "MinGW Makefiles"  \sack\binary -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_binaries_release
devenv SackBinaries.sln /Build release /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Debug\sack_deploy.exe"
mkdir intershell_Debug
cd intershell_Debug
cmake -G "MinGW Makefiles"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_intershell_debug -DVLC_PATH="e:/tools/vlc-1.1.4"
devenv InterShell.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Release\sack_deploy.exe"
mkdir intershell_Release
cd intershell_Release
cmake -G "MinGW Makefiles"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_intershell_release -DVLC_PATH="e:/tools/vlc-1.1.4"
devenv InterShell.sln /Build release /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Debug\sack_deploy.exe"
mkdir video_link_Debug
cd video_link_Debug
cmake -G "MinGW Makefiles"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_video_link_debug
devenv video_link.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Release\sack_deploy.exe"
mkdir video_link_Release
cd video_link_Release
cmake -G "MinGW Makefiles"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_video_link_release
devenv video_link.sln /Build release /Project Install.vcxproj
cd ..

:end
