cmake_minimum_required(VERSION 2.8)

set( bin
          ../../bin/video_map_editor.exe
../../bin/video_link_server.exe
../../bin/video_link_deploy.exe
../../bin/bdata.proxy.vplug
../../bin/vlc_launcher.vplug
../../bin/${CMAKE_SHARED_LIBRARY_PREFIX}sack_bag.dll
../../bin/video_event_notices.vplug
../../bin/video_mux.vplug
../../bin/vlc_test.exe
../../bin/${CMAKE_SHARED_LIBRARY_PREFIX}vlc_interface.dll
../../bin/ClearShell.exe
)

set( frames
   ../../resources/frames/edit_video_link_site.Frame
)

install( FILES ${bin} DESTINATION video_link_server/bin )
install( FILES ${bin} DESTINATION video_link_server/resources/frames )


