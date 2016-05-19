#! /usr/bin/python
# Neutron generator slow control

import os
import time

root_dir = r"/home/darkmatters/"
ts_dir = root_dir + r"SlowControl/Images/"
backup_name = root_dir + r"Backup/NG_SC_data.tar"
archive_name = ts_dir + r"NG_SC_data.tar"
minor_period = 10 # seconds between captures
major_period = 5 # minutes, same value as in the crontab

def unix_ts(): return int(time.time())

def Parse(timestamp):
	ret = [0,0,0] # temp, HV, current
	# crop into sub-pics and gocr those
	crop = os.system("convert " + ts_dir + str(timestamp) + r".pgm -crop 31x15+79+35 " + root_dir + r"/temp.pgm") # temperature
	temp = os.system("gocr -i " + root_dir + r"/temp.pgm")
	crop = os.system("convert " + ts_dir + str(timestamp) + r".pgm -crop 42x16+5+188 " + root_dir + r"/hv.pgm") # hv
	hv = os.system("gocr -i " + root_dir + r"/hv.pgm")
	crop = os.system("convert " + ts_dir + str(timestamp) + r".pgm -crop 42x16+6+358 " + root_dir + r"/current.pgm") # current
	current = os.system("gocr -i " + root_dir + r"/current.pgm")
	ret[0] = float(temp.replace('O','0'))
	ret[1] = float(hv.replace('O','0'))
	ret[2] = float(current.replace('O','0'))
	cleanup = os.system("rm -f " + root_dir + r"temp.pgm hv.pgm current.pgm")
	return ret

#	Old shell way of doing this:
#	WINID = `xwininfo -display :0.0 -all -root |egrep "\":" |grep Inbox |awk '{print $1}'`
#	xwd -out blah.xwd -root -display :0.0 -id $WINID
#	convert blah.xwd -crop XXX timestamp.pgm

def Capture(timestamp):
	output = os.system("xwd -out " + root_dir + r"blah.xwd -root -display :0.0 -name Remote\ Desktop\ Viewer") # TODO correct VNC client name?
	if output != 0: # 0 = success, 256 = failure
		return -1
	crop = os.system("convert " + root_dir + r"blah.xwd -crop 127x420+900+245 " + ts_dir + str(timestamp) + r".pgm")
	rm = os.system("rm -f " + root_dir + r"blah.xwd")
	return 0

def Backup(timestamp):
	os.system("tar --append --file " + archive_name + " " + ts_dir + str(timestamp) + ".pgm")
	os.system("rsync --archive --compress " + archive_name + " " + backup_name)
	return 0

def Backlog(start, end = 2147483647L): # Not finished?
	sc_file = open(root_dir + r"SlowControl/NG_SC.txt",'r+')
	this = 0L
	while this < start:
		line = sc_file.readline()
		if line == '': return # end of file
		this = long(line[:10])
	sc_file.seek(-len(line),1)
	while this <= end:
		line = sc_file.readline()
		if line == '': return # end of file
		this = long(line[:10])
		sc = Parse(this)
		sc_file.write(str(this) + " " + str(sc[0]) + " " + str(sc[1]) + " " + str(sc[2]) + '\n')
	sc_file.close()
	return

#----------------------------------#
#-------------# main #-------------#
#----------------------------------#

with open(root_dir + "SlowControl/NG_SC.txt",'a') as sc_file:
	for i in range(major_period*60/minor_period):
		now = unit_ts()
		err = Capture(now)
		if err != 0:
			sc_file.write(str(now) + " ERROR: CAPTURE FAILED\n")
			break
		sc = Parse(now)
		sc_file.write(str(now) + " " + str(sc[0]) + " " + str(sc[1]) + " " + str(sc[2]) + '\n')
		err = Backup(now)
		if sc[1] == 0.0 or sc[2] == 0.0: break # generator is off, no need to keep running
		time.sleep(minor_period)
	sc_file.close()
return