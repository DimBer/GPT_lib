#ifndef GPT_IO_H_
#define GPT_IO_H_

#include "gpt_defs.h"

void parse_commandline_args(int ,char**  , cmd_args* );

int64_t** give_edge_list( char* , uint64_t* );

void write_edgelist(int64_t** , uint64_t, char*);

void write_edge_array(char*, edge_list);

csr_graph csr_create( const uint64_t** , uint64_t, bool );

void csr_destroy( csr_graph );

#endif
