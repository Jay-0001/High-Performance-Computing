#include<stdio.h>
#include<pthread.h>
#include<stdatomic.h>
#include<time.h>

//What am I measuring? What am I benchmarking? 
    //The cost of mutex mechanims -- spinning lock vs system mutex lock
    //Single lock and Unlock count as the basic operation
    //Critical section is made minimal to allow this

int shared=0;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
atomic_flag spin_lock = ATOMIC_FLAG_INIT;

void acquire_spinlock(){
    while(atomic_flag_test_and_set(&spin_lock));
}

void release_spinlock(){
    atomic_flag_clear(&spin_lock);
}


void* system_increment(void* args){
    //suspends the calling thread if mutex is already locked
    pthread_mutex_lock(&mutex_lock);
    shared++;
    pthread_mutex_unlock(&mutex_lock);
}

void* spinning_increment(void* args){
    acquire_spinlock(&spin_lock);
    shared++;
    release_spinlock(&spin_lock);
}

void benchmark(){
    //measure performance inside a function instead? 
}

int main(){
    int n=50;
    pthread_t threads[n];
    clock_t start_time,end_time;
    struct timespec clk_start, clk_end;
    double clock_time;

    //Measuring system mutex 
    start_time=clock();
    clock_gettime(CLOCK_REALTIME,&clk_start);
    for(int i=0;i<n;i++){
        pthread_create(threads+i,NULL,system_increment,NULL);
    }

    for(int j=0;j<n;j++){
        pthread_join(threads[j],NULL);
    }
    clock_gettime(CLOCK_REALTIME,&clk_end);
    end_time=clock();

    clock_time=(clk_end.tv_sec - clk_start.tv_sec)+(clk_end.tv_nsec-clk_start.tv_nsec)*1e-9;
    //printf("CPU time elapsed for system mutex implementation of %d threads = %ld\n",n,end_time-start_time);
    printf("Clock time elapsed for system mutex implementation of %d threads = %f\n",n,clock_time);
    
    //Measuring spinning lock mutex
    start_time=clock();
    clock_gettime(CLOCK_REALTIME,&clk_start);
    for(int i=0;i<n;i++){
        pthread_create(threads+i,NULL,spinning_increment,NULL);
    }

    for(int j=0;j<n;j++){
        pthread_join(threads[j],NULL);
    }
    clock_gettime(CLOCK_REALTIME,&clk_end);
    end_time=clock();

    clock_time=(clk_end.tv_sec - clk_start.tv_sec)+(clk_end.tv_nsec-clk_start.tv_nsec)*1e-9;
    //printf("CPU time elapsed for spinning mutex implementation of %d threads = %ld\n",n,end_time-start_time);
    printf("Clock time elapsed for spinning mutex implementation of %d threads = %f\n",n,clock_time);
    return 0;
}

/*
    Why do the data types end with a '_t'
    The difference between the wall clock and the CPU clock
    How to factor out the thread management overhead
    **Should i try doing summation over all basic operations instead
    I wonder if system implementations use atomic instructions underneath
*/

/*
Observations
    CPU time elapsed for system mutex implementation of 10 threads = 0.000000
        wrong data specifier in printf

    Spinning mutex - 505,290,110,154
    System mutex - 555,166,66,99

    Spinning mutex - 505,290,110,154
    System mutex - 555,166,66,99

    As the number of threads increase, spinning mutex performs worser !???
        Unable to reach a solid conclusion, findings keep varying

    CPU time in windows is much lower than that of wsl        
    
    CPU time does not measure the sleeping, waiting costs
    Shifting to wall clock time instead
        Diff arguments for diff cases

    How to factor out the thread management overhead
        Increase the locking operations instead of thread creations?
*/

