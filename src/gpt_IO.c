////////////////////////////////////////////////////////////////////////////////////////////////

/*
 Contains high-level routines for parsing the command line for arguments,
 and handling multiclass or multilabel input.


 Dimitris Berberidis
 University of Minnesota 2017-2018
*/

///////////////////////////////////////////////////////////////////////////////////////////////


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <stdbool.h>

#include "gpt_IO.h"
#include "gpt_defs.h"

static uint64_t read_adjacency_to_buffer(int64_t** , FILE* );
static int file_isreg(char* );
static uint64_t edge_list_to_csr(const uint64_t**, double*, uint64_t*,
			                     uint64_t*, uint64_t, uint64_t* , uint64_t*, bool);
static int compare ( const void*, const void*);

//Parsing command line arguments with getopt_long_only
void parse_commandline_args(int argc,char** argv , cmd_args* args){

	//set default arguments
	(*args) = (cmd_args) {.graph_file = DEFAULT_GRAPH,
						  .outfile = DEFAULT_OUTFILE,
						  .in_place = DEFAULT_IN_PLACE,
						  .graph_before = DEFAULT_BEFORE,
						  .graph_after = DEFAULT_AFTER,
						  .cmp_out = DEFAULT_CMP_OUT,
						  .sorted = DEFAULT_SORTED,
						  .rnd_walk_out = DEFAULT_RND_WALK_OUT,
						  .seed_file = DEFAULT_SEED_FILE,
						  .walk_len = DEFAULT_WALK_LEN };

	int opt= 0;
	//Specifying the expected options
	//The two options l and b expect numbers as argument
	static struct option long_options[] = {
		{"graph_file",    required_argument, 0,  'a' },
		{"outfile", required_argument, 0 , 'b' },
		{"in_place", no_argument, 0 , 'c' },
		{"enum_nodes", no_argument, 0, 'd' },
		{"add_one", no_argument, 0, 'e'  },
		{"compare", no_argument, 0, 'f'  },
		{"graph_before",required_argument, 0, 'g'},
		{"graph_after",required_argument, 0, 'h'},
		{"cmp_out",required_argument, 0, 'i'},
		{"sorted", no_argument, 0, 'k'},
		{"rnd_walk_out",required_argument, 0, 'm'},
		{"seed_file",required_argument, 0, 'n'},			
		{"walk_len",required_argument, 0, 'l'},			
		{"rand_walk", no_argument, 0, 'p'  },
		{0,           0,                 0,  0   }
	};

	int long_index =0;

	while ((opt = getopt_long_only(argc, argv,"",
					long_options, &long_index )) != -1) {
		switch (opt) {
		  case 'a' : args->graph_file = optarg;
				   if(file_isreg(args->graph_file)!=1){
				   	printf("ERROR: %s does not exist\n",args->graph_file);
				   	exit(EXIT_FAILURE);
				   	}
				   break;
		  case 'b' : args->outfile = optarg;
				   break;
		  case 'c' : args->in_place = true;
 				   break;
		  case 'd' : args->which_tool = 0;
				   break;
		  case 'e' : args->which_tool = 1;
 				   break;
		  case 'f' : args->which_tool = 2;
 				   break;		
		  case 'p' : args->which_tool = 3;
 				   break;									
		  case 'g' : args->graph_before = optarg;
				   if(file_isreg(args->graph_before)!=1){
				   	printf("ERROR: %s does not exist\n",args->graph_before);
				   	exit(EXIT_FAILURE);
				   	}
				   break;
		  case 'h' : args->graph_after = optarg;
				   if(file_isreg(args->graph_after)!=1){
				   	printf("ERROR: %s does not exist\n",args->graph_after);
				   	exit(EXIT_FAILURE);
				   	}
				   break;	
		  case 'i' : args->cmp_out = optarg;
				   break;					   			   					
		  case 'k' : args->sorted = true;
				   break;					   			   					
		  case 'm' : args->rnd_walk_out = optarg;
				   break;					   			   					
		  case 'n' : args->seed_file = optarg;
				   break;					   			   								
		  case 'l' : args->walk_len = atoi(optarg);
				   break;					   			   					
 				   exit(EXIT_FAILURE);														
		}
	}

}


