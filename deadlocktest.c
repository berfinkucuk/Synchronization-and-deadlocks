#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "pthread.h"
#include "ralloc.h"
#include <stdlib.h>

int handling_method;          // deadlock handling method

#define M 3                   // number of resource types
int exist[3] =  {7, 2, 6};  // resources existing in the system

#define N 5                   // number of processes - threads
pthread_t tids[N];            // ids of created threads
 int k = 30; 
 int k2 = 30;
void *aprocess (void *p)
{
    int pid = *((int*)p);
    int req[3];
   

	
	req[0] = 7;
	req[1] = 2;
	req[2] = 6;
	 
    ralloc_maxdemand(pid, req); 
    
		if(pid == 0){
			req[0] = 0;
			req[1] = 1;
			req[2] = 0;
			ralloc_request (pid, req);
		}
		else if(pid == 1){
			req[0] = 2;
			req[1] = 0;
			req[2] = 0;
			ralloc_request (pid, req);
		}
		else if(pid == 2){
			req[0] = 3;
			req[1] = 0;
			req[2] = 3;
			ralloc_request (pid, req);
		}
		else if(pid == 3){
			req[0] = 2;
			req[1] = 1;
			req[2] = 1;
			ralloc_request (pid, req);
		}
		else{
			req[0] = 0;
			req[1] = 0;
			req[2] = 2;
			ralloc_request (pid, req);
		}
		
		sleep(1);
		printf("Pid: %d\n",pid);
		printAll();
		sleep(k);
		k -= 5;
		//ralloc_release (pid, req);

		// call request and release as many times as you wish with
		// different parameters
    
    pthread_exit(NULL); 
}

void *a2process (void *p)
{
    int pid = *((int*)p);
    int req[3];
		
		if(pid == 0){
			req[0] = 0;
			req[1] = 0;
			req[2] = 0;
			ralloc_request (pid, req);
		}
		else if(pid == 1){
			req[0] = 2;
			req[1] = 0;
			req[2] = 2;
			ralloc_request (pid, req);
		}
		else if(pid == 2){
			req[0] = 0;
			req[1] = 0;
			req[2] = 1;
			ralloc_request (pid, req);
		}
		else if(pid == 3){
			req[0] = 1;
			req[1] = 0;
			req[2] = 0;
			ralloc_request (pid, req);
		}
		else{
			req[0] = 0;
			req[1] = 0;
			req[2] = 2;
			ralloc_request (pid, req);
		}
		
		
		sleep(1);
		printf("******Pid: %d\n",pid);
		printAll();
		sleep(5);
		k2 -= 5;
		//ralloc_release (pid, req);

		// call request and release as many times as you wish with
		// different parameters
    
    pthread_exit(NULL); 
}

int main(int argc, char **argv)
{
    int dn; // number of deadlocked processes
    int deadlocked[N]; // array indicating deadlocked processes
    int k;
    int i;
    int pid; 
    
    for (k = 0; k < N; ++k)
		deadlocked[k] = -1; // initialize    
    
    handling_method = DEADLOCK_DETECTION;
    ralloc_init (N, M, exist, handling_method);
    
    for (i = 0; i < N; ++i) {
        pid = i;
        sleep (1);
        pthread_create (&(tids[i]), NULL, (void *) &aprocess, (void *)&pid);
        sleep(1);
    }
    sleep(5);
    for (i = 0; i < N; ++i) {
        pid = i;
        sleep (1);
        pthread_create (&(tids[i]), NULL, (void *) &a2process, (void *)&pid);
        sleep(1);
    }
    
    while (1) {
        sleep (15); // detection period
        printRequest();
        if (handling_method == DEADLOCK_DETECTION) {
            dn = ralloc_detection(deadlocked);
            if (dn > 0) {
                printf ("there are deadlocked processes %d\n", dn);
            }
        }
        // write code for:
        // if all treads terminated, call ralloc_end and exit
    }    
    ralloc_end();
}

