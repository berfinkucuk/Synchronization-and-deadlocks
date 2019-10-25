#include <stdio.h>
#include <pthread.h>
#include "ralloc.h"
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>

pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

int *maxDemand;
int *allocation;
int *available;
int *request;//pending requests
int dead_handling;
int P, R; // p_count and r_count in order

int ralloc_init(int p_count, int r_count, int r_exist[], int d_handling)
{
	if(p_count > MAX_PROCESSES || r_count > MAX_RESOURCE_TYPES)
		return -1;
	
	maxDemand =  (int*)malloc(sizeof (int) * (r_count * p_count));
	allocation = (int*)malloc(sizeof (int) * (r_count * p_count));
	request = (int*)malloc(sizeof (int) * (r_count * p_count));
	available = (int*)malloc(sizeof (int) * (r_count));
	dead_handling = d_handling;
	
	int i;
	for(i = 0; i < r_count; i++)
		available[i] = r_exist[i];
	
	for(i = 0; i < r_count * p_count; i++)
		allocation[i] = 0;
	
	for(i = 0; i < r_count * p_count; i++)
		request[i] = 0;	
	
	P = p_count;
	R = r_count;
	
    printf ("ralloc_init called\n");

        
    return (0); 
}

int ralloc_maxdemand(int pid, int r_max[]){
    pthread_mutex_lock(&lock);	
    int i;
    int index = 0;
    for(i = pid*R; i < pid*R + R; i++){
		maxDemand[i] = r_max[index++];
	}
    
    printf ("ralloc_maxdemand called by %d\n", pid);
	pthread_mutex_unlock(&lock);	
    return (0); 
}
int ralloc_request (int pid, int demand[]) {
	pthread_mutex_lock(&lock);
	int j;
	int i;
	int count = 0;
	printf ("ralloc_request called by %d\n",pid);
	//add demand to the request
	pthread_mutex_lock(&lock2);	
	int count3 = 0;
	for(j = pid*R; j < pid*R + R; j++){
		request[j] += demand[count3++];	
	}
	pthread_mutex_unlock(&lock2);
	
	
	//check if demand is less than need
	
	int need[R * P];
	for(i = 0; i < P*R; i++)
		need[i] = maxDemand[i] - allocation[i];
		
	for(i = pid*R; i < pid*R + R; i++){
		if(need[i] < demand[count++]){
			//printf("need is %d",need[i]);
			//printf("demand is %d", demand[count - 1 
			printf("demand is greater than need, cannot be granted \n");
			pthread_mutex_unlock(&lock);
			return -1;
		}
	}
	
	
	//firstly check if it is available
	for(i = 0; i < R; i++){
		while(demand[i] > available[i]){	//***check it also
			pthread_cond_wait(&c,&lock);
			i = 0; //check it again from  beginning 	
		}	
	}
	
	
	//if deadlock_nothing allocate resource whatever it is safe or not
	if( dead_handling == DEADLOCK_NOTHING || dead_handling == DEADLOCK_DETECTION){
		for(i = 0; i < R; i++){
			available[i] = available[i] - demand[i]; 
		}
		
		//allocated = allocated + demand
		int index = 0;
		for(i = pid*R; i < pid*R + R; i++){
			allocation[i] = allocation[i] + demand[index++];
		}
		//if a thread reaches here then request is granted so make it 0
		int count4 = 0;
		for(j = pid*R; j < pid*R + R; j++)
			request[j] -= demand[count4++]; 
		printf ("ralloc_request exited by %d\n",pid);
		pthread_mutex_unlock(&lock);
		return 0;
	}
	else if( dead_handling == DEADLOCK_AVOIDANCE){ //then if handling is deadloc_avoidance check if it is safe
		//if it is safe allocate the resources
		//need = maxDemand - allocation is moved up
		int tempAva[P*R];
		int tempAlloc[P*R];
		for(i = 0; i < R; i++)
			tempAva[i] = available[i];
			
		for(i = 0; i < R; i++)
			printf("tempAva%d\n", tempAva[i]);
		
		for(i = 0; i < P*R; i++)
			tempAlloc[i] = allocation[i];
		
		
		
		for(i = 0; i < R; i++){
			tempAva[i] = tempAva[i] - demand[i];
			tempAlloc[pid*R + i] += demand[i];
			need[pid*R + i] = need[pid*R + i] - demand[i];
		}
		
		int path[P]; // en sonda processlerin hangi sırada safe olacağını tutuyo
		int check[P]; // processler allocate edilebildi mi diye bakıyo
		count = 0; //ind
		for(i = 0; i < P; i++)
			check[i] = 0;
		int t, k, bool;
		
		for(t = 0; t < P; t++) // iki tane P ye kadar for loop var çünkü hangi processin ilk allocate edilebileceğini bilmiyoruz
		{
			for(i = 0; i < P; i++)
			{
				if(check[i] == 0){  // çoktan allocate edildiyse buna girmiyo
					bool = 0;
					for(j = 0; j < R; j++){						
						if(need[i*R + j] > tempAva[j]) // need availabledan büyükse o iş yürümez
							bool = 1;
					}
					
					// eğer process allocate edilebilecekse buraya girip checki 1 e eşitliyo.
					if(bool == 0){ // if availables are ok for need[i]
						check[i] = 1;
						path[count] = i;
						count++;
						for(k = 0; k < R; k++)
							tempAva[k] += tempAlloc[i * R + k];
					}
				}
			}
		}

		// control time
		bool = 0;
		for(i = 0; i < P; i++)
			if(check[i] != 1)
				bool = 1;
		
		// bool 1 ise safe değil
		if(bool == 1){
			printf("not safe \n");
			pthread_cond_wait(&c,&lock);
			ralloc_request(pid,demand);	
		} 
		
		else{
			printf("Order of processes for a safe sequence is: ");
			for(i = 0; i < P; i++)
				printf("%d ",path[i]);
			printf("\n");
			// in this for, allocation of desired request to processes
			for(i = 0; i < R; i++){
				available[i] = available[i] - demand[i];
				allocation[pid*R + i] = tempAlloc[pid*R + i];
			}
			
			count = 0;
			for(i = pid*R; i < pid*R + R; i++)
				request[i] = request[i] - demand[count++];
			printf ("ralloc_request exited by-- %d\n",pid);
			pthread_mutex_unlock(&lock);
			return 0;
		}
		
	}	
	else{ // if wrong number entered as dead_handling
		pthread_mutex_unlock(&lock);
		return(-1); 
	}
}
int ralloc_release (int pid, int demand[]) {
	pthread_mutex_lock(&lock);
	//available = available + demand
	int i;
	for(i = 0; i < R; i++)
		available[i] += demand[i];
	//allocated = allocated - demand
	int index = 0;
	for(i = pid*R; i < pid*R + R; i++)
		allocation[i] = allocation[i] - demand[index++];
		
	pthread_cond_broadcast(&c); //broadcast all pending requests
		
    printf ("ralloc_release called** %d\n",pid);
    pthread_mutex_unlock(&lock); 
    return (0); 

}
int ralloc_detection(int procarray[]) { //1 = deadlocked *** -1 o/w
	printf("Detection:\n");
	pthread_mutex_lock(&lock);	
	int work[R];
	int t, w;
	for(t = 0; t < R; t++)
		work[t] = available[t];
	int flag; //1 if it is true 0 false
	for(t = 0; t < P; t++){
		flag = -1;
		for(w = t*R; w < t*R + R; w++){
			if(allocation[w] != 0)
				flag = 1;
		}
		procarray[t] = flag;
	}
	//find an index such that both
	//procarray[i] == 1
	//requesti<=work

	for(t = 0; t < P; t++){
		if(procarray[t] == 1){
			int valid = 1;
			int count5 = 0;
			for(w = t*R; w < t*R + R; w++){
				if(request[w] > work[count5++])
					valid = 0; //request is not OK
			}
			if(valid == 1){
				procarray[t] = -1;
				int tR = t*R;
				for(t = 0; t < R; t++)
					work[t] = work[t] + allocation[tR++]; 
				//go back step 2
				t = -1;
			}

		}
	}
	
	//return number of deadlocked process
	int deadlocked = 0;
	for(t = 0; t < P; t++){
		if(procarray[t] == 1 ){
			deadlocked++;
			printf("Deadlocked %d \n",t);
		}
	}
	pthread_mutex_unlock(&lock);
	return deadlocked; 
}

