#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

const char *portname = "/dev/ttyUSB0";
const speed_t rate = B9600;

int openSerialPort();
int * getCpuStats();
int * getMemStats();
int * getDiskStats();
char * getRaidStats(); //poor man's bool return type

void testOutput(int USB);
void clearScreen(int USB);
void newLine(int USB);
void newLineM(int USB, int iterations);
void backlightOn(int USB);
void backlightOff(int USB);
void beep(int USB);
void beeps(int USB, int numBeeps);
void beepsp(int USB, int numBeeps, int noteSpace);


int main(){
	int USB = openSerialPort();

	/*int * cpuStats = getCpuUsage();
	int * memStats = getMemStats();
	int * diskStats = getDiskStats();*/

	clearScreen(USB);
	backlightOn(USB);
	beep(USB);
	testOutput(USB);

	while (1){

	}
}

int openSerialPort(){
	int USB = open(portname, O_RDWR | O_NOCTTY);
	struct termios tty;
	memset (&tty, 0, sizeof tty);

	struct termios tty_old = tty;

	/* Set Baud Rate */
	cfsetospeed (&tty, rate);
	cfsetispeed (&tty, rate);

	/* Setting other Port Stuff */
	tty.c_cflag     &=  ~PARENB;            // Make 8n1
	tty.c_cflag     &=  ~CSTOPB;
	tty.c_cflag     &=  ~CSIZE;
	tty.c_cflag     |=  CS8;

	tty.c_cflag     &=  ~CRTSCTS;           // no flow control
	tty.c_cc[VMIN]   =  1;                  // read doesn't block
	tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);

	/* Flush Port, then applies attributes */
	tcflush( USB, TCIFLUSH );

	return USB;
}

void testOutput(int USB){
	unsigned char testString[] = "If you see this, then serial is working!\v";
	int numWritten = 0,
	    spot = 0;

	do {
		numWritten = write(USB, &testString[spot], 1);
		spot += numWritten;
	} while (testString[spot-1] != '\v' && numWritten > 0);
}

void clearScreen(int USB){ unsigned char seq[1] = {12}; write(USB, seq, 1); }

void newLine (int USB){ unsigned char seq[1] = {13}; write(USB, seq, 1); }

void newLineM (int USB, int iterations){
	unsigned char seq[1] = {13};

	do {
		write(USB, seq, 1);
		iterations--;
	} while(iterations >= 1);
}

void backlightOn (int USB){ unsigned char seq[1] = {17}; write(USB, seq, 1); }
void backlightOff (int USB){ unsigned char seq[1] = {18}; write(USB, seq, 1); }

void beep(int USB){ unsigned char seq[2] = {212, 220}; write(USB, seq, 1); }

void beeps(int USB, int numBeeps){
	unsigned char seq[2] = {212, 220};

	do {
		write(USB, seq, 1);
		usleep(750 * 1000);
		numBeeps--;
	} while(numBeeps >= 1);
}

void beepsp(int USB, int numBeeps, int noteSpace){
	unsigned char seq[2] = {212, 220};

	do {
		write(USB, seq, 1);
		usleep(noteSpace*1000);
		numBeeps--;
	} while(numBeeps >= 1);
}

int * getCpuStats(){
	FILE *fp;
	unsigned long long in fields[10], total_tick[MAX_CPU],
		total_tick_old[MAX_CPU], idle[MAX_CPU], idle_old[MAX_CPU],
		del_total_tick[MAX_CPU], del_idle[MAX_CPU];
	int update_cycle = 0, i, cpus = 0, count;
	double percent_usage;

	fp = fopen ("/proc/stat", "r");
	if (fp == NULL){
		perror ("Error");
	}

	while (readProcStat(fp, fields) != -1){
		for (i=0, total_tick[cpus] = 0; i<10; i++){
			total_tick[cpus] += fields[i];
		}
		idle[cpus] += fields[3]; // idle ticks index.
		cpus++;
	}
	
}

int readProcStat(FILE *fp, unsigned long long int *fields){
	int retval;
	char buffer[BUF_MAX];

	if(!fgets (buffer, BUF_MAX, fp)){
		perror ("Error");
	}
	// Line starts with c and a string.  This is to handle cpu, cpu[0-9]+
	retval = sscanf (buffer, "c%*s %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
		&fields[0],
		&fields[1],
		&fields[2],
		&fields[3],
		&fields[4],
		&fields[5],
		&fields[6],
		&fields[7],
		&fields[8],
		&fields[9]);

	if (retval == 0){
		return -1;
	}
	if (retval < 4){
		fprintf (stderr, "Error reading /proc/stat cpu field\n");
		return 0;
	}
	return 1;
}
