/*	smash.c
main file. This file contains the main function of smash
*******************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "commands.h"
#include "signals.h"
#include <time.h>
#define MAX_LINE_SIZE 80
#define MAXARGS 20
#define MAX_JOBS 100


char* L_Fg_Cmd;
char lineSize[MAX_LINE_SIZE];

int foregroundPid;   //saves the pid of the current job in the foreground
PJOB *jobs[MAX_JOBS];

//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
int main(int argc, char *argv[])
{
    char cmdString[MAX_LINE_SIZE]; 	   
    //This represents the list of jobs.

    struct sigaction act;

    /* set the INT (Ctrl-C) signal handler to 'control_c' */
    act.sa_handler = &control_c;
    sigaction(SIGINT, &act, NULL);

    /* set the INT (Ctrl-Z) signal handler to 'control_z' */
    act.sa_handler = &control_z;
    sigaction(SIGTSTP, &act, NULL);

    /* set the INT (SIGCHLD) signal handler to 'sig_child' */
    act.sa_handler = &sig_child;
    sigaction(SIGCHLD, &act, NULL);

	
	L_Fg_Cmd =(char*)malloc(sizeof(char)*(MAX_LINE_SIZE+1));
	if (L_Fg_Cmd == NULL) 
			exit (-1); 
	L_Fg_Cmd[0] = '\0';
	
    	while (1)
    	{
	 	printf("smash > ");
		fgets(lineSize, MAX_LINE_SIZE, stdin);
		strcpy(cmdString, lineSize);    	
		cmdString[strlen(lineSize)-1]='\0';
					// perform a complicated Command
		if(!ExeComp(lineSize)) continue; 
					// background command
	 	if(!BgCmd(lineSize, *jobs)) continue;
					// built in commands
		ExeCmd(*jobs, lineSize, cmdString);
		
		/* initialize for next line read*/
		lineSize[0]='\0';
		cmdString[0]='\0';
    	}
    return 0;
}

