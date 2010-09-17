#!/bin/bash
cd
cd work
. ./work
cd sack
mtn pull;mtn update
#make clean
cd ../altanik
mtn pull;mtn update
#make clean;
make
cd ../binglink
mtn pull;mtn update
#make clean;
make
cd ../testbed/bin/debug-lnx64/install
~/work/binglink/scripts/mkinst
mv /usr/src/FortuNet/alpha2/server.ini alpha2 -f
killall -9 watch_task
killall -9 alpha2server
killall -9 msgsvr
killall -9 bardservice
killall -9 video_event_relay
killall -9 launchpads
sleep 2
cp -R * /usr/src/FortuNet
killall X