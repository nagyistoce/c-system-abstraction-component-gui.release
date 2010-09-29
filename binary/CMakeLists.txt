cmake_minimum_required(VERSION 2.8)


PROJECT( SackBinaries )

GET_FILENAME_COMPONENT(SACK_SDK_ROOT_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Freedom Collective\\SACK;Install_Dir]" ABSOLUTE CACHE)

include( ${SACK_SDK_ROOT_PATH}/CMakePackage )
include_directories( ${SACK_INCLUDE_DIR} )
LINK_DIRECTORIES( ${SACK_LIBRARY_DIR} )

macro( add_subpath base )
string( REPLACE "." "_" base2 ${base} )
add_subdirectory( ${base} ${base2} )
endmacro(add_subpath)

add_subpath( ../src/psilib/paltst )

# includes some testdata in tstdisplay...
# the test programs do not have install targets, but they will end up building...
 if( NOT __NO_GUI__ )
  add_subpath( ../src/msgsvr/testdisplay )
 endif( NOT __NO_GUI__ )
#add_subpath( ../src/bench/mem )


 if( NOT __NO_GUI__ )
if( USE_OPTIONS )
 add_subpath( ../src/SQLlib/optlib/editoption )
endif( USE_OPTIONS )
 endif( NOT __NO_GUI__ )

if( NOT __NO_GUI__ )
add_subpath( ../src/msgsvr/server )
add_subpath( ../src/msgsvr/servicelist )
endif( NOT __NO_GUI__ )


add_subpath( ../src/netlib/user )
add_subpath( ../src/netlib/echo )
add_subpath( ../src/netlib/forward )
add_subpath( ../src/netlib/proxy )
add_subpath( ../src/SQLlib/testsql )
add_subpath( ../src/utils/launcher )
add_subpath( ../src/utils/strip )
add_subpath( ../src/psilib/console/cmdshell )
if( __WINDOWS__ )
add_subpath( ../src/utils/pcopy )
endif( __WINDOWS__ )



INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SACK Binaries")
SET(CPACK_PACKAGE_VENDOR "Freedom Collective")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../howto.compile")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../read.this")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "476")
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../doc\\\\BlankButton.jpg")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\sack_bag.dll")
  SET(CPACK_NSIS_DISPLAY_NAME "SACK ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
  SET(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "bin/MyExecutable")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")
INCLUDE(CPack)

