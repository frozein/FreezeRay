/* ------------------------------------------------------------------------
 * 
 * quickobj.h
 * author: Daniel Elwell (2023)
 * license: MIT
 * description: a single-header library for loading 3D meshes from .obj files, and their
 * associated materials from .mtl files
 * 
 * ------------------------------------------------------------------------
 * 
 * NOTE: only supports .obj files with triangles and quads, n-gons will result in an error
 * 
 * to use you must "#define QOBJ_IMPLEMENTATION" in exactly one source file before including the library
 * 
 * the following strutures and functions are defined for end use:
 * (all other functions/structures are meant for internal library use only and do not have documentation)
 * 
 * STRUCTURES:
 * ------------------------------------------------------------------------
 * QOBJvec2, QOBJvec3:
 * 		vectors of floats
 * QOBJvertex 
 * 		a single triangle vertex
 * 		layout: position (vec3); normal (vec3); texture coordinate (vec2)
 * QOBJmaterial
 * 		a single material (non-PBR)
 * 		contains:
 * 			ambient color (vec3); diffuse color (vec3); specular color (vec3)
 * 			ambient color map (char*); diffuse color map (char*); specular color map (char*); normal map (char*)
 * 			opacity (float); shininess/specular exponent (float); refraction index (float)
 * 		NOTE: if a map is NULL, then it does not exist, otherwise, it contains the path to the texture
 * QOBJmesh
 * 		a mesh with a single material, uses an index buffer
 * 		contains:
 * 			number of vertices (size_t); vertex buffer capacity (size_t) (for internal use, please ignore)
 * 			array of vertices (QOBJvertex*)
 * 			number of indices (size_t); index buffer capacity (size_t) (for internal use, please ignore)
 * 			array of indices (uint32_t*)
 * 			material index (uint32_t)
 * 		NOTE: the mesh MUST be rendered with the index buffer
 * 
 * FUNCTIONS:
 * ------------------------------------------------------------------------
 * qobj_load(const char* path, size_t* numMeshes, QOBJmesh** meshes, size_t* numMaterials, QOBJmaterial** materials)
 * 		loads a .obj file from [path], including any necessary .mtl files
 * 		the [numMeshes] and [numMaterials] fields are populated with the number of meshes and materials loaded. respectively
 * 		the [meshes] and [materials] fields are populated with the array of meshes and materials loaded, respectively
 * 		NOTE: in order to render the entire model, you must render each mesh in the array, using its corresponding material (the material index field)
 * qobj_free(size_t numMeshes, QOBJmesh* meshes, size_t numMaterials, QOBJmaterial* materials)
 * 		frees the memory created by a call to qobj_load, must be called in order to prevent memory leaks
 */

#ifndef QOBJ_H
#define QOBJ_H

#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdint.h>

//----------------------------------------------------------------------//
//DECLARATIONS:

//an RGB color
typedef struct QOBJcolor
{
	float r, g, b;
} QOBJcolor;

//a material (not pbr)
typedef struct QOBJmaterial
{
	char* name;

	QOBJcolor ambientColor;
	QOBJcolor diffuseColor;
	QOBJcolor specularColor;
	char* ambientMapPath;  //== NULL if one does not exist
	char* diffuseMapPath;  //== NULL if one does not exist
	char* specularMapPath; //== NULL if one does not exist
	char* normalMapPath;   //== NULL if one does not exist

	float opacity;
	float specularExp;
	float refractionIndex;
} QOBJmaterial;

//a mesh consisting of vertices and indices, contains only triangles
typedef struct QOBJmesh
{
	uint32_t vertexAttribs;        //bitfield of QOBJvertexAttributes
	uint32_t vertexStride;         //size of each vertex in number of floats
	uint32_t vertexPosOffset;      //offset of the position attribute in number of floats (or UINT32_MAX if no positions given)
	uint32_t vertexNormalOffset;   //offset of the normal attribute in number of floats (or UINT32_MAX if no normals given)
	uint32_t vertexTexCoordOffset; //offset of the texture coordinate attribute in number of floats (or UINT32_MAX if no tex coords given)

	size_t numVertices;
	size_t vertexCap;
	float* vertices;

	size_t numIndices; //mesh only contains triangles, so the number of tris is numIndices / 3
	size_t indexCap;
	uint32_t* indices;

	char* material;
} QOBJmesh;

