#include "u_thread.h"

typedef struct node{
	struct mthread thread;
	struct node *next;
}node;

typedef struct node* threadLL;

void initLL(threadLL *list);

void insertLL(threadLL *list,mthread* t);

int deleteLL(threadLL *list,mthread_t tid);

node *get(threadLL *head,mthread_t tid);
