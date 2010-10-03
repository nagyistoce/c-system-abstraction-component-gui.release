if %1x==videox goto videolink
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



cmd /c "M:\build\watcom_sack_core\bin\Debug\sack_deploy.exe"
mkdir binary_Debug
cd binary_Debug
cmake -G "Watcom WMake"  \sack\binary -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_binaries_debug
wmake install

cd ..

cmd /c "M:\build\watcom_sack_core\bin\Release\sack_deploy.exe"
mkdir binary_Release
cd binary_Release
cmake -G "Watcom WMake"  \sack\binary -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_binaries_release
wmake install
cd ..

cmd /c "M:\build\watcom_sack_core\bin\Debug\sack_deploy.exe"
mkdir intershell_Debug
cd intershell_Debug
cmake -G "Watcom WMake"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_intershell_debug -DVLC_PATH="e:/tools/vlc-1.1.4"
wmake install
cd ..

cmd /c "M:\build\watcom_sack_core\bin\Release\sack_deploy.exe"
mkdir intershell_Release
cd intershell_Release
cmake -G "Watcom WMake"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_intershell_release -DVLC_PATH="e:/tools/vlc-1.1.4"
wmake install
cd ..

:videolink

cmd /c "M:\build\watcom_sack_core\bin\Debug\sack_deploy.exe"
cmd /c "M:\build\watcom_sack_intershell_debug\intershell_deploy.exe"
cmd /c "M:\build\watcom_sack_binaries_debug\Sack.Binary.Deploy.exe"
mkdir video_link_Debug
cd video_link_Debug
cmake -G "Watcom WMake"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_video_link_debug
wmake install
cd ..

cmd /c "M:\build\watcom_sack_core\bin\Release\sack_deploy.exe"
cmd /c "M:\build\watcom_sack_intershell_Release\intershell_deploy.exe"
cmd /c "M:\build\watcom_sack_binaries_Release\Sack.Binary.Deploy.exe"
mkdir video_link_Release
cd video_link_Release
cmake -G "Watcom WMake"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/watcom_sack_video_link_release
wmake install
cd ..

:end
