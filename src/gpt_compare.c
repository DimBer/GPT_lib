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
#include "uthash.h"
#include "gpt_compare.h"

static csr_graph csr_create( const uint64_t** , uint64_t );
static uint64_t edge_list_to_csr(const uint64_t**, double*, uint64_t*,
			                     uint64_t*, uint64_t, uint64_t* , uint64_t*);
static int compare ( const void*, const void*);
static edge_list edge_list_init(void);
static void add_edge_to_list(edge_list* , edge );
static inline void edge_list_destroy(edge_list);
static void* find_added_links(void* );
static void* find_rem_links(void* );
static bool is_in_array(uint64_t , uint64_t* , int );

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
			  uint64_t* csr_row_pointer, uint64_t len, uint64_t* nnz, uint64_t* degrees){
	//Start bu making a 2D array twice the size where (i,j) exists for every (j,i)
	uint64_t count_nnz;
	uint64_t** edge_temp=(uint64_t **)malloc(2*len * sizeof(uint64_t *));
	for(uint64_t i=0;i<2*len;i++)
		edge_temp[i]=(uint64_t*)malloc(2*sizeof(uint64_t));

	for(uint64_t i=0;i<len;i++){
		edge_temp[i][0]= edge[i][0];
		edge_temp[i][1]= edge[i][1];
		edge_temp[i+len][1]= edge[i][0];
		edge_temp[i+len][0]= edge[i][1];
	}
	//QuickSort buffer_temp with respect to first column (Study and use COMPARATOR function for this)
	qsort(edge_temp, 2*len, sizeof(edge_temp[0]), compare); 

	//The first collumn of sorted array readily gives csr_row_pointer (just loop through and look for j s.t. x[j]!=x[j-1])
	//Not sure yet but i probably need to define small dynamic subarray with elements of second collumn and
	// sort it before stacking it it into csr_column (A: I dont need to)
	//This can all be done together in one loop over sorted buffer_temp
	csr_row_pointer[0]=0;
	csr_value[0]=1.0;
	csr_column[0]=edge_temp[0][1]-1;
	uint64_t j=1;
	count_nnz=1;
	for(uint64_t i=1;i<2*len;i++){
		if(!(edge_temp[i-1][0]==edge_temp[i][0] && edge_temp[i-1][1]==edge_temp[i][1])){
			csr_value[count_nnz]=1.0;
			csr_column[count_nnz]=edge_temp[i][1]-1;
			if(edge_temp[i][0]!=edge_temp[i-1][0]){
				csr_row_pointer[j]=count_nnz;
				j++;}
			count_nnz++;
		}
	}
	csr_row_pointer[j]=count_nnz;
	*nnz=count_nnz;
	
	for(uint64_t i=0;i<j;i++){degrees[i]=csr_row_pointer[i+1]-csr_row_pointer[i];}
	
	//Free temporary list
	for(uint64_t i=0;i<2*len;i++)
	{free(edge_temp[i]);}
	free(edge_temp);
	return j;
}


//Allocate memory and create csr_graph from edgelist input
static csr_graph csr_create( const uint64_t** edgelist, uint64_t num_edges ){
	
	csr_graph graph;
	
	//CSR matrix with three arrays, first is basically a dummy for now since networks I have are unweighted. 
	//However they will be weighted as sparse stochastic matrices so values will be needed
	graph.csr_value=(double*)malloc(2*num_edges*sizeof(double));

	graph.csr_column=(uint64_t*)malloc(2*num_edges*sizeof(uint64_t));
	
	graph.csr_row_pointer=(uint64_t*)malloc(2*num_edges*sizeof(uint64_t));

	graph.degrees=(uint64_t*)malloc(2*num_edges*sizeof(uint64_t));

	//Convert undirected edge list to CSR format and return graph size
	graph.num_nodes = edge_list_to_csr(edgelist, graph.csr_value, graph.csr_column, graph.csr_row_pointer,
					   num_edges, &graph.nnz, graph.degrees); 
	
	graph.csr_row_pointer = realloc(graph.csr_row_pointer, (graph.num_nodes+1)*sizeof(uint64_t));
	graph.csr_value = realloc(graph.csr_value, graph.nnz*sizeof(double));
	graph.csr_column = realloc(graph.csr_column, graph.nnz*sizeof(uint64_t));
	graph.degrees = realloc(graph.degrees, graph.num_nodes*sizeof(uint64_t));
			
	return graph;
	
}

// Free memory allocated to csr_graph
static void csr_destroy( csr_graph graph ){
	free(graph.csr_value);
	free(graph.csr_column);
	free(graph.csr_row_pointer);
	free(graph.degrees);
}

//Initiallize list of edges (dynamic array)
static edge_list edge_list_init(void){
    edge_list list;
    list.len = 0;
    list.max_len = BUFF_INIT;
    list.item = (edge*)malloc(BUFF_INIT*sizeof(edge)); 
    return list;
}

//Append node to list of nodes
//Extend if out of bound
static void add_edge_to_list(edge_list* list, edge edge_to_add ){

    if(list->len >= list->max_len){ 
        list->item = realloc(list->item,(list->max_len+BUFF_EXT_BLOCK)*sizeof(edge));
        list->max_len += BUFF_EXT_BLOCK;  
    }

    list->item[list->len] = edge_to_add;
    list->len += 1;
}

//Destroy edgelist
static inline void edge_list_destroy(edge_list list){
    free(list.item);
}

