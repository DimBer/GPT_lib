#ifndef REC_DEFS_H_
#define REC_DEFS_H_

#include <stdbool.h>

#define DEBUG true

//Input buffer sizes

#define EDGE_BUFF_SIZE 10000000

#define NODE_BUFF_SIZE 2000000

//Default command line arguments

#define DEFAULT_GRAPH "graphs/HomoSapiens/adj.txt"

#define DEFAULT_OUTFILE "/home/umhadmin/Desktop/EMB_out/node_embeddings.txt"

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
	bool in_place;
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

//Double matrix
typedef struct{
	double** val;
	uint64_tnum_rows;
	uint64_tnum_cols;
} d_mat;

//Double vector
typedef struct{
	double* val;
	uint64_tnum_entries;
} d_vec;


#endif
