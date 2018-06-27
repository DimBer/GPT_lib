#ifndef GPT_IO_H_
#define GPT_IO_H_

#include "gpt_defs.h"

void parse_commandline_args(int ,char**  , cmd_args* );

int64_t** give_edge_list( char* , uint64_t* );

void write_edgelist(int64_t** , uint64_t, char*);

#endif
