mkdir Debug
cd Debug
cmd /c "M:\build\vs10_sack_core\bin\Debug\sack_deploy.exe"
cmd /c "M:\build\vs10_sack_binaries_debug\sack.binary.deploy.exe"
cmd /c "M:\build\vs10_sack_intershell_debug\intershell_deploy.exe"
cmake -G "Visual Studio 10"  \sack_rel\sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_video_link_debug
devenv video_link.sln /Build debug /Project Install.vcxproj
devenv video_link.sln /Build debug /Project Package.vcxproj
cd ..

mkdir Release
cd Release
cmd /c "M:\build\vs10_sack_core\bin\Release\sack_deploy.exe"
cmd /c "M:\build\vs10_sack_binaries_release\sack.binary.deploy.exe"
cmd /c "M:\build\vs10_sack_intershell_release\intershell_deploy.exe"
cmake -G "Visual Studio 10"  \sack_rel\sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_video_link_release
devenv video_link.sln /Build release /Project Install.vcxproj
devenv video_link.sln /Build release /Project Package.vcxproj
cd ..
