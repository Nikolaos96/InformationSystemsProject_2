/* main.c */
#include "function.h"
#include "structs.h"
#define DONE "Done"



int main(int argc, char *argv[]){
  char *workload_file, *directory,*query_file, done[20];
  int relation_number;
  main_array *array;



  take_arguments(argc, argv, &workload_file, &directory,&query_file);

  relation_number = create_init_relations(directory, workload_file, &array);


  int rep = 0;
  do{
      if( !rep ){
	  printf("Give me str. \n");
	  scanf("%s", done);
      }else{
	  printf("False.Give again the str. \n");
	  scanf("%s", done);
      }
      rep++;
  }while(strcmp(DONE, done));

  read_queries(query_file,&array,relation_number);




  delete_all_array(&array, relation_number, &directory, &workload_file,&query_file);
  return 0;
}
