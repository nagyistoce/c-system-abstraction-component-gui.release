#!/bin/bash
export PATH=/usr/kerberos/sbin:/usr/kerberos/bin:/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/usr/X11R6/bin:/root/bin:/usr/bin/X11
alsactl restore
nvida-settings -l -
xset -dpms
setterm -blank 0
xset s off


#cd ~/bingolink/fbi_replicator_backup

cd /usr/src/FortuNet
./ipc.fix.sems
./launchpads >/var/log/launchpad.log &
./msgsvr > /var/log/msgsvr.log &
sleep 1
./bardservice > /var/log/bardservice.log &
sleep 1
./alpha2server >/var/log/alpha2server.log 2>&1 &

# support for login dialog
#/usr/bin/xterm -geometry 80x20+1024+51 -e ./fbi-rep-server -p&
# some sort of login dialog...
#/usr/bin/xterm -geometry 80x20+1024+400 -e ./fbi-rep-gui BoulderStation& 
#./blctrl setup
#./blctrl host
#DISPLAY=:0.1 ./ianson broadcast 0.0.0.0:1234 &
           
#/root/work/testbed/bin/debug-wc/milk
exec /usr/bin/X11/xclock
