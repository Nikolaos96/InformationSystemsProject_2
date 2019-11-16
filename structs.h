#ifndef __STRUCTS__
#define __STRUCTS__
#include <stdint.h>

typedef struct tuple{
        uint64_t key;
        uint64_t payload;
}tuple;

typedef struct relation{
        tuple *tuples;
        uint64_t num_tuples;
}relation;

typedef struct main_array{
        uint64_t num_tuples;
        uint64_t num_columns;
        uint64_t *relation_array;
        int      *index;
}main_array;


#endif
