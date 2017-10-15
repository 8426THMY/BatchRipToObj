#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "vector.h"


#ifdef _WIN32
	#define MAX_PATH_SIZE (_MAX_PATH + 1)
#else
	#include <limits.h>
	#define MAX_PATH_SIZE (PATH_MAX + 1)
#endif


//Forward declare the function so we can have main at the top!
unsigned char convertRIP(FILE *oldFile, const char objFlags[2], char *fileName, const size_t fileNameLength);


int main(int argc, char *argv[]){
	char inputPath[MAX_PATH_SIZE];
	size_t inputPathLength = 0;
	char outputPath[MAX_PATH_SIZE];
	size_t outputPathLength = 0;

	/*
		objFlags[0] = Should we ignore UVs?
		objFlags[1] = Should we ignore normals?
	*/
	char objFlags[2] = {0};


	size_t i;
	/** Loop through the program's arguments. **/
	for(i = 1; i < argc; ++i){
		//If this argument is "-i", set the input file path to the argument that follows it.
		if(strcmp(argv[i], "-i") == 0){
			++i;
			if(i < argc){
				inputPathLength = strlen(argv[i]);
				memcpy(inputPath, argv[i], inputPathLength);
				//If it doesn't end in a backslash or forwardslash, add one!
				if(inputPath[inputPathLength - 1] != '\\' && inputPath[inputPathLength - 1] != '/'){
					memcpy(inputPath + inputPathLength, "\\", sizeof("\\") - sizeof(char));
					++inputPathLength;
				}
			}
		}
		//If this argument is "-o", set the output file path to the argument that follows it.
		if(strcmp(argv[i], "-o") == 0){
			++i;
			if(i < argc){
				outputPathLength = strlen(argv[i]);
				memcpy(outputPath, argv[i], outputPathLength);
				//If it doesn't end in a backslash or forwardslash, add one!
				if(outputPath[outputPathLength - 1] != '\\' && outputPath[outputPathLength - 1] != '/'){
					memcpy(outputPath + outputPathLength, "\\", sizeof("\\") - sizeof(char));
					++outputPathLength;
				}
			}
		}

		//Ignore UV coordinates.
		if(strcmp(argv[i], "-ignoreUVs") == 0){
			objFlags[0] = 1;
		}
		//Ignore normals.
		if(strcmp(argv[i], "-ignoreNormals") == 0){
			objFlags[1] = 1;
		}
	}

	//Add a null terminator to the paths just in case!
	inputPath[inputPathLength] = '\0';
	outputPath[outputPathLength] = '\0';


	DIR *fileDir;
	/** If the directory couldn't be opened, use the program's directory! **/
	if((fileDir = opendir(inputPath)) == NULL || errno == ENOENT){
		if(inputPathLength == 0){
			fputs("Input directory not specified, using the program's directory...\n\n", stdout);
		}else{
			fputs("The specified directory could not be opened, using the program's directory...\n\n", stdout);
		}

		inputPathLength = sizeof(".\\") - sizeof(char);
		memcpy(inputPath, ".\\", sizeof(".\\"));

		fileDir = opendir(inputPath);
	}
	/** If an output path wasn't specified, set it to a folder called "out" within the inputPath. **/
	if(outputPathLength == 0){
		outputPathLength = inputPathLength + (sizeof("out\\") - sizeof(char));
		memcpy(outputPath, inputPath, inputPathLength);
		memcpy(outputPath + inputPathLength, "out\\", sizeof("out\\"));

		printf("No output path was specified, saving files to \"%s\".\n\n", outputPath);
	}
	//Be sure to create the output directory!
	mkdir(outputPath);


	struct dirent *dirFile;
	unsigned int successCount = 0;
	unsigned int failCount = 0;

	//No point in doing this multiple times. It's not like these paths will change.
	char inputFileName[MAX_PATH_SIZE];
	memcpy(inputFileName, inputPath, inputPathLength);
	char outputFileName[MAX_PATH_SIZE];
	memcpy(outputFileName, outputPath, outputPathLength);
	printf("%s\n", inputPath);

	/** Loop through all the files in the specified directory. **/
	while((dirFile = readdir(fileDir)) != NULL){
		//Combine the file's directory and name to get the full path.
		memcpy(inputFileName + inputPathLength, dirFile->d_name, dirFile->d_namlen);
		inputFileName[inputPathLength + dirFile->d_namlen] = '\0';


		FILE *oldFile;
		const char *fileExtension = strrchr(dirFile->d_name, '.');
		/** Open the file and convert it if the extension is ".rip"! **/
		if(fileExtension != NULL && strcmp(fileExtension, ".rip") == 0 && (oldFile = fopen(inputFileName, "rb")) != NULL){
			//We don't add the file extension to the end because we change it later on.
			const size_t outputFileNameLength = outputPathLength + dirFile->d_namlen - 3;
			memcpy(outputFileName + outputPathLength, dirFile->d_name, dirFile->d_namlen - 3);
			outputFileName[outputFileNameLength] = '\0';


			printf("Currently converting \"%s\"...\n", inputFileName);
			if(convertRIP(oldFile, objFlags, outputFileName, outputFileNameLength)){
				fputs("Conversion successful.\n\n", stdout);

				++successCount;
			}else{
				fputs("Conversion unsuccessful.\n\n", stdout);

				remove(outputFileName);

				++failCount;
			}


			fclose(oldFile);
		}
	}
	closedir(fileDir);


	if(successCount > 0){
		printf("\nSuccessfully converted %d files.", successCount);
	}
	if(failCount > 0){
		printf("\nFailed to convert %d files.", failCount);
	}


	fputs("\n\nPress enter to exit.\n", stdout);
	getc(stdin);

	return(1);
}


