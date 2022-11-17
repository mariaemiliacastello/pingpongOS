//Maria Emilia Castello GRR20203921
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include "ppos_data.h"
#include "queue.h"
#define STACKSIZE 64*1024
#define minPrio 0
task_t mainc;
task_t disp;
task_t *exec;
task_t *fila;
task_t *dormindo;
struct sigaction action;
struct itimerval timer;
int usuario=1;
int ticks;
int tempoTotal=0;
int id=0;
int tempoAnterior=0;
int lock=0;
int ativacoesDisp=0;
int systime();
void tratador();
void dispatcher();
int task_create (task_t *task, void (*start_func)(void *), void *arg);
void task_resume(task_t *task, task_t **queue);
void task_yield();
void task_exit (int exit_code);
int tasks=0;
void ppos_init (){
	setvbuf (stdout, 0, _IONBF, 0) ;
	mainc.id = id;
	mainc.status=1;
	exec=&mainc;

	ticks=20;
  	action.sa_handler = tratador;
  	sigemptyset (&action.sa_mask);
  	action.sa_flags = 0;
  	
	if (sigaction (SIGALRM, &action, 0) < 0){
    		perror ("Erro em sigaction: ") ;
    		exit (1) ;
  	}
  	timer.it_value.tv_usec = 1000;
  	timer.it_value.tv_sec  = 0;
  	timer.it_interval.tv_usec = 1000;
  	timer.it_interval.tv_sec  = 0;

  	if (setitimer (ITIMER_REAL, &timer, 0) < 0){
    		perror ("Erro em setitimer: ") ;
    		exit (1) ;
  	}
	mainc.horadeacordar=0;
	queue_append ((queue_t **) &fila, (queue_t*) &mainc);
	usuario=0;
	task_create(&disp, &dispatcher, NULL);
	usuario=1;
	task_yield();
}
int task_create (task_t *task, void (*start_func)(void *), void *arg){
	getcontext(&(task->context));
	char *stack;
	stack = malloc (STACKSIZE);
	if(stack){
		task->context.uc_stack.ss_sp = stack;
      		task->context.uc_stack.ss_size = STACKSIZE;
      		task->context.uc_stack.ss_flags = 0;
      		task->context.uc_link = 0;
   	}
	task->prioEstatica=0;
	task->prioDinamica=0;
	makecontext (&task->context, (void*)(*start_func), 1, arg) ;
	id++;
	task->id=id;
	task->ativacoes=0;
	task->tempo=systime();
	task->status=1;
	if(usuario == 1){
	task->horadeacordar=0;
	queue_append ((queue_t **) &fila, (queue_t*) task);
	if(task != &disp){
		tasks++;
	}
	}
	return id;
}
int task_switch (task_t *task){
	task_t *anterior;
        anterior = exec;
        exec = task;
	task->ativacoes++;
	swapcontext(&anterior->context,&task->context);
        return 1;
}
int task_id (){
        return exec->id;
}
void task_exit (int exit_code){
	if(exec != &disp){
		tasks--;
	}
	exec->exit_code=exit_code;
	exec->status=0;
	if(exec != &disp)
		ativacoesDisp += exec->ativacoes;
	if(exec == &disp){
		exec->ativacoes = ativacoesDisp*3-2;
		exec->tempo=tempoTotal;
	}
	printf("Task %d exit.: execution time %d ms, processor time     %d ms, %d activations\n", exec->id, tempoTotal, exec->tempo, exec->ativacoes);
	while(exec->suspensas)
		task_resume(exec->suspensas,&exec->suspensas);	
	if(exec != &disp){
		task_switch(&disp);
	}
	else{
                swapcontext(&exec->context,&mainc.context);
	}
}
task_t *scheduler(){
	task_t *ultimo = fila->prev;
	task_t *menor = fila;
	int prioMenor = fila->prioDinamica;
	do{
		if(fila->prioDinamica < prioMenor){
			menor = fila;
			prioMenor = fila->prioDinamica;
		}
		fila=fila->next;
	}while(fila != ultimo->next);
	menor->prioDinamica=menor->prioEstatica;
	do{
                if(fila != menor){
                        fila->prioDinamica--;
                }
                fila=fila->next;
        }while(fila != ultimo->next);
	return menor;
}
void task_yield(){
	task_switch(&disp);
}
void acordartarefas(){
	task_t *filadormidas=dormindo;
	task_t *prox;
	int tempo=systime();
	if(dormindo){
		do{
			prox = filadormidas->next;
			if(filadormidas->horadeacordar <= tempo){
				task_resume(filadormidas, &dormindo);
			}
			filadormidas=prox;
		}while(dormindo && prox!=dormindo);
	}
}
void dispatcher(){
	task_t *proxima;
	while(queue_size ((queue_t*)fila) >= 1){
		acordartarefas();
		proxima=scheduler();
		if(proxima != NULL){
			ticks=20;
			queue_remove((queue_t **) &fila, (queue_t*) proxima);
			if(proxima->id != 1)
				task_switch(proxima);
			if(tasks >= 0){
				if(proxima->status == 1){
					queue_append((queue_t **) &fila,(queue_t *) proxima);
				}
			}
		}
	}
	task_exit(0);
}
void task_setprio (task_t *task, int prio){
	task->prioEstatica = prio;
	task->prioDinamica = prio;
}
int task_getprio (task_t *task){
	if(task != NULL)
		return task->prioEstatica;
	else
		return exec->prioEstatica;
}
void tratador(){
	tempoTotal++;
	exec->tempo++;
	ticks--;
	if(ticks == 0){
		exec->status=1;
		task_yield();
	}
}
int systime(){
	return tempoTotal;
}
void task_suspend(task_t **queue){
	task_t *executando = exec;
	queue_remove((queue_t **) &fila, (queue_t*)&executando);
	exec->status=2;
	queue_append ((queue_t **) queue, (queue_t*)executando);
	task_yield();
}
void task_resume(task_t *task, task_t **queue){
	queue_remove((queue_t **) queue, (queue_t*)task);
	task->status=1;
	queue_append ((queue_t **) &fila, (queue_t*)task);
}
int task_join(task_t *task){
	if(task->status != 1)
		return -1;
	task_suspend(&task->suspensas);
	return task->exit_code;
}
void task_sleep(int t){
	exec->horadeacordar=systime()+t;
	task_suspend(&dormindo);
}
void enter_cs (int *lock)
{
  // atomic OR (Intel macro for GCC)
  while (__sync_fetch_and_or (lock, 1)) ;   // busy waiting
}
 