//an error value, returned by all functions which can have errors
typedef enum QOBJerror
{
	QOBJ_SUCCESS = 0,
	QOBJ_ERROR_INVALID_FILE,
	QOBJ_ERROR_IO,
	QOBJ_ERROR_OUT_OF_MEM,
	QOBJ_ERROR_UNSUPPORTED_DATA_TYPE
} QOBJerror;

typedef enum QOBJvertexAttributes
{
	QOBJ_VERTEX_ATTRIB_POSITION   = (1 << 0),
	QOBJ_VERTEX_ATTRIB_NORMAL     = (1 << 1),
	QOBJ_VERTEX_ATTRIB_TEX_COORDS = (1 << 2)
} QOBJvertexAttributes;

QOBJerror qobj_load(const char* path, size_t* numMeshes, QOBJmesh** meshes, size_t* numMaterials, QOBJmaterial** materials);
void qobj_free(size_t numMeshes, QOBJmesh* meshes, size_t numMaterials, QOBJmaterial* materials);

//----------------------------------------------------------------------//

#ifdef QOBJ_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define QOBJ_ATTRIB_SIZE_POSITION   3
#define QOBJ_ATTRIB_SIZE_NORMAL     3
#define QOBJ_ATTRIB_SIZE_TEX_COORDS 2

//----------------------------------------------------------------------//
//IMPLEMENTATION STRUCTS:

//a reference to a vertex (specified in the "f" command in an OBJ file)
typedef struct QOBJvertexRef
{
	uint32_t pos;
	uint32_t normal;
	uint32_t texCoord;
} QOBJvertexRef;

//a hashmap with a vec3 of vertex data indices for keys
typedef struct QOBJvertexHashmap
{
	size_t size;
	size_t cap;
	QOBJvertexRef* keys; //an x component of UINT32_MAX signifies an unused index
	uint32_t* vals;
} QOBJvertexHashmap;

//----------------------------------------------------------------------//
//HASH MAP FUNCTIONS:

QOBJerror qobj_hashmap_create(QOBJvertexHashmap* map)
{
	map->size = 0;
	map->cap = 32;
	map->keys = (QOBJvertexRef*)malloc(map->cap * sizeof(QOBJvertexRef));
	if(!map->keys)
		return QOBJ_ERROR_OUT_OF_MEM;
	map->vals = (uint32_t*)malloc(map->cap * sizeof(uint32_t));
	if(!map->vals)
	{
		free(map->keys);
		return QOBJ_ERROR_OUT_OF_MEM;
	}

	memset(map->keys, UINT8_MAX, map->cap * sizeof(QOBJvertexRef));

	return QOBJ_SUCCESS;
}

void qobj_hashmap_free(QOBJvertexHashmap map)
{
	free(map.keys);
	free(map.vals);
}

inline size_t qobj_hashmap_hash(QOBJvertexRef key)
{
	return 12637 * key.pos + 16369 * key.normal + 20749 * key.texCoord;
}

