#pragma once
#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 
#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <string> 
#include <vector> 
#include "Transform.h"

#define MAX_NUM_BONES_PER_VERTEX 4 

class VertexBoneData {
public:
	uint32_t BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 }; 
	float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

	VertextBoneData();

	void AddBoneData(uint32_t BoneID, float Weight);
};

struct Mesh {
	GLuint VAO, VBO, EBO;
	GLuint textureID;
	int indexCount = 0;
}; 





class SkinnedMesh {

};


#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices ) 


class SceneLoader {
public: 
	std::vector<Mesh*> meshes; 


	std::vector<VertexBoneData> vertexID2BoneDatas; // vertex id -> Bone data
	std::vector<int> mesh_base_vertex;
	std::map<std::string, uint32_t> bone_name_to_index_map;

	SceneLoader();

	~SceneLoader();


	int GetBoneID(const aiBone* pBone); 
	void ParseSingleBone(int mesh_index, const aiBone* pBone);
	void ParseMeshBones(int mesh_index, const aiMesh* pMesh); 
	void ParseMeshes(const aiScene* pScene); 
	void ParseScene(const aiScene* pScene);

	Mesh LoadModel(const std::string& path);
	void AddMesh(const std::string& path);
}; 


class RiggedMesh {
public:
	struct VertexBoneData {
		uint32_t BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 }; 
		float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f }; 

		VertexBoneData() {

		}

		void AddBoneData(uint32_t BoneID, float Weight) {
			for (uint32_t i = 0; i < std::size(BoneIDs); i++) {
				if (Weights[i] == 0.0) { // if one of the weight is empty, 
					BoneIDs[i] = BoneID; // take its 
					Weights[i] = Weight;
					//printf("Adding bone %d weight %f at index %i\n", BoneID, Weight, i);
					return;
				}
			}

			// should never get here - more bones than we have space for
			assert(0);
		}
	};

#define INVALID_MATERIAL 0xFFFFFFFF

	enum BUFFER_TYPE {
		INDEX_BUFFER = 0,
		POS_VB = 1,
		TEXCOORD_VB = 2,
		NORMAL_VB = 3,
		BONE_VB = 4,
		NUM_BUFFERS = 5
	};

	Transform model2WorldTransform; 
	GLuint VAO = 0; 
	GLuint Buffers[NUM_BUFFERS] = { 0 }; 

	struct BasicMeshEntry {
		BasicMeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}

		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};
	
	Assimp::Importer Importer;
	const aiScene* ptrScene = NULL; 
	std::vector<BasicMeshEntry> meshes;
	// std::vector<Material> m_Materials; materials are currently not implemented 

	// Temporary space for vertex stuff before we load them into the GPU
	std::vector<glm::vec3> positions; 
	std::vector<glm::vec3> normals; 
	std::vector<glm::vec2> texCoords; 
	std::vector<unsigned int> indices; 
	std::vector<VertexBoneData> bones; // this has to be manually constructed  

	std::map<std::string, uint32_t> boneName2IndexMap;

	struct BoneInfo { 
		glm::mat4 currBone2ParentBoneCoord; // matrix that chnages coord from CurrentBoneCoord ParentBoneCoord
		glm::mat4 currBone2LocalCoord; 

		BoneInfo(const glm::mat4 Offset) {
			currBone2ParentBoneCoord = Offset; 
			currBone2LocalCoord = glm::mat4(1.0f); 
		}
	};

	std::vector<BoneInfo> boneID2BoneInfo;  // map from boneID -> BoneInfo 
	glm::mat4 GlobalInverseTransform; // wtf is this??????

	RiggedMesh() {}; 

	~RiggedMesh(); 

	bool LoadMesh(const std::string& fileName); 

	void Render(); 

	uint32_t NumBones() {
		return (uint32_t)boneName2IndexMap.size(); 
	}

	Transform& GetModel2WorldTransform() {
		return model2WorldTransform; 
	} 

	void GetBoneTransforms(float animationTimeSec, std::vector<glm::mat4>& transforms); 

private: 
	void Clear(); 

	bool InitFromScene(const aiScene* ptrScene, const std::string& filename); // why need file name when you have scene? 

	void CountVerticesAndIndices(const aiScene* ptrScene, unsigned int& numVertices, unsigned int& numIndices); // hmm.. 

	void ReserveSpace(unsigned int numVertices, unsigned int numIndices); // why? why reserve space? just use vector 

	void InitAllMeshes(const aiScene* ptrScene); 

	void InitSingleMesh(uint32_t MeshIndex, const aiMesh* ptraiMesh);

	void LoadMeshBones(uint32_t MeshIndex, const aiMesh* ptrMesh); 

	void LoadSingleBone(uint32_t MeshIndex, const aiBone* ptrBone); 

	int GetBoneId(const aiBone* ptrBone);

	// bool InitMaterials(const aiScene* ptrScene, const std::string& filename); todo or to trash

	void PopulateBuffers(); // I can imagine what this does, create the buffers load the data, take there address 

	void LoadTextures(const std::string& textureImagePath); 


};

