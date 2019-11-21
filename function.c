/* function.c */
#include "function.h"





 /*
   take arguments from command line and init variables
 */
 void take_arguments(int argc,char *argv[],char **file,char **dir,char **query_file){
    char *arg;
    if(argc != 7 ){
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
        else if(!strcmp(arg, "-Q")){
            *query_file = malloc((strlen(*++argv) + 1) * sizeof(char));
            strcpy(*query_file, *argv);
        }
        if(argc > 1) argc--;
     }

     return;
 }




 int find_relation_number(char* init_file){
   int lines = 0;
   char s[20];

   FILE *f=fopen(init_file,"r");
   if(f==NULL){
     printf("error in opening init file\n");
     exit(1);
   }

   while(1){
       if( fscanf(f, "%s", s) != 1) break;
       lines++;
   }
   fclose(f);

   return lines;
 }




 /*
  function to read all the files and save to memory
 */
 int create_init_relations(char *directory, char *workload_file, main_array **array){
   char init_file[100];
   char r[10];
   strcpy(init_file,directory);
   strcat(init_file,"/");
   strcat(init_file,workload_file);

   int relation_number = find_relation_number(init_file);
   //printf("----------------------- %d \n", relation_number);


   *array = malloc(relation_number * sizeof(main_array));
   if(array==NULL){
     printf("Error in array malloc\n");
     exit(1);
   }

   FILE *f=fopen(init_file,"r");
   for(int i=0 ; i < relation_number ; i++){
       fscanf(f,"%s[^\n]",r);

       char file[100];
       strcpy(file,directory);
       strcat(file,"/");
       strcat(file,r);
       //printf("file to open= %s\n",file);


       FILE *fp = fopen(file, "rb");
       if(fp == NULL){
	        printf("Error in fopen for file  %s \n", file);
	        exit(1);
       }

       uint64_t tuples, columns, x;
       fread(&tuples, sizeof(tuples), 1, fp);
       fread(&columns, sizeof(columns), 1, fp);


       /////////////////////////////////////////////////

       (*array)[i].num_tuples = tuples;
       (*array)[i].num_columns = columns;

       (*array)[i].index = malloc((*array)[i].num_columns * sizeof(int));
       if((*array)[i].index == NULL){
	        printf("Error in malloc array[%d]->index \n", i);
	        exit(1);
       }

       (*array)[i].relation_array = malloc( ((*array)[i].num_tuples * (*array)[i].num_columns) * sizeof(uint64_t));
       if((*array)[i].relation_array == NULL){
	        printf("Error in malloc array[%d]->relation_array \n", i);
	        exit(1);
       }
       //////////////////////////////////////////////////


       for(int j = 0 ; j < ((*array)[i].num_tuples * (*array)[i].num_columns) ; j++){
	        fread(&x, sizeof(x), 1, fp);
	        (*array)[i].relation_array[j] = x;
       }

       (*array)[i].index[0] = 0;
       for(int j = 1 ; j < (*array)[i].num_columns ; j++)
          (*array)[i].index[j] = j * (*array)[i].num_tuples;
       ////////////////////////////////////////////////////

       fclose(fp);
   }
   fclose(f);


   /*for(int j=0;j<13;j++){
    printf("%lu   %lu \n", (*array)[j].num_tuples, (*array)[j].num_columns);
    printf("\n____________________________________________ \n\n");
  }
  for(int j = 0 ; j < (*array)[1].num_columns ; j++) printf("%d \n", (*array)[1].index[j]);
   printf("\n____________________________________________ \n\n");
   for(int j = 0 ; j < (*array)[1].num_tuples ; j++) printf("%lu \n", (*array)[1].relation_array[j]); */


   return relation_number;
 }




 /*
  epistrefei pinaka me tis sxeseis
 */
 void take_relations(char *query, int *tables, int tables_size){
     char *relations;
     char *relation;

     relations  = strsep(&query,"|");

     int i = 0;
     while(1){
         relation = strsep(&relations," ");
         if(relation == NULL) break;
         tables[i] = atoi(relation);
         i++;
     }

     for(i = i ; i < tables_size ; i++) tables[i] = -1;
 }



 // theloume sinartisi pou na epistrefei 4 times diladi tiw 2 sxesei kai tiw 2 stiles pou tha ginei to join
 // episis prepei na epistrefei kai to filtro
 // episis otan exei ftasei sto telos prepei na epistredei oti den exoyme alo join h kapoio filtro

 int take_number_of_predicates(char *query){

     int a = 0;
     char *token, c1 = '&';
     token = strtok(query, "|");
     token = strtok(NULL, "|");

     for(int i = 0 ; i < (int)strlen(token) ; i++){
         if(token[i] == c1) a++;
     }

     return a+1;
 }






 void take_predicates(q *predicates, int number_of_predicates, char *query){
     strsep(&query,"|");
     char *preds  = strsep(&query,"|");
     char *temp_intval;
     printf("preds= %s\n",preds);

     for(int i = 0 ; i < number_of_predicates ; i++){
         char *predicate = strsep(&preds, "&");
         printf("predicate= %s\n",predicate);
         temp_intval= strsep(&predicate,"."); //pairnoume tin prwti sxesi,sto predicate menei oti uparxei meta tin .
         predicates[0].relationA=atoi(temp_intval); //to 0 proswrino,stin katallili 8esi
//edw isws la8os ama to colA einai >10
         predicates[0].columnA=atoi(&predicate[0]);//pairnoume tin stili tis prwtis sxesis
         predicate++; //kovoume tin stili tis prwtis sxesis

         printf("new predicate= %s\n",predicate);
         if(predicate[0]=='>' ){ //sigoura filtro
           predicates[0].join=false;
           predicates[0].relationB=1;
           predicate++; //kovoume to >
           temp_intval= strsep(&predicate," ");
           predicates[0].columnB=atoi(temp_intval);
           printf("join=%d relA= %d,columnA= %d,relB= %d,columnB= %lu\n",predicates[0].join,predicates[0].relationA,predicates[0].columnA,predicates[0].relationB,predicates[0].columnB);

         }
         else if(predicate[0]=='<' ){
           predicates[0].join=false;
           predicates[0].relationB=2;
           predicate++; //kovoume to <
           temp_intval= strsep(&predicate," ");
           predicates[0].columnB=atoi(temp_intval);
           printf("join=%d relA= %d,columnA= %d,relB= %d,columnB= %lu\n",predicates[0].join,predicates[0].relationA,predicates[0].columnA,predicates[0].relationB,predicates[0].columnB);
         }
         else if(predicate[0]=='='){//den kseroume an prokeitai gia filtro i join

         }

         }





     }


 }







 void read_queries(char *query_file,main_array **array,int relation_number){
   char *query, query2[100];
   size_t len = 0;

   FILE *f=fopen(query_file,"r");
   if(f==NULL){
     printf("error in opening query_file\n");
     exit(1);
   }


   while(getline(&query,&len,f)!= -1){
        if(!strcmp(query,"F\n")){
           printf("End of batch,press anything to procced to next batch\n");
           getchar();
           continue;
       }

       int *tables = malloc(relation_number * sizeof(int));
       if(tables == NULL){
	        printf("Error malloc tables \n");
	        exit(1);
       }
       strcpy(query2, query);



       take_relations(query2, &tables[0], relation_number); 	     // exoume tis sxeseis ston pinaka tables
       strcpy(query2, query);


       int number_of_predicates = take_number_of_predicates(query2); // pernoume to plithos ton join+fitler
       strcpy(query2, query);
       q *predicates = malloc(number_of_predicates * sizeof(q));
       if(predicates == NULL){
	   printf("Error malloc queries \n");
           exit(1);
       }

       take_predicates(predicates, number_of_predicates, query2);
       strcpy(query2, query);




       //for(int i = 0 ; i < relation_number ; i++) printf("%d   ", tables[i]);
      // printf("\n\n");
      // printf("take predicates    %d \n", number_of_predicates);




       free(predicates);
       free(tables);
   }

   fclose(f);
   free(query);

   return;
 }








 void delete_all_array(main_array **array, int relation_number, char **directory, char **file,char **query_file){

   for(int i = 0 ; i < relation_number; i++){
       free( (*array)[i].index );
       free( (*array)[i].relation_array );
   }
   free( *array );


   free(*directory);
   free(*file);
   free(*query_file);

   return;
 }
