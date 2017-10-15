#include "vector.h"


#include <string.h>


void vectorInit(vector *v, const size_t elementSize){
	v->data = malloc(elementSize);
	v->elementSize = elementSize;
	v->capacity = 1;
	v->size = 0;
}

void vectorResize(vector *v, const size_t capacity){
	void *tempData = realloc(v->data, v->elementSize * capacity);
	if(tempData != NULL){
		v->data = tempData;
		v->capacity = capacity;
	}
}

void vectorAdd(vector *v, const void *data){
	if(v->size == v->capacity){
		vectorResize(v, v->capacity * 2);
	}
	memcpy(v->data + v->size * v->elementSize, data, v->elementSize);
	v->size++;
}

void vectorRemove(vector *v, const size_t pos){
	if(pos < v->size){
		v->size--;
		if(pos != v->size){
			memmove(v->data + pos * v->elementSize, v->data + (pos + 1) * v->elementSize, (v->size - pos) * v->elementSize);
		}
	}
}

void *vectorGet(const vector *v, const size_t pos){
	if(pos < v->size){
		return(v->data + pos * v->elementSize);
	}

	return(NULL);
}

void vectorClear(vector *v){
	free(v->data);
}