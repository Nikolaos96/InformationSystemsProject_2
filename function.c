/* function.c */
#include "function.h"
#include "sort_join.h"
#include "join_list.h"
#include "mid_list.h"
#include "HashTable.h"
#define  BYTEPOS  7
#define HASH_TABLE_SIZE 10000


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
   char init_file[150];
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

       char file[150];
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



int find_checksum_number(char* query){
   strsep(&query,"|");
   strsep(&query,"|");

   return take_tokens(query);
}




void take_checksums(checksum_struct *checksums,int number_of_checksums,char* query){
  strsep(&query,"|");
  strsep(&query,"|");

  for(int i=0;i<number_of_checksums;i++){
    checksums[i].table = atoi(&query[0]);
    query+=2;

    checksums[i].row = atoi(&query[0]);
    query+=2;

  }
}



 void take_predicates(q *predicates, int number_of_predicates, char *query){
     strsep(&query,"|");
     char *preds  = strsep(&query,"|");

     for(int i = 0 ; i < number_of_predicates ; i++){
         char *predicate = strsep(&preds, "&");
         char A[100];
         strcpy(A,predicate);

         if(take_tokens(A) == 1){  // filtro

	     int  a1, a2;
	     uint64_t a3;
	     char c1, c2;
             sscanf(predicate, "%d %c %d %c %lu", &a1, &c1, &a2, &c2, &a3);

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
     free(*Rr1);
     free(*Rr2);

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




 void make_Rr1_Rr2__2(main_array **array, main_pointer *mid_result, int *tables, q *predicates, int number_of_predicates, int jj, relation **Rr1, relation **Rr2, int a,int *sort_needed){
     int r, c;
     *sort_needed=1;
     if(a == 1){
         r = predicates[jj].relationA;
	 c = predicates[jj].columnA;
     }else{
	 r = predicates[jj].relationB;
	 c = predicates[jj].columnB;
     }

     int find_imid_result = 0;
     for(int i = 0 ; i < take_columns(mid_result) ; i++){
	     if(r == take_relation(mid_result, i)){
	        find_imid_result = i+1;
          if(c == take_col(mid_result,i)){
             *sort_needed=0;
           }
        }
     }


     if(find_imid_result == 0){
         // kalese tin proigoumeni make rr1 rr2
       make_Rr1_Rr2(array, tables, predicates, number_of_predicates, jj, Rr1, Rr2, a);
	     return;
     }

     int k;
     if(find_imid_result == 1)      k = 0;
     else if(find_imid_result == 2) k = 1;
     else                           k = 2;


     ///////////////////////////////////////////////////////////////////////////////////////////////////
     // se periptwsi pou thelw akrivos mia stili kai pinaka apo to endiameso
     // ftiaxnw tin sxesi Rr h opoia ta rowId  einai akribos mia apo tis stiles tou endiamesou kai pairnw kai ta key tis analogis stilis
      int rep = take_columns(mid_result);
      if(*sort_needed==0){ //ama exoume tin stili tou pinaka ston endiameso
        (*Rr1)->num_tuples = take_crowd_results_mid(mid_result);
        (*Rr1)->tuples = malloc((*Rr1)->num_tuples * sizeof(tuple));
        if((*Rr1)->tuples == NULL){
          printf("Error malloc (*Rr1)->tuples \n");
          exit(1);
        }
        (*Rr2)->num_tuples = take_crowd_results_mid(mid_result);
        (*Rr2)->tuples = malloc((*Rr2)->num_tuples * sizeof(tuple));
        if((*Rr2)->tuples == NULL){
            printf("Error malloc (*Rr2)->tuples \n");
            exit(1);
        }
        for(int i=0;i<(*Rr1)->num_tuples;i++){
          (*Rr1)->tuples[i].payload =  take_rowid(mid_result, k);
          uint64_t rowid = (*Rr1)->tuples[i].payload;
          (*Rr1)->tuples[i].key = (*array)[tables[r]].relation_array[(*array)[tables[r]].index[c] + rowid - 1];
          k+=rep;
        }
        return;
      }


     // prepei na vrw posa diaforetika roid yparxoun ston endiameso
    /* uint64_t *aa = malloc(take_crowd_results_mid(mid_result) * sizeof(uint64_t));
     uint64_t *bb = malloc(take_crowd_results_mid(mid_result) * sizeof(uint64_t));

     if(aa == NULL){
	 printf("Error malloc a \n");
	 exit(1);
 }*/
     deiktis_ht *H_Table = malloc(HASH_TABLE_SIZE * sizeof(deiktis_ht));
     if(H_Table == NULL) { printf("Error malloc memory H_Table. \n\n"); exit(1); }

     for(int i = 0 ; i < HASH_TABLE_SIZE ; i++){
          H_Table[i] = HashTable_dimiourgia(&H_Table[i]);
          dimoiourgeia_arxikwn_bucket(&H_Table[i]);
      }
      for(int j = 0 ; j < take_crowd_results_mid(mid_result) ; j++){
           uint64_t row_id = take_rowid(mid_result, k);
           int hash_r = hash(row_id, HASH_TABLE_SIZE);
           eisagogi_rowId(&H_Table[hash_r], row_id, 0, 0, 1);
           k += rep;
      }

     ////////////////////////////////////////////////////////////
     // neos tropos gia na min pairnoume ta diplotipa

    /* for(int i = 0 ; i < take_crowd_results_mid(mid_result) ; i++){
         bb[i] = take_rowid(mid_result, k);
         k += rep;
     }
     quicksort2(bb, 0, take_crowd_results_mid(mid_result) - 1);	/////

     int j = 0;
     for(int i = 0 ; i < take_crowd_results_mid(mid_result) ; i++){
         if(i == 0){
	     aa[j] = bb[i];
	     j++;
	 }else{
	     if(bb[i] != aa[j-1]){
	        aa[j] = bb[i];
	        j++;
	     }
	 }
 }*/

     ///////////////////////////////////////////////////////////
     uint64_t ids_counter=0;
     for(int i=0;i<HASH_TABLE_SIZE;i++){
       ids_counter+=take_unique_ids(&H_Table[i]);
     }
     (*Rr1)->num_tuples = ids_counter;
     (*Rr1)->tuples = malloc((*Rr1)->num_tuples * sizeof(tuple));
     if((*Rr1)->tuples == NULL){
	 printf("Error malloc (*Rr1)->tuples \n");
	 exit(1);
     }
     (*Rr2)->num_tuples = ids_counter;
     (*Rr2)->tuples = malloc((*Rr2)->num_tuples * sizeof(tuple));
     if((*Rr2)->tuples == NULL){
         printf("Error malloc (*Rr2)->tuples \n");
         exit(1);
     }
     ////////////////////////////////////////////////////////////

     k=0;
     for(int i = 0 ; i < HASH_TABLE_SIZE ; i++){
       for(int j=0;j<take_unique_ids(&H_Table[i]);j++){
	      (*Rr1)->tuples[k].payload = emfanisi_ht(&H_Table[i],j);
        k++;
      }
     }
     // gia kathe roid prepei na paw ston katalilo pinaka kai stili kai na parw to key
     for(int i = 0 ; i < (*Rr1)->num_tuples ; i++){
	 uint64_t rowid = (*Rr1)->tuples[i].payload;
	 (*Rr1)->tuples[i].key = (*array)[tables[r]].relation_array[(*array)[tables[r]].index[c] + rowid - 1];	//////////////// sos thelei -1
     }
     for(int i = 0 ; i < HASH_TABLE_SIZE ; i++) HashTable_diagrafi(&H_Table[i]);
     free(H_Table);
    // free(aa);
    // free(bb);

     return;
 }





 void  make_second_intermid(info_deikti *join_list, main_pointer *imid_list, int size_imid_list, int rel,int rel2, int join_stil_A, int join_stil_B){
     int k=rel,k1,k2;
     // prepei na elegxoyme oxi tin k alla tin alli an einai idia me mia apo tis 2 stiles tou join
     int l;
     int join,index,val,val2;

     if(rel2==-1){
       if(k == 0) l = 1;
       else       l = 0;

       if(take_relation(&imid_list[0], l) == join_stil_A)
           join = 0;	// key
       else
           join = 1;	// payload





       deiktis_ht *H_Table = malloc(HASH_TABLE_SIZE * sizeof(deiktis_ht));
       if(H_Table == NULL) { printf("Error malloc memory H_Table. \n\n"); exit(1); }

       for(int i = 0 ; i < HASH_TABLE_SIZE ; i++){
            H_Table[i] = HashTable_dimiourgia(&H_Table[i]);
            dimoiourgeia_arxikwn_bucket(&H_Table[i]);
        }

       ///////////////////////////////////////////////////////////////////////////
	// deimiourgia   hash_table
       index = l;
       val = k;
       for(int j = 0 ; j < take_crowd_results_mid(&imid_list[0]) ; j++){
            uint64_t row_id = take_rowid(&imid_list[0], index);
            int hash_r = hash(row_id, HASH_TABLE_SIZE);
            eisagogi_rowId(&H_Table[hash_r], row_id, take_rowid(&imid_list[0], val), 0, 1);

            val += 2;
            index += 2;
       }

	//////////////////////////////////////////////////////////////////////////////


       for(int i = 0 ; i < take_crowd_results(join_list) ; i++){
            tuple tt = take_row(join_list, i);
            uint64_t row_id;

	    if(join == 0) row_id = tt.key;
	    else	  row_id = tt.payload;

	    int hash_r = hash(row_id, HASH_TABLE_SIZE);
            rows_node *t = take_list(&H_Table[hash_r], row_id);

            while(t != NULL){
                eisagogi_eggrafis_mid(&imid_list[1], t->row_id);
		eisagogi_eggrafis_mid(&imid_list[1], tt.key);
		eisagogi_eggrafis_mid(&imid_list[1], tt.payload);
	        t = t->next_row;
	    }
       }

       for(int i = 0 ; i < HASH_TABLE_SIZE ; i++) HashTable_diagrafi(&H_Table[i]);
       free(H_Table);

/*
       for(int i = 0 ; i < take_crowd_results(join_list) ; i++){
         tuple t = take_row(join_list, i);
          index = l;
          val = k;
          for(int j = 0 ; j < take_crowd_results_mid(&imid_list[0]) ; j++){
              if(join == 0){
                if( take_rowid(&imid_list[0], index) == t.key){
                    eisagogi_eggrafis_mid(&imid_list[1], take_rowid(&imid_list[0], val));
                    eisagogi_eggrafis_mid(&imid_list[1], t.key);
                    eisagogi_eggrafis_mid(&imid_list[1], t.payload);
                 }
               val += 2;
               index += 2;
             }else{
                 if( take_rowid(&imid_list[0], index) == t.payload){
                    eisagogi_eggrafis_mid(&imid_list[1], take_rowid(&imid_list[0], val));
                    eisagogi_eggrafis_mid(&imid_list[1], t.key);
                    eisagogi_eggrafis_mid(&imid_list[1], t.payload);
                 }
                 val +=  2;
                 index += 2;
             }
          }
        }
*/

   }else{

     k1=rel,k2=rel2;

     if((k1==0 && k2==1) || (k1==1 && k2==0))
        l=2;
     else if((k1==0 && k2==2) || (k1==2 && k2==0))
        l=1;
     else{
        l=0;
     }


     int thesi, thesi2, thesi3;
     for(int i = 0 ; i < take_columns(&imid_list[1]) ; i++){
        if(l == take_relation(&imid_list[1], i)) thesi = i;
        if(k1 == take_relation(&imid_list[1], i)) thesi2 = i;
        if(k2 == take_relation(&imid_list[1], i)) thesi3 = i;
     }


     if(take_relation(&imid_list[1], thesi) == join_stil_A)
         join = 0;	// key
     else
         join = 1;	// payload





       deiktis_ht *H_Table = malloc(HASH_TABLE_SIZE * sizeof(deiktis_ht));
       if(H_Table == NULL) { printf("Error malloc memory H_Table. \n\n"); exit(1); }

       for(int i = 0 ; i < HASH_TABLE_SIZE ; i++){
            H_Table[i] = HashTable_dimiourgia(&H_Table[i]);
            dimoiourgeia_arxikwn_bucket(&H_Table[i]);
        }

       ///////////////////////////////////////////////////////////////////////////
        // deimiourgia   hash_table

       index = thesi;   // 1
       val = thesi2;    // 0
       val2= thesi3;    // 2

       for(int j = 0 ; j < take_crowd_results_mid(&imid_list[1]) ; j++){
            uint64_t row_id = take_rowid(&imid_list[1], index);
            int hash_r = hash(row_id, HASH_TABLE_SIZE);
            eisagogi_rowId(&H_Table[hash_r], row_id, take_rowid(&imid_list[1], val), take_rowid(&imid_list[1], val2), 2);

            val += 3;
	    val2 += 3;
            index += 3;
       }
        //////////////////////////////////////////////////////////////////////////////

       for(int i = 0 ; i < take_crowd_results(join_list) ; i++){
            tuple tt = take_row(join_list, i);
            uint64_t row_id;

            if(join == 0) row_id = tt.key;
            else          row_id = tt.payload;

            int hash_r = hash(row_id, HASH_TABLE_SIZE);
            rows_node *t = take_list(&H_Table[hash_r], row_id);

            while(t != NULL){
                eisagogi_eggrafis_mid(&imid_list[0], t->row_id);
		eisagogi_eggrafis_mid(&imid_list[0], t->row_id2);
                eisagogi_eggrafis_mid(&imid_list[0], tt.key);
                eisagogi_eggrafis_mid(&imid_list[0], tt.payload);
                t = t->next_row;
            }
       }

       for(int i = 0 ; i < HASH_TABLE_SIZE ; i++) HashTable_diagrafi(&H_Table[i]);
       free(H_Table);

/*
    for(int i = 0 ; i < take_crowd_results(join_list) ; i++){
       tuple t = take_row(join_list, i);
       index = thesi;	// 1
       val = thesi2;	// 0
       val2= thesi3;	// 2
       for(int j = 0 ; j < take_crowd_results_mid(&imid_list[1]) ; j++){
           if(join == 0){
             if( take_rowid(&imid_list[1], index) == t.key){

                 eisagogi_eggrafis_mid(&imid_list[0], take_rowid(&imid_list[1], val));
                 eisagogi_eggrafis_mid(&imid_list[0], take_rowid(&imid_list[1], val2));
                 eisagogi_eggrafis_mid(&imid_list[0], t.key);
                 eisagogi_eggrafis_mid(&imid_list[0], t.payload);
              }
            val += 3;
            val2 += 3;
            index += 3;
          }else{
              if( take_rowid(&imid_list[1], index) == t.payload){
                 eisagogi_eggrafis_mid(&imid_list[0], take_rowid(&imid_list[1], val));
                 eisagogi_eggrafis_mid(&imid_list[0], take_rowid(&imid_list[1], val2));
                 eisagogi_eggrafis_mid(&imid_list[0], t.key);
                 eisagogi_eggrafis_mid(&imid_list[0], t.payload);
              }
              val += 3;
              val2 += 3;
              index += 3;
          }
       }
     }
*/
  }
     return;
 }




 void print_checksums(main_array **array, int *tables, checksum_struct *checksums, int number_of_checksums, main_pointer *imid_list, int imid_index){

        int rep = take_columns(&imid_list[imid_index]);
        for(int loop = 0 ; loop < number_of_checksums ; loop++){//gia ka8e checksum
            uint64_t sum = 0;
            int k = 0;
            int number_of_columns = take_columns(&imid_list[imid_index]);

            for(int index = 0 ; index < number_of_columns ; index++){//vriskoume gia to ka8e table se poia 8esi tou endiamesou apo8ikeuetai
                if(checksums[loop].table==take_relation(&imid_list[imid_index],index)){
                    k = index;
                    break;
                }
            }

            for(int i = 0 ; i < take_crowd_results_mid(&imid_list[imid_index]) ; i++){
                uint64_t row = take_rowid(&imid_list[imid_index], k);
                k += rep;
	        sum += (*array)[tables[checksums[loop].table]].relation_array[(*array)[tables[checksums[loop].table]].index[checksums[loop].row] + row -1];
            }
            if(sum==0)
              printf("NULL ");
            else
              printf("%lu ",sum);
        }
     return;
 }




 void edit_itermid(main_array **array, int *tables, main_pointer *imid_list, main_pointer *imid_list2, int relA, int colA, int relB, uint64_t colB, int ii){

  if(ii == 1){
     if(relA != take_relation(&imid_list[0], 0)){
         uint64_t temp = relA;
	 relA = relB;
	 relB = temp;

 	 temp = colA;
	 colA = colB;
	 colB = temp;
     }

     imid_list[1] = MID_dimiourgia(&imid_list[1], 2, take_relation(&imid_list[0], 0), take_col(&imid_list[0], 0), take_relation(&imid_list[0], 1), take_col(&imid_list[0], 1), -1, -1, -1, -1);

     int k = 0, row1, row2;
     for(int i = 0 ; i < take_crowd_results_mid(&imid_list[0]) ; i++){
         row1 = take_rowid(&imid_list[0], k);
         row2 = take_rowid(&imid_list[0], k+1);


         if( (*array)[tables[relA]].relation_array[(*array)[tables[relA]].index[colA] + row1 -1] ==
	     (*array)[tables[relB]].relation_array[(*array)[tables[relB]].index[colB] + row2 -1] ){

             eisagogi_eggrafis_mid(&imid_list[1], row1);
	     eisagogi_eggrafis_mid(&imid_list[1], row2);
	 }
	 k += 2;

     }
     lista_diagrafi_mid(&imid_list[0]);

     imid_list[0] = MID_dimiourgia(&imid_list[0], 2, take_relation(&imid_list[1], 0), take_col(&imid_list[1], 0), take_relation(&imid_list[1], 1), take_col(&imid_list[1],1), -1, -1, -1, -1);

     k = 0;
     for(int i = 0 ; i < take_crowd_results_mid(&imid_list[1]) ; i++){
         row1 = take_rowid(&imid_list[1], k);
	 row2 = take_rowid(&imid_list[1], k+1);

	 eisagogi_eggrafis_mid(&imid_list[0], row1);
	 eisagogi_eggrafis_mid(&imid_list[0], row2);
         k += 2;
     }
     lista_diagrafi_mid(&imid_list[1]);


  }else{

      int x, y;
      for(int i = 0 ; i < take_columns(&imid_list[0]) ; i++){
	  if(take_relation(&imid_list[0], i) == relA) x = i;
      }

      for(int i = 0 ; i < take_columns(&imid_list[0]) ; i++){
          if(take_relation(&imid_list[0], i) == relB) y = i;
      }

      imid_list2[0] = MID_dimiourgia(&imid_list2[0], 3, take_relation(&imid_list[0], 0), take_col(&imid_list[0], 0), take_relation(&imid_list[0], 1), take_col(&imid_list[0], 1), take_relation(&imid_list[0], 2), take_col(&imid_list[0], 2), -1, -1);

      int k = 0, row1, row2, row3;

      for(int i = 0 ; i < take_crowd_results_mid(imid_list) ; i++){
          row1 = take_rowid(imid_list, k);
          row2 = take_rowid(imid_list, k+1);
	  row3 = take_rowid(imid_list, k+2);


          int ok = 0;
          if(x == 0){
	      if(y == 1){
	          if( (*array)[tables[relA]].relation_array[(*array)[tables[relA]].index[colA] + row1 -1] ==
            	      (*array)[tables[relB]].relation_array[(*array)[tables[relB]].index[colB] + row2 -1] ){

		       ok = 1;
		   }
	      }else{
		   if( (*array)[tables[relA]].relation_array[(*array)[tables[relA]].index[colA] + row1 -1] ==
                      (*array)[tables[relB]].relation_array[(*array)[tables[relB]].index[colB] + row3 -1] ){

		       ok = 1;
                   }
	      }
	  }else if(x == 1){
	      if(y == 0){
	          if( (*array)[tables[relA]].relation_array[(*array)[tables[relA]].index[colA] + row2 -1] ==
                      (*array)[tables[relB]].relation_array[(*array)[tables[relB]].index[colB] + row1 -1] ){

		       ok = 1;
                   }
	      }else{
	 	  if( (*array)[tables[relA]].relation_array[(*array)[tables[relA]].index[colA] + row2 -1] ==
                      (*array)[tables[relB]].relation_array[(*array)[tables[relB]].index[colB] + row3 -1] ){

		       ok = 1;
                   }
	      }
	  }else{
	      if(y == 0){
		  if( (*array)[tables[relA]].relation_array[(*array)[tables[relA]].index[colA] + row3 -1] ==
                      (*array)[tables[relB]].relation_array[(*array)[tables[relB]].index[colB] + row1 -1] ){

		       ok = 1;
                   }
	      }else{
		  if( (*array)[tables[relA]].relation_array[(*array)[tables[relA]].index[colA] + row3 -1] ==
                      (*array)[tables[relB]].relation_array[(*array)[tables[relB]].index[colB] + row2 -1] ){

		       ok = 1;
                   }
	      }
	  }

	  if( ok ){
	      eisagogi_eggrafis_mid(imid_list2, row1);
	      eisagogi_eggrafis_mid(imid_list2, row2);
	      eisagogi_eggrafis_mid(imid_list2, row3);
	  }

          k += 3;
      }



      lista_diagrafi_mid(imid_list);
      imid_list[0] = MID_dimiourgia(imid_list, 3, take_relation(imid_list2, 0), take_col(imid_list2, 0), take_relation(imid_list2, 1), take_col(imid_list2, 1), take_relation(imid_list2, 2), take_col(imid_list2, 2), -1, -1);


      k = 0;
      for(int i = 0 ; i < take_crowd_results_mid(imid_list2) ; i++){
          row1 = take_rowid(imid_list2, k);
	  row2 = take_rowid(imid_list2, k+1);
  	  row3 = take_rowid(imid_list2, k+2);

	  eisagogi_eggrafis_mid(imid_list, row1);
	  eisagogi_eggrafis_mid(imid_list, row2);
	  eisagogi_eggrafis_mid(imid_list, row3);

	  k += 3;
      }

      lista_diagrafi_mid(imid_list2);
  }

     return;
 }



 void lets_go_for_predicates(main_array **array, int *tables, int relation_number, q *predicates, int number_of_predicates,checksum_struct *checksums,int number_of_checksums){
     int i, ii = 0,sort_needed=0,flag=0;
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
     imid_list[0] = NULL;
     imid_list[1] = NULL;


     for(i = 0 ; i < number_of_predicates ; i++){
         flag=0;

         if(predicates[i].join == false) continue; 	// trexoume to for mono gia ta join predicate

	 if(ii == 0){
	     ii++;

	     make_Rr1_Rr2(array, tables, predicates, number_of_predicates, i, &Rr1, &Rr2, 1);
	     make_Rr1_Rr2(array, tables, predicates, number_of_predicates, i, &Ss1, &Ss2, 2);

	     recurseFunc(&Rr1, &Rr2, 0, Rr1->num_tuples, BYTEPOS);
	     recurseFunc(&Ss1, &Ss2, 0, Ss1->num_tuples, BYTEPOS);

             imid_list[0] = MID_dimiourgia(&imid_list[0], 2, predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB, -1, -1, -1, -1);

	     Sort_Merge_Join(&Rr1, &Ss1, &join_list, &imid_list[0],&imid_list[1],1,0);


	     free(Rr1->tuples);	free(Rr2->tuples);
	     free(Ss1->tuples);	free(Ss2->tuples);

	 }else if(ii == 1){

               if((predicates[i].relationA == take_relation(&imid_list[0], 0) && predicates[i].relationB == take_relation(&imid_list[0], 1)) ||
		  ((predicates[i].relationA == take_relation(&imid_list[0], 1) && predicates[i].relationB == take_relation(&imid_list[0], 0)))){

	           edit_itermid(array, tables, &imid_list[0], &imid_list[1], predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB, ii);

	       }else{

	           ii++;
	           join_list = LIST_dimiourgia(&join_list);


                   make_Rr1_Rr2__2(array, &imid_list[0], tables, predicates, number_of_predicates, i, &Rr1, &Rr2, 1,&sort_needed);
                   if(sort_needed) recurseFunc(&Rr1, &Rr2, 0, Rr1->num_tuples, BYTEPOS);
          	   else		   flag=1; //o R pairnetai apo ton endiameso etoimos


     	   	   make_Rr1_Rr2__2(array, &imid_list[0], tables, predicates, number_of_predicates, i, &Ss1, &Ss2, 2,&sort_needed);

         	   if(sort_needed) recurseFunc(&Ss1, &Ss2, 0, Ss1->num_tuples, BYTEPOS);
          	   else		   flag=2;//o S pairnetai apo ton endiameso etoimos


          	   if(flag==1)
            	       imid_list[1] = MID_dimiourgia(&imid_list[1], 3, take_relation(imid_list, 0), take_col(imid_list, 0), take_relation(imid_list, 1), take_col(imid_list, 1), predicates[i].relationB, predicates[i].columnB, -1, -1);
          	   if(flag==2)
            	       imid_list[1] = MID_dimiourgia(&imid_list[1], 3, take_relation(imid_list, 0), take_col(imid_list, 0), take_relation(imid_list, 1), take_col(imid_list, 1), predicates[i].relationA, predicates[i].columnA, -1, -1);


	           Sort_Merge_Join(&Rr1, &Ss1, &join_list,&imid_list[0],&imid_list[1],0,flag);

		   if(flag==0){//an flag=0 kanoume to klassiko
    	     	       if(take_relation(&imid_list[0], 0) != predicates[i].relationA && take_relation(&imid_list[0], 0) != predicates[i].relationB){
    	                   imid_list[1] = MID_dimiourgia(&imid_list[1], 3, take_relation(imid_list, 0), take_col(imid_list, 0), predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB, -1, -1);
    		           make_second_intermid(&join_list, imid_list, 2, take_relation(imid_list, 0), -1, predicates[i].relationA, predicates[i].relationB);
    	     	       }else{
    	                   imid_list[1] = MID_dimiourgia(&imid_list[1], 3, take_relation(imid_list, 1), take_col(imid_list, 1), predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB,-1, -1);
    	                   make_second_intermid(&join_list, imid_list, 2, take_relation(imid_list, 1), -1, predicates[i].relationA, predicates[i].relationB);
    	     	       }
           	   }



	     	   lista_diagrafi_mid(&imid_list[0]);
	     	   imid_list[0] = NULL;
	     	   lista_diagrafi(&join_list);
             	   free(Rr1->tuples);     free(Rr2->tuples);
             	   free(Ss1->tuples);     free(Ss2->tuples);
	     }
	 }else if(ii == 2){

	     if( (predicates[i].relationA == take_relation(&imid_list[1], 0) || predicates[i].relationA == take_relation(&imid_list[1], 1) || predicates[i].relationA == take_relation(&imid_list[1], 2)) &&
		  (predicates[i].relationB == take_relation(&imid_list[1], 0) || predicates[i].relationB == take_relation(&imid_list[1], 1) || predicates[i].relationB == take_relation(&imid_list[1], 2)) ){
	         edit_itermid(array, tables, &imid_list[1], &imid_list[0], predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB, ii);

	     }else{

	         ii++;
       	   	 join_list = LIST_dimiourgia(&join_list);
       	   	 make_Rr1_Rr2__2(array, &imid_list[1], tables, predicates, number_of_predicates, i, &Rr1, &Rr2, 1,&sort_needed);

           	 if(sort_needed) recurseFunc(&Rr1, &Rr2, 0, Rr1->num_tuples, BYTEPOS);
          	 else 		 flag=1;

           	 make_Rr1_Rr2__2(array, &imid_list[1], tables, predicates, number_of_predicates, i, &Ss1, &Ss2, 2,&sort_needed);

           	 if(sort_needed) recurseFunc(&Ss1, &Ss2, 0, Ss1->num_tuples, BYTEPOS);
            	 else 	         flag=2;


            	 if(flag==1)
                     imid_list[0] = MID_dimiourgia(&imid_list[1], 4, take_relation(&imid_list[1], 0), take_col(&imid_list[1], 0), take_relation(&imid_list[1], 1), take_col(&imid_list[1], 1),take_relation(&imid_list[1], 2), take_col(&imid_list[1], 2), predicates[i].relationB, predicates[i].columnB);
                 if(flag==2)
                     imid_list[0] = MID_dimiourgia(&imid_list[1], 4, take_relation(&imid_list[1], 0), take_col(&imid_list[1], 0), take_relation(&imid_list[1], 1), take_col(&imid_list[1], 1),take_relation(&imid_list[1], 2), take_col(&imid_list[1], 2), predicates[i].relationA, predicates[i].columnA);

	         Sort_Merge_Join(&Rr1, &Ss1, &join_list,&imid_list[1],&imid_list[0],0,flag);

                 if(flag==0){

    	             if(take_relation(&imid_list[1], 0) != predicates[i].relationA && take_relation(&imid_list[1], 0) != predicates[i].relationB &&
    		         take_relation(&imid_list[1], 1) != predicates[i].relationA && take_relation(&imid_list[1], 1) != predicates[i].relationB){

                         imid_list[0] = MID_dimiourgia(&imid_list[1], 4, take_relation(&imid_list[1], 0), take_col(&imid_list[1], 0), take_relation(&imid_list[1], 1), take_col(&imid_list[1], 1),
    		   	                               predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB);

    		         make_second_intermid(&join_list, imid_list, 2, take_relation(&imid_list[1], 0), take_relation(&imid_list[1], 1), predicates[i].relationA, predicates[i].relationB);

      	             }else if(take_relation(&imid_list[1], 0) != predicates[i].relationA && take_relation(&imid_list[1], 0) != predicates[i].relationB &&
    		               take_relation(&imid_list[1], 2) != predicates[i].relationA && take_relation(&imid_list[1], 2) != predicates[i].relationB){

    	                 imid_list[0] = MID_dimiourgia(&imid_list[1], 4, take_relation(&imid_list[1], 0), take_col(&imid_list[1], 0), take_relation(&imid_list[1], 2), take_col(&imid_list[1], 2),
    			                               predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB);

    		         make_second_intermid(&join_list, imid_list, 2, take_relation(&imid_list[1], 0), take_relation(&imid_list[1], 2), predicates[i].relationA, predicates[i].relationB);

    	             }else if(take_relation(&imid_list[1], 1) != predicates[i].relationA && take_relation(&imid_list[1], 1) != predicates[i].relationB &&
    		               take_relation(&imid_list[1], 2) != predicates[i].relationA && take_relation(&imid_list[1], 2) != predicates[i].relationB){

    	                 imid_list[0] = MID_dimiourgia(&imid_list[1], 4, take_relation(&imid_list[1], 1), take_col(&imid_list[1], 1), take_relation(&imid_list[1], 2), take_col(&imid_list[1], 2),
   	                                               predicates[i].relationA, predicates[i].columnA, predicates[i].relationB, predicates[i].columnB);

    	 	         make_second_intermid(&join_list, imid_list, 2, take_relation(&imid_list[1], 1), take_relation(&imid_list[1], 2), predicates[i].relationA, predicates[i].relationB);
                     }
                 }


                 lista_diagrafi_mid(&imid_list[1]);
	         imid_list[1] = NULL;
	         lista_diagrafi(&join_list);
	         free(Rr1->tuples);     free(Rr2->tuples);
                 free(Ss1->tuples);     free(Ss2->tuples);
	     }
	 }
     }

     if(ii==1 || ii==3)
         print_checksums(array,&tables[0],checksums,number_of_checksums,imid_list,0);
     else
         print_checksums(array,&tables[0],checksums,number_of_checksums,imid_list,1);



     delete_Rr_Ss(&Rr1, &Rr2);
     delete_Rr_Ss(&Ss1, &Ss2);

     if(imid_list[0] != NULL){
         lista_diagrafi_mid(&imid_list[0]);
     }else{
         lista_diagrafi_mid(&imid_list[1]);
     }
     free(imid_list);


     return;
 }



 void read_queries(char *query_file,main_array **array,int relation_number){
   char *query, query2[100];
   size_t len = 0;
   clock_t time;

   FILE *f=fopen(query_file,"r");
   if(f==NULL){
       printf("error in opening query_file\n");
       exit(1);
   }

   int xx =  0;
   while(getline(&query,&len,f)!= -1){
        xx++;

        if(!strcmp(query,"F\n")){
            printf("\nEnd of batch.\n\n");
            continue;
        }
        if(xx < 30 && xx==33) continue;


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

        //checksum
        int number_of_checksums=find_checksum_number(query2);
        checksum_struct *checksums = malloc(number_of_checksums * sizeof(checksum_struct));
        strcpy(query2, query);



        take_checksums(checksums,number_of_checksums,query2);//edw exoume to struct me ta checksums
        time = clock();
        lets_go_for_predicates(array, &tables[0], relation_number, predicates, number_of_predicates,checksums,number_of_checksums);
        time = clock() - time;
        printf("                                    (Cpu time:  %lf  sec)\n", ((double)time) / CLOCKS_PER_SEC);



       free(checksums);
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
