#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// argv[1] = window ID
// argv[2] = number of runs per minute
// argv[3] = timestamp for backlog (optional)

char HOME[] = {"/home/darkmatters"};

int Parse(int* temp1, int* temp2, char voltage[], char current[]) {
	FILE* fin = fopen("/home/darkmatters/SlowControl/file.txt","r");
	char buffer[64];
	int i = 0, j = 0;
	fgets(buffer, sizeof(buffer), fin);
	if (buffer[0] != '_') {
		fclose(fin);
		return -1;
	}
	for (i = 0; i < 3; i++) fgets(buffer, sizeof(buffer), fin);
	if (buffer[0] == 'A') sscanf(buffer, "Actual Temp %i %i _C", temp1, temp2);
	for (i = 0; i < 12; i++) fgets(buffer, sizeof(buffer), fin);
	if (buffer[0] == 'O') sprintf(voltage,"0%c",'\0');
	else {
		i = j = 0;
		while (i < 9) {
			if ((buffer[i] >= '0') && (buffer[i] <= '9')) voltage[j++] = buffer[i];
			i++;
		}
		voltage[j] = '\0';
	}
	for (i = 0; i < 13; i++) fgets(buffer, sizeof(buffer), fin);
	if (buffer[0] == 'O') sprintf(current, "0%c", '\0');
	else {
		i = j = 0;
		while (i < 9) {
			if ((buffer[i] >= '0') && (buffer[i] <= '9')) current[j++] = buffer[i];
			i++;
		}
		current[j] = '\0';
	}
	return 0;
}

void Capture(char win_id[], long now) {
	int temp1 = 0, temp2 = 0;
	char sh_command[192], voltage[10], current[10];
	sprintf(sh_command,"/usr/bin/xwd -out %s/out.xwd -root -display :0.0 -id %s %c", HOME, win_id, '\0');
	system(sh_command);
	sprintf(sh_command,"/usr/bin/convert %s/out.xwd -crop 127x420+900+245 %s/SlowControl/Images/%li.pgm %c",HOME, HOME, now,'\0');
	system(sh_command);
	system("rm /home/darkmatters/out.xwd");
}

void SlowControl(FILE* fout, long now) {
	int temp1 = 0, temp2 = 0;
	char sh_command[192], voltage[10], current[10];
	sprintf(sh_command, "/usr/local/bin/gocr -i %s/SlowControl/Images/%li.pgm -o %s/SlowControl/file.txt %c", HOME, now, HOME, '\0');
	system(sh_command);
	if (Parse(&temp1, &temp2, voltage, current)) {
		fputs("ERROR\n",fout);
		return;
	}
	fprintf(fout, "%li %i.%i %s %s\n", now, temp1, temp2, voltage, current);
}

int main(int argc, char** argv) {
//	puts("Parsing\n");
	FILE* fout = fopen("/home/darkmatters/SlowControl/SlowControlData.txt","a");
	int num_per_minute = atoi(argv[2]), i = 0;
	char win_id[12];
	strcpy(win_id, argv[1]);
	long now = (argc == 4 ? atol(argv[3]) : time(0));
	int delta_t = 60/num_per_minute;
	for (i = 0; i < num_per_minute; i++) {
		if (argc != 4) Capture(win_id, now);
		SlowControl(fout, now);
		if (i != (num_per_minute-1)) sleep(delta_t);
	}
	fclose(fout);
	return 0;
}