/** Read four bytes from the file as a little-endian signed long. **/
long signed int readLong(FILE *file){
	long signed int returnVal;

	fread((void *)&returnVal, sizeof(long signed int), 1, file);

	return(returnVal);
}

/** Read four bytes from the file as a little-endian unsigned long. **/
long unsigned int readULong(FILE *file){
	long unsigned int returnVal;

	fread((void *)&returnVal, sizeof(long unsigned int), 1, file);

	return(returnVal);
}

/** Read four bytes from the file as a little-endian float. **/
float readFloat(FILE *file){
	float returnVal;

	fread((void *)&returnVal, sizeof(float), 1, file);

	return(returnVal);
}

/** Read a string from the file and store it in stringBuffer. **/
void readString(char *stringBuffer, FILE *file){
	//Record the current position of the file pointer.
	const long curPos = ftell(file);
	//Add characters to stringBuffer until we reach a null terminator or the end of the file!
	fgets(stringBuffer, 1000, file);
	//Set the file pointer to the position after the null terminator we just reached!
	fseek(file, curPos + strlen(stringBuffer) + 1, SEEK_SET);
	//Make sure the last character is a null terminator just in case!
	stringBuffer[strlen(stringBuffer)] = '\0';
}


/** Ensure the file's signature is correct. **/
unsigned char checkSignature(FILE *file){
	if(readULong(file) == 0xDEADC0DE){
		printf("File signature is 0xDEADC0DE!\n");

		return(1);
	}


	printf("File signature incorrect, aborting...\n");
	return(0);
}

/** Ensure the file's version is correct. **/
unsigned char checkVersion(FILE *file){
	if(readULong(file) == 4){
		fputs("'.rip' version is 4!\n", stdout);

		return(1);
	}


	fputs("'.rip' version isn't 4, aborting...\n", stdout);
	return(0);
}

