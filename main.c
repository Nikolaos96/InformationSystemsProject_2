/* main.c */
#include "function.h"
#include "structs.h"




int main(int argc, char *argv[]){
  char *workload_file, *directory;
  int relation_number;
  main_array *array;



  take_arguments(argc, argv, &workload_file, &directory);

  relation_number = create_init_relations(directory, workload_file, &array);









  delete_all_array(&array, relation_number, &directory, &workload_file);
  return 0;
}
