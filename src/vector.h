#ifndef vector_h
#define vector_h


#include <string.h>
#include <stdlib.h>


enum type {
	VOID,
	CHAR,
	INT,
	LONG,
	FLOAT,
	DOUBLE,
	LONG_DOUBLE
};

typedef struct vector {
	void **data;
	size_t capacity;
	size_t size;
} vector;


void vectorInit(vector *vec);
void vectorResize(vector *vec, size_t capacity);
void vectorAdd(vector *vec, void *data, enum type dataType, unsigned int length);
void vectorRemove(vector *vec, unsigned int pos);
void *vectorGet(vector *vec, unsigned int pos);
void vectorSet(vector *vec, unsigned int pos, void *data, enum type dataType, unsigned int length);
size_t vectorSize(vector *vec);
void vectorClear(vector *vec);


#endif