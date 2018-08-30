#ifndef GPT_COMPARE_H_
#define GPT_COMPARE_H_

#include "gpt_defs.h"

void compare_graphs(cmd_args );

//Used for multithreading
typedef struct{ csr_graph graph_a;
                csr_graph graph_b;
                edge_list* edge_rem;
                edge_list* edge_add;
                edge_list* edge_unc;
              } pass_to_thread;


#endif