#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#define MAXARGS 128
#define MAXLINE 256

typedef struct job // Implementado como lista duplamente encadadeada
{
	pid_t pid;
	int jid;
	short int status; // 0 = Parado ; 1 = Primeiro plano ; 2 = Segundo plano
	char *nome;
	struct job *anterior;
	struct job *proximo;

}job_t;

job_t *processos = NULL;
job_t *processos_bg = NULL;

job_t *acha_processo(int jid)
{
	job_t *atual = processos;

	if (atual->proximo == processos)
		return atual;

	atual = atual->proximo;

	while (atual != processos)
	{
		if (atual->jid == jid)
			return atual;
		atual = atual->proximo;
		
	}

	return NULL;
}

job_t *acha_processo_bg(int jid)
{
	job_t *atual = processos_bg;

	if (atual->proximo == processos_bg)
		return atual;

	atual = atual->proximo;

	while (atual != processos_bg)
	{
		if (atual->jid == jid)
			return atual;

		atual = atual->proximo;
	}

	return NULL;
}

void insere_bg(job_t *novo)
{
	job_t *copia = (job_t *) malloc(sizeof(job_t));
	*copia = *novo;

	if (processos_bg == NULL)
	{
		processos_bg = copia;
		processos_bg->jid = 1;
		processos_bg->anterior = processos_bg;
		processos_bg->proximo = processos_bg;
		return;
	}

	else
	{
		copia->anterior = processos_bg->anterior;
		processos_bg->anterior->proximo= copia;			
		copia->proximo = processos_bg;
		processos_bg->anterior = copia;
	}
}

void remove_job(int jid)
{
	job_t *excluido = acha_processo(jid);
	printf("%p\n",excluido);
	
	if (excluido != NULL)
	{
		if (excluido->anterior == excluido)
		{
			free(processos);
			processos = NULL;
		}

		else
		{
			excluido->anterior->proximo = excluido->proximo;
			excluido->proximo->anterior = excluido->anterior;
		}
	}

	
}

void remove_bg(int jid)
{
	job_t *excluido = acha_processo_bg(jid);
	printf("%p\n",excluido);
	
	if (excluido != NULL)
	{
		if (excluido->anterior == excluido)
		{
			puts("t");
			free(processos_bg);
			processos_bg = NULL;
		}

		else
		{
			excluido->anterior->proximo = excluido->proximo;
			excluido->proximo->anterior = excluido->anterior;
		}
	}
}



job_t *adiciona_job(pid_t pid, char *nome)
{	
	job_t *novo = (job_t *) malloc(sizeof(job_t));

	if (processos == NULL)
	{
		processos = novo;
		processos->jid = 1;
		processos->proximo = processos;
		processos->anterior = processos;
		
	}

	else
	{
		novo->jid = processos->anterior->jid + 1;
		processos->anterior->proximo = novo;
		novo->anterior = processos->anterior;
	}

	novo->pid = pid;
	novo->nome = (char *) malloc(sizeof(char)*strlen(nome));
	strcpy(novo->nome, nome);
	novo->proximo = processos;
	processos->anterior = novo;

	job_t *atual = processos;

	do
	{
		printf("%d %d %d %s\n", atual->pid, atual->jid, atual->status, atual->nome);
		atual = atual->proximo;

	}while (atual != processos);
	

	return novo;
}



void bg(int *jid)
{
	if (processos == NULL)
	{
		printf("bg: atual: trabalho não existe.\n");
		return;
	}

	if (jid == NULL)
	{
		job_t *maior = processos->anterior;

		if (maior->status == 2)
		{
			printf("bg: o trabalho %d já está em plano de fundo.\n", maior->jid);
			return;
		}

		maior->status = 2;

		printf("[%d]+ %s\n", maior->jid, maior->nome);

		return;

	}

	job_t *processo = acha_processo(*jid);

	if (processo == NULL)
	{
		printf("bg: %d: trabalho não existe.\n", *jid);
		return;
	}


	





}

void remove_aspas_simples(char *buf)
{
	int i,j;
    
    for (i = 0, j = 0 ; *(buf + j) != 0 ; j++) 
        if (*(buf+j) != 39 && *(buf + j) != 34) 
            *(buf + i++) = *(buf+j); 
    *(buf+i) = 0; 
}

