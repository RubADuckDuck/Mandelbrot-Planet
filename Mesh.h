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
#include "Texture.h"
#include "Light.h" 
#include "ShaderProgramLoader.h"

#define MAX_NUM_BONES_PER_VERTEX 4 

//class VertexBoneData {
//public:
//	uint32_t BoneIndexs[MAX_NUM_BONES_PER_VERTEX] = { 0 }; 
//	float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };
//
//	VertextBoneData();
//
//	void AddBoneData(uint32_t BoneIndex, float Weight);
//};

class CameraObject;

struct Mesh {
	GLuint VAO, VBO, EBO;
	GLuint textureIndex;
	int indexCount = 0;
}; 


#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices ) 

#define INVALID_MATERIAL 0xFFFFFFFF

class GeneralMesh {
public: 
	static GLuint sharedShaderProgram; // Static shader program for all Gizmos
	static bool shaderInitialized;     // Tracks if the shader is loaded 

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

	Transform model2WorldTransform;
	GLuint VAO = 58;

	Assimp::Importer Importer;
	const aiScene* ptrScene = NULL;
	std::vector<BasicMeshEntry> meshes;
	std::vector<Texture> textures; 

	// std::vector<Material> m_Materials; materials are currently not implemented 

	// Temporary space for vertex stuff before we load them into the GPU
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices; 

	GeneralMesh();

	void InitShader();

	virtual ~GeneralMesh() {}

	virtual bool LoadMesh(const std::string& fileName) = 0;

	virtual void Render(CameraObject& camObj, glm::mat4& tranform) = 0; 

	virtual void Render(CameraObject& camObj, glm::mat4& tranform, Texture* texture) = 0;

protected:
	virtual void LoadSharedShader();

	virtual void Clear();

	virtual bool InitFromScene(const aiScene* ptrScene, const std::string& filename); // why need file name when you have scene? 

	virtual void CountVerticesAndIndices(const aiScene* ptrScene, unsigned int& numVertices, unsigned int& numIndices); // hmm.. 

	virtual void ReserveSpace(unsigned int numVertices, unsigned int numIndices); // why? why reserve space? just use vector 

	virtual void InitAllMeshes(const aiScene* ptrScene);

	virtual void InitSingleMesh(uint32_t MeshIndex, const aiMesh* ptraiMesh);

	// virtual bool InitMaterials(const aiScene* ptrScene, const std::string& filename); todo or to trash

	virtual void PopulateBuffers(); // I can imagine what this does, create the buffers load the data, take there address 

	virtual void LoadTextures(const std::string& textureImagePath); 

	virtual DirectionalLight& GetDirectionalLight() { 
		DirectionalLight& light = DirectionalLight::getInstance(); 
		return light;
	} // directionalLight is singleton. dot product is biliniar that's why
};

class StaticMesh : public GeneralMesh {
public: 
	enum BUFFER_TYPE {
		INDEX_BUFFER = 0,
		POS_VB = 1,
		TEXCOORD_VB = 2,
		NORMAL_VB = 3,
		NUM_BUFFERS = 4
	};

	GLuint Buffers[NUM_BUFFERS] = { 0 }; 

	std::vector<glm::mat4> meshIndex2meshTransform; 

	StaticMesh();

	~StaticMesh();

	bool LoadMesh(const std::string& fileName) override;

	void Render(CameraObject& camObj, glm::mat4& tranform) override; 

	void Render(CameraObject& camObj, glm::mat4& tranform, Texture* texture) override;

protected:
	void LoadSharedShader() override{
		// Load shader from files
		std::string vertShaderPath = "C:\\Users\\kmo73\\source\\repos\\apple\\websocketsharp\\duckfishing\\duckfishing\\shaders\\staticMesh_vertex.vs";
		std::string fragShaderPath = "C:\\Users\\kmo73\\source\\repos\\apple\\websocketsharp\\duckfishing\\duckfishing\\shaders\\staticMesh_fragment.fs";

		sharedShaderProgram = LoadAndCreateShaderProgram(vertShaderPath, fragShaderPath);
		std::cout << "Shared shader loaded for Gizmos: " << sharedShaderProgram << std::endl;
	}
	
	void ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const glm::mat4& ParentTransform);

	void PopulateBuffers() override; // I can imagine what this does, create the buffers load the data, take there address 
};

