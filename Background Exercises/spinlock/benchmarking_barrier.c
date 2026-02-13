#include<stdio.h>
#include<pthread.h>
#include<stdatomic.h>
#include<time.h>
#include<stdint.h>
#include<x86intrin.h>

int shared=0;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
atomic_flag spin_lock = ATOMIC_FLAG_INIT;
pthread_barrier_t barrier_start, barrier_end;

//spinning, even while atomic involves memory access !!                 -- exactly! and therefore is costly too
//make the functions inline
void acquire_spinlock(){
    while(atomic_flag_test_and_set(&spin_lock));
}

void release_spinlock(){
    atomic_flag_clear(&spin_lock);
}


void* system_operations(void* args){
    //Synchronizing threads before measurement
    pthread_barrier_wait(&barrier_start);
    for(int i=0;i<1000000;i++){
        pthread_mutex_lock(&mutex_lock);
        shared++;
        pthread_mutex_unlock(&mutex_lock);
    }
    //cannot complete execution until all threads have completed and barrier is lifted! 
    pthread_barrier_wait(&barrier_end);
    return NULL;
}

void* spinning_operations(void* args){
    //Synchronizing threads before measurement
    pthread_barrier_wait(&barrier_start);
    for(int i=0;i<1000000;i++){
        acquire_spinlock(&spin_lock);
        shared++;
        release_spinlock(&spin_lock);
    }
    //cannot complete execution until all threads have completed and barrier is lifted! 
    pthread_barrier_wait(&barrier_end);
    return NULL;
}

//function pointer as argument
//using a thread barrier to reduce overhead
double benchmark(int n,void* (*fn)(void*)){
    pthread_t threads[n];    
    pthread_barrier_init(&barrier_start, NULL, n+1);
    pthread_barrier_init(&barrier_end, NULL, n+1);
    struct timespec clk_start, clk_end;
    double clock_time;

    //initializing shared variable to 0
    shared=0;

    for(int i=0;i<n;i++){
        pthread_create(threads+i,NULL,fn,NULL);
    }

    //Start recording time once all threads have been created and barrier has been lifted
    pthread_barrier_wait(&barrier_start);       //n from threads and 1 here
    clock_gettime(CLOCK_MONOTONIC,&clk_start);
    //completely isolating thead overhead
    pthread_barrier_wait(&barrier_end);
    clock_gettime(CLOCK_MONOTONIC,&clk_end);

    for(int j=0;j<n;j++){
        pthread_join(threads[j],NULL);
    }

    pthread_barrier_destroy(&barrier_start);
    pthread_barrier_destroy(&barrier_end);
    clock_time=(clk_end.tv_sec - clk_start.tv_sec)+(clk_end.tv_nsec-clk_start.tv_nsec)*1e-9;
    
    return clock_time;
}

//using the time stamp counter
uint64_t benchmark_tsc(int n,void* (*fn)(void*)){
    pthread_t threads[n];    
    pthread_barrier_init(&barrier_start, NULL, n+1);
    pthread_barrier_init(&barrier_end, NULL, n+1);
    //what if i use double instead of this type?
    uint64_t start_time,end_time;

    //initializing shared variable to 0
    shared=0;
    
    for(int i=0;i<n;i++){
        pthread_create(threads+i,NULL,fn,NULL);
    }

    //Measuring using instrintic rdtsc
    pthread_barrier_wait(&barrier_start); 
    start_time=__rdtsc();
    pthread_barrier_wait(&barrier_end);
    end_time=__rdtsc();
    //isolating thread overhead using two barriers
    for(int j=0;j<n;j++){
        pthread_join(threads[j],NULL);
    }
    
    pthread_barrier_destroy(&barrier_start);
    pthread_barrier_destroy(&barrier_end);

    return end_time-start_time;
}


int main(){
    //Measuring spinning lock mutex
    int n=1;
    double measured_time;

    //easured_time=benchmark(n,spinning_operations);
    //printf("Clock time -- spinning mutex -- of %d threads = %f\t\t",n,measured_time);
    //Initialize a barrier to ensure that the all threads contend at once   -- can be reused?
    
    for(int n=1;n<=150;n+=10){
        //safe to have multiple calls, becuase of 'join()'
        measured_time=benchmark(n,spinning_operations);
        printf("Clock time -- spinning mutex -- of %d threads = %f\t\t",n,measured_time);
        measured_time=benchmark(n,system_operations);
        printf("Clock time -- system mutex -- of %d threads = %f\n",n,measured_time);
        if(n>10)
            n+=15;
    }
    
    
    return 0;
}

/*
    Should I reuse the barrier or set up another one?
*/

/*
Observations
    I'll need two barriers to completely isolate thread overhead from the performance analysis
*/