QOBJerror qobj_hashmap_get_or_add(QOBJvertexHashmap* map, QOBJvertexRef key, uint32_t* val)
{
	//get hash:
	size_t hash = qobj_hashmap_hash(key) % map->cap;

	//linear probing:
	int32_t found = 0;
	while(map->keys[hash].pos != UINT32_MAX)
	{		
		if(map->keys[hash].pos == key.pos && map->keys[hash].normal == key.normal && map->keys[hash].texCoord == key.texCoord)
		{
			found = 1;
			break;
		}

		hash++;
		hash %= map->cap;
	}

	if(found)
		*val = map->vals[hash];
	else
	{
		map->keys[hash] = key;
		map->vals[hash] = *val;
		map->size++;
	}

	//resize and rehash if needed:
	if(map->size >= map->cap / 2)
	{
		size_t oldCap = map->cap;
		map->cap *= 2;

		QOBJvertexRef* newKeys = (QOBJvertexRef*)malloc(map->cap * sizeof(QOBJvertexRef));
		if(!newKeys)
			return QOBJ_ERROR_OUT_OF_MEM;
		uint32_t* newVals = (uint32_t*)malloc(map->cap * sizeof(uint32_t));
		if(!newVals)
		{
			free(newKeys);
			return QOBJ_ERROR_OUT_OF_MEM;
		}

		memset(newKeys, UINT8_MAX, map->cap * sizeof(QOBJvertexRef));

		for(uint32_t i = 0; i < oldCap; i++)
		{
			if(map->keys[i].pos == UINT32_MAX)
				continue;

			size_t newHash = qobj_hashmap_hash(map->keys[i]) % map->cap;
			newKeys[newHash] = map->keys[i];
			newVals[newHash] = map->vals[i];
		}

		free(map->keys);
		free(map->vals);
		map->keys = newKeys;
		map->vals = newVals;
	}

	return QOBJ_SUCCESS;
}

#define QOBJ_MAX_TOKEN_LEN 128

//----------------------------------------------------------------------//
//MESH FUNCTIONS

QOBJerror qobj_mesh_create(QOBJmesh* mesh, uint32_t vertexAttribs, const char* materialName)
{
	//determine strides and offsets of attributes:
	mesh->vertexAttribs = vertexAttribs;
	mesh->vertexStride = 0;

	if(mesh->vertexAttribs & QOBJ_VERTEX_ATTRIB_POSITION)
	{
		mesh->vertexPosOffset = mesh->vertexStride;
		mesh->vertexStride += QOBJ_ATTRIB_SIZE_POSITION;
	}
	else
		mesh->vertexPosOffset = UINT32_MAX;

	if(mesh->vertexAttribs & QOBJ_VERTEX_ATTRIB_NORMAL)
	{
		mesh->vertexNormalOffset = mesh->vertexStride;
		mesh->vertexStride += QOBJ_ATTRIB_SIZE_NORMAL;
	}
	else
		mesh->vertexNormalOffset = UINT32_MAX;
	
	if(mesh->vertexAttribs & QOBJ_VERTEX_ATTRIB_TEX_COORDS)
	{
		mesh->vertexTexCoordOffset = mesh->vertexStride;
		mesh->vertexStride += QOBJ_ATTRIB_SIZE_TEX_COORDS;
	}
	else
		mesh->vertexTexCoordOffset = UINT32_MAX;

	//allocate data:
	mesh->vertexCap   = 32;
	mesh->indexCap    = 32;
	mesh->numVertices = 0;
	mesh->numIndices  = 0;

	mesh->vertices = (float*)malloc(mesh->vertexCap * sizeof(float) * mesh->vertexStride);
	if(!mesh->vertices)
		return QOBJ_ERROR_OUT_OF_MEM;

	mesh->indices = (uint32_t*)malloc(mesh->indexCap * sizeof(uint32_t));
	if(!mesh->indices)
	{
		free(mesh->vertices);
		return QOBJ_ERROR_OUT_OF_MEM;
	}

	//copy material name:
	uint32_t nameSize = (uint32_t)strlen(materialName) + 1;
	mesh->material = (char*)malloc(nameSize);
	if(!mesh->material)
	{
		free(mesh->vertices);
		free(mesh->indices);
		return QOBJ_ERROR_OUT_OF_MEM;
	}

	strcpy_s(mesh->material, nameSize, materialName);

	return QOBJ_SUCCESS;
}

void qobj_mesh_free(QOBJmesh mesh)
{
	free(mesh.vertices);
	free(mesh.indices);
	free(mesh.material);
}

//----------------------------------------------------------------------//
//HELPER FUNCTIONS:

inline void qobj_next_token(FILE* fptr, char* token, char* endCh)
{
	char curCh;
	size_t curLen = 0;

	while(1)
	{
		if(curLen >= QOBJ_MAX_TOKEN_LEN)
		{
			curLen--;
			break;
		}

		curCh = fgetc(fptr);

		if(isspace(curCh) || curCh == EOF)
			break;

		token[curLen++] = curCh;
	}

	token[curLen] = '\0';
	*endCh = curCh;
}

