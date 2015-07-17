#!/bin/bash
echo "Starting"
PER_MINUTE=3
DATE=$(date +%Y%m%d%H%M)
echo $DATE
WINID=`xwininfo -display :0.0 -all -root |egrep "\":" |grep lfbVNCserver |awk '{print $1}'`
/home/darkmatters/SlowControlOps $WINID $PER_MINUTE

echo "Done"
