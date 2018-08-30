#ifndef GPT_DEFS_H_
#define GPT_DEFS_H_

#include <stdbool.h>

#define DEBUG true

//Input buffer sizes

#define EDGE_BUFF_SIZE 10000000

#define NODE_BUFF_SIZE 2000000

#define BUFF_INIT 10000

#define BUFF_EXT_BLOCK 5000


//Default command line arguments

#define DEFAULT_GRAPH "graphs/CA-CondMat.txt"

#define DEFAULT_OUTFILE "out/adj_enum.txt"

#define DEFAULT_BEFORE "graphs/vk2016.txt"

#define DEFAULT_AFTER "graphs/vk2017.txt"

#define DEFAULT_CMP_OUT "cmp_out"

#define NUM_TOOLS 3

#define DEFAULT_IN_PLACE false

//DATA STRUCTURES

//Double and index struct for sorting and keeping indexes

typedef struct{
	double val;
	int ind;
} val_and_ind;

//struct forcommand line arguments
typedef struct{
	char* graph_file;
	char* outfile;
	char* graph_before;
	char* graph_after;
	char* cmp_out;
	bool in_place;
	uint8_t which_tool;
} cmd_args;


//Csr graph struct
typedef struct{
	double* csr_value;
	uint64_t* csr_column;
	uint64_t* csr_row_pointer;
	uint64_t num_nodes;
	uint64_t nnz;
	uint64_t* degrees;
} csr_graph;

//Edge struct as node tuple
typedef struct{
	uint64_t node_a;
	uint64_t node_b;
} edge;

//Edge list as dynamic array of edge structs
typedef struct{
	edge* item;
	uint64_t len;
	uint64_t max_len;
} edge_list;

#endif
