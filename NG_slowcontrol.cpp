// Neutron generator slow control, c++
// USAGE: ./NG_slowcontrol <major period> <minor period>
// major period = minutes the program will operate, should match value in crontab
// minor period = seconds between captures when the generator is running

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include "pstream.h"

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

int Parse(long timestamp, double values[]) {
	string tmp, crops[] = {"31x15+79+35 ","42x16+5+188 ","42x16+6+358 "}, outputs[] = {"/temp","/hv","/current"};
	redi::ipstream redirect;
	int ret(0);
	for (auto i = 0; i < 3; i++) {
		ret = system(("convert " + storage_dir + to_string(timestamp) + ".pgm -crop " + crops[i] + root_dir + outputs[i] + ".pgm").c_str());
		if (ret != 0) return 1;
		redirect.open("gocr -i " + root_dir + outputs[i] + ".pgm"); // not sure how to capture return value
		redirect >> tmp;
		redirect.close();
		for (auto it = tmp.begin(); it < tmp.end(); it++) if ((*it == 'o') || (*it == 'O')) *it = '0';
		values[i] = stof(tmp);
	}
	system(("rm " + root_dir + "*.pgm").c_str());
	return 0;
}

int Capture(long timestamp) {
	int ret = 0;
//	WINID = `xwininfo -display :0.0 -all -root |egrep "\":" |grep Remote\ Desktop\ Viewer |awk '{print $1}'` \\ old method
//	xwd -out blah.xwd -root -display :0.0 -id $WINID
	ret = system(("xwd -out " + root_dir + "blah.xwd -root -display :0.0 -name Remote\\ Desktop\\ Viewer").c_str()); // 0 == success, 256 == failure
	if (ret != 0) return 1;
//	convert blah.xwd -crop XXX timestamp.pgm
	ret = system(("convert " + root_dir + "blah.xwd -crop 127x420+900+245 " + storage_dir + to_string(timestamp) + ".pgm").c_str()); // same
	if (ret != 0) return 2;
	system(("rm " + root_dir + "blah.xwd").c_str());
	return 0;
}

void AtExit() {
	if (f.IsOpen()) f.Close();
	if (f_log.is_open()) f_log.close();
}

int main(int argc, char** argv) {
	atexit(AtExit);
	Long64_t TS = unix_ts();
	long now = TS;
	int ret(0);
	if (f.IsZombie()) {
		f_log << now << " could not open root file\n";
		return 0;
	}
	if (argc != 3) {
		f_log << now << " incorrect arguments: ";
		for (auto i = 0; i < argc; i++) f_log << argv[i] << " "; f_log << '\n';
		cout << "Usage: ./NG_slowcontrol <major period> <minor period>\n";
		cout << "See documentation\n";
		return 0;
	}
	int major = atoi(argv[1]);
	int minor = atoi(argv[2]);
	TTree* t = (TTree*)f.Get("TSC");
	double values[3]; // temperature, HV, current
	t->SetBranchAddress("Unix_ts",&TS);
	t->SetBranchAddress("NG_temp",&values[0]);
	t->SetBranchAddress("NG_HV",&values[1]);
	t->SetBranchAddress("NG_current",&values[2]);
	for (auto i = 0; i < major*60/minor; i++) {
		ret = Capture(now);
		if (ret) {
			f_log << now << (ret == 1 ? " capture " : " convert ") << "failed\n";
			break;
		}
		ret = Parse(now, values);
		if (ret) {
			f_log << now << " parsing failed\n";
			break;
		}
		t->Fill();
		if ((values[1] == 0) || (values[2] == 0)) break; // HV or current == 0 mean generator is off, no need to keep running
		sleep(minor);
	}
	t->Write("",TObject::kOverwrite);
	return 0;
}
