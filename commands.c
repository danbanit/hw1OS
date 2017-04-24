//		commands.c
//********************************************
#include "commands.h"

extern int foregroundPid;   //saves the pid of the current job in the foreground

static int jobs_counter = 0; //saves the number of jobs in the jobs list
static int hist_counter = 0;  //saves the number of the processes that had executes
char prev_path[MAX_LINE_SIZE];
char history[50][MAX_LINE_SIZE]; //saves the processes that had executed

void insertJob(PJOB *jobs, char* name, int pid, time_t time,bool stopped, int serial)
{
	PJOB new_job = (PJOB)malloc(sizeof(PJOB));
	new_job->name = (char*)malloc(sizeof(char)*MAX_LINE_SIZE);
	strcpy(new_job->name,name);
	new_job->pid = pid;
	new_job->time = time;
	new_job->stopped = stopped;
	//jobs = new_job;
	jobs[serial] = new_job;
	jobs_counter++;
}

void removeJob(PJOB *jobs,int pid)
{
	int i,jobNumber=-1;
	for (i=0;i<jobs_counter;i++)
	{
		if (jobs[i]->pid == pid)
		{
			jobNumber = i;
			break;
		}
	}
	if (jobNumber>-1)
	{
		PJOB tmp = jobs[jobNumber];

		if (jobs_counter-1 == jobNumber)
		{
			free(tmp->name);
			free(tmp);
			jobs_counter--;
		}
		else
		{
			int k;
			for(k=jobNumber ; k<jobs_counter-1 ; k++)
			{
				 jobs[k]=jobs[k+1];
			}
			free(tmp->name);
			free(tmp);
			jobs_counter--;
		}
	}
}


void printJob(PJOB *jobs, int jobNumber)
{
	time_t actual_time;
	time(&actual_time);
	if (jobs[jobNumber]->stopped)
		fprintf(stdout,"[%d] %s : %d %ld secs (Stopped)\n", jobNumber+1, jobs[jobNumber]->name, jobs[jobNumber]->pid, actual_time-jobs[jobNumber]->time);
	else
		fprintf(stdout,"[%d] %s : %d %ld secs\n", jobNumber+1, jobs[jobNumber]->name, jobs[jobNumber]->pid, actual_time-jobs[jobNumber]->time);
}

int getLatestJob(PJOB *jobs, bool fg_or_bg)
{
	int i;
	int latest_job;
	time_t max;
	latest_job=-1;
	max = 0;

	if (fg_or_bg == FALSE) //fg called the function to get the latest process that has played
	{
		for(i = 0; i < jobs_counter; i++ )
		{
			if (jobs[i]->time > max )
			{
				max = jobs[i]->time;
				latest_job = i;
			}
		}
	}
	else //bg called the function
	{
		for(i = 0; i < jobs_counter; i++ )
		{
			if (jobs[i]->time > max && jobs[i]->stopped == TRUE)
			{
				max = jobs[i]->time;
				latest_job = i;
			}
		}

	}



	return latest_job;
}


//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//********************************************
int ExeCmd(PJOB *jobs, char* lineSize, char* cmdString)
{
	char* cmd; 
	char* args[MAX_ARG];
	char pwd[MAX_LINE_SIZE];
	char* delimiters = " \t\n";  
	int i = 0, num_arg = 0;
	bool illegal_cmd = FALSE; // illegal command
    	cmd = strtok(lineSize, delimiters);
	if (cmd == NULL)
		return 0; 
   	args[0] = cmd;
	for (i=1; i<MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters); 
		if (args[i] != NULL) 
			num_arg++; 
 
	}

	if (strcmp(cmd, "history"))
	{
		strcpy(history[hist_counter%50],cmdString);
		hist_counter++;
	}

