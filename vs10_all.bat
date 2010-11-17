mkdir Debug
cd Debug
cmake -G "Visual Studio 10"  \sack -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_core
devenv sack.sln /Build debug /Project Install.vcxproj
devenv sack.sln /Build release /Project Install.vcxproj
devenv sack.sln /Build relwithdebinfo /Project Install.vcxproj
devenv sack.sln /Build MinSizeRel /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\bin\Debug\sack_deploy.exe"
mkdir binary_Debug
cd binary_Debug
cmake -G "Visual Studio 10"  \sack\binary -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_binaries_debug
devenv Sack.Binaries.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\bin\Release\sack_deploy.exe"
mkdir binary_Release
cd binary_Release
cmake -G "Visual Studio 10"  \sack\binary -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_binaries_release
devenv Sack.Binaries.sln /Build release /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\bin\Debug\sack_deploy.exe"
mkdir intershell_Debug
cd intershell_Debug
cmake -G "Visual Studio 10"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_intershell_debug -DVLC_PATH="c:/tools/vlc-1.1.4"
devenv InterShell.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\bin\Release\sack_deploy.exe"
mkdir intershell_Release
cd intershell_Release
cmake -G "Visual Studio 10"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_intershell_release -DVLC_PATH="c:/tools/vlc-1.1.4"
devenv InterShell.sln /Build release /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\bin\Debug\sack_deploy.exe"
cmd /c "M:\build\vs10_sack_intershell_Release\intershell_deploy.exe"
cmd /c "M:\build\vs10_sack_binaries_Release\Sack.Binary.Deploy.exe"
mkdir video_link_Debug
cd video_link_Debug
cmake -G "Visual Studio 10"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_video_link_debug
devenv video_link.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\bin\Release\sack_deploy.exe"
cmd /c "M:\build\vs10_sack_intershell_Release\intershell_deploy.exe"
cmd /c "M:\build\vs10_sack_binaries_Release\Sack.Binary.Deploy.exe"
mkdir video_link_Release
cd video_link_Release
cmake -G "Visual Studio 10"  \sack\src\apps\video_link -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_video_link_release
devenv video_link.sln /Build release /Project Install.vcxproj
cd ..

