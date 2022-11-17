//Maria Emilia Castello GRR20203921
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

// Estrutura que define um Task Control Block (TCB)

typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  short preemptable ;			// pode ser preemptada?
  int prioEstatica;
  int prioDinamica;
  int ativacoes;
  int tempo;
  int exit_code;
  struct task_t *suspensas;
  int horadeacordar;
  // ... (outros campos serão adicionados mais tarde)
} task_t ;
extern task_t *fila;
extern task_t *exec;
extern int usuario;

// estrutura que define um semáforo
typedef struct
{
  int value;
  struct task_t *semaforo;
  int d;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  struct task_t *filaMensagens;
  int tamanho;
  int maxMensagens;
  int contarMensagens;
  int contador1;
  int contador2;
  int acabou;
  semaphore_t svaga;
  semaphore_t sitem;
  semaphore_t sbuffer;
} mqueue_t ;

#endif
