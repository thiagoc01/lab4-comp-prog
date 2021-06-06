#ifndef FG_H
#define FG_H

#include "data.h"

int parse_fg(char *argv);

void inicializa_processo_fg(job_t **novo);

void reinicia_processo(job_t *atual);

void fg(int *jid);

#endif