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

void vectorAdd(vector *vec, void *data, enum type dataType, unsigned int length){
	if(vec->size == vec->capacity){
		vectorResize(vec, vec->capacity * 2);
	}
	switch(dataType){
		case CHAR:
			vec->data[vec->size] = malloc(length * sizeof(char));
			memcpy(vec->data[vec->size++], data, length * sizeof(char));
		break;
		case INT:
			vec->data[vec->size] = malloc(length * sizeof(int));
			memcpy(vec->data[vec->size++], data, length * sizeof(int));
		break;
		case LONG:
			vec->data[vec->size] = malloc(length * sizeof(long));
			memcpy(vec->data[vec->size++], data, length * sizeof(long));
		break;
		case FLOAT:
			vec->data[vec->size] = malloc(length * sizeof(float));
			memcpy(vec->data[vec->size++], data, length * sizeof(float));
		break;
		case DOUBLE:
			vec->data[vec->size] = malloc(length * sizeof(double));
			memcpy(vec->data[vec->size++], data, length * sizeof(double));
		break;
		case LONG_DOUBLE:
			vec->data[vec->size] = malloc(length * sizeof(long double));
			memcpy(vec->data[vec->size++], data, length * sizeof(long double));
		break;
		default:
			vec->data[vec->size] = malloc(length * sizeof(void*));
			memcpy(vec->data[vec->size++], data, length * sizeof(void*));
		break;
	}
}

void vectorRemove(vector *vec, unsigned int pos){
	if(pos < vec->size){
		free(vec->data[pos]);

		unsigned int i = 0;
		for(i = pos; i < vec->size - 1; i++){
			vec->data[i] = vec->data[i + 1];
		}

		free(vec->data[--vec->size]);

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

void vectorSet(vector *vec, unsigned int pos, void *data, enum type dataType, unsigned int length){
	if(pos < vec->size){
		switch(dataType){
			case CHAR:
				free(vec->data[pos]);
				vec->data[pos] = malloc(length * sizeof(char));
				memcpy(vec->data[pos], data, length * sizeof(char));
			break;
			case INT:
				free(vec->data[pos]);
				vec->data[pos] = malloc(length * sizeof(int));
				memcpy(vec->data[pos], data, length * sizeof(int));
			break;
			case LONG:
				free(vec->data[pos]);
				vec->data[pos] = malloc(length * sizeof(long));
				memcpy(vec->data[pos], data, length * sizeof(long));
			break;
			case FLOAT:
				free(vec->data[pos]);
				vec->data[pos] = malloc(length * sizeof(float));
				memcpy(vec->data[pos], (float *)data, length * sizeof(float));
			break;
			case DOUBLE:
				free(vec->data[pos]);
				vec->data[pos] = malloc(length * sizeof(double));
				memcpy(vec->data[pos], (double *)data, length * sizeof(double));
			break;
			case LONG_DOUBLE:
				free(vec->data[pos]);
				vec->data[pos] = malloc(length * sizeof(long double));
				memcpy(vec->data[pos], (long double *)data, length * sizeof(long double));
			break;
			default:
				free(vec->data[pos]);
				vec->data[pos] = malloc(length * sizeof(void*));
				memcpy(vec->data[pos], data, length * sizeof(void*));
			break;
		}
	}
}

size_t vectorSize(vector *vec){
	return(vec->size);
}

void vectorClear(vector *vec){
	unsigned int i;
	for(i = 0; i < vec->size; i++){
		free(vec->data[i]);
	}
	free(vec->data);
}