inline void qobj_fgets(FILE* fptr, char* token, char* endCh)
{
	fgets(token, QOBJ_MAX_TOKEN_LEN, fptr);

	size_t last = strlen(token) - 1;
	if(token[last] == '\n')
	{
		*endCh = token[last];
		token[last] = '\0';
	}
	else
		*endCh = EOF;
}

//----------------------------------------------------------------------//
//MTL FUNCTIONS:

QOBJmaterial qobj_default_material()
{
	QOBJmaterial result = {0};

	result.opacity = 1.0f;
	result.specularExp = 1.0f;
	result.refractionIndex = 1.0f;

	return result;
}

void qobj_mtl_free(size_t numMaterials, QOBJmaterial* materials)
{
	for(uint32_t i = 0; i < numMaterials; i++)
	{
		if(materials[i].name)
			free(materials[i].name);

		if(materials[i].ambientMapPath)
			free(materials[i].ambientMapPath);
		if(materials[i].diffuseMapPath)
			free(materials[i].diffuseMapPath);
		if(materials[i].specularMapPath)
			free(materials[i].specularMapPath);
		if(materials[i].normalMapPath)
			free(materials[i].normalMapPath);
	}

	if(materials)
		free(materials);
}

QOBJerror qobj_mtl_load(const char* path, size_t* numMaterials, QOBJmaterial** materials)
{
	//ensure file is valid and able to be opened:
	size_t pathLen = strlen(path);
	if(pathLen < 4 || strcmp(&path[pathLen - 4], ".mtl") != 0)
		return QOBJ_ERROR_INVALID_FILE;

	FILE* fptr = fopen(path, "r");
	if(!fptr)
		return QOBJ_ERROR_IO;

	QOBJerror errorCode = QOBJ_SUCCESS;

	//allocate memory:
	*materials = (QOBJmaterial*)malloc(sizeof(QOBJmaterial));
	*numMaterials = 0;
	size_t curMaterial = 0;

	if(!*materials)
	{
		errorCode = QOBJ_ERROR_OUT_OF_MEM;
		goto cleanup;
	}

	//main loop:
	char curToken[QOBJ_MAX_TOKEN_LEN];
	char curTokenEnd;

	while(1)
	{
		qobj_next_token(fptr, curToken, &curTokenEnd);

		if(curTokenEnd == EOF)
			break;

		if(curToken[0] == '\0')
			continue;

		if(curToken[0] == '#' || strcmp(curToken, "illum") == 0 ||
		   strcmp(curToken, "Tf") == 0) //comments / ignored commands
		{
			if(curTokenEnd == ' ')
				qobj_fgets(fptr, curToken, &curTokenEnd);
		}
		else if(strcmp(curToken, "newmtl") == 0)
		{
			qobj_fgets(fptr, curToken, &curTokenEnd);

			curMaterial = *numMaterials;
			(*numMaterials)++;
			*materials = (QOBJmaterial*)realloc(*materials, *numMaterials * sizeof(QOBJmaterial));
			if(!*materials)
			{
				errorCode = QOBJ_ERROR_OUT_OF_MEM;
				goto cleanup;
			}

			(*materials)[curMaterial] = qobj_default_material();
			(*materials)[curMaterial].name = (char*)malloc(QOBJ_MAX_TOKEN_LEN * sizeof(char));
			strcpy((*materials)[curMaterial].name, curToken);
		}
		else if(strcmp(curToken, "Ka") == 0)
		{
			QOBJcolor col;
			fscanf(fptr, "%f %f %f\n", &col.r, &col.g, &col.b);

			(*materials)[curMaterial].ambientColor = col;
		}
		else if(strcmp(curToken, "Kd") == 0)
		{
			QOBJcolor col;
			fscanf(fptr, "%f %f %f\n", &col.r, &col.g, &col.b);

			(*materials)[curMaterial].diffuseColor = col;
		}
		else if(strcmp(curToken, "Ks") == 0)
		{
			QOBJcolor col;
			fscanf(fptr, "%f %f %f\n", &col.r, &col.g, &col.b);

			(*materials)[curMaterial].specularColor = col;
		}
		else if(strcmp(curToken, "d") == 0)
		{
			float opacity;
			fscanf(fptr, "%f\n", &opacity);

			(*materials)[curMaterial].opacity = opacity;
		}
		else if(strcmp(curToken, "Ns") == 0)
		{
			float specularExp;
			fscanf(fptr, "%f\n", &specularExp);

			(*materials)[curMaterial].specularExp = specularExp;
		}
		else if(strcmp(curToken, "Ni") == 0)
		{
			float refractionIndex;
			fscanf(fptr, "%f\n", &refractionIndex);

			(*materials)[curMaterial].refractionIndex = refractionIndex;
		}
		else if(strcmp(curToken, "map_Ka") == 0)
		{
			char* mapPath = (char*)malloc(QOBJ_MAX_TOKEN_LEN * sizeof(char));
			qobj_fgets(fptr, mapPath, &curTokenEnd);

			(*materials)[curMaterial].ambientMapPath = mapPath;
		}
		else if(strcmp(curToken, "map_Kd") == 0)
		{
			char* mapPath = (char*)malloc(QOBJ_MAX_TOKEN_LEN * sizeof(char));
			qobj_fgets(fptr, mapPath, &curTokenEnd);

			(*materials)[curMaterial].diffuseMapPath = mapPath;
		}
		else if(strcmp(curToken, "map_Ks") == 0)
		{
			char* mapPath = (char*)malloc(QOBJ_MAX_TOKEN_LEN * sizeof(char));
			qobj_fgets(fptr, mapPath, &curTokenEnd);

			(*materials)[curMaterial].specularMapPath = mapPath;
		}
		else if(strcmp(curToken, "map_Bump") == 0)
		{
			char* mapPath = (char*)malloc(QOBJ_MAX_TOKEN_LEN * sizeof(char));
			qobj_fgets(fptr, mapPath, &curTokenEnd);

			(*materials)[curMaterial].normalMapPath = mapPath;
		}
	}

	cleanup: ;

	if(errorCode != QOBJ_SUCCESS)
	{
		qobj_mtl_free(*numMaterials, *materials);
		*numMaterials = 0;
	}

	fclose(fptr);
	return errorCode;	
}

