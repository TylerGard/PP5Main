#pragma once

#ifdef FBXDLL_EXPORTS  
#define fbxDLL_API __declspec(dllexport)   
#else  
#define fbxDLL_API __declspec(dllimport)   
#endif
#include <vector>

struct Matrix {
	float e00, e01, e02, e03,
		e10, e11, e12, e13,
		e20, e21, e22, e23,
		e30, e31, e32, e33;
};

struct xyzw {
	float x, y, z, w;
};

struct regJoint {
	xyzw jXYZW;
	Matrix jMatrix;
};

struct meshStruct {
	std::vector<regJoint> jointVec;
	int indexCount;
};

struct vertex {
	float position[4];
	float color[4];
	float U;
	float V;
};

struct meshCollection {
	std::vector<vertex> verts;
	std::vector<int> indices;
	std::vector<std::vector<xyzw>> boneVerticesX;
	//std::vector<xyzw > boneVerticesX;
};

namespace fbxNS {
	std::vector<meshCollection> meshColVector;
	class fbxFunctions
	{
	public:
		/*fbxDLL_API std::vector<vertex> getVertex() { return verts; }
		fbxDLL_API std::vector<int> getIndices() { return indices; }
		fbxDLL_API std::vector<xyzw> getBoneVertex() { return boneVerticesX; }*/
		static fbxDLL_API std::vector<meshCollection> & getMeshCollection(){ return meshColVector; };
		static fbxDLL_API void initializeFBX();
		static fbxDLL_API void ProcessStuff(meshCollection & theCollection, float scalevalue);
		static fbxDLL_API meshStruct *getMesh();
		static fbxDLL_API int returnInt();
	};
}



