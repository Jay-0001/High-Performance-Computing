#include<stdio.h>
#include<pthread.h>

//Shared variable 
int shared=0;
//System given mutex implementation, there are diff kinds of mutexes
pthread_mutex_t mutex_lock=PTHREAD_MUTEX_INITIALIZER;

//Simple function to allow large amounts of threads
void* increment(void* args){
    //suspends the calling thread if mutex is already locked
    pthread_mutex_lock(&mutex_lock);
    shared++;
    pthread_mutex_unlock(&mutex_lock);
}

int main(){
    //is Uppercase preferred for MACROS?
    int n=25;
    pthread_t thread[n];

    printf("Shared variable before thread creation: %d\n",shared);
    //create n threads
    for(int i=0;i<n;i++){
        pthread_create(thread+i,NULL,increment,NULL);
    }

    //Ensure that all threads are reaped before the end of main thread
    for(int i=0;i<n;i++){
        //join takes thread id as argument, not its address
        pthread_join(thread[i],NULL);
    }

    //Destroy mutex after usage ?
    pthread_mutex_destroy(&mutex_lock);
    printf("Shared variable after thread execution: %d\n",shared);
    return 0;
}

/*
pthread mutex functions for initialization, lock, unlock && destruction
*/


/*
Observations
Shared variable is incremented 'n' times as expected
    In case of wrong interleaving, the sum would be lower than intended
*/