#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
// #include <linux/sched.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include<sys/time.h>
#include<signal.h>
#include<string.h>
// #include <sys/syscall.h>
// #include <sys/wait.h>
#include <limits.h>
#include <ucontext.h>
#include<errno.h>
#include "mythread.h"

#define RUNNABLE 0
#define TERMINATED 1
#define RUNNING 3
#define EMBRYO 4
#define WAITING 5

struct itimerval timer;
thDesc *curr_thread = NULL;
thDesc *main_thread = NULL;
thDesc *sched_thread = NULL;
thDesc *prev_thread = NULL;
struct itimerval clock;
mythread_t thread_id;
th_ll thread_list;
exited_thread exit_arr;
struct sigaction sa;

int ok = 0;
int thread_no = 0;

void scheduler();
void timer_activate();
void print_ll();
// spinlock sl;

// finding the running thread and shift it to back of list
void shift_thread_list(th_ll *thread_list)
{
	node *temp = thread_list->start;
	if (temp == NULL) // list is empty
	{
		return;
	}
	if (temp->th->status == RUNNING)
	{
		// printf("here shifting main thread with id = %lu\n",temp->th->tid);
		thread_list->start = thread_list->start->next;
		temp->next = NULL;
		temp->th->status = RUNNABLE;
		thread_list->end->next = temp;
		thread_list->end = temp;
		// free(temp);
		return;
	}
	while (temp->next)
	{
		node *temp1 = temp->next;
		if (temp1->th->status == RUNNING)
		{
			temp->next = temp1->next;
			temp1->th->status = RUNNABLE;
			temp1->next = NULL;
			thread_list->end->next = temp1;
			thread_list->end = temp1;

			// free(temp1);
			break;
		}
		temp = temp->next;
	}
	return;
}

void shift_to_sched()
{
	// printf("In Shift to Sched\n");
	// ok = 1;
	// print_ll();
	scheduler();
	// swapcontext(curr_thread->context,sched_thread->context);
	// timer_activate();
}

void setup_timer()
{
	sigset_t mask;
	sigfillset(&mask);

	struct sigaction timer_handler;
	memset(&timer_handler, 0, sizeof(timer_handler));
	timer_handler.sa_handler = shift_to_sched;
	timer_handler.sa_flags = SA_RESTART;
	timer_handler.sa_mask = mask;

	sigaction(SIGVTALRM, &timer_handler, NULL);

	clock.it_interval.tv_sec = 0;
	clock.it_interval.tv_usec = 1;
	clock.it_value.tv_sec = 0;
	clock.it_value.tv_usec = 20;
	// start timer
	setitimer(ITIMER_VIRTUAL, &clock, 0);
}

void timer_activate()
{
	// printf("######## TIMER ACTIVE ###########\n");
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	// signal(SIGALRM,shift_to_sched);
	return;
}

void timer_deactivate()
{
	// signal(SIGALRM,SIG_IGN);
	// printf("######## TIMER DEACTIVE ###########\n");
	// signal(SIGALRM,SIG_IGN);
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
	return;
}



void *make_stack(size_t size)
{
	void *stack;
	stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if(stack == MAP_FAILED)
	{
		perror("stack allocation error");
		return NULL;
	}
	return stack;
}

int insert_ll(th_ll *thread_list, thDesc *t)
{
	node *temp = (node *)malloc(sizeof(node));
	if (!temp)
	{
		return -1;
	}
	temp->th = t;
	temp->next = NULL;

	if (thread_list->start == NULL && thread_list->end == NULL) // empty list
	{
		thread_list->start = temp;
		thread_list->end = temp;
		thread_list->thread_count++;
		return 0;
	}
	else
	{
		thread_list->end->next = temp;
		thread_list->end = temp;
		thread_list->thread_count++;
		return 0;
	}
}

void scheduler()
{
	// printf("In Scheduler\n");
	timer_deactivate();
	if(thread_list.start == thread_list.end)
	{	
		// exit(0);
		return ;
	}
	shift_thread_list(&thread_list);
	// getcontext(curr_thread->context);

	node *temp = thread_list.start;

	thDesc *now = NULL;
	while (temp)
	{
		if (temp->th->status == RUNNABLE)
		{
			now = temp->th;
			break;
		}
		temp = temp->next;
	}
	if (now)
	{
		prev_thread = curr_thread;
		now->status = RUNNING;
		curr_thread = now;
		curr_thread->args = now->args;
		curr_thread->context = now->context;

		// printf("In now\n");
	}
	// setcontext(curr_thread->context);
	swapcontext(prev_thread->context, curr_thread->context);

	return;
}




void print_ll()
{
	node *temp = thread_list.start;

	while(temp)
	{
		// printf("%lu Status = %d ",temp->th->tid,temp->th->status);
		thDesc *hr = temp->th;
		if(hr->args)
		{
			int ar = *(int*)hr->args;
			// printf(" Args = %d , TID= %lu \n",ar,hr->tid);
		}
		temp = temp->next;
		printf("\n");
	}
	return;
}

void routine(void *thread)
{
	// thDesc *temp= (thDesc*) thread;
	// printf("In Routine\n");
	thDesc *temp = NULL;
	temp = (thDesc *)thread;
	timer_activate();

	temp->res = temp->fn(temp->args);

	// printf("Function Done !!\n");
	timer_deactivate();

	thread_exit(&temp->res);
	// shift_to_sched();
	// raise(SIGVTALRM);
}


