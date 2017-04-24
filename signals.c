// signals.c
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"

extern int foregroundPid;   //saves the pid of the current job in the foreground
extern PJOB *jobs[MAX_JOBS];

extern int jobs_counter;

void control_c(int n)
{
	if (foregroundPid != -1)
	{
		fprintf(stdout,"smash > signal SIGINT was sent to pid %d\n",foregroundPid);
		kill(foregroundPid,SIGINT);
		foregroundPid = -1;
	}
}

void control_z(int n)
{/*
	if (foregroundPid != -1)
	{
		fprintf(stdout,"smash > signal SIGTSTP was sent to pid %d\n",foregroundPid);
		kill(foregroundPid,SIGTSTP);

		// TODO add the foreground process to jobs array as stopped
		int i;
		for (i=0;i<jobs_counter;i++)
		{
			if (jobs[i]->pid == foregroundPid)
			{
				jobs[i]->stopped = TRUE;
				foregroundPid = -1;
				break;
			}
		}

	}*/
}

void sig_child(int n) //handles the process that stopped / continued / killed -> this function will handle the fg and bg commands
{
	int pid;
	while ( (pid = waitpid(-1, NULL, WNOHANG)) > 0)
		// pid > 0 - the pid that had changed
		// pid = 0 no child had changed
		// pid = -1 there are no children
	{
			removeJob(*jobs,pid); // remove father process (in the foreground)
	}
}