//----------------------------------------------------------------------//
//OBJ FUNCTIONS:

QOBJerror qobj_load(const char* path, size_t* numMeshes, QOBJmesh** meshes, size_t* numMaterials, QOBJmaterial** materials)
{
	*numMeshes = 0;
	*meshes = NULL;
	*numMaterials = 0;
	*materials = NULL;

	//ensure file is valid and able to be opened:
	size_t pathLen = strlen(path);
	if(pathLen < 4 || strcmp(&path[pathLen - 4], ".obj") != 0)
		return QOBJ_ERROR_INVALID_FILE;

	FILE* fptr = fopen(path, "r");
	if(!fptr)
		return QOBJ_ERROR_IO;

	QOBJerror errorCode = QOBJ_SUCCESS;

	//allocate memory:
	size_t positionSize = 0 , normalSize = 0 , texCoordSize = 0;
	size_t positionCap  = 32, normalCap  = 32, texCoordCap  = 32;
	float* positions = (float*)malloc(positionCap * sizeof(float) * QOBJ_ATTRIB_SIZE_POSITION);
	float* normals   = (float*)malloc(normalCap   * sizeof(float) * QOBJ_ATTRIB_SIZE_NORMAL);
	float* texCoords = (float*)malloc(texCoordCap * sizeof(float) * QOBJ_ATTRIB_SIZE_TEX_COORDS);

	*numMeshes = 0;
	*meshes = (QOBJmesh*)malloc(sizeof(QOBJmesh));
	QOBJvertexHashmap* meshVertexMaps = (QOBJvertexHashmap*)malloc(sizeof(QOBJvertexHashmap));

	//ensure memory was properly allocated:
	if(!positions || !normals || !texCoords || !*meshes || !meshVertexMaps)
	{
		free(positions);
		free(normals);
		free(texCoords);

		free(meshes);
		free(meshVertexMaps);

		fclose(fptr);

		return QOBJ_ERROR_OUT_OF_MEM;
	}

	//main loop:
	char curToken[QOBJ_MAX_TOKEN_LEN];
	char curTokenEnd;

	char curMaterial[QOBJ_MAX_TOKEN_LEN] = {}; //no material specified (yet)
	uint32_t curMesh = UINT32_MAX;             //no working mesh (yet)

	while(1)
	{
		qobj_next_token(fptr, curToken, &curTokenEnd);

		if(curTokenEnd == EOF)
			break;

		if(curToken[0] == '\0')
			continue;

		if(curToken[0] == '#'         || strcmp(curToken, "o") == 0 || 
		   strcmp(curToken, "g") == 0 || strcmp(curToken, "s") == 0) //comments / ignored commands
		{
			if(isspace(curTokenEnd))
				qobj_fgets(fptr, curToken, &curTokenEnd);
		}
		else if(strcmp(curToken, "v") == 0)
		{
			uint32_t insertIdx = (uint32_t)positionSize++ * QOBJ_ATTRIB_SIZE_POSITION;

			fscanf(fptr, "%f %f %f\n", 
				&positions[insertIdx + 0], 
				&positions[insertIdx + 1], 
				&positions[insertIdx + 2]
			);

			if(positionSize >= positionCap)
			{
				positionCap *= 2;
				positions = (float*)realloc(positions, positionCap * sizeof(float) * QOBJ_ATTRIB_SIZE_POSITION);
				if(!positions)
				{
					errorCode = QOBJ_ERROR_OUT_OF_MEM;
					break;
				}
			}
		}
		else if(strcmp(curToken, "vn") == 0)
		{
			uint32_t insertIdx = (uint32_t)normalSize++ * QOBJ_ATTRIB_SIZE_NORMAL;

			fscanf(fptr, "%f %f %f\n", 
				&normals[insertIdx + 0], 
				&normals[insertIdx + 1], 
				&normals[insertIdx + 2]
			);

			if(normalSize >= normalCap)
			{
				normalCap *= 2;
				normals = (float*)realloc(normals, normalCap * sizeof(float) * QOBJ_ATTRIB_SIZE_NORMAL);
				if(!normals)
				{
					errorCode = QOBJ_ERROR_OUT_OF_MEM;
					break;
				}
			}
		}
		else if(strcmp(curToken, "vt") == 0)
		{
			//TODO: v is optional, account for this

			uint32_t insertIdx = (uint32_t)texCoordSize++ * QOBJ_ATTRIB_SIZE_TEX_COORDS;

			fscanf(fptr, "%f %f\n", 
				&texCoords[insertIdx + 0], 
				&texCoords[insertIdx + 1]
			);

			if(texCoordSize >= texCoordCap)
			{
				texCoordCap *= 2;
				texCoords = (float*)realloc(texCoords, texCoordCap * sizeof(float) * QOBJ_ATTRIB_SIZE_TEX_COORDS);
				if(!texCoords)
				{
					errorCode = QOBJ_ERROR_OUT_OF_MEM;
					break;
				}
			}
		}
		else if(strcmp(curToken, "f") == 0)
		{
			//if no mesh is active yet, try to find an existing mesh with the same material:
			if(curMesh == UINT32_MAX)
				for(uint32_t i = 0; i < *numMeshes; i++)
				{
					if(strcmp(curMaterial, meshes[i]->material) == 0)
					{
						curMesh = i;
						break;
					}
				}

			//if no valid active mesh was found, create a new one:
			if(curMesh == UINT32_MAX)
			{
				//set curMesh:
				curMesh = (uint32_t)*numMeshes;

				//determine vertex attributes to include:

				//TODO: base this off of the first face instead!

				uint32_t vertexAttribs = 0;
				if(positionSize > 0)
					vertexAttribs |= QOBJ_VERTEX_ATTRIB_POSITION;
				if(normalSize > 0)
					vertexAttribs |= QOBJ_VERTEX_ATTRIB_NORMAL;
				if(texCoordSize > 0)
					vertexAttribs |= QOBJ_VERTEX_ATTRIB_TEX_COORDS;

				//allocate mem and create new mesh:
				(*numMeshes)++;
				*meshes = (QOBJmesh*)realloc(*meshes, *numMeshes * sizeof(QOBJmesh));
				meshVertexMaps = (QOBJvertexHashmap*)realloc(meshVertexMaps, *numMeshes * sizeof(QOBJvertexHashmap));

				if(!*meshes || !meshVertexMaps)
				{
					errorCode = QOBJ_ERROR_OUT_OF_MEM;
					break;
				}

				QOBJerror meshCreateError = qobj_mesh_create(&(*meshes)[curMesh], vertexAttribs, curMaterial);
				if(meshCreateError != QOBJ_SUCCESS)
				{
					errorCode = meshCreateError;
					break;
				}

				meshCreateError = qobj_hashmap_create(&meshVertexMaps[curMesh]);
				if(meshCreateError != QOBJ_SUCCESS)
				{
					errorCode = meshCreateError;
					break;
				}
			}

			//read face data:
			uint32_t numVertices = 0; //TODO: support n-gons, not just quads
			QOBJvertexRef vertices[6];

			while(numVertices < 4)
			{
				uint32_t vert = numVertices++;

				//read position:
				if(fscanf(fptr, "%d", &vertices[vert].pos) < 1)
				{
					numVertices--;
					break;
				}

				char nextCh = fgetc(fptr);
				if(nextCh != '/')
				{
					vertices[vert].normal = 0;
					vertices[vert].texCoord = 0;
					continue;
				}

				//read normal (if no texture coordinates exist):
				nextCh = fgetc(fptr);
				if(nextCh == '/')
				{
					fscanf(fptr, "%d", &vertices[vert].normal);
					
					vertices[vert].texCoord = 0;
					continue;
				}
				else
					ungetc(nextCh, fptr);
				
				//read texture coordinates:
				fscanf(fptr, "%d", &vertices[vert].texCoord);
					
				//read normal (if texture coordinates exist):
				nextCh = fgetc(fptr);
				if(nextCh == '/')
					fscanf(fptr, "%d", &vertices[vert].normal);
				else
				{
					ungetc(nextCh, fptr);

					vertices[vert].normal = 0;
				}
			}

			//split quad into 2 triangles:
			if(numVertices == 4)
			{
				QOBJvertexRef temp = vertices[3];
				vertices[3] = vertices[0];
				vertices[4] = vertices[2];
				vertices[5] = temp;

				numVertices = 6;
			}

			QOBJmesh* mesh = &(*meshes)[curMesh];
			QOBJvertexHashmap* map = &meshVertexMaps[curMesh]; 

			//resize if needed:
			if(mesh->numIndices + numVertices >= mesh->indexCap)
			{
				mesh->indexCap *= 2;
				mesh->indices = (uint32_t*)realloc(mesh->indices, mesh->indexCap * sizeof(uint32_t));
				if(!mesh->indices)
				{
					errorCode = QOBJ_ERROR_OUT_OF_MEM;
					break;
				}
			}

			if(mesh->numVertices + numVertices >= mesh->vertexCap)
			{
				//potentially wasteful since we dont necessarily add each vertex (can be reused)

				mesh->vertexCap *= 2;
				mesh->vertices = (float*)realloc(mesh->vertices, mesh->vertexCap * sizeof(float) * mesh->vertexStride);
				if(!mesh->vertices)
				{
					errorCode = QOBJ_ERROR_OUT_OF_MEM;
					break;
				}
			}

			//add vertices + indices:
			for(uint32_t i = 0; i < numVertices; i++)
			{
				uint32_t indexToAdd = (uint32_t)mesh->numVertices;
				qobj_hashmap_get_or_add(map, vertices[i], &indexToAdd); //TODO: FIX!!!!

				mesh->indices[mesh->numIndices++] = indexToAdd;
				if(indexToAdd < (uint32_t)mesh->numVertices) //skip if vertex already exists in mesh
					continue;

				uint32_t insertIdx = (uint32_t)mesh->numVertices++ * mesh->vertexStride;

				if(mesh->vertexAttribs & QOBJ_VERTEX_ATTRIB_POSITION)
				{
					uint32_t posIdx = (vertices[i].pos - 1) * QOBJ_ATTRIB_SIZE_POSITION; //.obj files are 1-indexed

					mesh->vertices[insertIdx + mesh->vertexPosOffset + 0] = positions[posIdx + 0];
					mesh->vertices[insertIdx + mesh->vertexPosOffset + 1] = positions[posIdx + 1];
					mesh->vertices[insertIdx + mesh->vertexPosOffset + 2] = positions[posIdx + 2];
				}

				if(mesh->vertexAttribs & QOBJ_VERTEX_ATTRIB_NORMAL)
				{
					uint32_t normalIdx = (vertices[i].normal - 1) * QOBJ_ATTRIB_SIZE_NORMAL; //.obj files are 1-indexed

					mesh->vertices[insertIdx + mesh->vertexNormalOffset + 0] = normals[normalIdx + 0];
					mesh->vertices[insertIdx + mesh->vertexNormalOffset + 1] = normals[normalIdx + 1];
					mesh->vertices[insertIdx + mesh->vertexNormalOffset + 2] = normals[normalIdx + 2];
				}

				if(mesh->vertexAttribs & QOBJ_VERTEX_ATTRIB_TEX_COORDS)
				{
					uint32_t texCoordIdx = (vertices[i].texCoord - 1) * QOBJ_VERTEX_ATTRIB_TEX_COORDS; //.obj files are 1-indexed

					mesh->vertices[insertIdx + mesh->vertexTexCoordOffset + 0] = texCoords[texCoordIdx + 0];
					mesh->vertices[insertIdx + mesh->vertexTexCoordOffset + 1] = texCoords[texCoordIdx + 1];
				}
			}
		}
		else if(strcmp(curToken, "usemtl") == 0)
		{
			qobj_fgets(fptr, curToken, &curTokenEnd);
			
			strcpy_s(curMaterial, QOBJ_MAX_TOKEN_LEN, curToken);
			curMesh = UINT32_MAX;
		}
		else if(strcmp(curToken, "mtllib") == 0)
		{
			qobj_fgets(fptr, curToken, &curTokenEnd);
			
			//get directory of current file:
			uint32_t i = 0;
			uint32_t lastSlash = 0;

			while(path[i])
			{
				if(path[i] == '/' || path[i] == '\\')
					lastSlash = i;

				i++;
			}

			char mtlPath[QOBJ_MAX_TOKEN_LEN];
			strncpy(mtlPath, path, lastSlash + 1);
			mtlPath[lastSlash + 1] = '\0';
			strcat(mtlPath, curToken);

			QOBJerror mtlError = qobj_mtl_load(mtlPath, numMaterials, materials);
			if(mtlError != QOBJ_SUCCESS) //TODO: decide if we should actually throw an error on failed mtl file load since materials arent fully necessary
			{
				errorCode = mtlError;
				break;
			}
		}
		else
		{
			errorCode = QOBJ_ERROR_UNSUPPORTED_DATA_TYPE;
			break;
		}
	}

	//cleanup:
	for(uint32_t i = 0; i < *numMeshes; i++)
		qobj_hashmap_free(meshVertexMaps[i]);

	free(meshVertexMaps);

	if(errorCode != QOBJ_SUCCESS)
	{
		qobj_free(*numMeshes, *meshes, *numMaterials, *materials);
		*numMeshes = 0;
	}

	free(positions);
	free(normals);
	free(texCoords);

	fclose(fptr);
	return errorCode;
}

void qobj_free(size_t numMeshes, QOBJmesh* meshes, size_t numMaterials, QOBJmaterial* materials)
{
	for(uint32_t i = 0; i < numMeshes; i++)
		qobj_mesh_free(meshes[i]);

	if(meshes)
		free(meshes);

	qobj_mtl_free(numMaterials, materials);
}

//----------------------------------------------------------------------//

#endif //#ifdef QOBJ_IMPLEMENTATION

#ifdef __cplusplus
} //extern "C"
#endif

#endif //QOBJ_H