// initialize main thread and insert in the thread_list
// setup the timer
void init_t()
{
	// printf("\nSetuping many one for first time\n");
	thread_list.start = NULL;
	thread_list.end = NULL;
	thread_list.thread_count = -1;
	exit_arr.exit_array = (int *)malloc(4 * sizeof(int));
	exit_arr.exit_count = 0;
	main_thread = (thDesc *)malloc(sizeof(thDesc));
	ucontext_t *main_thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(main_thread_context);
	main_thread->status = RUNNING;
	main_thread->tid = getpid();
	main_thread->fn = NULL;
	main_thread->args = NULL;
	main_thread->stack = NULL;
	sigemptyset(&(main_thread->signal_set));
	main_thread->context = main_thread_context;
	curr_thread = main_thread;
	prev_thread = main_thread;
	thread_id = getpid() + 1; // increase thread_if for next coming threads
	insert_ll(&thread_list, main_thread);

	setup_timer();
}
static int test_and_set(int * flag){
    int result, set=1;
    asm("lock xchgl %0, %1"       //this is a atomic instruction as it should not be interuppted while updating the flag.
        : "+m"(*flag), "=a"(result)
        : "1"(set)
        : "cc");
    return result;
}

void spinlock_init(spinlock*sl)
{
    sl->flag=0;
}

int thread_lock(spinlock *sl)
{
    while(1)
    {
        int flag_status = test_and_set(&sl->flag);
        if(flag_status == 0 ) 
        {
            break;
        }
    }
}

int thread_unlock(spinlock *sl)
{
    sl->flag = 0;
    return 1;
}



int thread_create(mythread_t *tid, void *(*fn)(void *), void *args)
{
	timer_deactivate();
	// ok = 0;
	// printf("\nIn thread create function\n");
	static int isFirst = 0;
	if (!isFirst)
	{
		init_t();
		isFirst = 1;
	}
	thDesc *thread = (thDesc *)malloc(sizeof(thDesc));
	ucontext_t *thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(thread_context);
	// thread->stack = (void *)malloc(sizeof(char) * STACK_SIZE);
	thread->stack =make_stack(STACK_SIZE);

	thread_context->uc_stack.ss_sp = thread->stack;
	thread_context->uc_stack.ss_size = STACK_SIZE;
	thread_context->uc_link = main_thread->context;
	thread->ok = 0;
	thread->status = RUNNABLE;
	thread->tid = thread_id++;
	// thread_id++;
	thread->fn = fn;
	thread->args = args;
	sigemptyset(&thread->signal_set);
	thread->context = thread_context;
	makecontext(thread_context, (void *)&routine, 1, (void *)(thread));
	insert_ll(&thread_list, thread);
	thread_no++;
	// printf("Thread %d created and inserted with id = %lu\n", thread_no, thread->tid);
	*tid = thread->tid;
	// setcontext(thread_context);
	timer_activate();
	return 0;
}

thDesc *get_thread(th_ll *list, mythread_t tid)
{
	node *temp = list->start;
	if(temp->th->tid == tid)
	{
		return temp->th;
	}
	while (temp->th->tid != tid)
	{
		// printf("%lu->",temp->th->tid);
		temp = temp->next;
	}
	if (temp)
	{
		return temp->th;
	}
	else
	{
		return NULL;
	}
}


int thread_join(mythread_t tid, void **retval)
{
	thDesc *th = get_thread(&thread_list, tid);
	if (th)
	{
		// sigaddset(&(th->signal_set), SIGALRM);

		// // Block SIGALRM signal for thread
		// sigprocmask(SIG_BLOCK, &(th->signal_set), NULL);
		while (th->ok != 1)
		{
			;
		}
	}
	return 0;
}


void delete_thread(th_ll *list, mythread_t tid)
{
	// Store head node
	// printf("In delete thread\n");
	if (list->thread_count <= 0) // empty list with only main thread
	{
		return;
	}
	struct node *temp = list->start;

	// If head node itself holds the key to be deleted
	if (temp != NULL && temp->th->tid == tid)
	{
		list->start = temp->next; // Changed head
		free(temp);
		list->thread_count--; // free old head
		return;
	}
	node* prev =NULL;
	// Search for the key to be deleted, keep track of the
	// previous node as we need to change 'prev->next'
	while (temp != NULL && temp->th->tid != tid)
	{
	    node *prev = temp;
	    temp = temp->next;
	}

	// If key was not present in linked list
	if (temp == NULL) return;

	// Unlink the node from linked list
	prev->next = temp->next;

	free(temp);  // Free memory
}
void thread_exit(void *retval)
{
	// printf("In thread exit to delete thread %lu\n", curr_thread->tid);
	if (retval == NULL)
	{
		return;
	}
	timer_deactivate();
	curr_thread->status = TERMINATED;

	curr_thread->ok = 1;
	delete_thread(&thread_list, curr_thread->tid);
	curr_thread = main_thread;
	// curr_thread = prev_thread;

	// print_ll();
	// printf("After Print\n");
	timer_activate();
	// scheduler();
}

int thread_kill(mythread_t tid, int sign)\
{
    timer_deactivate();
    if(sign < 0 || sign > 64){
       
        return EINVAL;
    }
	thDesc *temp = get_thread(&thread_list,tid);
    if(temp->tid == tid){
        raise(sign);
        timer_activate();
		
        return 0;
    }
    if(sign == SIGSTOP || sign == SIGCONT || sign == SIGINT){
        kill(temp->tid, sign);
    }
    else{
        thDesc* thread_to_signal = get_thread(&thread_list, tid);
        if(!thread_to_signal){
            
            return ESRCH;
        }
        sigaddset(&thread_to_signal->signal_set, sign);
    }
 
    return 0;
}