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

     return;
 }



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



 int take_tokens(char *str){
     int a = 0;
     for(int i = 0 ; i < (int)strlen(str) ; i++){
         if(str[i] == '.') a++;
     }
     return a;
 }




 void take_predicates(q *predicates, int number_of_predicates, char *query){
     strsep(&query,"|");
     char *preds  = strsep(&query,"|");
     //char *temp_intval;
     printf("preds= %s\n\n",preds);

     for(int i = 0 ; i < number_of_predicates ; i++){
         char *predicate = strsep(&preds, "&");
         char A[100];
         strcpy(A,predicate);

         printf("------------  predicate= %s \n", predicate);

         if(take_tokens(A) == 1){  // filtro

	     int  a1, a2;
	     uint64_t a3;
	     char c1, c2;
             sscanf(predicate, "%d %c %d %c %lu", &a1, &c1, &a2, &c2, &a3);
             printf("%d %c %d %c %lu\n", a1, c1, a2, c2, a3);

             predicates[i].flag = false;
	     predicates[i].join = false;
	     predicates[i].relationA = a1;
 	     predicates[i].columnA = a2;
             if(c2 == '=') 	predicates[i].relationB = 0;
	     else if(c2 == '>') predicates[i].relationB = 1;
             else 		predicates[i].relationB = 2;
	     predicates[i].columnB = a3;

	 }else{  // join

             int  a1, a2, a3;
	     uint64_t a4;
             char c1, c2, c3;
             sscanf(predicate, "%d %c %d %c %d %c %lu", &a1, &c1, &a2, &c2, &a3, &c3, &a4);
             printf("%d %c %d %c %d %c %lu \n", a1, c1, a2, c2, a3, c3, a4);

             predicates[i].flag = false;
	     predicates[i].join = true;
	     predicates[i].relationA = a1;
	     predicates[i].columnA = a2;
	     predicates[i].relationB = a3;
	     predicates[i].columnB = a4;

	 }

/*         temp_intval= strsep(&predicate,"."); //pairnoume tin prwti sxesi,sto predicate menei oti uparxei meta tin .
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
*/

     }


 }

 void lets_go_for_predicates(main_array **array, int *tables, int relation_number, q *predicates, int number_of_predicates){




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


       int number_of_predicates = take_number_of_predicates(query2);
       strcpy(query2, query);
       q *predicates = malloc(number_of_predicates * sizeof(q));
       if(predicates == NULL){
           printf("Error malloc queries \n");
           exit(1);
       }

       take_predicates(predicates, number_of_predicates, query2);  // edw exoume ena pinaka apo ta predicate tou query
       strcpy(query2, query);


       lets_go_for_predicates(array, &tables[0], relation_number, predicates, number_of_predicates);


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
