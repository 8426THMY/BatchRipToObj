#ifndef vector_h
#define vector_h


#include <string.h>
#include <stdlib.h>


enum type {
	VOID_T,
	CHAR_T,
	INT_T,
	LONG_T,
	FLOAT_T,
	DOUBLE_T,
	LONG_DOUBLE_T
};

typedef struct vector {
	void **data;
	size_t capacity;
	size_t size;
} vector;


void vectorInit(vector *vec);
void vectorResize(vector *vec, const size_t capacity);
void vectorAdd(vector *vec, void *data, const enum type dataType, const unsigned int length);
void vectorRemove(vector *vec, const unsigned int pos);
void *vectorGet(const vector *vec, const unsigned int pos);
void vectorSet(vector *vec, const unsigned int pos, void *data, const enum type dataType, const unsigned int length);
size_t vectorSize(const vector *vec);
void vectorClear(vector *vec);


#endif