//Find added links
static void* find_added_links(void* param){

    pass_to_thread* data = (pass_to_thread*) param;
    csr_graph graph_a = data->graph_a;
    csr_graph graph_b = data->graph_b;
    edge_list* edge_add = data->edge_add;
    uint64_t N = graph_a.num_nodes;

    int one_per_cent = N / 100;

    for(uint64_t i=0;i<N;i++){
        uint64_t len_b = graph_b.degrees[i];
        uint64_t len_a = graph_a.degrees[i];
        uint64_t* list_adj = &graph_b.csr_column[graph_b.csr_row_pointer[i]];
        for(int j=0; j< len_a; j++){
            uint64_t node = graph_a.csr_column[graph_a.csr_row_pointer[i]+j] ;
            if(!is_in_array(node, list_adj,len_b)){
                edge edge_to_add = (edge) { .node_a = i,
                                            .node_b = node}; 
                add_edge_to_list(edge_add, edge_to_add);
            }
        }

        //printf( "%"PRIu64" / %"PRIu64" \n",i,N );
    if( !(i % one_per_cent) )  printf("%d %% complete\n", (int) i/one_per_cent );
    
    }

    printf("\n\n");

    pthread_exit(0);
}

//Find removed and unchanged links
static void* find_rem_links(void* param){

    pass_to_thread* data = (pass_to_thread*) param;
    csr_graph graph_a = data->graph_a;
    csr_graph graph_b = data->graph_b;
    edge_list* edge_rem = data->edge_rem;
    edge_list* edge_unc = data->edge_unc;
    uint64_t N = graph_a.num_nodes;


    for(uint64_t i=N-1; i>=1 ;i--){
        uint64_t len_b = graph_b.degrees[i];
        uint64_t len_a = graph_a.degrees[i];
        uint64_t* list_adj =  &graph_a.csr_column[graph_a.csr_row_pointer[i]];        
        for(int j=0; j< len_b; j++){
            uint64_t node = graph_b.csr_column[graph_b.csr_row_pointer[i]+j];
            edge edge_to_add = (edge) { .node_a = i,
                                        .node_b = node}; 
            if(is_in_array(node, list_adj ,len_a)){
                    add_edge_to_list( edge_unc, edge_to_add );
                }else{
                    add_edge_to_list( edge_rem, edge_to_add );
                } 
        }        
    }

    pthread_exit(0);
}

//check if x is in array
static bool is_in_array(uint64_t x, uint64_t* arr, int len){

    for(int i=0;i<len;i++) 
        if(x == arr[i])
            return true;
    
    return false;
} 

void compare_graphs(cmd_args args ){

    // Read graph before
	printf("Loading 'before' graph..\n\n");
	uint64_t num_edges_b = 0;
	int64_t** edgelist_b = give_edge_list( args.graph_before, &num_edges_b );
    csr_graph graph_b = csr_create( (const uint64_t**) edgelist_b, num_edges_b );

    // Read graph after
	printf("Loading 'after' graph..\n\n");
	uint64_t num_edges_a = 0;
	int64_t** edgelist_a = give_edge_list( args.graph_after, &num_edges_a );
    csr_graph graph_a = csr_create( (const uint64_t**) edgelist_a, num_edges_a );

    if(graph_a.num_nodes!=graph_b.num_nodes) printf("ERROR: Graph size mismatch\n");

    // Comparing graphs
    printf("Comparing graphs..\n\n");
    printf("Edges before: %"PRIu64"\n",graph_b.nnz);
    printf("Edges after: %"PRIu64"\n\n",graph_a.nnz);

    //Allocate edge arrays for storage (will be dynamically extended if needed )
    edge_list edge_rem = edge_list_init();
    edge_list edge_add = edge_list_init();
    edge_list edge_unc = edge_list_init();


    //To save time use two threads (one for added links, and one for removed and unchanged)
    pthread_t thread_add, thread_rem;

    pass_to_thread thread_data = (pass_to_thread) { .graph_a = graph_a,
                                                    .graph_b = graph_b,
                                                    .edge_add = &edge_add,
                                                    .edge_rem = &edge_rem,
                                                    .edge_unc = &edge_unc };           
                   

    //Spawn threads
    //To minimize (read) collisions, find_rem_links() will be accesing
    //the graphs backwards 
    pthread_create(&thread_add,NULL,find_added_links,(void*)&thread_data);
    pthread_create(&thread_rem,NULL,find_rem_links,(void*)&thread_data);

    //Wait for both threads to finish before continuing	
    pthread_join( thread_add, NULL);
    pthread_join( thread_rem, NULL);
    

    //Save results
    char file_add[50];
    strcpy(file_add,args.cmp_out);
    write_edge_array(strcat(file_add, "/added_links.txt"), edge_add);


    char file_rem[50];
    strcpy(file_rem,args.cmp_out);
    write_edge_array(strcat(file_rem, "/removed_links.txt"), edge_rem);
    
    char file_unc[50];
    strcpy(file_unc,args.cmp_out);
    write_edge_array(strcat(file_unc, "/unchanged_links.txt"), edge_unc);

    printf("SUMMARY");
    printf("\n\n");
    printf("Edges added: %"PRIu64"\n",edge_add.len);    
    printf("Edges removed: %"PRIu64"\n",edge_rem.len);    
    printf("Edges unchanged: %"PRIu64"\n",edge_unc.len);    
    printf("\n\n");

    // Free memory
    csr_destroy(graph_a);
    csr_destroy(graph_b); 
    edge_list_destroy(edge_rem);
    edge_list_destroy(edge_add);
    edge_list_destroy(edge_unc);
}