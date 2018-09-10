//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>

#include "gpt_IO.h"
#include "gpt_defs.h"
#include "gpt_rand_walk.h"


//Node structure for adjacency linked-list representation
typedef struct node{
	uint64_t ind;
	uint64_t degree; 
	struct node** adj_node;
}node;


void rand_walk(cmd_args args ){
    // Read graph
	printf("Loading graph..\n\n");
	uint64_t num_edges = 0;
	int64_t** edgelist = give_edge_list( args.graph_file, &num_edges );

    // Read seeds
    int num_seeds=0;
    uint64_t* seeds = read_seed_file(args.seed_file, &num_seeds);

    for(int i=0;i<num_seeds;i++) printf("%"PRIu64"\n",seeds[i]);

	//Free memory
	for(uint64_t i=0; i<num_edges; i++) free(edgelist[i]);
	free(edgelist);
    free(seeds);    
}