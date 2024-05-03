#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>

static pthread_barrier_t barr, barr_end;
volatile int x, y, r1, r2;

static void* thread1(void *arg)
{
    while(1) {
        pthread_barrier_wait(&barr);
        x = 1;    // store
        __asm__ __volatile__("mfence" ::: "memory");
        r1 = y;   // load
        pthread_barrier_wait(&barr_end);
    }
 
    return NULL;
}
 
static void* thread2(void *arg)
{
    while (1) {
        pthread_barrier_wait(&barr);
        y = 1;   // store
        __asm__ __volatile__("mfence" ::: "memory");
        r2 = x;  // load
        pthread_barrier_wait(&barr_end);
    }
 
    return NULL;
}
 
int main()
{
    pthread_barrier_init(&barr, NULL, 3);
    pthread_barrier_init(&barr_end, NULL, 3);
 
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    int cpu_1 = 0;
    int cpu_2 = 1;
    cpu_set_t cs;
    CPU_ZERO(&cs);
    CPU_SET(cpu_1, &cs);
    pthread_setaffinity_np(t1, sizeof(cs), &cs);
    CPU_ZERO(&cs);
    CPU_SET(cpu_2, &cs);
    pthread_setaffinity_np(t2, sizeof(cs), &cs);
    
    // wait result
    while(1) {
        // init variable
        x = y = r1 = r2 = 0;
        pthread_barrier_wait(&barr);
        pthread_barrier_wait(&barr_end);
        printf("r1 = %d, r2 = %d\n", r1, r2);
        assert(!(r1 == 0 && r2 == 0));
    }

    pthread_barrier_destroy(&barr);
    pthread_barrier_destroy(&barr_end);
    return 0;
}