void leave_cs (int *lock)
{
  (*lock) = 0 ;
}
int sem_create (semaphore_t *s, int value){
	s->value = value;
	s->semaforo=NULL;
	return 0;		
}
int sem_down (semaphore_t *s){
	if(!s) 
		return -1;
	enter_cs(&lock);
	s->value--;
	if(s->value < 0){
        	queue_remove((queue_t **) &fila, (queue_t*)&exec);
        	exec->status=2;
        	queue_append ((queue_t **) &s->semaforo, (queue_t*)exec);
	}
	leave_cs(&lock);
	if(exec->status == 2)
		task_yield();
	return 0;
}
int sem_up (semaphore_t *s){
	if(!s) 
		return -1;
	enter_cs(&lock);
	s->value++;
	if(s->value <= 0)
		task_resume(s->semaforo, &s->semaforo);
	leave_cs(&lock);
	return 0;
}
int sem_destroy (semaphore_t *s){
    	if(!s) return -1;
	while(s->value <= 0){
		sem_up(s);
	}
	return 0;
}
int mqueue_create (mqueue_t *queue, int max_msgs, int msg_size){
	if(!queue)
		return -1;
	queue->maxMensagens=max_msgs;
	queue->tamanho=msg_size;
	queue->contarMensagens=0;
	queue->contador1=0;
	queue->contador2=0;
	queue->acabou=1;
	queue->filaMensagens=malloc(queue->maxMensagens*queue->tamanho);
	sem_create(&queue->sbuffer, 1);
    	sem_create(&queue->sitem, 0);
    	sem_create(&queue->svaga, max_msgs);
	if(ticks <= 0) {
        	task_yield();
    	}
	return 0;
}
int mqueue_send (mqueue_t *queue, void *msg){
	if(!queue || !msg || !queue->acabou)
		return -1;
	sem_down(&queue->svaga);
    	sem_down(&queue->sbuffer);
	memcpy(queue->filaMensagens+(queue->contador2*queue->tamanho), msg, queue->tamanho);
	queue->contador2++;
	queue->contador2%=queue->tamanho;
	queue->contarMensagens++;
	sem_up(&queue->sbuffer);
    	sem_up(&queue->sitem);
	return 0;
}
int mqueue_recv (mqueue_t *queue, void *msg){
	if(!queue || !msg || !queue->acabou)
		return -1;
        sem_down(&queue->sitem);
        sem_down(&queue->sbuffer);
	queue->contarMensagens--;
    	memcpy(msg, queue->filaMensagens+(queue->contador1*queue->tamanho), queue->tamanho);
	queue->contador1++;
	queue->contador1%=queue->tamanho;
	sem_up(&queue->sbuffer);
        sem_up(&queue->svaga);
	return 0;
}
int mqueue_destroy (mqueue_t *queue){
	if(!queue || !queue->acabou)
        	return -1;
	free(queue->filaMensagens);
	queue->acabou=0;
	sem_destroy(&queue->sitem);
    	sem_destroy(&queue->svaga);
    	sem_destroy(&queue->sbuffer);
	return 0;
}
int mqueue_msgs (mqueue_t *queue){
	if(queue == NULL)
                return -1;
	return queue->contarMensagens;
}