//*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
//*************************************************/
	getcwd(pwd, sizeof(pwd));

	if (!strcmp(cmd, "cd") ) 
	{
        if (num_arg != 1){
            illegal_cmd = TRUE;
        }
        else
        {
            if (strcmp(args[1],"-")==0) // "cd -"
            {
            	if (strcmp(prev_path,"\0"))
            	{
            		chdir(prev_path);
            		fprintf(stdout,"%s\n", prev_path);
            		strcpy(prev_path,pwd);
            	}
            	else
            	{
            		fprintf(stderr,"cd: OLDPWD not set\n");
            		return 1;
            	}
            }
            else // "cd <path>"
            {
            	if (chdir(args[1]) != 0)
            		fprintf(stderr, "smash error:> %s - path not found\n",args[1]);
            	else
            	{
            		strcpy(prev_path,pwd);
            	}
            }
        }
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "pwd")) 
	{
		if(num_arg > 0)
			illegal_cmd = TRUE;
		else
		{
			fprintf(stdout,"%s\n", pwd);
		}
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "mkdir"))
	{

		return 0;
	}
	/*************************************************/
	else if (!strcmp(cmd, "history"))
	{
		int i;
 		if (hist_counter < 50 )
 		{
 			for(i=0; i<hist_counter; i++)
 				fprintf(stdout,"%s\n", history[i]);
 		}
 		else
 		{
 			for( i=(hist_counter+1)%50 ; i < 50 ; i++ )
 				fprintf(stdout,"%s\n", history[i]);
 			for( i=0 ; i < (hist_counter+1)%50 ; i++ )
 		 		fprintf(stdout,"%s\n", history[i]);
 		}
 		return 0;
	}

	else if (!strcmp(cmd, "jobs"))
	{
 		if (num_arg > 0)
 			illegal_cmd = TRUE;
 		else
 		{
 			int i;
 			for (i=0; i<jobs_counter; i++)
 				printJob(jobs,i);
 		}
 		return 0;
	}

	else if (!strcmp(cmd, "kill"))
	{
 		if (num_arg != 2)
 			illegal_cmd = TRUE;
 		else
 		{
 			int jobNumber = atoi(args[2]);
 			if (jobNumber > jobs_counter)
 			{
 				fprintf(stderr, "smash error:> kill %d - job does not exist\n",jobNumber);
 				return 1;
 			}
 			else
 			{
 				int pid = jobs[jobNumber]->pid;
				int signum = atoi(&args[1][1]);

 				if(kill(pid,signum)==0)
 				{
					switch (signum)
					{
					case SIGCONT:
						jobs[jobNumber]->stopped = FALSE;
						fprintf(stdout,"smash > signal SIGCONT was sent to pid %d\n",jobs[jobNumber]->pid);
						break;
					case SIGSTOP:
						jobs[jobNumber]->stopped = TRUE;
						fprintf(stdout,"smash > signal SIGSTOP was sent to pid %d\n",jobs[jobNumber]->pid);
						break;
					case SIGTSTP:
						jobs[jobNumber]->stopped = TRUE;
						fprintf(stdout,"smash > signal SIGTSTP was sent to pid %d\n",jobs[jobNumber]->pid);
						break;

					default:
						fprintf(stderr, "smash error:> kill %d - cannot send signal\n",jobNumber);
						return 1;
					}
					return 0;

 				}
				else
				{
					fprintf(stderr, "smash error:> kill %d - cannot send signal\n",jobNumber);
					return 1;
				}

			}
 		}
	}

	/*************************************************/
	else if (!strcmp(cmd, "showpid")) 
	{
		if (num_arg > 0)
			illegal_cmd = TRUE;
		else
		{
			int pid = getpid();
			fprintf(stdout,"smash pid is %d\n", pid);
		}
		return 0;
	}

	/*************************************************/
	else if (!strcmp(cmd, "fg")) // TODO fg needs to rerun the process that had been stopped by cntl-z
		//answer : will run the latest that was in the foreground.
		// check on linux kernel!!!
	{
		if (num_arg > 1)
			illegal_cmd = TRUE;
		else
		{
			int jobNumber;
			if (num_arg == 1)
			{
				jobNumber = atoi(args[1]);
			}
			else
			{
				jobNumber = getLatestJob(jobs,FALSE);
			}

			if (jobNumber > 0 && jobNumber < jobs_counter)
			{
				printf("%s\n", jobs[jobNumber]->name);
				kill(jobs[jobNumber]->pid ,SIGCONT);
				foregroundPid = jobs[jobNumber]->pid;
				fprintf(stdout,"smash > signal SIGCONT was sent to pid %d\n",jobs[jobNumber]->pid);
				waitpid(jobs[jobNumber]->pid,NULL,0); //don't stop here, continue the code from here

			}
			else if (jobNumber == -1)
			{
				perror("smash error:> no jobs to move to fg\n");
				return 1;
			}
			else
			{
				perror("smash error:> job index is not legal\n");
				return 1;
			}
			return 0;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "bg"))  //TODO remove bg process that ended
	{
		if (num_arg > 1)
			illegal_cmd = TRUE;
		else
		{
			int jobNumber;
			if (num_arg == 1)
			{
				jobNumber = atoi(args[1]);
			}
			else
			{
				jobNumber = getLatestJob(jobs,TRUE);
			}

			if (jobNumber > 0 && jobNumber < jobs_counter)
			{
				int pid;
				pid = jobs[jobNumber]->pid;
				fprintf(stdout,"%s\n", jobs[jobNumber]->name);
				kill(pid,SIGCONT);
				fprintf(stdout,"smash > signal SIGCONT was sent to pid %d\n",jobs[jobNumber]->pid);
			}
			else
				return 1;

			return 0;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
		if (num_arg > 1)
			illegal_cmd = TRUE;
		else if (num_arg == 0)
		{
			while(jobs_counter)
			{
				removeJob(jobs,jobs[jobs_counter-1]->pid);
			}
			exit(0);
		}

	else if (strcmp(args[1],"kill")==0)
	{
		int i;
		int end_loop = jobs_counter;
		for(i = 0; i < end_loop; i++)
		{
			int pid = jobs[i]->pid;
			kill(pid,SIGTERM);
			fprintf(stdout,"smash > signal SIGTERM was sent to pid %d\n",jobs[i]->pid);
			time_t start_time;
			time(&start_time);
			while ( waitpid(pid,NULL,WNOHANG) == 0) //there are children who haven't terminate already
			{
				time_t current_time;
				time(&current_time);
				if ( current_time - start_time > 5)
				{
					kill(pid,SIGKILL);
					fprintf(stdout,"smash > signal SIGKILL was sent to pid %d\n",jobs[i]->pid);
				}
			}
		}
		while(jobs_counter)
		{
			removeJob(jobs,jobs[jobs_counter-1]->pid);
		}
		exit(0);
		}
	} 
	/*************************************************/
	else // external command
	{
 		ExeExternal(args, cmdString);
	 	return 0;
	}
	if (illegal_cmd == TRUE)
	{
		fprintf(stderr,"smash error: > \"%s\"\n", cmdString);
		return 1;
	}
    return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char *args[MAX_ARG], char* cmdString) // TODO ask how possible that external is handled by contolZ/C
{
	int pID;
    switch(pID = fork())
	{
    		case -1: 
    			perror("Fork command was failed");
    			exit(1);
        	case 0 :
                	// Child Process
               		setpgrp();
               		execvp(*args,args);
               		perror("external command has faild");
               		exit(1);
			default:
                	// parent process
				waitpid(pID,NULL,0);
	}
}
//**************************************************************************************
// function name: ExeComp
// Description: executes complicated command
// Parameters: command string
// Returns: 0- if complicated -1- if not
//**************************************************************************************
int ExeComp(char* lineSize)
{

	int pID;
    if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
    {
    	char* ExtCmd[] = {"csh","-f","-c",lineSize,"\0"};
        switch(pID = fork())
    	{
        		case -1:
        			perror("Fork command was failed");
        			exit(1);
            	case 0 :
                    	// Child Process
                   		setpgrp();
                   		execvp(*ExtCmd,ExtCmd);
                   		perror("external complicated command has failed");
                   		exit(1);
    			default:
                    	// parent process
    				waitpid(pID,NULL,WNOHANG);
    	}
        return 0;
	} 
	return -1;
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
int BgCmd(char* lineSize, PJOB *jobs)
{
	char* Command;
	char* delimiters = " \t\n";
	char *args[MAX_ARG];
	time_t proc_time;

	if (lineSize[strlen(lineSize)-2] == '&')
	{
		lineSize[strlen(lineSize)-2] = '\0';

		int i = 0, num_arg = 0, pID;
	    Command = strtok(lineSize, delimiters);
		if (Command == NULL)
			return 0;
	   	args[0] = Command;
		for (i=1; i<MAX_ARG; i++)
		{
			args[i] = strtok(NULL, delimiters);
			if (args[i] != NULL)
				num_arg++;
		}

        switch(pID = fork())
    	{
        		case -1:
        			perror("Fork command was failed\n");
        			exit(1);
            	case 0 :
                    	// Child Process
                   		setpgrp();
                   		execvp(*args,args);
                   		perror("Background command has failed\n");
                   		exit(1);
    			default:
                    	// parent process
    				time(&proc_time);
					insertJob(jobs,Command,pID,proc_time,FALSE,jobs_counter);

    	}
		

        return 0;
	}
	return -1;
}

