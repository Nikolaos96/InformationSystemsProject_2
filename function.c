/* function.c */
#include "function.h"
 /*
   take arguments from command line and init variables
 */
 void take_arguments(int argc,char *argv[],char **file,char **dir){

    char *arg;
    if(argc != 5 ){
        printf("\nError in arguments command line. \n\n");
        exit(1);
    }

    while(--argc){
        arg = *++argv;
        if(!strcmp(arg, "-F")){
	         *file = malloc((strlen(*++argv) + 1) * sizeof(char));
	          strcpy(*file, *argv);
        }else if(!strcmp(arg, "-D")){
          *dir = malloc((strlen(*++argv) + 1) * sizeof(char));
           strcpy(*dir, *argv);
        }
        if(argc > 1) argc--;
     }

     return;
 }

 int find_relation_number(char* init_file){
   int lines=0;
   char ch;
   FILE *f=fopen(init_file,"r");
   if(f==NULL){
     printf("error in opening init file\n");
     exit(1);
   }
   while(!feof(f)){
     ch = fgetc(f);
     if(ch == '\n')
     {
       lines++;
     }
   }
   fclose(f);
   return lines;
 }
/* function to read all the files and save to memory */
 void create_init_relations(char* directory,char *workload_file,main_array **array){
   char init_file[100];
   char r[10];
   strcpy(init_file,directory);
   strcat(init_file,"/");
   strcat(init_file,workload_file);
   printf("path= %s\n",init_file);

   int relation_number=find_relation_number(init_file);
   printf("relation_number= %d\n",relation_number);

   *array=malloc(relation_number * sizeof(main_array));
   if(array==NULL){
     printf("error in array malloc\n");
     exit(1);
   }
   FILE* f=fopen(init_file,"r");
   for(int i=0;i<relation_number;i++){
     fscanf(f,"%s[^\n]",r);

     char file[100];
     strcpy(file,directory);
     strcat(file,"/");
     strcat(file,r);
     printf("file to open= %s\n",file);
   }
 }

 void delete_all_array(char** directory,char** file){
   free(*directory);
   free(*file);
 }
