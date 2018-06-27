///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 Contains PDF program that implements personalized diffusions for recommendations.

 Dimitris Berberidis
 University of Minnesota 2017-2018
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <stdbool.h>

#include "gpt_IO.h"
#include "gpt_defs.h"
#include "uthash.h"


// Node struct that will populate the hash table
struct node {
    int64_t id;                /* key */
    uint64_t index;
    UT_hash_handle hh;         /* makes this structure hashable */
};

//Hash table is initialized to be empty
struct node *nodes = NULL;

// Finds a node using its id as key..
// Returns NULL if node is not found
struct node* find_node(int64_t node_id) {
    struct node *v;

    HASH_FIND_INT( nodes, &node_id, v );  /* v: output pointer */
    return v;
}

// Adds new entry to hash table
void add_node(int64_t node_id, uint64_t node_index ) {
		struct node *v;

		v = malloc(sizeof(struct node));
		v->id = node_id;
		v->index = node_index;
		HASH_ADD_INT( nodes, id, v );  /* id: name of key field */
}

//Frees all structs and deletes hash
void delete_all() {
  struct node *current_node, *tmp;

  HASH_ITER(hh, nodes, current_node, tmp) {
    HASH_DEL(nodes, current_node);  /* delete it (users advances to next) */
    free(current_node);             /* free it */
  }
}

// Enumerate all nodes of graph
// Takes as input an edgelist containing unique node ids
void enumerate_nodes(int64_t** edgelist, uint64_t num_nodes){

	uint64_t last_index = 0;

	for(uint64_t i=0; i<num_nodes; i++){
		for(uint64_t j=0; j<2; j++){

			int64_t node_id = edgelist[i][j];

			//Check if this node has already been recorded
			struct node* current_node = find_node(node_id);
			bool not_found = ( current_node  == NULL ) ;

			if(not_found){
				last_index ++ ;
				add_node(node_id, last_index);
				edgelist[i][j] = last_index;
			}else{
				edgelist[i][j] = current_node->index;
			}

		}
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//MAIN
int main( int argc, char **argv )
{
	//Parse arguments using argument parser
	cmd_args args;
	parse_commandline_args(argc, argv, &args);

	// Read rating matrix
	printf("Loading graph..\n\n");
	uint64_t num_edges = 0;
  int64_t** edgelist = give_edge_list( args.graph_file, &num_edges );

	//Enumerate nodes of edgelist in_place using uthash
	printf("Enumerating nodes..\n\n");
	enumerate_nodes( edgelist, num_edges );

	//Write resulting edgelist to file
	printf("Writing edgelist to file..\n\n");
	if(args.in_place) args.outfile = args.graph_file;
	write_edgelist(edgelist, num_edges, args.outfile);

  //Free memory
	delete_all();
	for(uint64_t i=0; i<num_edges; i++) free(edgelist[i]);
	free(edgelist);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
