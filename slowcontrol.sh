#!/bin/bash
echo "Starting"
MAJ_PER = 1
MIN_PER = 10
DATE=$(date +%Y%m%d%H%M)
echo $DATE
#WINID=`xwininfo -display :0.0 -all -root |egrep "\":" |grep lfbVNCserver |awk '{print $1}'`
/home/darkmatters/NG_slowcontrol $MAJ_PER $MIN_PER
/home/darkmatters/SlowControl/Images/syncdaemon.sh
echo "Done"
