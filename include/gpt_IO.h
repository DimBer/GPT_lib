#ifndef REC_IO_H_
#define REC_IO_H_

#include "emb_defs.h"

void parse_commandline_args(int ,char**  , cmd_args* );

int64_t** give_edge_list( char* , uint64_t* );

void write_edge_list(uint64_t** , uint64_t);

#endif
