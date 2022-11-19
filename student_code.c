#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>

void logStart(char* tID);//function to log that a new thread is started
void logFinish(char* tID);//function to log that a thread has finished its time

void startClock();//function to start program clock
long getCurrentTime();//function to check current time since clock was started
time_t programClock;//the global timer/clock for the program

sem_t sem_Even;
sem_t sem_Odd; // global int semaphore pointers

typedef struct thread //represents a single thread, you can add more members if required
{
	char tid[4];//id of the thread as read from file
	unsigned int startTime;
	int state;
	pthread_t handle;
	int retVal;
} Thread;

//you can add more functions here if required

void* threadRun(void* t);//the thread function, the code executed by each thread
int readFile(char* fileName, Thread** threads);//function to read the file content and build array of threads

int main(int argc, char *argv[])
{
	if(argc<2)
	{
		printf("Input file name missing...exiting with error code -1\n");
		return -1;
	}

    //you can add some suitable code anywhere in main() if required
	sem_init(&sem_Even, 0, 1);
	sem_init(&sem_Odd, 0, 1);

	

	Thread* threads = NULL;
	int threadCount = readFile(argv[1],&threads);

	startClock();
    //write some suitable code here to initiate, progress and terminate the threads following the requirements
	int i;
	for(i=0; i < threadCount; i++) 
	{
		if(atoi(threads[i].tid) % 2 == 0) 
		{
			threads[i].retVal = 0;
		}
		else 
		{
			threads[i].retVal =  1;
		}
	}

	int maximum = threads[0].startTime;

	for(i = 0; i < threadCount; i++) 
	{
		if(threads[i].startTime > maximum)
		{
			maximum = threads[i].startTime;
		}
	}

	while(getCurrentTime() <= maximum)
	{
		for(i = 0; i < threadCount; i++)
		{
			if((threads[i].startTime == getCurrentTime()) && threads[i].state == 0) 
			{
				pthread_create(&threads[i].handle, NULL, &threadRun, &threads[i]);
				threads[i].state = 1;
				break;
			}
		}
	}

	for(i=0; i < threadCount; i++) 
	{
		pthread_join(threads[i].handle, NULL);
	}

	sem_destroy(&sem_Even);
	sem_destroy(&sem_Odd);

	return threadCount;
}

int readFile(char* fileName, Thread** threads)//do not modify this method
{
	FILE *in = fopen(fileName, "r");
	if(!in)
	{
		printf("Child A: Error in opening input file...exiting with error code -1\n");
		return -1;
	}

	struct stat st;
	fstat(fileno(in), &st);
	char* fileContent = (char*)malloc(((int)st.st_size+1)* sizeof(char));
	fileContent[0]='\0';	
	while(!feof(in))
	{
		char line[100];
		if(fgets(line,100,in)!=NULL)
		{
			strncat(fileContent,line,strlen(line));
		}
	}
	fclose(in);

	char* command = NULL;
	int threadCount = 0;
	char* fileCopy = (char*)malloc((strlen(fileContent)+1)*sizeof(char));
	strcpy(fileCopy,fileContent);
	command = strtok(fileCopy,"\r\n");
	while(command!=NULL)
	{
		threadCount++;
		command = strtok(NULL,"\r\n");
	}
	*threads = (Thread*) malloc(sizeof(Thread)*threadCount);

	char* lines[threadCount];
	command = NULL;
	int i=0;
	command = strtok(fileContent,"\r\n");
	while(command!=NULL)
	{
		lines[i] = malloc(sizeof(command)*sizeof(char));
		strcpy(lines[i],command);
		i++;
		command = strtok(NULL,"\r\n");
	}

	for(int k=0; k<threadCount; k++)
	{
		char* token = NULL;
		int j = 0;
		token =  strtok(lines[k],";");
		while(token!=NULL)
		{
//if you have extended the Thread struct then here
//you can do initialization of those additional members
//or any other action on the Thread members
			(*threads)[k].state=0;
			if(j==0)
				strcpy((*threads)[k].tid,token);
			if(j==1)
				(*threads)[k].startTime=atoi(token);
			j++;
			token = strtok(NULL,";");
		}
	}
	return threadCount;
}

void logStart(char* tID)
{
	printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char* tID)
{
	printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

void* threadRun(void* t)//implement this function in a suitable way
{
	logStart(((Thread*)t)->tid);
	
	//your synchronization logic will appear here
	if(((Thread*)t) -> retVal == 0) 
	{
		sem_wait(&sem_Even);
	}
	else
	{
		sem_wait(&sem_Odd);
	}

	//critical section starts here, it has only the following printf statement
	printf("[%ld] Thread %s is in its critical section\n",getCurrentTime(), ((Thread*)t)->tid);
	//critical section ends here
		if(((Thread*)t) -> retVal == 0) 
	{
		sem_wait(&sem_Odd);
	}
	else
	{
		sem_wait(&sem_Even);
	}

//your synchronization logic will appear here

	logFinish(((Thread*)t)->tid);
	((Thread*)t)->state = -1;
	pthread_exit(0);
}

void startClock()
{
	programClock = time(NULL);
}

long getCurrentTime()//invoke this method whenever you want check how much time units passed
//since you invoked startClock()
{
	time_t now;
	now = time(NULL);
	return now-programClock;
}