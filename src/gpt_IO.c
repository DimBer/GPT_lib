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

//Parsing command line arguments with getopt_long_only
void parse_commandline_args(int argc,char** argv , cmd_args* args){

	//set default arguments
	(*args) = (cmd_args) {.graph_file = DEFAULT_GRAPH,
									  		.outfile = DEFAULT_OUTFILE,
												.in_place = DEFAULT_IN_PLACE };

	int opt= 0;
	//Specifying the expected options
	//The two options l and b expect numbers as argument
	static struct option long_options[] = {
		{"graph_file",    required_argument, 0,  'a' },
		{"outfile", required_argument, 0 , 'b' },
		{"in_place", no_argument, 0 , 'c' },
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
	printf("Number of edges: %"PRIu64"\n", (uint64_t) *count);

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

//Check if file is valid
static int file_isreg(char *path) {
    struct stat st;

    if (stat(path, &st) < 0)
        return -1;

    return S_ISREG(st.st_mode);
}


//Writes edgelist to file
void write_edgelist(int64_t** edgelist, uint64_t num_edge, char* fielname){
	FILE* file = fopen(filename, "w");
	if(file==NULL) printf("Failure to open outfile.\n");
	for(uint64_t i =0; i<num_edge; i++)
		fprintf(file, "%"PRId64"\t%"PRId64"\n", edgelist[i][0], edgelist[i][1]);
	fclose(file);
}