//Return edge list and count
int64_t** give_edge_list( char* filename, uint64_t* count ){

	int64_t** buffer= (int64_t**)malloc(EDGE_BUFF_SIZE * sizeof(int64_t*));

	for(int64_t i=0;i<EDGE_BUFF_SIZE;i++)
		buffer[i]= (int64_t*) malloc(2*sizeof( int64_t));

	FILE* file= fopen(filename, "r");

	if(!file) printf("ERROR: Cannot open graph file");

	// Read adjacency into buffer into buffer and return length count=edges
	*count= read_adjacency_to_buffer(buffer,file);
	//printf("Number of edges: %"PRIu64"\n", (uint64_t) *count);

	//print_edge_list( buffer, *count);

	//Free excess memory
	for(uint64_t i=*count+1; i<EDGE_BUFF_SIZE; i++)
	{free(buffer[i]);}
	buffer = realloc(buffer,(*count)*sizeof(int64_t*));

	return buffer;
}

//Read .txt file into buffer
static uint64_t read_adjacency_to_buffer(int64_t** buffer, FILE* file){
  uint64_t count = 0;
	for (; count < EDGE_BUFF_SIZE; ++count)
	{
		int got = fscanf(file, "%"SCNd64"\t%"SCNd64"\n", &buffer[count][0] , &buffer[count][1]);
		if ((got != 2)||( (buffer[count][0]==0) && (buffer[count][1]==0))) break;
		// Stop scanning if wrong number of tokens (maybe end of file) or zero input
	}
	fclose(file);
	return count;
}

//Read file woth random walk seeds
uint64_t* read_seed_file(char* filename, int* num_seeds){
  uint64_t* seeds = (uint64_t*) malloc(NODE_BUFF_SIZE*sizeof(uint64_t));
  int count = 0;
	FILE* file= fopen(filename, "r");
	for (; count < NODE_BUFF_SIZE; ++count)
	{
		int got = fscanf(file, "%"SCNd64, &seeds[count]);
		if (got != 1) break;
		// Stop scanning if wrong number of tokens (maybe end of file) or zero input
	}
	fclose(file);
	seeds = realloc(seeds,count*sizeof(uint64_t));
	*num_seeds = count;
	return seeds;
}

//Check if file is valid
static int file_isreg(char *path) {
    struct stat st;

    if (stat(path, &st) < 0)
        return -1;

    return S_ISREG(st.st_mode);
}


//Writes edgelist to file
void write_edgelist(int64_t** edgelist, uint64_t num_edge, char* filename){
	FILE* file = fopen(filename, "w");
	if(file==NULL) printf("Failure to open outfile.\n");
	for(uint64_t i =0; i<num_edge; i++)
		fprintf(file, "%"PRId64"\t%"PRId64"\n", edgelist[i][0], edgelist[i][1]);
	fclose(file);
}

//Write (dynamic array) edge list to file 
void write_edge_array(char* filename, edge_list list){
	FILE* file = fopen(filename, "w");
	if(file==NULL) printf("Failure to open outfile.\n");
	for(uint64_t i =0; i<list.len; i++)
		fprintf(file, "%"PRId64"\t%"PRId64"\n", list.item[i].node_a +1, list.item[i].node_b +1);
	fclose(file);
	}	


//My comparator for two collumn array. Sorts second col according to first
static int compare ( const void *pa, const void *pb ) 
{
	const uint64_t *a = *(const uint64_t **)pa;
	const uint64_t *b = *(const uint64_t **)pb;
	if(a[0] == b[0])
		return a[1] - b[1];
	else
		return a[0] - b[0];
}  

