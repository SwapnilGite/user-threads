#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include<sys/mman.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include<unistd.h>

#define STACK_SIZE 65536;


typedef pid_t mthread_t;

typedef struct mthread{
	int tid; //thread ID
	void *stack; //stack
	void *(*fn)(void *) ; //function 
	void *retval; //return value
	void *arg; //arguments
	mthread_t kernel_tid; //kernel_tid
}mthread;


//thread functions:
int thread_create(mthread_t *tid, void *(*fn)(void *),void *arg);
int thread_join(int tid,void **retval);
void thread_exit(void *retval);
int thread_kill(pid_t tid,int signal);
