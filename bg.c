#include "jobs.h"
#include "data.h"
#include "bg.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

int parse_bg(char **argv)
{
	if (argv[1] == NULL)
		bg(NULL);

	else if (!strcmp(argv[1], "--help"))
	{
		const char *ajuda = "bg: bg [ESPEC-JOB ...]\n"
					    "	Move trabalhos para o plano de fundo.\n\n"
					    
					    "	Coloca os trabalhos identificados por ESPEC-JOB em plano de fundo,\n"
					    "	como se eles tivessem sido iniciado com `&'. Se ESPEC-JOB não\n"
					    "	estiver presente, a noção do shell de trabalho atual é usada.\n\n"
						   
					    "	Status de saída:\n"
					    "	Retorna sucesso, a menos que controle de trabalho não esteja\n"
					    "	habilitado ou ocorra um erro.\n";

		printf("%s", ajuda);

	}

	else
	{
		int i = 1;
		int process;

		while (argv[i] != NULL)
		{

			if (!strchr(argv[i],'%'))
			{
				process = atoi(argv[i]);

				bg(&process);
			}

			else
			{
				remove_caractere(argv[i],'%');
				process = atoi(argv[i]);

				bg(&process);
			}

			i++;

		}
	}

	return 1;
}

void inicializa_processo_bg(job_t **novo, char *programa)
{
	int teste = open(programa, 256);				

	if (teste == -1)
	{
		remove_job((*novo)->jid);
		kill((*novo)->pid, SIGTERM);
		wait(NULL);					
					
	}
	
	else
	{
		if (fdopendir(teste) != NULL)
		{
			remove_job((*novo)->jid);
			kill((*novo)->pid, SIGTERM);
			wait(NULL);

		}

		else
		{
			(*novo)->status = 2;

			insere_bg(*novo);

			setpgid((*novo)->pid,0);

			tcsetpgrp(STDIN_FILENO, getpgrp());

			printf("[%d] %d\n", (*novo)->jid, (*novo)->pid);

		}	
	}
				
	close(teste);
}

void bg(int *jid)
{
	if (processos->proximo == processos)
	{
		if (jid == NULL)
		{
			printf("bg: atual: trabalho não existe.\n");
			return;
		}

		printf("bg: %d: trabalho não existe.\n", *jid);

		return;
	}

	if (jid == NULL)
	{
		job_t *processo = acha_processo(processos_bg->anterior->jid);
		job_t *ultimo = processos_bg->anterior;

		ultimo->status = 2;
		processo->status = 2;

		kill(ultimo->pid, SIGCONT);

		if (acha_processo_bg(ultimo->jid) == processos_bg->anterior)
			printf("[%d]+ %s &\n", ultimo->jid, ultimo->nome);

		else if (acha_processo_bg(ultimo->jid) == processos_bg->anterior->anterior)
			printf("[%d]- %s &\n", ultimo->jid, ultimo->nome);

		else
			printf("[%d] %s &\n", ultimo->jid, ultimo->nome);

		job_t *contexto = (job_t *) malloc(sizeof(job_t));
		memcpy(contexto, ultimo, sizeof(job_t));

		int removido = ultimo->jid;

		remove_bg(removido);
		insere_bg(contexto);

	}

	else
	{
		job_t *atual = acha_processo_bg(*jid);

		if (atual == NULL)
		{
			printf("bg: %d: trabalho não existe.\n", *jid);
			return;
		}

		job_t *processo = acha_processo(atual->jid);

		atual->status = 2;
		processo->status = 2;

		kill(atual->pid, SIGCONT);

		if (acha_processo_bg(atual->jid) == processos_bg->anterior)
			printf("[%d]+ %s &\n", atual->jid, atual->nome);

		else if (acha_processo_bg(atual->jid) == processos_bg->anterior->anterior)
			printf("[%d]- %s &\n", atual->jid, atual->nome);

		else
			printf("[%d] %s &\n", atual->jid, atual->nome);

		job_t *contexto = (job_t *) malloc(sizeof(job_t));
		memcpy(contexto, atual, sizeof(job_t));

		int removido = atual->jid;

		remove_bg(removido);
		insere_bg(contexto);

	}
	
}