#ifndef __FUNCTION__
#define __FUNCTION__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structs.h"
#include "mid_list.h"


void take_arguments(int argc,char *argv[],char **file, char **dir,char **query_file);
int find_relation_number(char *init_file);
int create_init_relations(char *directory, char *workload_file, main_array **array);
void take_relations(char *query,int *tables, int tables_size);

int take_number_of_predicates(char *query);
int take_tokens(char *str);

void take_predicates(q *predicates, int number_of_predicates, char *query);

void malloc_Rr_Ss(relation **Rr1, relation **Rr2, int size);
void make_Rr1_Rr2(main_array **array, int *tables, q *predicates, int number_of_predicates, int jj, relation **Rr1, relation **Rr2, int a);

void make_Rr1_Rr2__2(main_array **array, main_pointer *mid_result, int *tables, q *predicates, int number_of_predicates, int jj, relation **Rr1, relation **Rr2, int a);

void lets_go_for_predicates(main_array **array, int *tables, int relation_number, q *predicates, int number_of_predicates);
void read_queries(char *query_file,main_array **array,int relation_number);

void delete_all_array(main_array **array, int relation_number, char **directory, char **file,char **query_file);


#endif
