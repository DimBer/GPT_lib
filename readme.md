__OVERVIEW__

Graph Preprocessing Toolbox (GPT) : Some useful tools for preprocessign graph data 

__CONTAINS__

1. __Add one__ (`--add_one`): Simply adds 1 to indices of nodes that are enumerated from 0 to N-1 (e.g., Enron graph)

2. __Enumerate nodes__ (`--enum_nodes`): Uses dictionary to enumerate N nodes who are defined by arbitrary
IDs (e.g., CA-CondMat graph) with unique numbers between 1 and N. Enumeration happens with order of parsing. 

3. __Compare graphs__  (`--compare`): Compares two (enumerated) graphs that have the same set of nodes. Usefull for 
determining the connectivity changes that occur on time evolving graphs (e.g., vk2016 and vk2017 graphs). Takes as 
input the initial graph (`--graph_before`) and graph after changes have occured (`--graph_after`). Results are stored
in user-defined directory (`--cmp_out`) as 3 .txt files:
    * `cmp_out/added_links.txt`: Contains edges that are in `--graph_after` but __not__ in `--graph_before`. 
    * `cmp_out/removed_links.txt`: Contains edges that are in `--graph_before` but __not__ in `--graph_after`. 
    * `cmp_out/unchanged_links.txt`: Contains edges that are present both in __both__ `--graph_after` and `--graph_before`. 



__INPUT FORMAT__



__ARGUMENTS__

Command line optional arguments with values:

ARGUMENT | VALUES | DEFAULT | DESCRIPTION
-------- | ------ | ------- | -----------
`--graph_file` | - | `CA-cond.txt` | input file
`--outfile` | - | `out/adj_enum.txt` | output file
`--ind_list` | - | `out/ind_list.txt` | strores map of new indexes (1 to N) to old (values) 

Default values can be changed by editing `defs.h`

Command line optional arguments without values:

ARGUMENT | RESULT
-------- | ------
`--in_place` | overwrites output on input file
