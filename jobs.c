#include "jobs.h"
#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int parse_args(char **argv, int *campos, int *filtro_processos)
{
	int i = 1;

	while(argv[i] != NULL)
	{
		if (atoi(argv[i]) == 0)
		{
			//se argv nao é número, então é uma flag, logo argv[i][0] sempre será "-"
			//	argv[i][0] = "-" argv[i] = " - l a r \0"
			int j = 1;

			while(argv[i][j] != 0)
			{
				char filtro = argv[i][j];

				if (filtro == 'l')
				{
					 //assinala que todos os campos devem ser impressos
					campos[0] = 1;
					campos[1] = 1;
					campos[2] = 1;
					campos[3] = 1;
				}
				else if (filtro == 'p')
				{
					//assinala que APENAS o campo pid
					campos[0] = 0;
					campos[1] = 1;
					campos[2] = 0;
					campos[3] = 0;
				}
				else if (filtro == 'r')
				{
					int contador = 0;
					job_t * atual = processos->proximo;
					while(atual != processos)
					{
						if(atual->status != 2)
							filtro_processos[contador] = 0;
						else filtro_processos[contador] = 1;
						contador++;
						atual = atual->proximo;
					}
				}
				else if (filtro == 's')
				{
					int contador = 0;
					job_t * atual = processos->proximo;
					while(atual != processos)
					{
						if (atual->status != 0)
							filtro_processos[contador] = 0;
						else filtro_processos[contador] = 1;
						contador++;
						atual = atual->proximo;
					}
				}

				else if (!strcmp(argv[i],"--help"))
				{
					const char *ajuda = "jobs: jobs [-lprs] [ESPEC-JOB ...]\n"
						    "	Exibe status de trabalhos.\n\n"
						    
						    "	Lista os trabalhos ativos. ESPEC-JOB restringe a saída àquele trabalho.\n"
						    "	Não sendo informado qualquer opção, o status de todos os trabalhos\n"
						    "	ativos é exibido.\n\n"

						    "	Opções:\n"
							     "	-l   	lista IDs de processo junto com a informação normal\n"							
							     "	-p		lista apenas IDs de processo"
							     "	-r		restringe a saída apenas a trabalhos em execução\n"
							     "	-s		restringe a saída apenas a trabalhos parados\n"
						   
						    "	Status de saída:\n"
						    "	Retorna sucesso, a menos que uma opção inválida seja fornecida ou\n"
						    "	ocorra um erro.\n";

					printf("%s\n", ajuda);
					return -1;

				}
				else 
				{
						printf("jobs: %s: opção inválida\n"
							"jobs: uso: jobs [-lprs] [ESPEC-JOB ...].\n", argv[i]);

					return -1;
				}
				j++;
			}
			i++;
		}
		else
			break;
	}

	return i;

}


void printa_processo(int campos[], job_t * imprimir)
{
	if(campos[0] == 1)
	{
		printf("[%d]", imprimir->jid);
		//imprimir sinal
		if (acha_processo_bg(imprimir->jid) == processos_bg->anterior)
			printf("+");

		else if (acha_processo_bg(imprimir->jid) == processos_bg->anterior->anterior)
			printf("-");

		else
			printf(" ");
		printf("\t");
	}

	if(campos[1] == 1)
	{
		printf("%d\t", imprimir->pid);
	}

	if(campos[2] == 1)
	{
		if(imprimir->status == 0)
			printf("Parado    \t");
		else
			printf("Executando\t");
	}
	if(campos[3] == 1)
	{
		printf("%s ",imprimir->nome);
		if(imprimir->status == 2 || imprimir->status == 0)
			printf("&");
	}
	printf("\n");
}


void printa_processos_geral(int campos[], int filtro_processos[], int num_processos[])
{
	job_t *atual = processos->proximo;

	int contador = 0;

	while(atual != processos)
	{
		if(filtro_processos[contador] == 1 && num_processos[contador] == 1)
		{
			printa_processo(campos, atual);
		}
		contador++;
		atual = atual->proximo;
	}
}

void printa_processo_especifico(int campos[], int filtro_processos[], job_t *atual, int pos)
{

	if(filtro_processos[pos] == 1)
	{
		printa_processo(campos, atual);
	}

}


void jobs(char * argv[])
{
	int i = 1;

	int campos[] = {1, 0, 1, 1};

	int *num_processos = (int *) calloc(tamanho_bg, sizeof(int));

	int *filtro_processos = (int *) malloc(sizeof(int) * tamanho_bg);

	bool nums = false;

	for (int a = 0 ; a < tamanho_bg ; a++)
		filtro_processos[a] = 1;

	i = parse_args(argv, campos, filtro_processos);

	if (i == -1)
	{
		free(num_processos);
		free(filtro_processos);
		return;
	}

	if (argv[i] != NULL)
	{
		job_t *atual;
		int aux;
		int pos_num_processos;

		while (argv[i] != NULL)
		{
			aux = atoi(argv[i]);
			atual = acha_processo_bg_overload(aux, &pos_num_processos);

			if ( atual == NULL)
				printf("jobs: %d: trabalho não existe.\n", aux);
			else
			{
				nums = true;
				num_processos[pos_num_processos] = 1;
				printa_processo_especifico(campos, filtro_processos, atual, pos_num_processos);

			}

			i++;

		}
		return;
	}

	else if (!nums)
	{
		for (int a = 0 ; a < tamanho_bg ; a++)
		{
			num_processos[a] = 1;
		}

		printa_processos_geral(campos, filtro_processos, num_processos);
	}

	free(num_processos);
	free(filtro_processos);
}