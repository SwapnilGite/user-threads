#include<stdio.h>
#include<stdlib.h>
#include <signal.h>
#include "mythread.h"

#define ERROR "\033[1;31m"
#define SUCCESS "\033[1;32m"
#define BYELLOW "\033[1;33m"
#define WHITE "\033[1;37m"
#define NONE "\033[0m"

int x = 20;
spinlock sl;
unsigned long c1,c2,c;
int flag=1;

void *f(void*arg){
	int a;
	// return NULL;
}
//functions for thread lock test
void *f1(){
    // unsigned long limit = 1000000;
    if(flag==1){
        c1++;
        thread_lock(&sl);
        c++;
        thread_unlock(&sl);
    }
}

void *f2(){
    // unsigned long limit = 1000000;
    if (flag==1){
        c2++;
        thread_lock(&sl);
        c++;
        thread_unlock(&sl);

    }
}


void thread_create_test() 
{
	printf(BYELLOW"\ntesting thread_create function:\n"NONE);
	int create_flag=0;
	// int p = *(int *)arg;
	mythread_t threads[5];
	for ( int i=0 ; i<5 ; i++)
	{
		mythread_t tid;
		tid=thread_create(&threads[i],f,NULL);
		if( tid!=0 )
		{
			create_flag++;
		}
		else
		{
			printf("Thread Created with tid %ld \n",threads[i]);
		}
	}
	if( create_flag==0){
		printf("Many-one threaad create test Success\n");
	}
	else
	{
		printf("Many-one create threads test failed\n");
	}
	return;
}

void thread_join_test()
{
	printf(BYELLOW"\ntesting many_one thread_Join function:\n"NONE);
	// mythread_t threads[5];
	int create_flag=0,flag;
	int join_flag=0;
	// int p = *(int *)arg;
	mythread_t threads[5];
	for ( int i=0 ; i<5 ; i++)
	{
		// mythread_t tid;
		flag=thread_create(&threads[i],f,NULL);
		if( flag!=0 )
		{
			create_flag++;
		}
		else
		{
			printf("Thread Created with tid %ld \n",threads[i]);
		}
	}
	if( create_flag==0){
		printf("Many-one thread create test Success\n");
	}
	else
	{
		printf("Many-one create threads test failed\n");
	}
	int *retval = (int *)malloc(sizeof(int));
	for( int i=0 ; i<5 ;i++)
	{
		flag=thread_join(threads[i],(void *)retval);
		if(flag != 0){
            join_flag++;
			// printf("join flag=%d\n",join_flag);

        }
        else{
            printf(WHITE"Thread_Joined with tid:%ld\n"NONE,threads[i]);
        }
	}
	// printf("final join flag=%d\n",join_flag);
	if(join_flag==0)
        printf(SUCCESS" thread_join test passed!\n\n"NONE);
    else
        printf(ERROR"thread_join test failed!\n\n"NONE);
    return;

}


void thread_lock_test(){
    printf(BYELLOW"\nTesting thread_lock unlock functions:\n"NONE);
    mythread_t t1,t2;
    spinlock_init(&sl);
    thread_create(&t1,f1,NULL);
    thread_create(&t2,f2,NULL);
	
	thread_join(t1,NULL);
	thread_join(t2,NULL);
	unsigned long limit = 10000000;
	while(limit){
		limit--;
	}
	flag=0;
    // sleep(4);
	
    printf("c1 = %ld, c2 = %ld\n"NONE,c1,c2);
    printf("c = %ld\n"NONE,c);
    if(c==c1+c2)
        printf(SUCCESS"thread lock test passed\n"NONE);
    else
        printf(WHITE"thread_lock test failed!\n\n"NONE);
    return;
}
struct limits {
	int l, h, res;
};
void *f6(void *arg) {
	struct limits *p = (struct limits *)arg;
	int prod = 1, k;
	for(k = p->l; k <= p->h; k++)
		prod *= k;
	p->res = prod;
	// sleep(2);
	return NULL;
}
void test_factorial()
{
	printf(BYELLOW"\nTesting Factorial functions:\n"NONE);
	mythread_t tid[2];
	// pthread_attr_t pattr[2];
	int j, k, n;
	struct limits lim[2];
	printf("Calculating Factorial for 6\n");
	n = 6;
	lim[0].l = 1;
	lim[0].h = n/2;	
	lim[1].l = n/2 + 1;
	lim[1].h = n;	
	thread_create(&tid[0], f6, &lim[0]);
	thread_create(&tid[1], f6, &lim[1]);
	thread_join(tid[0],NULL);
	thread_join(tid[1],NULL);
	printf("Answer: %d\n", lim[0].res * lim[1].res);
	return;
}

void* f11(void *arg)
{
	printf("Running thread to test Signals\n");
	
}
void* f12(void *arg)
{
	printf("Running thread2 to test Signal after first thread\n");
	
}

void signal_handler()
{
	printf("Signal Handled Successfully\n");
	
}

void testing_signal()
{

	printf(BYELLOW"\nTesting Signal Handling:\n"NONE);
	mythread_t thread[2];
	signal(SIGUSR1,signal_handler);

	thread_create(&thread[0],f11,NULL);
	thread_create(&thread[1],f12,NULL);

	thread_kill(thread[0],SIGUSR1);
	thread_join(thread[0],NULL);
	thread_join(thread[1],NULL);

}



int main() {

	// thread_create_test();
	thread_join_test();
	thread_lock_test();
	test_factorial();
	testing_signal();

	
	return 0;
}

//   This is one test code 










