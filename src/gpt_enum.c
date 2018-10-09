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
#include "gpt_enum.h"


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
static struct node* find_node(int64_t node_id) {
    struct node *v;

    HASH_FIND_INT( nodes, &node_id, v );  /* v: output pointer */
    return v;
}

// Adds new entry to hash table
static void add_node(int64_t node_id, uint64_t node_index ) {
		struct node *v;

		v = malloc(sizeof(struct node));
		v->id = node_id;
		v->index = node_index;
		HASH_ADD_INT( nodes, id, v );  /* id: name of key field */
}

//Frees all structs and deletes hash
static void delete_all() {
  struct node *current_node, *tmp;

  HASH_ITER(hh, nodes, current_node, tmp) {
    HASH_DEL(nodes, current_node);  /* delete it (users advances to next) */
    free(current_node);             /* free it */
  }
}


// Enumerate all nodes of graph
// Takes as input an edgelist containing unique node ids
void enumerate_nodes(cmd_args args){

	// Read rating matrix
	printf("Loading graph..\n\n");
	uint64_t num_edges = 0;
	int64_t** edgelist = give_edge_list( args.graph_file, &num_edges );

    printf("Enumerating nodes..\n\n");
	uint64_t last_index = 0;

	int64_t* ind_list = (int64_t*) malloc(num_edges * sizeof(int64_t));

	for(uint64_t i=0; i<num_edges; i++){
		for(uint64_t j=0; j<2; j++){

			int64_t node_id = edgelist[i][j];

			//Check if this node has already been recorded
			struct node* current_node = find_node(node_id);
			bool not_found = ( current_node  == NULL ) ;

			if(not_found){
				last_index ++ ;
				add_node(node_id, last_index);
				edgelist[i][j] = last_index;
				ind_list[last_index-1] = node_id;
			}else{
				edgelist[i][j] = current_node->index;
			}

		}
	}

	ind_list = realloc( ind_list, last_index * sizeof(int64_t) );

	// Write resulting edgelist to file
	printf("Writing edgelist to file..\n\n");
	if(args.in_place) args.outfile = args.graph_file;
	write_edgelist(edgelist, num_edges, args.outfile);

	// Also write ind_list to file
	FILE* file = fopen(args.ind_list, "w");
	for(uint64_t i =0; i<last_index; i++)
		fprintf(file,"%"PRIu64"\n",ind_list[i]);
	fclose(file);

	//Free memory
	delete_all();
	for(uint64_t i=0; i<num_edges; i++) free(edgelist[i]);
	free(edgelist);
	free(ind_list);
}

// +1 to every node in edgelist
void add_one(cmd_args args){

	// Read graph
	printf("Loading graph..\n\n");
	uint64_t num_edges = 0;
	int64_t** edgelist = give_edge_list( args.graph_file, &num_edges );

    printf("Adding one..\n\n");
	for(uint64_t i=0; i<num_edges;i++){
    	edgelist[i][0] +=1;
    	edgelist[i][1] +=1;
 	 }

	//Write resulting edgelist to file
	printf("Writing edgelist to file..\n\n");
	if(args.in_place) args.outfile = args.graph_file;
	write_edgelist(edgelist, num_edges, args.outfile);


	//Free memory
	delete_all();
	for(uint64_t i=0; i<num_edges; i++) free(edgelist[i]);
	free(edgelist);

}