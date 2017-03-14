#ifndef vector_h
#define vector_h


#include <stdlib.h>


typedef struct vector {
	void **data;
	size_t capacity;
	size_t size;
} vector;


void vectorInit(vector *vec);
void vectorResize(vector *vec, size_t capacity);
void vectorAdd(vector *vec, void *data);
void vectorRemove(vector *vec, unsigned int pos);
void *vectorGet(vector *vec, unsigned int pos);
void vectorSet(vector *vec, unsigned int pos, void *data);
size_t vectorSize(vector *vec);
void vectorClear(vector *vec);


#endif