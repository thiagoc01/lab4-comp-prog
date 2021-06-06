#ifndef BG_H
#define BG_H

#include "data.h"

int parse_bg(char **argv);

void inicializa_processo_bg(job_t **novo, char *programa);

void bg(int *jid);

#endif