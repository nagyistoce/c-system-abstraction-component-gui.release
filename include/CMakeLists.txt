
# something like, 
# if !(MAKE_DEV)

SET( SACK_SEXPAT_HEADERS
../src/sexpat/expat.h
../src/sexpat/expat_external.h
)

SET( SACK_GENX_HEADERS
../src/genx/genx.h
)

SET( SYS_IPC_HEADERS 
sys.sack/ipc.h sys.sack/msg.h
)

SET( SACK_HEADERS
cache.h
colordef.h
configscript.h
construct.h
controls.h
deadstart.h
exit_priorities.h
filedotnet.h
filemon.h
filesys.h
fractions.h
idle.h
image.h
interface.h
keybrd.h
loadsock.h
logging.h
menuflags.h
msgclient.h
msgprotocol.h
msgserver.h
netservice.h
network.h
procreg.h
psi.h
pssql.h
rcomlib.h
render.h
sack_types.h
sha1.h
sharemem.h
skelhdr.h
spacetree.h
sql_support.h
sqlgetoption.h
sqloptint.h
stdhdrs.h
final_types.h
sackcomm.h
strlib.hpp
symbits.h
system.h
systray.h
timers.h
typelib.h
vectlib.h
vectypes.h
vidlib.h
)

# sqlite3.h

SET( VIDLIB_HEADERS 
vidlib/keydef.h
vidlib/vidstruc.h
)

SET( IMGLIB_HEADERS 
imglib/fontstruct.h
imglib/imagestruct.h
)

SET( PSI_HEADERS
psi/buttons.h
psi/clock.h
psi/console.h
psi/controls.h
psi/edit.h
psi/mkns.c
psi/namespaces.h
psi/shadewell.h
psi/slider.h
)

if( CMAKE_PROJECT_NAME )
INSTALL( 
  FILES
    ${SACK_HEADERS}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}
)

INSTALL( 
  FILES
    ${VIDLIB_HEADERS}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}/vidlib
)
INSTALL( 
  FILES
    ${IMGLIB_HEADERS}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}/imglib
)
INSTALL( 
  FILES
    ${PSI_HEADERS}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}/psi
)

INSTALL( 
  FILES
    ${SACK_GENX_HEADERS}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}/genx
)

INSTALL( 
  FILES
    ${SACK_SEXPAT_HEADERS}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}/sexpat
)

INSTALL( 
  FILES
    ${SYS_IPC_HEADERS}
  DESTINATION
    include/${CMAKE_PROJECT_NAME}/sys.sack
)

endif()
