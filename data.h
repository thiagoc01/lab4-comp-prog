#ifndef DATA_H
#define DATA_H

#include <wait.h>

typedef struct job // Implementado como lista duplamente encadadeada circular
{
	pid_t pid;
	int jid;
	short int status; // 0 = Parado ; 1 = Primeiro plano ; 2 = Segundo plano
	char *nome;
	struct job *anterior;
	struct job *proximo;

}job_t;

extern job_t *processos;
extern job_t *processos_bg;
extern int tamanho_bg;



void inicializa_listas();

void limpa_listas();

job_t *acha_processo(int jid);


job_t *acha_processo_bg(int jid);

job_t *acha_processo_bg_overload(int jid, int *posicao);


void insere_bg(job_t *novo);

void remove_job(int jid);

void remove_bg(int jid);

job_t *adiciona_job(pid_t pid, char *nome);


void remove_caractere(char *buf, char c);

#endif