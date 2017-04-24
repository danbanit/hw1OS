#ifndef _COMMANDS_H
#define _COMMANDS_H
#include <unistd.h> 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_SIZE 80
#define MAX_ARG 20
#define MAX_JOBS 100

typedef enum { FALSE , TRUE } bool;

typedef struct _JOB
{
	char* name;
	int pid;
	time_t time;
	bool stopped;
}*PJOB, JOB;


//auxiliary functions
void suspendJob(PJOB *jobs, int pid);
void insertJob(PJOB *jobs, char* name, int pid, time_t time,bool stopped, int serial);
void removeJob(PJOB *jobs, int pid);
void printJob(PJOB *jobs, int jobNumber);
int getLatestJob(PJOB *jobs, bool fg_or_bg); // false for fg and true for bg


int ExeComp(char* lineSize);
int BgCmd(char* lineSize, PJOB *jobs);
int ExeCmd(PJOB *jobs, char* lineSize, char* cmdString);
void ExeExternal(char *args[MAX_ARG], char* cmdString);

#endif

