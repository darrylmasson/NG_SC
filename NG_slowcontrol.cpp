// Neutron generator slow control, c++
// USAGE: ./NG_slowcontrol <major period> <minor period>
// major period = minutes the program will operate, should match value in crontab
// minor period = seconds between captures when the generator is running

#include <iostream>
#include <fstream>
#include <stdlib>
#include <unistd.h>
#include <ctime>

#include "TFile.h"
#include "TTree.h"

using namespace std;

const string root_dir = "/home/darkmatters/SlowControl/";
const string storage_dir = root_dir + "images/";

ofstream f_log(root_dir + "logfile.txt", ios::out | ios::app);
TFile f((root_dir + "NG_SC.root").c_str(),"update");

long unix_ts() {
	return time(0);
}

void Parse(long timestamp, double values[]) {
	string tmp, crops[] = {"31x15+79+35 ","42x16+5+188 ","42x16+6+358 "}, outputs[] = {"/temp","/hv","/current"};
	for (auto i = 0; i < 3; i++) {
		system(("convert " + storage_dir + to_string(timestamp) + ".pgm -crop " + crops[i] + root_dir + outputs[i] + ".pgm").c_str());
		tmp = system(("gocr -i " + root_dir + outputs[i] + ".pgm").c_str());
		for (auto it = tmp.begin(); it < tmp.end(); it++) if ((*it == 'o') || (*it == 'O')) *it = '0';
		values[i] = stof(tmp);
	}
	system("rm " + root_dir + "*.pgm");
}

void Capture(long timestamp) {
//	WINID = `xwininfo -display :0.0 -all -root |egrep "\":" |grep Inbox |awk '{print $1}'`
//	xwd -out blah.xwd -root -display :0.0 -id $WINID
	system(("xwd -out " + root_dir + "blah.xwd -root -display :0.0 -name XXX").c_str()); // TODO add VNC client name
//	if (ret != 0) return -1;
//	convert blah.xwd -crop XXX timestamp.pgm
	system(("convert " + root_dir + "blah.xwd -crop 127x420+900+245 " + ts_dir + to_string(timestamp) + ".pgm").c_str());
	system(("rm " + root_dir + "blah.xwd").c_str());
}

void AtExit() {
	if (f.IsOpen()) f.Close();
	if (f_log.is_open()) f_log.close();
}

int main(int argc, char** argv) {
	atexit(AtExit);
	Long64_t TS = unix_ts();
	long now = TS;
	if (f.IsZombie()) {
		f_log << now << " could not open root file\n";
		return 0;
	}
	if (argc != 3) {
		f_log << now << " incorrect arguments: ";
		for (auto i = 0; i < argc; i++) f_log << arg[i] << " "; f_log << '\n';
		cout << "Usage: ./NG_slowcontrol <major period> <minor period>\n";
		cout << "See documentation\n";
		return 0;
	}
	int major = atoi(argv[1]);
	int minor = atoi(argv[2]);
	TTree* t = (TTree*)f.Get("TSC");
	double values[3]; // temperature, HV, current
	t->SetBranchAddress("Unix_ts",&TS,"ts\L");
	t->SetBranchAddress("NG_temp",&values[0],"temp\D");
	t->SetBranchAddress("NG_HV",&values[1], "hv\D");
	t->SetBranchAddress("NG_current",&values[2], "current\D");
	for (i = 0; i < major*60/minor; i++) {
		Capture(now);
		Parse(now, values);
		t->Fill();
		t->Write("",TObject::kOverwrite);
		if ((values[1] == 0) || (values[2] == 0)) return 0; // HV or current == 0 mean generator is off, no need to keep running
		sleep(minor);
	}
	return 0;
}