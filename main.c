/* main.c */
#include "function.h"
#include "structs.h"

int main(int argc, char *argv[]){
  char *workload_file;
  char *directory;
  main_array *array;

  take_arguments(argc, argv,&workload_file,&directory);

  printf("filename=%s \n",workload_file);
  printf("dir=%s \n",directory);
  create_init_relations(directory,workload_file,&array);


  delete_all_array(&directory,&workload_file);
}
