#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

#include "vector.h"


//Forward declare the function so we can have main at the top!
unsigned short convertRIP(FILE *oldFile, char *objFlags, char *outputPath, char *fileName);


int main(int argc, char *argv[]){
	char inputPath[1000] = "";
	char outputPath[1000] = "";

	/*
		objFlags[0] = Should we ignore UVs?
		objFlags[1] = Should we ignore normals?
	*/
	char objFlags[2] = {0};


	unsigned int i;
	/** Loop through the program's arguments. **/
	for(i = 1; i < argc; i++){
		//If this argument is "-i", set the input file path to the argument that follows it.
		if(strcmp(argv[i], "-i") == 0){
			i++;
			if(i < argc){
				strcpy(inputPath, argv[i]);
				//If it doesn't end in a backslash or forwardslash, add one!
				if(inputPath[strlen(inputPath) - 1] != '\\' && inputPath[strlen(inputPath) - 1] != '/'){
					strcat(inputPath, "\\");
				}
			}
		}
		//If this argument is "-o", set the output file path to the argument that follows it.
		if(strcmp(argv[i], "-o") == 0){
			i++;
			if(i < argc){
				strcpy(outputPath, argv[i]);
				//If it doesn't end in a backslash or forwardslash, add one!
				if(outputPath[strlen(outputPath) - 1] != '\\' && outputPath[strlen(outputPath) - 1] != '/'){
					strcat(outputPath, "\\");
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


	DIR *fileDir;
	/** If the directory couldn't be opened, abort the program! **/
	if((fileDir = opendir(inputPath)) == NULL || errno == ENOENT){
		if(inputPath[0] == '\0'){
			printf("Input directory not specified, using the program's directory...\n\n");
		}else{
			printf("The specified directory could not be opened, using the program's directory...\n\n");
		}

		strncpy(inputPath, ".\\", 1000);
		fileDir = opendir(inputPath);
	}
	/** If an output path wasn't specified, set it to a folder called "out" within the inputPath. **/
	if(outputPath[0] == '\0'){
		strcpy(outputPath, inputPath);
		strcat(outputPath, "out\\");

		printf("No output path was specified, saving files to \"%s\".\n\n", outputPath);
	}
	//Be sure to create the output directory!
	mkdir(outputPath);


	struct dirent *dirFile;
	unsigned int successCount = 0;
	unsigned int failCount = 0;
	/** Loop throuh all the files in the specified directory. **/
	while((dirFile = readdir(fileDir)) != NULL){
		char fileName[1000] = "";
		//Combine the file's directory and name to get the full path.
		strcpy(fileName, inputPath);
		strcat(fileName, dirFile->d_name);


		FILE *oldFile;
		char *fileExtension = strrchr(dirFile->d_name, '.');
		/** Open the file and convert it if the extension is ".rip"! **/
		if(fileExtension != NULL && strcmp(fileExtension, ".rip") == 0 && (oldFile = fopen(fileName, "rb")) != NULL){
			//The new file's name should be the output path plus the original file name!
			char newName[1000] = "";
			strncpy(newName, dirFile->d_name, strlen(dirFile->d_name) - 3);


			printf("Currently converting \"%s\"...\n", fileName);
			if(convertRIP(oldFile, &objFlags[0], &outputPath[0], &newName[0])){
				printf("Conversion successful.\n\n");

				successCount++;
			}else{
				printf("Conversion unsuccessful.\n\n");

				remove(newName);

				failCount++;
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


	return(1);
}


long signed int readLong(FILE *file){
	unsigned char buffer[4];

	//Read four bytes from the file.
	fread(buffer, sizeof(char) * 4, 1, file);

	//'.rip' data is mostly little-endian, so we need to reverse it.
	return((long signed int)(buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]));
}

long unsigned int readULong(FILE *file){
	unsigned char buffer[4];

	//Read four bytes from the file.
	fread(buffer, sizeof(char) * 4, 1, file);

	//'.rip' data is mostly little-endian, so we need to reverse it.
	return((long unsigned int)(buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]));
}

float readFloat(FILE *file){
	unsigned char buffer[4];

	//Read four bytes from the file.
	fread(buffer, sizeof(char) * 4, 1, file);

	//'.rip' data is mostly little-endian, so we need to reverse it.
	return((float)(buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]));
}

void readString(char *stringBuffer, FILE *file){
	//Record the current position of the file pointer.
	long unsigned int curPos = ftell(file);
	//Add characters to stringBuffer until we reach a null terminator or the end of the file!
	fgets(stringBuffer, 1000, file);
	//Set the file pointer to the position after the null terminator we just reached!
	fseek(file, curPos + strlen(stringBuffer) + 1, SEEK_SET);
}


unsigned int checkSignature(FILE *file){
	//Check to see if the file's signature matches.
	if(readULong(file) == 0xDEADC0DE){
		printf("File signature is 0xDEADC0DE!\n");

		return(1);
	}


	printf("File signature incorrect, aborting...\n");
	return(0);
}

unsigned short checkVersion(FILE *file){
	//I only know how to work with version 4 '.rip' files.
	if(readULong(file) == 4){
		printf("'.rip' version is 4!\n");

		return(1);
	}


	printf("'.rip' version isn't 4, aborting...\n");
	return(0);
}

void getVertexAttribs(vector *typeVector, long unsigned int *posIndex, long unsigned int *normIndex, long unsigned int *uvIndex, long unsigned int numAttribs, FILE *file){
	//We only keep information for one of each different vertex attribute... for now!
	char gotEmPos  = 0;
	char gotEmNorm = 0;
	char gotEmUV   = 0;


	long unsigned int a;
	/** Loop through the vertex attribute information! **/
	for(a = 0; a < numAttribs; a++){
		char *curAttrib = malloc(sizeof(char) * 1000);
		readString(curAttrib, file);

		long unsigned int attribIndex  = readULong(file); //Currently unused.
		long unsigned int attribOffset = readULong(file);
		long unsigned int attribSize   = readULong(file); //Currently unused.
		long unsigned int numElements  = readULong(file);


		long unsigned int b;
		//Add the vertex attribute types to our vector!
		for(b = 0; b < numElements; b++){
			vectorAdd(typeVector, (void *)readULong(file));
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
			uvIndex[2] = uvIndex[0] + 2;

			gotEmUV = 1;
		}
    }
}

void getNames(vector *nameVector, long unsigned int totalNames, FILE *file){
	long unsigned int i;
	//Push the names into our vector!
	for(i = 0; i < totalNames; i++){
		char *curName = malloc(sizeof(char) * 1000);
		readString(curName, file);

		vectorAdd(nameVector, curName);
	}
}

void getFaces(vector *faceVector, long unsigned int totalFaces, FILE *file){
	long unsigned int i;
	for(i = 0; i < totalFaces; i++){
		long unsigned int curVert[3] = {0};
		//Store the indices for the current vertex.
		curVert[0] = readULong(file); curVert[1] = readULong(file); curVert[2] = readULong(file);

		//Now add them to our vector!
		vectorAdd(faceVector, (void *)curVert[0]); vectorAdd(faceVector, (void *)curVert[1]); vectorAdd(faceVector, (void *)curVert[2]);
	}
}

void getVertices(vector *posVector, vector *normVector, vector *uvVector, long unsigned int totalVerts, vector *attribTypes, long unsigned int *posIndex, long unsigned int *normIndex, long unsigned int *uvIndex, FILE *file){
	long unsigned int a;
	//Loop through all the vertices!
	for(a = 0; a < totalVerts; a++){
		float vertPos[3]  = {0};
		float vertNorm[3] = {0};
		float vertUV[2]   = {0};

		long unsigned int b;
		//Loop through all our vertex attributes!
		for(b = 0; b < vectorSize(attribTypes); b++){
			float curVal = 0.0f;
			/*
				The data type depends on what we've stored in attribTypes:

				0 = float
				1 = long unsigned int
				2 = long signed int
			*/
			switch((long unsigned int)vectorGet(attribTypes, b)){
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
				vertUV[1] = curVal;
			}
		}


		//Add the vertex information to our respective vectors!
		/** The problem is that I'm adding pointers to vertPos into the vector rather than the actual values. **/
		vectorAdd(posVector, &vertPos[0]);   vectorAdd(posVector, &vertPos[1]);   vectorAdd(posVector, &vertPos[2]);
		vectorAdd(normVector, &vertNorm[0]); vectorAdd(normVector, &vertNorm[1]); vectorAdd(normVector, &vertNorm[2]);
		vectorAdd(uvVector, &vertUV[0]);     vectorAdd(uvVector, &vertUV[1]);
	}
}


void writeMtl(vector *textureNames, char *objFlags, char *outputPath, char *fileName){
	char mtlName[1000];
	strcpy(mtlName, outputPath);
	strcat(mtlName, fileName);
	strcat(mtlName, "mtl\0");
	FILE *mtlFile = fopen(mtlName, "wb");


	long unsigned int i;
	//Create a very basic entry for each texture and write it to the file!
	for(i = 0; i < vectorSize(textureNames); i++){
		char *curName = (char *)vectorGet(textureNames, i);
		fprintf(mtlFile, "newmtl %s\r\nmap_Kd %s", curName, curName);

		//We don't want to have a blank line at the end of the file.
		if(i < vectorSize(textureNames) - 1){
			fwrite("\r\n", sizeof(char), sizeof("\r\n") - sizeof(char), mtlFile);
		}
	}


	fclose(mtlFile);
}

void writeObj(vector *faceVector, vector *posVector, vector *normVector, vector *uvVector, vector *textureNames, char *objFlags, char *outputPath, char *fileName){
	char objName[1000];
	strcpy(objName, outputPath);
	strcat(objName, fileName);
	strcat(objName, "obj\0");
	FILE *objFile = fopen(objName, "wb");


	/** Write '.mtl' File **/
	//If we're not ignoring UVs and the model uses atleast one texture, create a '.mtl' file!
	if(vectorSize(textureNames) > 0 && !objFlags[0]){
		fprintf(objFile, "mtllib %smtl\r\n", fileName);

		writeMtl(textureNames, objFlags, outputPath, fileName);

		//We only use the first texture... for now!
		fprintf(objFile, "usemtl %s\r\n", (char *)vectorGet(textureNames, 0));
	}


	long unsigned int a;
	//Write the vertex positions to the file!
	for(a = 0; a < vectorSize(posVector) / 3; a++){
		//Then write it to the file!
		fprintf(objFile, "v %f %f %f\r\n", *(float *)vectorGet(posVector, a * 3), *(float *)vectorGet(posVector, a * 3 + 1), *(float *)vectorGet(posVector, a * 3 + 2));
	}

	//Write the vertex UV coordinates to the file if we're not ignoring them!
	if(!objFlags[0]){
		for(a = 0; a < vectorSize(uvVector) / 2; a++){
			fprintf(objFile, "vt %f %f\r\n", *(float *)vectorGet(uvVector, a * 2), *(float *)vectorGet(uvVector, a * 2 + 1));
		}
	}

	//Write the vertex normals to the file if we're not ignoring them!
	if(!objFlags[1]){
		for(a = 0; a < vectorSize(normVector) / 3; a++){
			fprintf(objFile, "vn %f %f %f\r\n", *(float *)vectorGet(normVector, a * 3), *(float *)vectorGet(normVector, a * 3 + 1), *(float *)vectorGet(normVector, a * 3 + 2));
		}
	}


	//Write the faces to the file!
	for(a = 0; a < vectorSize(faceVector) / 3; a++){
		fwrite("f", sizeof(char), sizeof("f") - sizeof(char), objFile);

		long unsigned int b;
		for(b = 0; b < 3; b++){
			//Write the position's indices to the file!
			fprintf(objFile, " %ld/", (long unsigned int)vectorGet(faceVector, a * 3 + b) + 1);

			//If we're not ignoring UVs, write their indices to the file!
			if(!objFlags[0]){
				fprintf(objFile, "%ld", (long unsigned int)vectorGet(faceVector, a * 3 + b) + 1);
			}
			//If we're not ignoring normals, write their indices to the file!
			if(!objFlags[1]){
				fprintf(objFile, "/%ld", (long unsigned int)vectorGet(faceVector, a * 3 + b) + 1);
			}
		}

		//We don't want to have a blank line at the end of the file.
		if(a < vectorSize(faceVector) / 3 - 1){
			fwrite("\r\n", sizeof(char), sizeof("\r\n") - sizeof(char), objFile);
		}
	}


	fclose(objFile);
}


//Now we declare this function!
unsigned short convertRIP(FILE *oldFile, char *objFlags, char *outputPath, char *fileName){
	/** Make sure the file's signature and version number match! **/
	if(!checkSignature(oldFile) || !checkVersion(oldFile)){
		return(0);
	}


	//Good thing we understand how '.rip' files work, right?
	//...right?
	long unsigned int totalFaces    = readULong(oldFile);
    long unsigned int totalVerts    = readULong(oldFile);
    long unsigned int vertSize      = readULong(oldFile); //Currently unused.
    long unsigned int totalTextures = readULong(oldFile);
    long unsigned int totalShaders  = readULong(oldFile);
    long unsigned int totalAttribs  = readULong(oldFile);


	/** Vertex Attributes **/
	//Contains the data types for the different vertex attributes.
	vector attribTypes;
	vectorInit(&attribTypes);

	long unsigned int posIndex[3]  = {0};
	long unsigned int normIndex[3] = {0};
	long unsigned int uvIndex[2]   = {0};

	//Get the information for out vertex attributes!
	getVertexAttribs(&attribTypes, &posIndex[0], &normIndex[0], &uvIndex[0], totalAttribs, oldFile);


	/** Texture Names **/
	//Contains the names of each texture that the model uses!
	vector textureNames;
	vectorInit(&textureNames);

	//Now find and store 'em!
	getNames(&textureNames, totalTextures, oldFile);


	/** Shader Names - Currently Unused **/
	//Contains the names of each shader that the model uses!
	vector shaderNames;
	vectorInit(&shaderNames);

	//Now find and store 'em!
	getNames(&shaderNames, totalShaders, oldFile);


	/** Face Information **/
	//Contains the ordered indices for the vertices of each face!
	vector faceVector;
	vectorInit(&faceVector);

	//Now, you guessed it, find and store 'em!
	getFaces(&faceVector, totalFaces, oldFile);


	/** Vertex Information **/
	//Contains the position for each vertex!
	vector posVector;
	vectorInit(&posVector);
	//Contains the normals for each vertex!
	vector normVector;
	vectorInit(&normVector);
	//Contains the UV coordinates for each vertex!
	vector uvVector;
	vectorInit(&uvVector);

	//Load the vertex information!
	getVertices(&posVector, &normVector, &uvVector, totalVerts, &attribTypes, &posIndex[0], &normIndex[0], &uvIndex[0], oldFile);


	/** Write '.obj' File **/
	writeObj(&faceVector, &posVector, &normVector, &uvVector, &textureNames, objFlags, outputPath, fileName);


	//Free the memory held by our vectors!
	vectorClear(&attribTypes);
	vectorClear(&textureNames);
	vectorClear(&shaderNames);
	vectorClear(&faceVector);
	vectorClear(&posVector);
	vectorClear(&normVector);
	vectorClear(&uvVector);


	return(1);
}