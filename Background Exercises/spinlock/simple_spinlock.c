//Write a simple spin lock
//What is a spin lock, compare it with a mutex
//Consider two threads competing for the same resource -- threads, shared resource, lock

#include<stdio.h>
#include<stdatomic.h>
#include <pthread.h>


//The shared resource
int shared = 2;
//The lock variable, atomic flags are guranteed to be "lock-free"
//The atomic flag cannot be directly looked upon, if it can then it'd defeat the purpose of atomics                           -- I've forgotten the underlying reasoning
atomic_flag lock = ATOMIC_FLAG_INIT;
    
//spinlock functions
void acquire_spinlock(){
    //flg is set to "clear" on initialization
    while(atomic_flag_test_and_set(&lock)){
        //sets true and returns prev value
        //if lock is clear, then sets to true and returns clear
        //this allows the thread that acquires the lock to execute freely

        //the thread looking to acquite an occupied lock will be looping, busy waiting -- leading to a spin lock!
    }
    printf("Lock acquired\n");
}

//just abstraction
void release_spinlock(){
    atomic_flag_clear(&lock);
    printf("Lock released\n");
}

//Two functions in order to spawn two threads with diff tasks
//I wonder why thread functions cannot have fixed number of arguments                                   -- unanswered
void* multiply(void* thread_arg){
    printf("thread1 created successfully\n");
    acquire_spinlock();
    shared*=5;
    shared*=3;
    release_spinlock();
    printf("thread1 exited successfully\n");
}

void* subtract(void* thread_arg){
    printf("thread2 created successfully\n");
    acquire_spinlock();
    shared-=1;
    release_spinlock();
    printf("thread2 exited successfully\n");
}


//The need for atomic flag checks
int main(){
    //printf("%ld\n", __STDC_VERSION__);
    pthread_t thr1,thr2;
    
    printf("Shared variable value before thread creation: %d\n",shared);
    //Creating the threads 
    pthread_create(&thr1,NULL,multiply, NULL);
    //ended up using the same reference
    pthread_create(&thr2,NULL,subtract, NULL);    
    //threads execute in an interleaved manner once created, key idea of concurrency    
        //This feels counter productive in case of single core systems

    //exit() vs join(), join blocks the calling thread until target thread finishes execution
    pthread_join(thr1,NULL);
    pthread_join(thr2,NULL);
    printf("Shared variable value after thread execution: %d\n",shared);
    return 0;
}

/*
Ideas about concurrency, interleaving and atomicity 
Just two functions from pthread
flags from stdatomic

what are the lock free datatypes from stdatomic?
*/

/*
Observations:

Difference in performance between Windows and Linux
    MinGW wrapper in Windows, the program works as expected
    Facing segmentation fault in WSL after thread creation
        Ended up using the same thread reference for both threads
        failed join() led to the issue!
*/