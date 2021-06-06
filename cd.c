#include "cd.h"
#include "data.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int cd(char *caminho)
{
	if (caminho != NULL)
	{
		if (strchr(caminho, 39) || strchr(caminho, 34))
		{
			remove_caractere(caminho, 39);
			remove_caractere(caminho, 34);
		}
		int sucesso = chdir(caminho);

		if (sucesso == -1)
			printf("cd: %s: Arquivo ou diretório não existente.\n", caminho);

		return 1;
	}

	chdir(getenv("HOME"));
	return 1;
}