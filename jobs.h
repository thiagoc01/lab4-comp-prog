#ifndef JOBS_H
#define JOBS_H

#include "data.h"

int parse_args(char **argv, int *campos, int *filtro_processos);

void printa_processo(int campos[], job_t * imprimir);

void printa_processos_geral(int campos[], int filtro_processos[], int num_processos[]);

void printa_processo_especifico(int campos[], int filtro_processos[], job_t *atual, int pos);

void jobs(char * argv[]);

#endif