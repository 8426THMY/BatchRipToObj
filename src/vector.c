#include "vector.h"


void vectorInit(vector *vec){
	vec->capacity = 1;
	vec->size = 0;

	vec->data = malloc(sizeof(void *) * vec->capacity);
}

void vectorResize(vector *vec, size_t capacity){
	void **newData = realloc(vec->data, sizeof(void *) * capacity);

	if(newData != NULL){
		vec->data = newData;
		vec->capacity = capacity;
	}
}

void vectorAdd(vector *vec, void *data){
	if(vec->size == vec->capacity){
		vectorResize(vec, vec->capacity * 2);
	}
	vec->data[vec->size++] = data;
}

void vectorRemove(vector *vec, unsigned int pos){
	if(pos < vec->size){
		vec->data[pos] = NULL;

		unsigned int i = 0;
		for(i = pos; i < vec->size - 1; i++){
			vec->data[i] = vec->data[i + 1];
		}

		vec->data[--vec->size] = NULL;

		if(vec->size > 0 && vec->size == vec->capacity / 4){
			vectorResize(vec, vec->capacity / 2);
		}
	}
}

void *vectorGet(vector *vec, unsigned int pos){
	if(pos < vec->size){
		return(vec->data[pos]);
	}

	return(NULL);
}

void vectorSet(vector *vec, unsigned int pos, void *data){
	if(pos < vec->size){
		vec->data[pos] = data;
	}
}

size_t vectorSize(vector *vec){
	return(vec->size);
}

void vectorClear(vector *vec){
	free(vec->data);
}