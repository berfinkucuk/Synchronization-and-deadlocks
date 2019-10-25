#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "ralloc.h"

int handling_method;          // deadlock handling method

#define M 3                   // number of resource types
int exist[3] =  {12, 8, 10};  // resources existing in the system

#define N 5                   // number of processes - threads
pthread_t tids[N];            // ids of created threads

void *aprocess (void *p)
{
    int pid = *((int*)p); 
    int req[3];
    int k; 

    req[0] = 2;
    req[1] = 2;
    req[2] = 2;
    ralloc_maxdemand(pid, req); 
    
    for (k = 0; k < 10; ++k) {
	req[0] = 1;
	req[1] = 1;
	req[2] = 1;
	ralloc_request (pid, req);
	
	// do something with the resources
	
	ralloc_release (pid, req);

	// call request and release as many times as you wish with
	// different parameters
    }
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
    
    //handling_method = DEADLOCK_DETECTION;
    //handling_method = DEADLOCK_AVOIDANCE;
    handling_method = DEADLOCK_NOTHING;
    ralloc_init (N, M, exist, handling_method);
    for (i = 0; i < N; ++i) {
        pid = i;
        pthread_create (&(tids[i]), NULL, (void *) &aprocess,
                        (void *)&pid);
        sleep(1);
    }
    
    if (handling_method == DEADLOCK_DETECTION) {
            dn = ralloc_detection(deadlocked);
            if (dn > 0) {
                printf ("there are deadlocked processes\n");
            }
    }
    ralloc_end();

}
