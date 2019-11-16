#ifndef __FUNCTION__
#define __FUNCTION__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structs.h"
//#include "lista.h"

void take_arguments(int argc,char *argv[],char **file,char** dir);
int find_relation_number(char* init_file);
void create_init_relations(char* directory,char *workload_file,main_array **array);
void delete_all_array(char** directory,char** file);


#endif