int ralloc_end() {
    printf ("ralloc_end called\n");
    free(maxDemand);
	free(allocation);
	free(available);
	free(request);//pending requests
    return (0); 
}

void setAllocation(int pid, int demand[]){//for easy testing
	int i;
	int count = 0;
	for(i = pid*R; i < pid*R + R; i++){ //increase allocation
		allocation[i] += demand[count++]; 
	}
	
	for(i = 0; i < R; i++){
		available[i] -= demand[i];
	}
}
void printAvailable(){
	int i ;
	for(i = 0; i <  R; i++){
		printf("av: %d %d \n",i,available[i]);
	}
}

void printMaxDemand(){
	int i ;
	for(i = 0; i < P * R; i++){
		printf("max: %d %d \n",i,maxDemand[i]);
	}
}

void printAlloc(){
	int i ;
	for(i = 0; i < P*R; i++){
		printf("a: %d %d \n",i, allocation[i]);
	}
}

void printRequest(){
	int i ;
	for(i = 0; i < P * R; i++){
		printf("req: %d %d \n",i, request[i]);
	}
}

void printAll(){
	int i ;
	printf("alloc: \n");
	for(i = 0; i < P*R; i++){
		printf(" %d  ", allocation[i]);
	}
	printf(" \n");
	
	printf("avail: \n");
	for(i = 0; i <  R; i++){
		printf("av: %d  \n",available[i]);
	}
	printf(" \n");

	printf("request: \n");
	for(i = 0; i < P*R; i++){
		printf(" %d  ", request[i]);
	}
	printf(" \n");
	printf("max: ");
	for(i = 0; i < P * R; i++){
		printf(" %d",maxDemand[i]);
	}
	printf(" \n");
}
