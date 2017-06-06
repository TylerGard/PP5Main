#pragma once

#ifdef FBXDLL_EXPORTS  
#define fbxDLL_API __declspec(dllexport)   
#else  
#define fbxDLL_API __declspec(dllimport)   
#endif
#include <vector>

namespace fbxNS {
	struct vertex {
		float position[4];
		float color[4];
	};
	std::vector<vertex> verts;
	std::vector<int> indices;
	class fbxFunctions 
	{
	public:
		fbxDLL_API std::vector<vertex> getVertex() { return verts; }
		fbxDLL_API std::vector<int> getIndices() { return indices; }
		static fbxDLL_API void initializeFBX();
		static fbxDLL_API int returnInt();
	};
}