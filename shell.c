#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <setjmp.h>

#define MAXARGS 128
#define MAXLINE 256

extern char **environ;

typedef struct job // Implementado como lista duplamente encadadeada circular
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

void print_listas()
{
	job_t *a = processos->proximo;

	while(a != processos)
	{
		printf("%d %d %d %s \n", a->pid, a->jid, a->status, a->nome);
		a = a->proximo;
	}
	a = processos_bg->proximo;
	while(a != processos_bg)
	{
		printf("%d %d %d %s \n", a->pid, a->jid, a->status, a->nome);
		a = a->proximo;
	}
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


void insere_bg(job_t *novo)
{
	job_t *copia = (job_t *) malloc(sizeof(job_t));
	memcpy(copia, novo, sizeof(job_t));

	
	copia->anterior = processos_bg->anterior;
	processos_bg->anterior->proximo= copia;			
	copia->proximo = processos_bg;
	processos_bg->anterior = copia;
	
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
		job_t *ultimo = processos_bg->anterior;

		ultimo->status = 2;

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

		atual->status = 2;

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
	{
		signal(SIGTTIN, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);

		printf("%s\n", processos_bg->anterior->nome);

		tcsetpgrp(STDIN_FILENO, processos_bg->anterior->pid);

		kill(processos_bg->anterior->pid, SIGCONT);

		int status;

		waitpid(processos_bg->anterior->pid, &status, WUNTRACED);
		tcsetpgrp(STDIN_FILENO, getpgrp());

		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);

		if (status == 5247)
		{
			job_t *contexto = (job_t *) malloc(sizeof(job_t));
			memcpy(contexto,processos_bg->anterior, sizeof(job_t));
			contexto->status = 0;
			int remover = contexto->jid;

			remove_bg(remover);
			insere_bg(contexto);	
			
			return;
		}

		else
		{
			remove_job(processos_bg->anterior->jid);
			remove_bg(processos_bg->anterior->jid);
		}

		
	}

	else
	{
		job_t *atual = acha_processo_bg(*jid);

		if (atual == NULL)
		{
			printf("fg: %d: trabalho não existe.\n", *jid);
			return;
		}

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
			job_t *contexto = (job_t *) malloc(sizeof(job_t));
			memcpy(contexto,atual, sizeof(job_t));

			int remover = atual->jid;

			contexto->status = 0;

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
	


}

void remove_caractere(char *buf, char c)
{
	int i,j;
    
    for (i = 0, j = 0 ; *(buf + j) != 0 ; j++) 
        if (*(buf+j) != c) 
            *(buf + i++) = *(buf+j); 
    *(buf+i) = 0;

}


int builtin_command(char **argv)
{
	if (!strcmp(argv[0], "quit"))
	{
		while (processos->proximo != processos)
		{
			kill(processos->anterior->pid, SIGKILL);
			remove_job(processos->anterior->jid);
		}
		exit(0);
	}
	if (!strcmp(argv[0], "&"))
		return 1;
	if (!strcmp(argv[0], "cd"))
	{
		if (argv[1] != NULL)
		{
			if (strchr(argv[1], 39) || strchr(argv[1], 34))
			{
				remove_caractere(argv[1], 39);
				remove_caractere(argv[1], 34);
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
		if (argv[1] == NULL)
			fg(NULL);

		else if (!strcmp(argv[1], "--help"))
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

		else if (!strchr(argv[1],'%'))
		{
			int process = atoi(argv[1]);

			fg(&process);
		}
		else
		{
			remove_caractere(argv[1],'%');
			int process = atoi(argv[1]);

			fg(&process);

		}
		return 1;
	}
	if(!strcmp(argv[0], "bg"))
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
	if(!strcmp(argv[0], "print"))
	{
		print_listas();
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
		if ((pid = fork()) == 0)
		{
			signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);			
			setpgid(0,0);

			if (execve(argv[0],argv, environ) < 0)
			{
				printf("%s : Comando não encontrado. \n", argv[0]);
				exit(1);
			}
			return;
			

		}
		else{
			job_t *novo = adiciona_job(pid, argv[0]);
			

			if (!bg)
			{
				novo->status = 1;

				signal(SIGTTIN, SIG_IGN);
				signal(SIGTTOU, SIG_IGN);

				setpgid(novo->pid, 0);
				tcsetpgrp(STDIN_FILENO, novo->pid);


				int status;
				
				if (waitpid(novo->pid, &status, WUNTRACED) < 0)
					return;



				tcsetpgrp(STDIN_FILENO, getpgrp());


				signal(SIGTTIN, SIG_DFL);
				signal(SIGTTOU, SIG_DFL);

				if (status == 5247)
				{
					insere_bg(novo);
					return;
				}

				remove_job(novo->jid);

			}
			else
			{
				int teste = open(argv[0], 256);
				

				if (teste == -1)
				{
					remove_job(novo->jid);
					kill(novo->pid, SIGTERM);
					wait(NULL);
					
					
				}
				else
				{

					if (fdopendir(teste) != NULL)
					{
						remove_job(novo->jid);
						kill(novo->pid, SIGTERM);
						wait(NULL);

					}

					else
					{
						novo->status = 2;
						insere_bg(novo);
						setpgid(novo->pid,0);
						tcsetpgrp(STDIN_FILENO,getpgrp());
						printf("[%d] %d\n", novo->jid, novo->pid);

					}	
				}
				
				close(teste);

			}
		}

	}
	return;


}



int main()
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	char cmdline[MAXLINE];

	processos = (job_t *) malloc(sizeof(job_t));
	processos_bg = (job_t *) malloc(sizeof(job_t));

	processos->anterior = processos;
	processos->proximo = processos;
	processos_bg->anterior = processos_bg;
	processos_bg->proximo = processos_bg;
	processos_bg->jid = 0;
	processos->jid = 0;


	while (1)
	{

		printf("mabshell> ");
		fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin))
			exit(0);

		eval(cmdline);
	}
	free(processos);
	free(processos_bg);
}