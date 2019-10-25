#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "pthread.h"
#include "ralloc.h"
#include <stdlib.h>

int handling_method;          // deadlock handling method

#define M 3                   // number of resource types
int exist[3] =  {10, 5, 7};  // resources existing in the system

#define N 5                   // number of processes - threads
pthread_t tids[N];            // ids of created threads
 int k = 30; 
 int k2 = 30;
void *aprocess (void *p)
{
    int pid = *((int*)p);
    int req[3];
   
	    req[0] = 7;
	    req[1] = 5;
		req[2] = 3;
		ralloc_maxdemand(0, req); 
		
		req[0] = 3;
		req[1] = 2;
		req[2] = 2;
		ralloc_maxdemand(1, req); 
		
		
		req[0] = 9;
		req[1] = 0;
		req[2] = 2;
		ralloc_maxdemand(2, req); 
		
		
		req[0] = 2;
		req[1] = 2;
		req[2] = 2;
		ralloc_maxdemand(3, req); 
			
	
		req[0] = 4;
		req[1] = 3;
		req[2] = 3;
		ralloc_maxdemand(4, req); 
		
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
    
    handling_method = DEADLOCK_AVOIDANCE;
    ralloc_init (N, M, exist, handling_method);
    
    for (i = 0; i < 1; ++i) {
        pid = i;
        sleep (1);
        pthread_create (&(tids[i]), NULL, (void *) &aprocess, (void *)&pid);
        sleep(2);
    }
    
    int req[3];
	req[0] = 0;
	req[1] = 1;
	req[2] = 0;
    setAllocation(0,req);
    
    req[0] = 2;
	req[1] = 0;
	req[2] = 0;
    setAllocation(1,req);
    
    req[0] = 3;
	req[1] = 0;
	req[2] = 2;
    setAllocation(2,req);
    
    req[0] = 2;
	req[1] = 1;
	req[2] = 1;
    setAllocation(3,req);
    
    req[0] = 0;
	req[1] = 0;
	req[2] = 2;
    setAllocation(4,req);
    
    
    
    printf("LETS SEE\n");
    sleep(1);
    printAll();
    
    
    //slide38
    req[0] = 0;
	req[1] = 0;
	req[2] = 0;
	sleep(1);
    ralloc_request(0,req);
    
    printf("LETS SEE\n");
    sleep(1);
    printAll();
    
    //slide41
    req[0] = 1;
	req[1] = 0;
	req[2] = 2;
    ralloc_request(1,req);
    
    sleep(1);
    printAll();
    
    
    //slide43
    req[0] = 0;
	req[1] = 2;
	req[2] = 0;
    ralloc_request(0,req);
    
    sleep(1);
    printAll();
    
}

