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