void getVertexAttribs(vector *typeVector, size_t *posIndex, size_t *normIndex, size_t *uvIndex, const size_t numAttribs, FILE *file){
	//We only keep information for one of each different vertex attribute... for now!
	char gotEmPos  = 0;
	char gotEmNorm = 0;
	char gotEmUV   = 0;
	char curAttrib[1000];


	size_t a;
	size_t b;
	/** Loop through the vertex attribute information! **/
	for(a = 0; a < numAttribs; ++a){
		readString(curAttrib, file);

		readULong(file); //Skip attribute index as it is currently unused
		size_t attribOffset = readULong(file);
		readULong(file); //Skip attribute size as it is currently unused
		size_t numElements  = readULong(file);


		unsigned long val;
		//Add the vertex attribute types to our vector!
		for(b = 0; b < numElements; ++b){
			val = readULong(file);
			vectorAdd(typeVector, &val);
		}

		//Vertex position data.
		if(strcmp(curAttrib, "POSITION") == 0 && gotEmPos == 0){
			posIndex[0] = attribOffset / 4;
			posIndex[1] = posIndex[0] + 1;
			posIndex[2] = posIndex[0] + 2;

			gotEmPos = 1;

		//Vertex normal data.
		}else if(strcmp(curAttrib, "NORMAL") == 0 && gotEmNorm == 0){
			normIndex[0] = attribOffset / 4;
			normIndex[1] = normIndex[0] + 1;
			normIndex[2] = normIndex[0] + 2;

			gotEmNorm = 1;

		//Vertex UV data.
		}else if(strcmp(curAttrib, "TEXCOORD") == 0 && gotEmUV == 0){
			uvIndex[0] = attribOffset / 4;
			uvIndex[1] = uvIndex[0] + 1;

			gotEmUV = 1;
		}
    }
}

void getNames(vector *nameVector, const size_t totalNames, FILE *file){
	char tempName[MAX_PATH_SIZE];
	size_t i;
	//Push the names into our vector!
	for(i = 0; i < totalNames; ++i){
		readString(tempName, file);

		const size_t tempNameLength = strlen(tempName);
		char *curName = malloc((tempNameLength + 1) * sizeof(char));
		memcpy(curName, tempName, tempNameLength);
		curName[tempNameLength] = '\0';

		vectorAdd(nameVector, &curName);
	}
}

void getFaces(vector *faceVector, const size_t totalFaces, FILE *file){
	size_t i;
	for(i = 0; i < totalFaces; ++i){
		size_t curVert[3] = {0};
		//Store the indices for the current vertex.
		curVert[0] = readULong(file); curVert[1] = readULong(file); curVert[2] = readULong(file);

		//Now add them to our vector!
		vectorAdd(faceVector, &curVert[0]); vectorAdd(faceVector, &curVert[1]); vectorAdd(faceVector, &curVert[2]);
	}
}

void getVertices(vector *posVector, vector *normVector, vector *uvVector,
                 const size_t totalVerts, const vector *attribTypes,
                 const size_t *posIndex, const size_t *normIndex, const size_t *uvIndex, FILE *file){

	long unsigned int a;
	long unsigned int b;
	//Loop through all the vertices!
	for(a = 0; a < totalVerts; ++a){
		float vertPos[3]  = {0};
		float vertNorm[3] = {0};
		float vertUV[2]   = {0};

		//Loop through all our vertex attributes!
		for(b = 0; b < attribTypes->size; ++b){
			float curVal = 0.f;
			/*
			The data type depends on what we've stored in attribTypes:

			0 = float
			1 = long unsigned int
			2 = long signed int
			*/
			switch(*((long unsigned int *)vectorGet(attribTypes, b))){
				case 0:
					curVal = readFloat(file);
				break;

				case 1:
					curVal = (float)readULong(file);
				break;

				case 2:
					curVal = (float)readLong(file);
				break;
			}

			//Now check which attribute this information belongs to and add it to the respective array!
			//Positions
			if(b == posIndex[0]){
				vertPos[0] = curVal;
			}else if(b == posIndex[1]){
				vertPos[1] = curVal;
			}else if(b == posIndex[2]){
				vertPos[2] = curVal;

			//Normals
			}else if(b == normIndex[0]){
				vertNorm[0] = curVal;
			}else if(b == normIndex[1]){
				vertNorm[1] = curVal;
			}else if(b == normIndex[2]){
				vertNorm[2] = curVal;

			//UV Coordinates
			}else if(b == uvIndex[0]){
				vertUV[0] = curVal;
			}else if(b == uvIndex[1]){
				vertUV[1] = 1.f - curVal;
			}
		}


		//Add the vertex information to our respective vectors!
		vectorAdd(posVector, &vertPos[0]);   vectorAdd(posVector, &vertPos[1]);   vectorAdd(posVector, &vertPos[2]);
		vectorAdd(normVector, &vertNorm[0]); vectorAdd(normVector, &vertNorm[1]); vectorAdd(normVector, &vertNorm[2]);
		vectorAdd(uvVector, &vertUV[0]);     vectorAdd(uvVector, &vertUV[1]);
	}
}


