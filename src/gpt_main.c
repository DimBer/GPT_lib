///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 Contains usefull tools for graph preprocessing.

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
#include "gpt_compare.h"
#include "gpt_enum.h"
#include "gpt_rand_walk.h"

//Array of pointers to possible tools
void (*tools[NUM_TOOLS])() = { enumerate_nodes,
                               add_one,
							   compare_graphs,
							   rand_walk } ;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//MAIN
int main( int argc, char **argv )
{
	//Parse arguments using argument parser
	cmd_args args;
	parse_commandline_args(argc, argv, &args);

	//Enumerate nodes of edgelist in_place using uthash
	(*tools[args.which_tool])( args );

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
