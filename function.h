[B#ifndef __FUNCTION__
#define __FUNCTION__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structs.h"
//#include "lista.h"

void take_arguments(int argc,char *argv[],char **file, char **dir,char **query_file);
int find_relation_number(char *init_file);
int create_init_relations(char *directory, char *workload_file, main_array **array);
void take_relations(char *query,int *tables, int tables_size);

int take_number_of_predicates(char *query);

void take_predicates(q *predicates, int number_of_predicates, char *query);
void lets_go_for_predicates(main_array **array, int *tables, int relation_number, q *predicates, int number_of_predicates);
void read_queries(char *query_file,main_array **array,int relation_number);
void delete_all_array(main_array **array, int relation_number, char **directory, char **file,char **query_file);


#endif
