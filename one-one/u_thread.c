#include "list.h"
#include "u_thread.h"
#include <sys/mman.h>
#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <sys/resource.h>
#include <stdbool.h>
#define CLONE_ALL_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_SYSVSEM|CLONE_PARENT_SETTID|SIGCHLD

threadLL thread_ll;
int u_ID = 1;
bool firstThread = true; //to check the first thread 


void* make_stack(int size)
{	
	stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 	0);
    if (stack == MAP_FAILED){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

//wrapper function for clone sys call
int wrapper(void* farg){
    mrthread *temp;
    temp = (mthread*)farg;
 
    temp->retval = temp->fn(temp->arg);
    return 0;
}

int thread_create(mthread_t *tid,void* (*fn) (void *), void *arg)
{	
	if(tid == NULL || fn == NULL)
	{	
		return EINVAL; //invalid args
	}
	
	if(firstThread)
	{	
		firstThread = false;
		initLL(&thread_ll);
	}
	mthread* thread = (mthread*)malloc(sizeof(mthread));
	if(thread == NULL)
	{	
		return -1;
	}
	
	void* stack = make_stack(STACK_SIZE);
	thread->tid  = u_ID;
	thread->stack = stack;
	thread->fn = fn;
	thread->arg = arg;
	//int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND;
	mthread kernel_tid = clone(wrapper,thread->stack+STACK_SIZE,CLONE_ALL_FLAGS,NULL);
	if(kernel_tid == -1)
	{	
		perror("Clone error");
		free(stack);
		free(thread);
		return -1;
	}
	thread->kernel_tid = kernel_tid;
	*tid = kernel_tid;
	insertLL(&thread_ll,thread);
	u_ID++;
	
	
	return 0;

}

int thread_join(int tid,void **retval)
{	
	node *temp = get(&thread_ll,tid);
	int wstatus;
	if(temp)
	{	
		int w = waitpid(tid,&wstatus,0);
		if(w == -1);
		{	
			perror("waitpid");
		}
	}
	else
	{	
		return ESRCH;
	}
	
	if(retval)
	{	
		*retval = temp->thread->retval;
		
	}
	if(!deleteLL(&thread_ll,tid))
	{	
		printf("delete error\n");
	}
	u_ID--;
	return 0;
}


















