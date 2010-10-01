mkdir Debug
cd Debug
cmake -G "Visual Studio 10"  \sack -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_core
devenv sack.sln /Build debug /Project Install.vcxproj
cd ..

mkdir Release
cd Release
cmake -G "Visual Studio 10"  \sack -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_core
devenv sack.sln /Build release /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Debug\sack_deploy.exe"
mkdir binary_Debug
cd binary_Debug
cmake -G "Visual Studio 10"  \sack\binary -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_core_binaries_debug
devenv SackBinaries.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Release\sack_deploy.exe"
mkdir binary_Release
cd binary_Release
cmake -G "Visual Studio 10"  \sack\binary -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_binaries_release
devenv SackBinaries.sln /Build release /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Debug\sack_deploy.exe"
mkdir intershell_Debug
cd intershell_Debug
cmake -G "Visual Studio 10"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_intershell_debug -DVLC_PATH="e:/tools/vlc-1.1.4"
devenv InterShell.sln /Build debug /Project Install.vcxproj
cd ..

cmd /c "M:\build\vs10_sack_core\Release\sack_deploy.exe"
mkdir intershell_Release
cd intershell_Release
cmake -G "Visual Studio 10"  \sack\src\InterShell -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=M:/build/vs10_sack_intershell_release -DVLC_PATH="e:/tools/vlc-1.1.4"
devenv InterShell.sln /Build release /Project Install.vcxproj
cd ..