class RiggedMesh : public GeneralMesh{
public:
	struct VertexBoneData {
		uint32_t BoneIndexs[MAX_NUM_BONES_PER_VERTEX] = { 0 }; 
		float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f }; 

		VertexBoneData() {

		}

		void AddBoneData(uint32_t BoneIndex, float Weight) {
			for (uint32_t i = 0; i < std::size(BoneIndexs); i++) {
				if (Weights[i] == 0.0) { // if one of the weight is empty, 
					BoneIndexs[i] = BoneIndex; // take its 
					Weights[i] = Weight;
					//printf("Adding bone %d weight %f at index %i\n", BoneIndex, Weight, i);
					return;
				}
			}

			// should never get here - more bones than we have space for
			assert(0);
		}
	};



	enum BUFFER_TYPE {
		INDEX_BUFFER = 0,
		POS_VB = 1,
		TEXCOORD_VB = 2,
		NORMAL_VB = 3,
		BONE_VB = 4,
		NUM_BUFFERS = 5
	};

	GLuint Buffers[NUM_BUFFERS] = { 0 }; 

	// Temporary space for vertex stuff before we load them into the GPU
	//std::vector<glm::vec3> positions; // these exist in the parent class 
	//std::vector<glm::vec3> normals; 
	//std::vector<glm::vec2> texCoords; 
	//std::vector<unsigned int> indices; 
	std::vector<VertexBoneData> bones; // this has to be manually constructed  

	std::map<std::string, uint32_t> boneName2IndexMap;

	struct BoneInfo { 
		glm::mat4 boneOffset; // matrix that chnages coord from CurrentBoneCoord ParentBoneCoord
		glm::mat4 bone2GlobalCoord; 

		BoneInfo(const glm::mat4 Offset) {
			boneOffset = Offset; 
			bone2GlobalCoord = glm::mat4(1.0f); 
		}
	};

	std::vector<BoneInfo> boneIndex2BoneInfo;  // map from boneIndex -> BoneInfo 
	glm::mat4 GlobalInverseTransform; // wtf is this??????

	~RiggedMesh(); 

	bool LoadMesh(const std::string& fileName) override; 

	void Render(CameraObject& camObj, glm::mat4& tranform) override; 

	void Render(CameraObject& camObj, glm::mat4& tranform, Texture* texture) override;

	uint32_t FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim); 

	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

	uint32_t FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

	uint32_t FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

	uint32_t NumBones() {
		return (uint32_t)boneName2IndexMap.size(); 
	}

	Transform& GetModel2WorldTransform() {
		return model2WorldTransform; 
	} 

	void GetBoneTransforms(float animationTimeSec, std::vector<glm::mat4>& transforms); 

private: 
	void LoadSharedShader() override{
		// Load shader from files
		std::string vertShaderPath = "C:\\Users\\kmo73\\source\\repos\\apple\\websocketsharp\\duckfishing\\duckfishing\\shaders\\gizmo_vertex.vs";
		std::string fragShaderPath = "C:\\Users\\kmo73\\source\\repos\\apple\\websocketsharp\\duckfishing\\duckfishing\\shaders\\gizmo_fragment.fs";

		sharedShaderProgram = LoadAndCreateShaderProgram(vertShaderPath, fragShaderPath);
		std::cout << "Shared shader loaded for Gizmos: " << sharedShaderProgram << std::endl;
	}

	void Clear(); 

	// bool InitFromScene(const aiScene* ptrScene, const std::string& filename); // why need file name when you have scene? 

	// void CountVerticesAndIndices(const aiScene* ptrScene, unsigned int& numVertices, unsigned int& numIndices); implementation exists in Parent Class

	void ReserveSpace(unsigned int numVertices, unsigned int numIndices); // why? why reserve space? just use vector 

	// void InitAllMeshes(const aiScene* ptrScene); 

	void InitSingleMesh(uint32_t MeshIndex, const aiMesh* ptraiMesh) override;

	// bone related 
	void LoadMeshBones(uint32_t MeshIndex, const aiMesh* ptrMesh); 

	void LoadSingleBone(uint32_t MeshIndex, const aiBone* ptrBone); 

	int GetBoneIndex(const aiBone* ptrBone);

	// bool InitMaterials(const aiScene* ptrScene, const std::string& filename); // todo or to trash

	void PopulateBuffers(); // I can imagine what this does, create the buffers load the data, take there address 

	// void LoadTextures(const std::string& textureImagePath); 

	void ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const glm::mat4& ParentTransform);
};

