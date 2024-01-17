#include <unistd.h>
#include <setjmp.h>
// #include"locks.h"
#include<ucontext.h>

#define STACK_SIZE 65536
typedef unsigned long mythread_t;

typedef struct threadDesc{
    mythread_t tid;
    ucontext_t *context;
    int stacksize;
    void *stack;
    void *args;
    sigset_t signal_set;
    void *(*fn)(void *);
    void *res;
    int ok;
    int status;

}thDesc;

//linked list node data structure for storing threads created.
typedef struct node{
    thDesc *th;
    struct node *next;
}node;

//actual linked list structure to access the whole LL of threads.  
typedef struct{
    node *start;
    node *end ;
    int thread_count;
}th_ll ;  // thread linked list

typedef struct exited_thread{
    int *exit_array;
    int exit_count;
}exited_thread;

typedef  struct {
    int flag;    // if flag == 0 , then one thread can acquire the lock. while acquiring it changes the value of flag to 1 .
}spinlock;

typedef struct {
    int flag;
    int futex;
} mutex;


int insert_ll(th_ll *list,thDesc *t);

int thread_create(mythread_t *tid, void* (*fn)(void*), void *args);
int thread_join(mythread_t tid,void **retval);
void thread_exit(void *retval);
int thread_kill(mythread_t tid,int signal);


void spinlock_init(spinlock*sl);
int thread_lock(spinlock *sl);
int thread_unlock(spinlock *sl);

void mutex_init(mutex* m);
void mutex_lock(mutex* m);
void mutex_unlock(mutex* m);