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

static edge_list edge_list_init(void);
static void add_edge_to_list(edge_list* , edge );
static inline void edge_list_destroy(edge_list);
static void* find_added_links(void* );
static void* find_rem_links(void* );
static bool is_in_array(uint64_t , uint64_t* , int );


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
    csr_graph graph_b = csr_create( (const uint64_t**) edgelist_b, num_edges_b, args.sorted );

    // Read graph after
	printf("Loading 'after' graph..\n\n");
	uint64_t num_edges_a = 0;
	int64_t** edgelist_a = give_edge_list( args.graph_after, &num_edges_a );
    csr_graph graph_a = csr_create( (const uint64_t**) edgelist_a, num_edges_a, args.sorted );

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