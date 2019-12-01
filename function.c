/* function.c */
#include "function.h"
#include "sort_join.h"
#include "join_list.h"
#include "mid_list.h"


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
     }

     return;
 }





 void malloc_Rr_Ss(relation **Rr1, relation **Rr2){
     *Rr1 = malloc(sizeof(relation));
     if(Rr1 == NULL){
	 printf("Error malloc Rr1 \n");
	 exit(1);
     }

     *Rr2 = malloc(sizeof(relation));
     if(Rr2 == NULL){
         printf("Error malloc Rr2 \n");
         exit(1);
     }

     return;
 }

 void delete_Rr_Ss(relation **Rr1, relation **Rr2){
     free(Rr1);
     free(Rr2);

     return;
 }







 void make_Rr1_Rr2(main_array **array, int *tables, q *predicates, int number_of_predicates, int jj, relation **Rr1, relation **Rr2, int a){
     int r1;
     uint64_t c1;

     if( a == 1 ){
         r1 = predicates[jj].relationA;
         c1 = predicates[jj].columnA;
     }else{
         r1 = predicates[jj].relationB;
	 c1 = predicates[jj].columnB;
     }

     // elegxo an gia tin sxesi r1 exoume kapoio filtro
     int find_filter = 0;
     int col, telestis;
     uint64_t value;
     for(int i = 0 ; i < number_of_predicates ; i++){
	 if(predicates[i].join == false && predicates[i].flag == false){
	     if(predicates[i].relationA == r1){ //exoume kapoio filtro gia tin sxesi r1
	         find_filter = 1;
		 predicates[i].flag = true;
		 col = predicates[i].columnA;
		 telestis = predicates[i].relationB;
		 value = predicates[i].columnB;
                 break;
	     }
	 }
     }

     printf("r1 %d\nc1 %lu\ncol %d\ntelestis %d\n value %lu \n", r1, c1, col, telestis, value);

     // ara twra mporw na ftiaksw tin sxesi Rr1
     int lines = 0;
     if( find_filter ){ // einai 1 diladi exei filtro
	 // prepei na metrisw poses grammes tou arxikou ikanopoun to filtro

         for(int i = (*array)[tables[r1]].index[col] ; i < (*array)[tables[r1]].index[col] + (*array)[tables[r1]].num_tuples ; i++){
	     if(telestis == 0){
	         if( (*array)[tables[r1]].relation_array[i] == value) lines++;
	     }else if(telestis == 1){
		 if( (*array)[tables[r1]].relation_array[i] > value) lines++;
	     }else{
	         if( (*array)[tables[r1]].relation_array[i] < value) lines++;
	     }
	 }


	 /////////////////////////////////////////////////
	 (*Rr1)->num_tuples = lines;
         (*Rr1)->tuples = malloc((*Rr1)->num_tuples * sizeof(tuple));
         if((*Rr1)->tuples == NULL){
	     printf("Error malloc Rr");
	     exit(1);
	 }
	 (*Rr2)->num_tuples = lines;
         (*Rr2)->tuples = malloc((*Rr2)->num_tuples * sizeof(tuple));
	 if((*Rr2)->tuples == NULL){
             printf("Error malloc Rr");
             exit(1);
         }
	 ////////////////////////////////////////////////////


         int j = 0;
	 for(int i = (*array)[tables[r1]].index[col] ; i < (*array)[tables[r1]].index[col] + (*array)[tables[r1]].num_tuples ; i++){
             if(telestis == 0){
                 if( (*array)[tables[r1]].relation_array[i] == value ) {
		     if(col > c1){
		         (*Rr1)->tuples[j].key = (*array)[tables[r1]].relation_array[i - ((col-c1) * (*array)[tables[r1]].num_tuples)];
		         (*Rr1)->tuples[j].payload = i - (*array)[tables[r1]].index[col] + 1;
		     }else{
			 (*Rr1)->tuples[j].key = (*array)[tables[r1]].relation_array[i + ((c1-col) * (*array)[tables[r1]].num_tuples)];
                         (*Rr1)->tuples[j].payload = i - (*array)[tables[r1]].index[col] + 1;
		     }
		     j++;
		 }
             }else if(telestis == 1){
                 if( (*array)[tables[r1]].relation_array[i] > value ){
		     if(col > c1){
                         (*Rr1)->tuples[j].key = (*array)[tables[r1]].relation_array[i - ((col-c1) * (*array)[tables[r1]].num_tuples)];
                         (*Rr1)->tuples[j].payload = i - (*array)[tables[r1]].index[col] + 1;
                     }else{
                         (*Rr1)->tuples[j].key = (*array)[tables[r1]].relation_array[i + ((c1-col) * (*array)[tables[r1]].num_tuples)];
                         (*Rr1)->tuples[j].payload = i - (*array)[tables[r1]].index[col] + 1;
                     }
                     j++;
		 }
             }else{
                 if((*array)[tables[r1]].relation_array[i] < value ){
		     if(col > c1){
                         (*Rr1)->tuples[j].key = (*array)[tables[r1]].relation_array[i - ((col-c1) * (*array)[tables[r1]].num_tuples)];
                         (*Rr1)->tuples[j].payload = i - (*array)[tables[r1]].index[col] + 1;
                     }else{
                         (*Rr1)->tuples[j].key = (*array)[tables[r1]].relation_array[i + ((c1-col) * (*array)[tables[r1]].num_tuples)];
                         (*Rr1)->tuples[j].payload = i - (*array)[tables[r1]].index[col] + 1;
                     }
                     j++;
		 }
             }
         }

     }else{ // einai 0 diladi den exei filtro
	 lines = (*array)[tables[r1]].num_tuples;


	 /////////////////////////////////////////////////////
         (*Rr1)->num_tuples = lines;
	 (*Rr1)->tuples = malloc((*Rr1)->num_tuples * sizeof(tuple));
         if((*Rr1)->tuples == NULL){
             printf("Error malloc Rr1");
             exit(1);
         }
	 (*Rr2)->num_tuples = lines;
         (*Rr2)->tuples = malloc((*Rr2)->num_tuples * sizeof(tuple));
         if((*Rr2)->tuples == NULL){
             printf("Error malloc R2");
             exit(1);
         }
	 //////////////////////////////////////////////////////////


         int k = (*array)[tables[r1]].index[c1];
         for(int i = 0 ; i < lines ; i++){
	     (*Rr1)->tuples[i].key = (*array)[tables[r1]].relation_array[k];
             k++;
	     (*Rr1)->tuples[i].payload = i + 1;
	 }
     }

 }





 void make_Rr1_Rr2__2(main_array **array, main_pointer *mid_result, int *tables, q *predicates, int number_of_predicates, int jj, relation **Rr1, relation **Rr2, int a){
     int r, c;

     if(a == 1){
         r = predicates[jj].relationA;
	 c = predicates[jj].columnA;
     }else{
	 r = predicates[jj].relationB;
	 c = predicates[jj].columnB;
     }

     printf("pinakas   %d\nstili   %d\n", r, c);


     int find_imid_result = 0;
     for(int i = 0 ; i < take_columns(mid_result) ; i++){
	 if(r == take_relation(mid_result, i))
	     find_imid_result = i+1;
     }

     printf("Find_imid_result   %d \n", find_imid_result);



     int x;
     printf("\nPata ena arithmo kai enter gia na sinexisw \n");
     scanf("%d", &x);



     if(find_imid_result == 0){
         // kalese tin proigoumeni make rr1 rr2
         make_Rr1_Rr2(array, tables, predicates, number_of_predicates, jj, Rr1, Rr2, a);
	 return;
     }


     printf("\nDen mpika pata ena arithmo kai enter gia na sinexisw \n");
     scanf("%d", &x);



	 int k;
         if(find_imid_result == 1)      k = 0;
         else if(find_imid_result == 2) k = 1;
	 else 				k = 2;


         // prepei na vrw posa diaforetika roid yparxoun ston endiameso
         uint64_t *aa = malloc(take_crowd_results_mid(mid_result) * sizeof(uint64_t));
	 if(aa == NULL){
	     printf("Error malloc a \n");
	     exit(1);
	 }

	 int rep = take_columns(mid_result);

	 int j = 0;
         for(int i = 0 ; i < take_crowd_results_mid(mid_result) ; i++){
	     uint64_t row = take_rowid(mid_result, k);
	     k += rep;
	     if(i == 0){
		 aa[j] = row;
		 j++;
	     }else{
		 int find = 0;
	         for(int l = 0 ; l < j ; l++){
		     if(aa[l] == row){
		         find = 1;
			 break;
		     }
		 }
		 if(find == 0){
		     aa[j] = row;
		     j++;
		 }
	     }
	 }

	 // exoume j-1 diaforetika rowid kai xeroume poia einai apo ton pinaka a
	 ///////////////////////////////////////////////////////////
         (*Rr1)->num_tuples = j-1;
         (*Rr1)->tuples = malloc((*Rr1)->num_tuples * sizeof(uint64_t));
         if((*Rr1)->tuples == NULL){
	     printf("Error malloc (*Rr1)->tuples \n");
	     exit(1);
         }
         (*Rr2)->num_tuples = j-1;
         (*Rr2)->tuples = malloc((*Rr2)->num_tuples * sizeof(uint64_t));
         if((*Rr2)->tuples == NULL){
             printf("Error malloc (*Rr2)->tuples \n");
             exit(1);
         }
         ////////////////////////////////////////////////////////////

	 for(int i = 0 ; i < j ; i++){
	     (*Rr1)->tuples[i].payload = aa[i];
         }

	 // gia kathe roid prepei na paw ston katalilo pinaka kai stili kai na parw to key
	 for(int i = 0 ; i < (*Rr1)->num_tuples ; i++){
	     uint64_t rowid = (*Rr1)->tuples[i].payload;
	     (*Rr1)->tuples[i].key = (*array)[tables[r]].relation_array[(*array)[tables[r]].index[c] + rowid];
	 }
	 free(aa);

     return;
 }


 void  make_second_intermid(info_deikti *join_list, main_pointer *imid_list, int size_imid_list){



 }



 void lets_go_for_predicates(main_array **array, int *tables, int relation_number, q *predicates, int number_of_predicates){
     int i, ii = 0;
     relation *Rr1, *Rr2;
     relation *Ss1, *Ss2;

     malloc_Rr_Ss(&Rr1, &Rr2);
     malloc_Rr_Ss(&Ss1, &Ss2);


     info_deikti join_list = NULL;

     main_pointer *imid_list = NULL;
     imid_list = malloc(2 * sizeof(main_pointer));
     if(imid_list == NULL){
         printf("Error malloc imid_list \n");
	 exit(1);
     }



     for(i = 0 ; i < number_of_predicates ; i++){
         if(predicates[i].join == false) continue; 	// trexoume to for mono gia ta join predicate

	 if(ii == 0){
	     ii++;
	     if(predicates[i].relationA == predicates[i].relationB){
		 printf("Self join \n");
		 //
	 	 //
	     }else{
		 join_list = LIST_dimiourgia(&join_list);

		 make_Rr1_Rr2(array, tables, predicates, number_of_predicates, i, &Rr1, &Rr2, 1);
		 make_Rr1_Rr2(array, tables, predicates, number_of_predicates, i, &Ss1, &Ss2, 2);

		 recurseFunc(&Rr1, &Rr2, 0, Rr1->num_tuples, 7);	// exoume ena thema edw vazei to apotelesma ston Rr2 enw tha eprepe ston Rr1
		 recurseFunc(&Ss1, &Ss2, 0, Ss1->num_tuples, 7);
//		 for(int jj=0 ; jj < Rr1->num_tuples ; jj++) printf("%lu   %lu \n", Rr1->tuples[jj].payload, Rr1->tuples[jj].key);
//		 for(int jj=0 ; jj < Ss1->num_tuples ; jj++) printf("%lu   %lu \n", Ss1->tuples[jj].payload, Ss1->tuples[jj].key);


		 // twra akolouthei to join opou ta apotelesmata tha mpoun stin lista gia ta join
		 Sort_Merge_Join(&Rr1, &Ss1, &join_list);


		 imid_list[0] = MID_dimiourgia(&imid_list[0], 2, predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB, -1, -1, -1, -1);


		 for(int k = 0 ; k < take_crowd_results(&join_list) ; k++){
		     tuple t = take_row(&join_list, k);

		     eisagogi_eggrafis_mid(&imid_list[0], t.key);
		     eisagogi_eggrafis_mid(&imid_list[0], t.payload);
		 }

		 lista_diagrafi(&join_list);	// diagrafw tin lista me to join - tha prepei na dimioyrgithei ksana gia ii == 2 kai ii == 3
		 free(Rr1->tuples);	free(Rr2->tuples);
		 free(Ss1->tuples);	free(Ss2->tuples);
	     }

	 }else if(ii == 1){
	     ii++;
	     join_list = LIST_dimiourgia(&join_list);

	     make_Rr1_Rr2__2(array, &imid_list[0], tables, predicates, number_of_predicates, i, &Rr1, &Rr2, 1);
             //for(int k = 0 ; k < Rr1->num_tuples ; k++) printf("%lu    %lu \n", Rr1->tuples[k].payload, Rr1->tuples[k].key);
	     make_Rr1_Rr2__2(array, &imid_list[0], tables, predicates, number_of_predicates, i, &Ss1, &Ss2, 2);
	     //for(int k = 0 ; k < Ss1->num_tuples ; k++) printf("%lu    %lu \n", Ss1->tuples[k].payload, Ss1->tuples[k].key);
	     recurseFunc(&Rr1, &Rr2, 0, Rr1->num_tuples, 7);
	     recurseFunc(&Ss1, &Ss2, 0, Ss1->num_tuples, 7);

	     Sort_Merge_Join(&Rr1, &Ss1, &join_list);


             exit(0);


	     if(take_relation(imid_list, 0) == predicates[i].relationA || take_relation(imid_list, 0) == predicates[i].relationB)
	         imid_list[1] = MID_dimiourgia(&imid_list[1], 3, take_relation(imid_list, 0), take_col(imid_list, 0), predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB, -1, -1;
	     else
		imid_list[1] = MID_dimiourgia(&imid_list[1], 3, take_relation(imid_list, 1), take_col(imid_list, 1), predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB,-1, -1);


	     // make_second_intermid(join_list, imid_list, 2);




	     // kapou edw prepei na diagrapsw to imid_list[0]
	     // efoson dimiourgisw to imid_list[1]
	     lista_diagrafi(&join_list);    // diagrafw tin lista me to join - tha prepei na dimioyrgithei ksana gia ii == 2 kai ii == 3
             free(Rr1->tuples);     free(Rr2->tuples);
             free(Ss1->tuples);     free(Ss2->tuples);
	 }else if(ii == 2){
	     ii++;

	 }
     }
     delete_Rr_Ss(&Rr1, &Rr2);
     delete_Rr_Ss(&Ss1, &Ss2);

     free(imid_list);
     return;
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
