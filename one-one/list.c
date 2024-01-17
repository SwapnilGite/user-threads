#include "u_thread.h"
#include "list.h"




void initLL(threadLL *l)
{	
	*l = NULL; //initializing empty LL
	return;
}

void insert(threadLL *list,mthread* t)
{	
	node *l = *list;
	node *prev = l;
	node *temp = (node *)malloc(sizeof(node));
	if(temp == NULL)
	{	
		perror("malloc failed");
		return;
	}
	temp->thread = t;
	temp->next = NULL;
	
	if(l == NULL) //if empty list
	{
		l = temp;
		return;
	}
	while(prev->next!=NULL)
	{	
		prev = prev->next;
	}
	prev->next = temp;
	return;
}

int deleteLL(threadLL *head,mthread_t tid)
{	
    node* temp = *head;
    node* prev = NULL;

    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->thread->kernel_tid == tid) {
        *head = temp->next; // Changed head
        free(temp);         // free old head
        return 1;
    }

    // Search for the key to be deleted, keeping track of the previous node
    while (temp != NULL && temp->thread->kernel_tid == tid) {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL) {
    	perrorl("Thread not present");
        return 0;
    }

    // Unlink the node from the linked list
    prev->next = temp->next;

    free(temp);  // Free memory
    return 1;
	
}

node *get(threadLL *head,mthread_t tid)
{	
	node* temp = *head;
	while(temp)
	{	
		if(temp->thread->kernel_tid == tid)
		{	
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