void remove_aspas_duplas(char *buf)
{
	int i,j;
    
    for (i = 0, j = 0 ; *(buf + j) != 0 ; j++) 
        if (*(buf + j) != 34) 
            *(buf + i++) = *(buf+j); 
    *(buf+i) = 0; 
}


int builtin_command(char **argv)
{
	if (!strcmp(argv[0], "quit"))
		exit(0);
	if (!strcmp(argv[0], "&"))
		return 1;
	if (!strcmp(argv[0], "cd"))
	{
		if (argv[1] != NULL)
		{
			if (strchr(argv[1], 39) || strchr(argv[1], 34))
			{
				remove_aspas_simples(argv[1]);
				remove_aspas_duplas(argv[1]);
			}
			int sucesso = chdir(argv[1]);

			if (sucesso == -1)
				printf("cd: %s: Arquivo ou diretório não existente.\n", argv[1]);

			return 1;
		}
		else
		{
			chdir(getenv("HOME"));
			return 1;
		}
	}
	if(!strcmp(argv[0], "fg"))
	{

		if (processos_bg != NULL)
		{
			kill(processos_bg->anterior->pid, SIGCONT);
			waitpid(processos_bg->anterior->pid, NULL, WUNTRACED);
			
			remove_job(processos_bg->anterior->jid);
			remove_bg(processos_bg->anterior->jid);
			

		}
		else
		{
			puts("erro");
		}
		return 1;
	}
	return 0;
}

int parseline(char *buf, char **argv)
{
	char *delim;
	int argc;
	int bg;
	char *antigo;

	buf[strlen(buf)-1] = ' ';

	while (*buf && (*buf == ' '))
		buf++;

	argc = 0;

	antigo = buf;

	while (*buf)
	{
		if (*buf == 39)
		{
			buf++;
			while (*buf && *buf != 39)
			{
				if (*buf == ' ')
					*buf = '\n';
				buf++;
			}
			if (!(*buf))
			{
				return -1;
			}
		}
		else if (*buf == 34)
		{
			buf++;
			while (*buf && *buf != 34)
			{
				if (*buf == ' ')
					*buf = '\n';
				buf++;
			}
			if (!(*buf))
			{

				return -1;
			}

		}

		buf++;
	}

	buf = antigo;

	while ((delim = strchr(buf, ' ')))
	{
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' '))
			buf++;
	}

	for (int i = 0 ; argv[i] != NULL ; i++)
	{
		while ((delim = strchr(argv[i], '\n')))
		{
			*delim = ' ';
		}
	}

	argv[argc] = NULL;

	if (argc == 0)
		return 1;
	if ((bg = (*argv[argc-1] == '&')) != 0)
		argv[--argc] = NULL;

	return bg;
}

void eval(char *cmdline)
{
	char *argv[MAXARGS];
	char buf[MAXLINE];

	int bg;
	pid_t pid;
	sigset_t mask;

	strcpy(buf, cmdline);

	bg = parseline(buf,argv);

	if (bg == -1)
	{
		if ((pid = fork()) == 0)
		{
			printf("Argumentos inválidos.\n");
			exit(1);
		}
		else
			wait(NULL);
	}

	if (argv[0] == NULL)
		return;

	if (!builtin_command(argv))
	{
		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);
		sigprocmask(SIG_BLOCK, &mask, NULL);

		if ((pid = fork()) == 0)
		{
			sigprocmask(SIG_UNBLOCK, &mask, NULL);
			if (execve(argv[0],argv, NULL) < 0)
			{
				printf("%s : Comando não encontrado. \n", argv[0]);
				exit(1);
			}
		}
			job_t *novo = adiciona_job(pid, argv[0]);
			

			if (!bg)
			{
				novo->status = 1;

				int status;

				if (waitpid(pid, &status, 0) < 0)
					return;

				remove_job(novo->jid);

			}
			else
			{
				novo->status = 2;
				insere_bg(novo);
				kill(novo->pid, SIGSTOP);
			}
			job_t *atual = processos_bg;
			do
			{
				printf("%d %d %d %s\n", atual->pid, atual->jid, atual->status, atual->nome);
				atual = atual->proximo;

			}while (atual != processos_bg);

	}
	return;


}



int main()
{
	char cmdline[MAXLINE];
	while (1)
	{

		printf("mabshell> ");
		fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin))
			exit(0);

		eval(cmdline);
	}
}