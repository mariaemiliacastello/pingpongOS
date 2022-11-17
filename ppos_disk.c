//Maria Emilia Castello GRR20203921
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include "ppos.h"
#include "queue.h"
#include "disk.h"
#include "ppos_disk.h"
#include "ppos_data.h"
#define STACKSIZE 64*1024
#define minPrio 0

void tratarsinal();
task_t tarefaDisco;
disk_t disco;
struct sigaction action;

void diskDriverBody(void * args){
	disk_request_t *req;
    	while(1){
        	sem_down(&disco.acesso);
        	if(disco.sinal==1){
            		task_resume((task_t *) req->task, (task_t **) &disco.filaespera);
            		queue_remove((queue_t **) &disco.requests, (queue_t *) req);
            		disco.sinal=0;
        	}
        	if(disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE){
        		if(disco.requests){
				req=disco.requests;
            			disk_cmd(req->operacao, req->block, req->buffer);
			}
        	}
        	if(!(disco.requests || disco.filaespera)){
        		queue_remove((queue_t **) &fila, (queue_t *) &tarefaDisco);
        		tarefaDisco.status=2;
        	}
        	else
			tarefaDisco.status=1;
        	sem_up(&disco.acesso);
        	task_yield();
    	}
}
void tratarsinal(){
    	disco.sinal = 1;
}
int disk_mgr_init(int *numBlocks, int *blockSize){
	int tamanho, bloco;
	action.sa_handler=tratarsinal;
    	sigemptyset(&action.sa_mask);
    	action.sa_flags = 0 ;
    	if(sigaction(SIGUSR1, &action, 0) < 0){
        	perror("Erro");
        	exit(1);
    	}
    	if(disk_cmd(DISK_CMD_INIT, 0, 0))
		return -1;
    	sem_create(&disco.acesso, 1);
    	disco.requests=NULL;
    	disco.filaespera=NULL;
    	disco.sinal=0;
    	usuario=0;
    	task_create(&tarefaDisco, diskDriverBody, NULL);
    	usuario=1;
        tarefaDisco.status=2;
	tamanho=disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    	bloco=disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
	*numBlocks=tamanho;
    	*blockSize=bloco;
    	if(tamanho < 0 || bloco < 0)
		return -1;
    	return 0;
}
int disk_block_read(int block, void *buffer){
    	sem_down(&disco.acesso);
	disk_request_t *req=malloc(sizeof(disk_request_t));
        if(!req)
                exit(-1);
        req->prev=NULL;
        req->next=NULL;
        req->block=block;
        req->buffer=buffer;
        req->task=exec;
        req->operacao=DISK_CMD_READ;
    	queue_append((queue_t **) &disco.requests, (queue_t *) req);
    	if(tarefaDisco.status == 2){
    	        tarefaDisco.status=1;
        	queue_append((queue_t **) &fila, (queue_t*)&tarefaDisco);
    	}
    	sem_up(&disco.acesso);
    	task_suspend((task_t **) &disco.filaespera);
    	return 0;
}
int disk_block_write(int block, void *buffer){
	sem_down(&disco.acesso);
    	disk_request_t *req=malloc(sizeof(disk_request_t));
        if(!req)
                exit(-1);
        req->prev=NULL;
        req->next=NULL;
        req->block=block;
        req->buffer=buffer;
        req->task=exec;
        req->operacao=DISK_CMD_WRITE;
	queue_append((queue_t **) &disco.requests, (queue_t *) req);
    	if(tarefaDisco.status == 2){
        	tarefaDisco.status=1;
                queue_append((queue_t **) &fila, (queue_t*)&tarefaDisco);
    	}
    	sem_up(&disco.acesso);
    	task_suspend((task_t **) &disco.filaespera);
    	return 0;
}

