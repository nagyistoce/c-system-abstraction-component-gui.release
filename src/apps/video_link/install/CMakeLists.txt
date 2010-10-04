cmake_minimum_required(VERSION 2.8)

set( bin
          ../../bin/video_map_editor.exe
../../bin/video_link_server.exe
../../bin/video_link_deploy.exe
../../bin/bdata.proxy.vplug
../../bin/vlc_launcher.vplug
../../bin/video_event_notices.vplug
../../bin/video_mux.vplug
../../bin/vlc_test.exe
../../bin/ClearShell.exe
../../bin/interface.conf
../../bin/bag.sqlite3.external
)

EXECUTE_PROCESS( COMMAND pcopy.portable.exe -l ${bin} 
      OUTPUT_VARIABLE full_bin
      )

set( frames
   ../../resources/frames/edit_video_link_site.Frame
)

set( maps 
../../resources/Stations.Map 
)

install( FILES ${full_bin} DESTINATION video_link_server/bin )
install( FILES ${frames} DESTINATION video_link_server/bin/frames )
install( FILES ${maps} DESTINATION video_link_server/resources )
install( FILES ../../bin/proxy.service.exe DESTINATION video_link_server/bin RENAME mysql.proxy.service.exe )

