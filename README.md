# Batch RIP to OBJ Converter
To use this program, create a batch file in the same directory as it with the following contents:
```
BatchRipToObj -i "inputDir" -o "outputDir"
```
Where "inputDir" is the directory containing the files you want to convert, and "outputDir" is where to put the converted files.

Additional paramaters include:
```
-ignoreUVs     - OBJs will not contain UV coordinates.
-ignoreNormals - OBJs will not contain normals.
```