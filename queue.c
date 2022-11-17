// GRR20203921 Maria Emilia Castello
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

int queue_size (queue_t *queue){
	int i=0;
	if(queue == NULL){
		return 0;
	}
        else{
		queue_t *ultimo = queue->prev;
		do{
			i++;
                	queue=queue->next;
        	}while(queue != ultimo->next);
	}
	return i;
}
void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
	if(queue == NULL)
		printf("\n");
	else{	
		queue_t *ultimo = queue->prev;
		do{
			print_elem(queue);
			queue=queue->next;
			printf(" ");
		}while(queue != ultimo->next);
	printf("\n");}
}
int queue_append (queue_t **queue, queue_t *elem){
	if((elem == NULL) || (elem->prev != NULL || elem->next != NULL))
		return -1;
	if(*queue == NULL){
		(*queue) = elem;
		(*queue)->next = elem;
                (*queue)->prev = elem;
	}
	else{
		(*queue)->prev->next = elem;
		elem->prev=(*queue)->prev;
		(*queue)->prev = elem;
		elem->next = (*queue);
	}
	return 0;
}
int queue_remove (queue_t **queue, queue_t *elem){
	if(((*queue) == NULL) || (elem == NULL) || (queue == NULL)){
		return -1;
	}
	queue_t *ultimo = (*queue)->prev;
	queue_t *elemento;
	queue_t *primeiro=(*queue);	
    	while(primeiro != elem){
        	primeiro=primeiro->next;
        	if (primeiro==(*queue)){
            		return -1;
        	}
    	}
	if((*queue)->prev == (*queue) && elem == (*queue))
		(*queue)=NULL;
	else{
		do{
                	if((*queue) == elem){
				elemento=(*queue);
                		(*queue)->next->prev=elemento->prev;
                		(*queue)->prev->next=elemento->next;
                	}
                	(*queue)=(*queue)->next;
        	}while((*queue) != ultimo->next);
	}
	elem->prev=NULL;
	elem->next=NULL;
	return 0;
}
