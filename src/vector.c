#include "vector.h"


void vectorInit(vector *vec){
	vec->capacity = 1;
	vec->size = 0;

	vec->data = malloc(sizeof(void *) * vec->capacity);
}

void vectorResize(vector *vec, size_t capacity){
	// Realloc causes problems in the Release build?
	void **newData = realloc(vec->data, sizeof(void *) * capacity);

	if(newData != NULL){
		vec->data = newData;
		vec->capacity = capacity;
	}
}

void vectorAdd(vector *vec, void *data, enum type dataType){
	if(vec->size == vec->capacity){
		vectorResize(vec, vec->capacity * 2);
	}
	/*
	** It would technically be faster to cast any integer (char, int, long) value
	** directly to a void pointer when passing it into this function and use VOID
	** as the dataType, but doing it this way keeps it consistent with floats.
	*/
	switch(dataType){
		case CHAR:
			((char *)(vec->data))[vec->size++] = *(char *)data;
		break;
		case INT:
			((int *)(vec->data))[vec->size++] = *(int *)data;
		break;
		case LONG:
			((long *)(vec->data))[vec->size++] = *(long *)data;
		break;
		case FLOAT:
			((float *)(vec->data))[vec->size++] = *(float *)data;
		break;
		case DOUBLE:
			((double *)(vec->data))[vec->size++] = *(double *)data;
		break;
		case LONG_DOUBLE:
			((long double *)(vec->data))[vec->size++] = *(long double *)data;
		break;
		default:
			vec->data[vec->size++] = data;
		break;
	}
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
		return(&vec->data[pos]);
	}

	return(NULL);
}

void vectorSet(vector *vec, unsigned int pos, void *data, enum type dataType){
	if(pos < vec->size){
		/*
		** It would technically be faster to cast any integer (char, int, long) value
		** directly to a void pointer when passing it into this function and use VOID
		** as the dataType, but doing it this way keeps it consistent with floats.
		*/
		switch(dataType){
			case CHAR:
				((char *)(vec->data))[pos] = *(char *)data;
			break;
			case INT:
				((int *)(vec->data))[pos] = *(int *)data;
			break;
			case LONG:
				((long *)(vec->data))[pos] = *(long *)data;
			break;
			case FLOAT:
				((float *)(vec->data))[pos] = *(float *)data;
			break;
			case DOUBLE:
				((double *)(vec->data))[pos] = *(double *)data;
			break;
			case LONG_DOUBLE:
				((long double *)(vec->data))[pos] = *(long double *)data;
			break;
			default:
				vec->data[pos] = data;
			break;
		}
	}
}

size_t vectorSize(vector *vec){
	return(vec->size);
}

void vectorClear(vector *vec){
	free(vec->data);
}
