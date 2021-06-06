#include "data.h"

#include <stdlib.h>
#include <string.h>


job_t *processos = NULL;
job_t *processos_bg = NULL;
int tamanho_bg = 0;

void inicializa_listas()
{
	processos = (job_t *) malloc(sizeof(job_t));
	processos_bg = (job_t *) malloc(sizeof(job_t));

	processos->anterior = processos;
	processos->proximo = processos;
	processos_bg->anterior = processos_bg;
	processos_bg->proximo = processos_bg;
	processos_bg->jid = 0;
	processos->jid = 0;
}

void limpa_listas()
{
	while (processos->anterior != processos)
	{
		kill(processos->anterior->pid, SIGKILL);
		free(processos->anterior->nome);
		remove_job(processos->anterior->jid);
	}

	while (processos_bg->anterior != processos_bg)
		remove_bg(processos_bg->anterior->jid);

	free(processos);
	free(processos_bg);


}

job_t *acha_processo(int jid)
{
	job_t *atual = processos->proximo;

	while (atual != processos && atual->jid != jid)
	{
		atual = atual->proximo;
	}

	if (atual->jid != jid)
		return NULL;
	else
		return atual;
}


job_t *acha_processo_bg(int jid)
{
	job_t *atual = processos_bg->proximo;

	while (atual != processos_bg && atual->jid != jid)
	{
		atual = atual->proximo;
	}

	if (atual->jid != jid)
		return NULL;
	else
		return atual;
}

job_t *acha_processo_bg_overload(int jid, int *posicao)
{
	job_t *atual = processos_bg->proximo;
	int pos = 0;

	while (atual != processos_bg && atual->jid != jid)
	{
		pos++;
		atual = atual->proximo;
	}

	if (atual->jid != jid)
	{
		*posicao = -1;
		return NULL;
	}
	else
	{
		*posicao = pos;
		return atual;
	}
}


void insere_bg(job_t *novo)
{
	job_t *copia = (job_t *) malloc(sizeof(job_t));

	memcpy(copia, novo, sizeof(job_t));

	
	copia->anterior = processos_bg->anterior;
	processos_bg->anterior->proximo= copia;			
	copia->proximo = processos_bg;
	processos_bg->anterior = copia;
	tamanho_bg++;
}

void remove_job(int jid)
{
	job_t *excluido = acha_processo(jid);

	if (excluido->jid != jid)
		return;

	excluido->anterior->proximo = excluido->proximo;
	excluido->proximo->anterior = excluido->anterior;
	free(excluido);
	
}

void remove_bg(int jid)
{
	job_t *excluido = acha_processo_bg(jid);
	
	if (excluido->jid != jid)
		return;

	excluido->anterior->proximo = excluido->proximo;
	excluido->proximo->anterior = excluido->anterior;
	free(excluido);
	tamanho_bg--;

}



job_t *adiciona_job(pid_t pid, char *nome)
{	
	job_t *novo = (job_t *) malloc(sizeof(job_t));

	novo->jid = processos->anterior->jid + 1;
	processos->anterior->proximo = novo;
	novo->anterior = processos->anterior;
	

	novo->pid = pid;
	novo->nome = (char *) malloc(sizeof(char)*strlen(nome));
	strcpy(novo->nome, nome);
	novo->proximo = processos;
	processos->anterior = novo;

	return novo;
}



void remove_caractere(char *buf, char c)
{
	int i,j;
    
    for (i = 0, j = 0 ; *(buf + j) != 0 ; j++) 
        if (*(buf+j) != c) 
            *(buf + i++) = *(buf+j); 
    *(buf+i) = 0;

}