void writeMtl(vector *textureNames, char *fileName, const size_t fileNameLength){
	memcpy(fileName + fileNameLength, "mtl", sizeof("mtl"));
	FILE *mtlFile = fopen(fileName, "wb");


	size_t i;
	//Create a very basic entry for each texture and write it to the file!
	for(i = 0; i < textureNames->size; ++i){
		const char *curName = *((char **)vectorGet(textureNames, i));
		fprintf(mtlFile, "newmtl %s\r\nmap_Kd %s", curName, curName);

		//We don't want to have a blank line at the end of the file.
		if(i < textureNames->size - 1){
			fwrite("\r\n", sizeof(char), sizeof("\r\n") - sizeof(char), mtlFile);
		}
	}


	fclose(mtlFile);
}

void writeObj(const vector *faceVector, const vector *posVector, const vector *normVector, const vector *uvVector,
              vector *textureNames, const char objFlags[2], char *fileName, const size_t fileNameLength){

	memcpy(fileName + fileNameLength, "obj", sizeof("obj"));
	FILE *objFile = fopen(fileName, "wb");


	/** Write '.mtl' File **/
	//If we're not ignoring UVs and the model uses atleast one texture, create a '.mtl' file!
	if(textureNames->size > 0 && !objFlags[0]){
		writeMtl(textureNames, fileName, fileNameLength);
		//"fileName" is modified to represent the material's file name.
		fprintf(objFile, "mtllib %s\r\n", fileName);

		//We only use the first texture... for now!
		fprintf(objFile, "usemtl %s\r\n", *((char **)vectorGet(textureNames, 0)));
	}


	size_t a;
	//Write the vertex positions to the file!
	for(a = 0; a < posVector->size / 3; ++a){
		//Then write it to the file!
		fprintf(objFile, "v %f %f %f\r\n", *((float *)vectorGet(posVector, a * 3)), *((float *)vectorGet(posVector, a * 3 + 1)), *((float *)vectorGet(posVector, a * 3 + 2)));
	}

	//Write the vertex UV coordinates to the file if we're not ignoring them!
	if(!objFlags[0]){
		for(a = 0; a < uvVector->size / 2; ++a){
			fprintf(objFile, "vt %f %f\r\n", *((float *)vectorGet(uvVector, a * 2)), *((float *)vectorGet(uvVector, a * 2 + 1)));
		}
	}

	//Write the vertex normals to the file if we're not ignoring them!
	if(!objFlags[1]){
		for(a = 0; a < normVector->size / 3; ++a){
			fprintf(objFile, "vn %f %f %f\r\n", *((float *)vectorGet(normVector, a * 3)), *((float *)vectorGet(normVector, a * 3 + 1)), *((float *)vectorGet(normVector, a * 3 + 2)));
		}
	}


	size_t b;
	//Write the faces to the file!
	for(a = 0; a < faceVector->size / 3; ++a){
		fwrite("f", sizeof(char), sizeof("f") - sizeof(char), objFile);

		for(b = 0; b < 3; ++b){
			//Write the position's indices to the file!
			fprintf(objFile, " %ld/", *((long unsigned int *)vectorGet(faceVector, a * 3 + b)) + 1);

			//If we're not ignoring UVs, write their indices to the file!
			if(!objFlags[0]){
				fprintf(objFile, "%ld", *((long unsigned int *)vectorGet(faceVector, a * 3 + b)) + 1);
			}
			//If we're not ignoring normals, write their indices to the file!
			if(!objFlags[1]){
				fprintf(objFile, "/%ld", *((long unsigned int *)vectorGet(faceVector, a * 3 + b)) + 1);
			}
		}

		//We don't want to have a blank line at the end of the file.
		if(a < faceVector->size / 3 - 1){
			fwrite("\r\n", sizeof(char), sizeof("\r\n") - sizeof(char), objFile);
		}
	}


	fclose(objFile);
}