//Convert directed edgelist into undirected csr_matrix
static uint64_t edge_list_to_csr(const uint64_t** edge, double* csr_value, uint64_t* csr_column,
			  uint64_t* csr_row_pointer, uint64_t len, uint64_t* nnz, uint64_t* degrees, bool sorted){
	//Start bu making a 2D array twice the size where (i,j) exists for every (j,i)
	uint64_t count_nnz;
	uint64_t** edge_temp=(uint64_t **)malloc(len * sizeof(uint64_t *));
	for(uint64_t i=0;i<len;i++)
		edge_temp[i]=(uint64_t*)malloc(2*sizeof(uint64_t));

	for(uint64_t i=0;i<len;i++){
		edge_temp[i][0]= edge[i][0];
		edge_temp[i][1]= edge[i][1];
	}
	//QuickSort buffer_temp with respect to first column (Study and use COMPARATOR function for this)
	if(!sorted) qsort(edge_temp, len, sizeof(edge_temp[0]), compare); 

	//The first collumn of sorted array readily gives csr_row_pointer (just loop through and look for j s.t. x[j]!=x[j-1])
	//Not sure yet but i probably need to define small dynamic subarray with elements of second collumn and
	// sort it before stacking it it into csr_column (A: I dont need to)
	//This can all be done together in one loop over sorted buffer_temp
	csr_row_pointer[0]=0;
	csr_value[0]=1.0;
	csr_column[0]=edge_temp[0][1]-1;
	uint64_t j=1;
	count_nnz=1;
	for(uint64_t i=1;i<len;i++){
			csr_value[count_nnz]=1.0;
			csr_column[count_nnz]=edge_temp[i][1]-1;
			if(edge_temp[i][0]!=edge_temp[i-1][0]){
				csr_row_pointer[j]=count_nnz;
				j++;}
			count_nnz++;
	}
	csr_row_pointer[j]=count_nnz;
	*nnz=count_nnz;
	
	for(uint64_t i=0;i<j;i++){degrees[i]=csr_row_pointer[i+1]-csr_row_pointer[i];}
	
	//Free temporary list
	for(uint64_t i=0;i<len;i++)
	{free(edge_temp[i]);}
	free(edge_temp);
	return j;
}


//Allocate memory and create csr_graph from edgelist input
csr_graph csr_create( const uint64_t** edgelist, uint64_t num_edges, bool sorted ){
	
	csr_graph graph;
	
	//CSR matrix with three arrays, first is basically a dummy for now since networks I have are unweighted. 
	//However they will be weighted as sparse stochastic matrices so values will be needed
	graph.csr_value=(double*)malloc(2*num_edges*sizeof(double));

	graph.csr_column=(uint64_t*)malloc(2*num_edges*sizeof(uint64_t));
	
	graph.csr_row_pointer=(uint64_t*)malloc(2*num_edges*sizeof(uint64_t));

	graph.degrees=(uint64_t*)malloc(2*num_edges*sizeof(uint64_t));

	//Convert undirected edge list to CSR format and return graph size
	graph.num_nodes = edge_list_to_csr(edgelist, graph.csr_value, graph.csr_column, graph.csr_row_pointer,
					   num_edges, &graph.nnz, graph.degrees, sorted); 
	
	graph.csr_row_pointer = realloc(graph.csr_row_pointer, (graph.num_nodes+1)*sizeof(uint64_t));
	graph.csr_value = realloc(graph.csr_value, graph.nnz*sizeof(double));
	graph.csr_column = realloc(graph.csr_column, graph.nnz*sizeof(uint64_t));
	graph.degrees = realloc(graph.degrees, graph.num_nodes*sizeof(uint64_t));
			
	return graph;
	
}

// Free memory allocated to csr_graph
void csr_destroy( csr_graph graph ){
	free(graph.csr_value);
	free(graph.csr_column);
	free(graph.csr_row_pointer);
	free(graph.degrees);
}	