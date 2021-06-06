#include "jobs.h"
#include "data.h"
#include "fg.h"


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

int parse_fg(char *argv)
{
	if (argv == NULL)
		fg(NULL);

	else if (!strcmp(argv, "--help"))
	{
		const char *ajuda = "fg: fg [ESPEC-JOB]\n"
					    "	Move um trabalho para o primeiro plano.\n\n"
					    
					    "	Coloca o trabalho identificado por ESPEC-JOB em primeiro plano,\n"
					    "	tornando o trabalho atual. Se ESPEC-JOB não estiver presente,\n"
					    "	a noção do shell de trabalho atual é usada.\n\n"
					   
					    "	Status de saída:\n"
					    "	Status do comando colocado em primeiro plano ou falha, se ocorrer um erro.\n";

		printf("%s", ajuda);

	}

	else if (!strchr(argv,'%'))
	{
		int process = atoi(argv);

		fg(&process);
	}
	
	else
	{
		remove_caractere(argv,'%');
		int process = atoi(argv);

		fg(&process);

	}
	return 1;
}

void inicializa_processo_fg(job_t **novo)
{
	(*novo)->status = 1;

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	setpgid((*novo)->pid, 0);
	tcsetpgrp(STDIN_FILENO, (*novo)->pid);


	int status;
				
	if (waitpid((*novo)->pid, &status, WUNTRACED) < 0)
		return;



	tcsetpgrp(STDIN_FILENO, getpgrp());


	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);

	if (status == 5247)
	{
		insere_bg(*novo);
		return;
	}

	remove_job((*novo)->jid);
}

void reinicia_processo(job_t *atual)
{
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	printf("%s\n", atual->nome);

	tcsetpgrp(STDIN_FILENO, atual->pid);

	kill(atual->pid, SIGCONT);

	int status;

	waitpid(atual->pid, &status, WUNTRACED);

	tcsetpgrp(STDIN_FILENO, getpgrp());

	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);

	if (status == 5247)
	{
		job_t *processo = acha_processo(atual->jid);

		processo->status = 0;

		job_t *contexto = (job_t *) malloc(sizeof(job_t));

		memcpy(contexto, processo, sizeof(job_t));

		contexto->status = 0;

		int remover = contexto->jid;

		remove_bg(remover);
		insere_bg(contexto);	
			
		return;
	}

	else
	{
		remove_job(atual->jid);
		remove_bg(atual->jid);
	}
}

void fg(int *jid)
{
	if (processos_bg->proximo == processos_bg)
	{
		if (jid == NULL)
		{
			printf("fg: atual: trabalho não existe.\n");
			return;
		}

		printf("fg: %d: trabalho não existe.\n", *jid);

		return;
	}

	if (jid == NULL)
		reinicia_processo(processos_bg->anterior);

	else
	{
		job_t *atual = acha_processo_bg(*jid);

		if (atual == NULL)
		{
			printf("fg: %d: trabalho não existe.\n", *jid);
			return;
		}

		reinicia_processo(atual);

	}
	
}