//Now we declare this function!
unsigned char convertRIP(FILE *oldFile, const char objFlags[2], char *fileName, const size_t fileNameLength){
	/** Make sure the file's signature and version number match! **/
	if(!checkSignature(oldFile) || !checkVersion(oldFile)){
		return(0);
	}


	//Good thing we understand how '.rip' files work, right?
	//... right?
	const size_t totalFaces    = readULong(oldFile);
    const size_t totalVerts    = readULong(oldFile);
    readULong(oldFile); //Skip vertex size as it is currently unused.
    const size_t totalTextures = readULong(oldFile);
    const size_t totalShaders  = readULong(oldFile);
    const size_t totalAttribs  = readULong(oldFile);


	/** Vertex Attributes **/
	//Contains the data types for the different vertex attributes.
	vector attribTypes;
	vectorInit(&attribTypes, sizeof(long unsigned int));

	size_t posIndex[3]  = {0};
	size_t normIndex[3] = {0};
	size_t uvIndex[2]   = {0};

	//Get the information for out vertex attributes!
	getVertexAttribs(&attribTypes, posIndex, normIndex, uvIndex, totalAttribs, oldFile);


	/** Texture Names **/
	//Contains the names of each texture that the model uses!
	vector textureNames;
	vectorInit(&textureNames, sizeof(char *));

	//Now find and store 'em!
	getNames(&textureNames, totalTextures, oldFile);


	/** Shader Names - Currently Unused **/
	//Contains the names of each shader that the model uses!
	vector shaderNames;
	vectorInit(&shaderNames, sizeof(char *));

	//Now find and store 'em!
	getNames(&shaderNames, totalShaders, oldFile);


	/** Face Information **/
	//Contains the ordered indices for the vertices of each face!
	vector faceVector;
	vectorInit(&faceVector, sizeof(long unsigned int));

	//Now, you guessed it, find and store 'em!
	getFaces(&faceVector, totalFaces, oldFile);


	/** Vertex Information **/
	//Contains the position for each vertex!
	vector posVector;
	vectorInit(&posVector, sizeof(float));
	//Contains the normals for each vertex!
	vector normVector;
	vectorInit(&normVector, sizeof(float));
	//Contains the UV coordinates for each vertex!
	vector uvVector;
	vectorInit(&uvVector, sizeof(float));

	//Load the vertex information!
	getVertices(&posVector, &normVector, &uvVector, totalVerts, &attribTypes, posIndex, normIndex, uvIndex, oldFile);


	/** Write '.obj' File **/
	writeObj(&faceVector, &posVector, &normVector, &uvVector, &textureNames, objFlags, fileName, fileNameLength);


	//Free the memory held by our vectors!
	vectorClear(&attribTypes);
	vectorClear(&faceVector);
	vectorClear(&posVector);
	vectorClear(&normVector);
	vectorClear(&uvVector);

	size_t i;
	//We need to free the data held by our strings manually.
	for(i = 0; i < textureNames.size; ++i){
		free(*((char **)vectorGet(&textureNames, i)));
	}
	vectorClear(&textureNames);

	for(i = 0; i < shaderNames.size; ++i){
		free(*((char **)vectorGet(&shaderNames, i)));
	}
	vectorClear(&shaderNames);


	return(1);
}