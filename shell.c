#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <signal.h>

#include "jobs.h"
#include "data.h"
#include "bg.h"
#include "fg.h"
#include "cd.h"

#define MAXARGS 128
#define MAXLINE 256

extern char **environ;


int builtin_command(char **argv)
{
	if (!strcmp(argv[0], "quit"))
	{
		limpa_listas();

		exit(0);
	}

	if (!strcmp(argv[0], "&"))
		return 1;

	if (!strcmp(argv[0], "cd"))
		return cd(argv[1]);

	if(!strcmp(argv[0], "fg"))
		return parse_fg(argv[1]);

	if(!strcmp(argv[0], "bg"))
		return parse_bg(argv);

	if(!strcmp(argv[0], "jobs"))
	{
		jobs(argv);
		
		return 1;
	}

	return 0;
}

int remove_espacos_entre_aspas(char *buf)
{
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
				return 0;
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
				return 0;

		}

		buf++;
	}
	return 1;
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

	if (!remove_espacos_entre_aspas(buf))
		return -1;

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
		{
			wait(NULL);
			fflush(stdout);
		}
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
		else
		{
			job_t *novo = adiciona_job(pid, argv[0]);
			

			if (!bg)
				inicializa_processo_fg(&novo);

			else
				inicializa_processo_bg(&novo, argv[0]);
		}

	}
	return;


}

int main()
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	char cmdline[MAXLINE];

	inicializa_listas();

	while (1)
	{
		printf("mabshell> ");

		fgets(cmdline, MAXLINE, stdin);

		if (feof(stdin))
			exit(0);

		eval(cmdline);
	}
	
	limpa_listas();
}