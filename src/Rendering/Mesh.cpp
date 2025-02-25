

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags 
#include <cassert>
#include <iostream>

#include "Rendering/Mesh.h"
#include "Core/GameObject.h"
#include "Utils/LOG.h"

#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   1
#define NORMAL_LOCATION      2
#define BONE_Index_LOCATION  3
#define BONE_WEIGHT_LOCATION 4



void log(std::string& msg) {
    std::cout << msg << std::endl;
}

// Function to convert aiMatrix4x4 to glm::mat4
glm::mat4 ConvertToGlmMat4(const aiMatrix4x4& aiMat) {
    glm::mat4 glmMat;

    glmMat[0][0] = aiMat.a1; glmMat[0][1] = aiMat.b1; glmMat[0][2] = aiMat.c1; glmMat[0][3] = aiMat.d1;
    glmMat[1][0] = aiMat.a2; glmMat[1][1] = aiMat.b2; glmMat[1][2] = aiMat.c2; glmMat[1][3] = aiMat.d2;
    glmMat[2][0] = aiMat.a3; glmMat[2][1] = aiMat.b3; glmMat[2][2] = aiMat.c3; glmMat[2][3] = aiMat.d3;
    glmMat[3][0] = aiMat.a4; glmMat[3][1] = aiMat.b4; glmMat[3][2] = aiMat.c4; glmMat[3][3] = aiMat.d4;

    return glmMat;
}

//int SceneLoader::GetBoneIndex(const aiBone* pBone)
//{
//    int bone_id = 0;
//    std::string bone_name(pBone->mName.C_Str());
//
//    // each bone has to have a different name to assign the Index
//    if (bone_name_to_index_map.find(bone_name) == bone_name_to_index_map.end()) { 
//        // Allocate an index for a new bone
//        bone_id = (int)bone_name_to_index_map.size();
//        bone_name_to_index_map[bone_name] = bone_id;
//    }
//    else {
//        bone_id = bone_name_to_index_map[bone_name];
//    }
//
//    return bone_id;
//}
//
//void SceneLoader::ParseSingleBone(int mesh_index, const aiBone* pBone)
//{
//    printf("      Bone '%s': num vertices affected by this bone: %d\n", pBone->mName.C_Str(), pBone->mNumWeights);
//
//    int bone_id = this->GetBoneIndex(pBone);
//    // printf("bone id %d\n", bone_id); 
//
//    
//
//    for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
//        if (i == 0) printf("\n");
//        const aiVertexWeight& vw = pBone->mWeights[i]; // vw has mVertexId and mWeight as attribute 
//
//        uint32_t global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId;  // calculate global id
//        printf("Vertex id %d ", global_vertex_id);
//
//        // for this to be true if bone1 refers to vertices in Mesh1 and Mesh2 but the bone is under Mesh1
//        // Vertices on mesh2 should not be here.
//        assert(global_vertex_id < vertexIndex2BoneDatas.size()); 
//        vertexIndex2BoneDatas[global_vertex_id].AddBoneData(bone_id, vw.mWeight);
//    }
//
//    printf("\n");
//}
//
//
//void SceneLoader::ParseMeshBones(int mesh_index, const aiMesh* pMesh)
//{
//    for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
//        ParseSingleBone(mesh_index, pMesh->mBones[i]);
//    }
//}
//
//
//void SceneLoader::ParseMeshes(const aiScene* ptrScene)
//{
//    printf("*******************************************************\n");
//    printf("Parsing %d meshes\n\n", ptrScene->mNumMeshes);
//
//    int total_vertices = 0;
//    int total_indices = 0;
//    int total_bones = 0;
//
//    mesh_base_vertex.resize(ptrScene->mNumMeshes);
//
//    for (unsigned int i = 0; i < ptrScene->mNumMeshes; i++) { const aiMesh* pMesh = ptrScene->mMeshes[i];
//        int num_vertices = pMesh->mNumVertices;
//        int nuindices = pMesh->mNumFaces * 3;
//        int nubones = pMesh->mNumBones;
//        printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, nuindices, nubones);
//        total_vertices += num_vertices;
//        total_indices += nuindices;
//        total_bones += nubones;
//
//        vertexIndex2BoneDatas.resize(total_vertices);
//
//        if (pMesh->HasBones()) {
//            this->ParseMeshBones(i ,pMesh);
//        }
//
//        printf("\n");
//    }
//
//    printf("\nTotal vertices %d total indices %d total bones %d\n", total_vertices, total_indices, total_bones);
//}
//
//
//void SceneLoader::ParseScene(const aiScene* ptrScene)
//{
//    this->ParseMeshes(ptrScene);
//} // SceneLoader class seems sort of useless

GLuint GeneralMesh::sharedShaderProgram = 0; // Static shader program for all Gizmos
bool GeneralMesh::shaderInitialized = false;     // Tracks if the shader is loaded 

GeneralMesh::GeneralMesh() {} 

void GeneralMesh::InitShader() {
    if (!shaderInitialized) {
        LoadSharedShader(); // Load shader only once
        shaderInitialized = true;
    }
}

void GeneralMesh::LoadSharedShader() {
    LOG(LOG_ERROR, "Loading shared shader for a GeneralMesh Class is not allowed");
}

StaticMesh::StaticMesh() {}

StaticMesh::~StaticMesh() {
    std::cout << "StaticMesh Destructor Called" << std::endl;

    // Release any resources here if necessary
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
    }
    if (Buffers[0]) {
        glDeleteBuffers(std::size(Buffers), Buffers);
    }
}

RiggedMesh::~RiggedMesh()
{
    this->Clear();
}

void GeneralMesh::Clear()
{
    //if (this->Buffers[0] != 0) {
    //    glDeleteBuffers(std::size(Buffers), Buffers);
    //}

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
} 

void GeneralMesh::PopulateBuffers() {
    // Implementation here
}

void GeneralMesh::LoadTextures(const std::string& texturePath) {
    // Implementation here
}

void RiggedMesh::Clear()
{
    if (this->Buffers[0] != 0) {
        glDeleteBuffers(std::size(Buffers), Buffers);
    }

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
}

bool StaticMesh::LoadMesh(const std::string& filename, bool retry) {
    // Init shader 
    this->InitShader();

    // Release previously loaded mesh 
    this->Clear();

    // Create VAO... this early? 
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create the buffers for the vertexs' attributes
    glGenBuffers(std::size(this->Buffers), this->Buffers); // forgot what this does 

    bool Ret = false; // what does this do? 

    this->ptrScene = Importer.ReadFile(filename.c_str(), ASSIMP_LOAD_FLAGS);

    if (ptrScene) {
        // GlobalInverseTransform = ConvertToGlmMat4(ptrScene->mRootNode->mTransformation);
        // GlobalInverseTransform = glm::inverse(GlobalInverseTransform); // calculate inverse transformation 
        // 
        // mTransforms map from RootNode -> Global
        // the inverse does the oppisite mapping Global -> RootNode 
        // this is needed since each accumulated transform in each bone maps from 
        // Bone_n -> ... -> Global 
        // however since meshes' vertices are represented respect to RootCoord, 
        // the Transformation that properly controls the vertices is 
        // Bone_n -> ... -> Global -> RootNode. Not, Bone_n -> ... -> Global
        // that is why we keep the inverse as member var.
        Ret = this->InitFromScene(ptrScene, filename);
        LOG(LOG_INFO, "Successfully Loaded Obj from:" + filename);
    }
    else {
        LOG(LOG_ERROR, "Failed to Load Obj from:" + filename); 

        if (!retry) {
            Ret = this->LoadMesh("E:/repos/[DuckFishing]/model/Error.obj", true);
        }
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}

bool RiggedMesh::LoadMesh(const std::string& filename, bool retry) {
    // Init shader 
    this->InitShader();

    // Release previously loaded mesh 
    this->Clear(); 

    // Create VAO... this early? 
    glGenVertexArrays(1, &this->VAO); 
    glBindVertexArray(this->VAO); 

    // Create the buffers for the vertexs' attributes
    glGenBuffers(std::size(this->Buffers), this->Buffers); // forgot what this does 

    bool Ret = false; // what does this do? 

    this->ptrScene = Importer.ReadFile(filename.c_str(), ASSIMP_LOAD_FLAGS);  

    if (ptrScene) {
        GlobalInverseTransform = ConvertToGlmMat4(ptrScene->mRootNode->mTransformation);
        GlobalInverseTransform = glm::inverse(GlobalInverseTransform); // calculate inverse transformation 
        // mTransforms map from RootNode -> Global
        // the inverse does the oppisite mapping Global -> RootNode 
        // this is needed since each accumulated transform in each bone maps from 
        // Bone_n -> ... -> Global 
        // however since meshes' vertices are represented respect to RootCoord, 
        // the Transformation that properly controls the vertices is 
        // Bone_n -> ... -> Global -> RootNode. Not, Bone_n -> ... -> Global
        // that is why we keep the inverse as member var.
        Ret = this->InitFromScene(ptrScene, filename); 

    } else {
        printf("Error parsing '%s': '%s'\n", filename.c_str(), Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}

bool GeneralMesh::InitFromScene(const aiScene* ptrScene, const std::string& filename) {
    this->meshes.resize(ptrScene->mNumMeshes); 
    if (StaticMesh* thisMesh = dynamic_cast<StaticMesh*>(this)) {
        // if this is a instance of StaticMesh 
        thisMesh->meshIndex2meshTransform.resize(ptrScene->mNumMeshes);
    }
   
    unsigned int numVertices = 0; 
    unsigned int numIndices = 0; 

    this->CountVerticesAndIndices(ptrScene, numVertices, numIndices); 

    this->ReserveSpace(numVertices, numIndices); 

    InitAllMeshes(ptrScene); 

    //if (!this->InitMaterials(ptrScene, Filename)) {
    //    return false;
    //}

    this->PopulateBuffers(); 
    
    return true; 
    // return GLCheckError(); // todo: why isn't glcheckerror available? legacy?
}

void GeneralMesh::CountVerticesAndIndices(const aiScene* ptrScene, unsigned int& numVertices, unsigned int& numIndices)
{
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].MaterialIndex = this->ptrScene->mMeshes[i]->mMaterialIndex;
        meshes[i].NumIndices = this->ptrScene->mMeshes[i]->mNumFaces * 3;
        meshes[i].BaseVertex = numVertices; // baseVertex is the Global index of the first vertex in mesh 
        meshes[i].BaseIndex = numIndices;   // goes same for Indices 

        numVertices += ptrScene->mMeshes[i]->mNumVertices;
        numIndices += meshes[i].NumIndices;
    }
}

//void RiggedMesh::CountVerticesAndIndices(const aiScene* ptrScene, unsigned int& numVertices, unsigned int& numIndices)
//{
//    for (unsigned int i = 0; i < meshes.size(); i++) {
//        meshes[i].MaterialIndex = this->ptrScene->mMeshes[i]->mMaterialIndex;
//        meshes[i].NumIndices = this->ptrScene->mMeshes[i]->mNumFaces * 3;
//        meshes[i].BaseVertex = numVertices; // baseVertex is the Global index of the first vertex in mesh 
//        meshes[i].BaseIndex = numIndices;   // goes same for Indices 
//
//        numVertices += ptrScene->mMeshes[i]->mNumVertices;
//        numIndices += meshes[i].NumIndices;
//    }
//} 

void GeneralMesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    this->positions.reserve(NumVertices);
    this->normals.reserve(NumVertices);
    this->texCoords.reserve(NumVertices);
    this->indices.reserve(NumIndices);
} 

void RiggedMesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    this->positions.reserve(NumVertices);
    this->normals.reserve(NumVertices);
    this->texCoords.reserve(NumVertices);
    this->indices.reserve(NumIndices);
    this->bones.resize(NumVertices);
} 

void GeneralMesh::InitAllMeshes(const aiScene* ptrScene)
{
    for (unsigned int i = 0; i < this->meshes.size(); i++) {
        const aiMesh* paiMesh = ptrScene->mMeshes[i];
        InitSingleMesh(i, paiMesh); 
    }
} 

void GeneralMesh::InitSingleMesh(uint32_t MeshIndex, const aiMesh* ptraiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < ptraiMesh->mNumVertices; i++) {
        // add vert pos 
        const aiVector3D& pPos = ptraiMesh->mVertices[i];
        positions.push_back(glm::vec3(pPos.x, pPos.y, pPos.z));

        // add normal vec
        if (ptraiMesh->mNormals) {
            const aiVector3D& pNormal = ptraiMesh->mNormals[i];
            normals.push_back(glm::vec3(pNormal.x, pNormal.y, pNormal.z));
        }
        else {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            normals.push_back(glm::vec3(Normal.x, Normal.y, Normal.z));
        }

        // add UV coord
        const aiVector3D& pTexCoord = ptraiMesh->HasTextureCoords(0) ? ptraiMesh->mTextureCoords[0][i] : Zero3D;
        texCoords.push_back(glm::vec2(pTexCoord.x, pTexCoord.y));
    }

    // Populate the index buffer
    for (unsigned int i = 0; i < ptraiMesh->mNumFaces; i++) {
        const aiFace& Face = ptraiMesh->mFaces[i];
        //        printf("num indices %d\n", Face.mNumIndices);
        //        assert(Face.mNumIndices == 3);
        indices.push_back(Face.mIndices[0]);
        indices.push_back(Face.mIndices[1]);
        indices.push_back(Face.mIndices[2]);
    }
}

void RiggedMesh::InitSingleMesh(uint32_t MeshIndex, const aiMesh* ptraiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < ptraiMesh->mNumVertices; i++) {
        // add vert pos 
        const aiVector3D& pPos = ptraiMesh->mVertices[i];
        positions.push_back(glm::vec3(pPos.x, pPos.y, pPos.z));

        // add normal vec
        if (ptraiMesh->mNormals) {
            const aiVector3D& pNormal = ptraiMesh->mNormals[i];
            normals.push_back(glm::vec3(pNormal.x, pNormal.y, pNormal.z));
        }
        else {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            normals.push_back(glm::vec3(Normal.x, Normal.y, Normal.z));
        }

        // add UV coord
        const aiVector3D& pTexCoord = ptraiMesh->HasTextureCoords(0) ? ptraiMesh->mTextureCoords[0][i] : Zero3D;
        texCoords.push_back(glm::vec2(pTexCoord.x, pTexCoord.y));
    }

    this->LoadMeshBones(MeshIndex, ptraiMesh);

    // Populate the index buffer
    for (unsigned int i = 0; i < ptraiMesh->mNumFaces; i++) {
        const aiFace& Face = ptraiMesh->mFaces[i];
        //        printf("num indices %d\n", Face.mNumIndices);
        //        assert(Face.mNumIndices == 3);
        indices.push_back(Face.mIndices[0]);
        indices.push_back(Face.mIndices[1]);
        indices.push_back(Face.mIndices[2]);
    }
}

void RiggedMesh::LoadMeshBones(uint32_t MeshIndex, const aiMesh* ptrMesh) {
    // iterate on bones 
    for (uint32_t i = 0; i < ptrMesh->mNumBones; i++) { aiBone* currBone = ptrMesh->mBones[i];
        this->LoadSingleBone(MeshIndex, currBone); 
    }
}

void RiggedMesh::LoadSingleBone(uint32_t MeshIndex, const aiBone* ptrBone) {
    int BoneIndex = this->GetBoneIndex(ptrBone); 

    if (BoneIndex == this->boneIndex2BoneInfo.size()) { // new unregistered bone
        // if the bone is new BoneIndex will be out of bound
        aiMatrix4x4 curOffsetMat = ptrBone->mOffsetMatrix;

        BoneInfo bi(ConvertToGlmMat4(curOffsetMat));
        boneIndex2BoneInfo.push_back(bi);
    }

    // iterate through affected vertices 
    // add boneIndex that we just made 
    // add the Weight value to the 'bone' 
    // bone is a map of GlobalVertexIndex -> (Array of Bone Index) x (Array of weight) 
    // it is nessesary and important to do this 
    // since the Renderer iterates on the Vertices, not on Bones
    for (uint32_t i = 0; i < ptrBone->mNumWeights; i++) { // list of bone weights 
        const aiVertexWeight& vw = ptrBone->mWeights[i];
        // globalVertexIndex = (index of first vertex in mesh) + (local Vertex Index)
        uint32_t GlobalVertexIndex = meshes[MeshIndex].BaseVertex + vw.mVertexId;  
        bones[GlobalVertexIndex].AddBoneData(BoneIndex, vw.mWeight); 
    }
}

//------------------


int RiggedMesh::GetBoneIndex(const aiBone* ptrBone) {
    int BoneIndex = 0;
    std::string BoneName(ptrBone->mName.C_Str());

    if (this->boneName2IndexMap.find(BoneName) == this->boneName2IndexMap.end()) { 
        // Allocate an index for a new bone
        BoneIndex = (int)this->boneName2IndexMap.size();
        this->boneName2IndexMap[BoneName] = BoneIndex;
    }
    else { 
        // if existing bone, return the boneIndex
        BoneIndex = this->boneName2IndexMap[BoneName];
    }

    return BoneIndex;
}


//bool RiggedMesh::InitMaterials(const aiScene* ptrScene, const string& Filename)
//{
//    string Dir = GetDirFromFilename(Filename);
//
//    bool Ret = true;
//
//    printf("Num materials: %d\n", ptrScene->mNumMaterials);
//
//    // Initialize the materials
//    for (unsigned int i = 0; i < ptrScene->mNumMaterials; i++) {
//        const aiMaterial* pMaterial = ptrScene->mMaterials[i];
//
//        LoadTextures(Dir, pMaterial, i);
//
//        LoadColors(pMaterial, i);
//    }
//
//    return Ret;
//}

//
//void RiggedMesh::LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index)
//{
//    LoadDiffuseTexture(Dir, pMaterial, index);
//    LoadSpecularTexture(Dir, pMaterial, index);
//}
//
//
//void RiggedMesh::LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index)
//{
//    m_Materials[index].pDiffuse = NULL;
//
//    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
//        aiString Path;
//
//        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
//            string p(Path.data);
//
//            if (p.substr(0, 2) == ".\\") {
//                p = p.substr(2, p.size() - 2);
//            }
//
//            string FullPath = Dir + "/" + p;
//
//            m_Materials[index].pDiffuse = new Texture(GL_TEXTURE_2D, FullPath.c_str());
//
//            if (!m_Materials[index].pDiffuse->Load()) {
//                printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
//                exit(0);
//            }
//            else {
//                printf("Loaded diffuse texture '%s'\n", FullPath.c_str());
//            }
//        }
//    }
//}
//
//
//void RiggedMesh::LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index)
//{
//    m_Materials[index].pSpecularExponent = NULL;
//
//    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
//        aiString Path;
//
//        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
//            string p(Path.data);
//
//            if (p == "C:\\\\") {
//                p = "";
//            }
//            else if (p.substr(0, 2) == ".\\") {
//                p = p.substr(2, p.size() - 2);
//            }
//
//            string FullPath = Dir + "/" + p;
//
//            m_Materials[index].pSpecularExponent = new Texture(GL_TEXTURE_2D, FullPath.c_str());
//
//            if (!m_Materials[index].pSpecularExponent->Load()) {
//                printf("Error loading specular texture '%s'\n", FullPath.c_str());
//                exit(0);
//            }
//            else {
//                printf("Loaded specular texture '%s'\n", FullPath.c_str());
//            }
//        }
//    }
//}
//
//void RiggedMesh::LoadColors(const aiMaterial* pMaterial, int index)
//{
//    aiColor4D AmbientColor(0.0f, 0.0f, 0.0f, 0.0f);
//    Vector4f AllOnes(1.0f, 1.0f, 1.0f, 1.0);
//
//    int ShadingModel = 0;
//    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) {
//        printf("Shading model %d\n", ShadingModel);
//    }
//
//    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS) {
//        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
//        m_Materials[index].AmbientColor.r = AmbientColor.r;
//        m_Materials[index].AmbientColor.g = AmbientColor.g;
//        m_Materials[index].AmbientColor.b = AmbientColor.b;
//    }
//    else {
//        m_Materials[index].AmbientColor = AllOnes;
//    }
//
//    aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);
//
//    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS) {
//        printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
//        m_Materials[index].DiffuseColor.r = DiffuseColor.r;
//        m_Materials[index].DiffuseColor.g = DiffuseColor.g;
//        m_Materials[index].DiffuseColor.b = DiffuseColor.b;
//    }
//
//    aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);
//
//    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) {
//        printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
//        m_Materials[index].SpecularColor.r = SpecularColor.r;
//        m_Materials[index].SpecularColor.g = SpecularColor.g;
//        m_Materials[index].SpecularColor.b = SpecularColor.b;
//    }
//}


void StaticMesh::PopulateBuffers()
{
    std::string debugLine = "";

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    debugLine = "Position buffer size:" + std::to_string(sizeof(positions[0]) * positions.size());
    log(debugLine);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    debugLine = "TexCoord buffer size:" + std::to_string(sizeof(texCoords[0]) * texCoords.size());
    log(debugLine);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    debugLine = "Normal buffer size:" + std::to_string(sizeof(normals[0]) * normals.size());
    log(debugLine);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW); 

    debugLine = "Index buffer size:" + std::to_string(sizeof(indices[0]) * indices.size());
    log(debugLine);
}

void RiggedMesh::PopulateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bones[0]) * bones.size(), &bones[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONE_Index_LOCATION);
    glVertexAttribIPointer(BONE_Index_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
        (const GLvoid*)(MAX_NUM_BONES_PER_VERTEX * sizeof(int32_t)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void StaticMesh::Render(CameraObject& cameraObj, glm::mat4& tranform)
{
    if (!sharedShaderProgram) {
        std::cout << "Object ??? Missing sharedShaderProgram" << std::endl; 
    }
    GLuint shaderProgram = this->sharedShaderProgram;

    glm::mat4 curMwvp = glm::mat4(1); 
    glm::mat4 viewProj = cameraObj.GetviewProjMat();
    
    glm::vec3 cameraGlobalPos = cameraObj.GetGlobalCameraPosition(); 

    GLuint curTextureIndex = 0; 

    GLuint mwvpLoc = glGetUniformLocation(shaderProgram, "mwvp");
    GLuint textureLoc = glGetUniformLocation(shaderProgram, "textureDiffuse");
    GLuint cameraGlobalPositionLoc = glGetUniformLocation(shaderProgram, "gCameraPos");

    { // set light
        DirectionalLight& directionalLight = this->GetDirectionalLight(); // IDK what the end sign does
        GLint location;

        // Set BaseLight fields
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.Color");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.base.color));

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.AmbientIntensity");
        glUniform1f(location, directionalLight.base.ambientIntensity);

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.DiffuseIntensity");
        glUniform1f(location, directionalLight.base.diffuseIntensity);

        // Set Direction field
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Direction");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.direction));
    }

    // set cameraPosition
    glUniform3f(
        cameraGlobalPositionLoc,
        cameraGlobalPos[0],
        cameraGlobalPos[1],
        cameraGlobalPos[2]
    );

    glBindVertexArray(VAO);

    for (unsigned int i = 0; i < meshes.size(); i++) {
        unsigned int MaterialIndex = meshes[i].MaterialIndex;

        // assert(MaterialIndex < m_Materials.size()); todo: how do I assert?

        //if (m_Materials[MaterialIndex].pDiffuse) { todo: material is currently ignored
        //    m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        //}

        //if (m_Materials[MaterialIndex].pSpecularExponent) {
        //    m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
        //}

        // get Mesh->World(SceneRoot)->World(InGame)->View->Projection transformation
        curMwvp = viewProj * tranform * this->meshIndex2meshTransform[i];

        // get TextureIndex 
        curTextureIndex = this->textures[i].GetTextureIndex(); 

        // set MWVP transformation
        glUniformMatrix4fv(mwvpLoc, 1, GL_FALSE, glm::value_ptr(curMwvp)); 
        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, curTextureIndex); 
        glUniform1i(textureLoc, 0); 

        glDrawElementsBaseVertex(GL_TRIANGLES,
            meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), // Specifies a pointer to the location where the indices are stored 
            meshes[i].BaseVertex
        ); // Specifies a constant that should be added to each element of indices when chosing elements from the enabled vertex arrays.
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}

void RiggedMesh::Render(CameraObject& cameraObj, glm::mat4& tranform)
{
    if (!sharedShaderProgram) {
        std::cout << "Object ??? Missing sharedShaderProgram" << std::endl;
    }
    GLuint shaderProgram = this->sharedShaderProgram;

    glm::mat4 curMwvp = glm::mat4(1);
    glm::mat4 viewProj = cameraObj.GetviewProjMat();

    glm::vec3 cameraGlobalPos = cameraObj.GetGlobalCameraPosition();
    

    GLuint curTextureIndex = 0;

    GLuint mwvpLoc = glGetUniformLocation(shaderProgram, "mwvp");
    GLuint textureLoc = glGetUniformLocation(shaderProgram, "textureDiffuse");
    GLuint cameraGlobalPositionLoc = glGetUniformLocation(shaderProgram, "gCameraPos");

    { // set light
        DirectionalLight& directionalLight = this->GetDirectionalLight(); // IDK what the end sign does
        GLint location;

        // Set BaseLight fields
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.Color");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.base.color));

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.AmbientIntensity");
        glUniform1f(location, directionalLight.base.ambientIntensity);

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.DiffuseIntensity");
        glUniform1f(location, directionalLight.base.diffuseIntensity);

        // Set Direction field
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Direction");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.direction));
    }

    // set cameraPosition
    glUniform3f(
        cameraGlobalPositionLoc,
        cameraGlobalPos[0],
        cameraGlobalPos[1],
        cameraGlobalPos[2]
    );

    // get Mesh->World(SceneRoot)->World(InGame)->View->Projection transformation
    curMwvp = viewProj * this->model2WorldTransform.GetTransformMatrix() * tranform;

    // set MWVP transformation
    glUniformMatrix4fv(mwvpLoc, 1, GL_FALSE, glm::value_ptr(curMwvp));

    glBindVertexArray(VAO);

    for (unsigned int i = 0; i < meshes.size(); i++) {
        unsigned int MaterialIndex = meshes[i].MaterialIndex;

        // assert(MaterialIndex < m_Materials.size()); todo: how do I assert?

        //if (m_Materials[MaterialIndex].pDiffuse) { todo: material is currently ignored
        //    m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        //}

        //if (m_Materials[MaterialIndex].pSpecularExponent) {
        //    m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
        //}

        // get TextureIndex 
        curTextureIndex = this->textures[i].GetTextureIndex(); 

        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, curTextureIndex);
        glUniform1i(textureLoc, 0);

        glDrawElementsBaseVertex(GL_TRIANGLES,
            meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * meshes[i].BaseIndex),
            meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void StaticMesh::Render(CameraObject& cameraObj, glm::mat4& tranform, Texture* ptrTexture)
{
    if (sharedShaderProgram==NULL) {
        std::cout << "Object ??? Missing sharedShaderProgram" << std::endl;
    }
    GLuint shaderProgram = this->sharedShaderProgram; 
    glUseProgram(shaderProgram);

    glm::mat4 curMwvp = glm::mat4(1);
    glm::mat4 viewProj = cameraObj.GetviewProjMat();
    glm::mat4 curModelMatrix = glm::mat4(1); 
    glm::mat3 curNormalMatrix = glm::mat3(1);

    curModelMatrix = tranform;
    curNormalMatrix = glm::transpose(glm::inverse(glm::mat3(curModelMatrix)));

    glm::vec3 cameraGlobalPos = cameraObj.GetGlobalCameraPosition();

    GLuint curTextureIndex = 0;

    // get Locations 
    GLuint mwvpLoc = glGetUniformLocation(shaderProgram, "mwvp"); 
    GLuint modelMatrixLoc = glGetUniformLocation(shaderProgram, "modelMatrix"); 
    GLuint normalMatrixLoc = glGetUniformLocation(shaderProgram, "normalMatrix"); 
    GLuint textureLoc = glGetUniformLocation(shaderProgram, "textureDiffuse"); 
    GLuint cameraGlobalPositionLoc = glGetUniformLocation(shaderProgram, "gCameraPos"); 

    // set ModelMatrix and NormalMatrix 
    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(curModelMatrix)); 
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(curNormalMatrix));

    { // set light
        DirectionalLight& directionalLight = this->GetDirectionalLight(); // IDK what the end sign does
        GLint location;

        // Set BaseLight fields
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.Color");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.base.color));

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.AmbientIntensity");
        glUniform1f(location, directionalLight.base.ambientIntensity);

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.DiffuseIntensity");
        glUniform1f(location, directionalLight.base.diffuseIntensity);

        // Set Direction field
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Direction");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.direction));
    }

    // set cameraPosition
    glUniform3f(
        cameraGlobalPositionLoc,
        cameraGlobalPos[0],
        cameraGlobalPos[1],
        cameraGlobalPos[2]
    );

    glBindVertexArray(VAO);

    for (unsigned int i = 0; i < meshes.size(); i++) {
        unsigned int MaterialIndex = meshes[i].MaterialIndex;

        // assert(MaterialIndex < m_Materials.size()); todo: how do I assert?

        //if (m_Materials[MaterialIndex].pDiffuse) { todo: material is currently ignored
        //    m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        //}

        //if (m_Materials[MaterialIndex].pSpecularExponent) {
        //    m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
        //}

        // get Mesh->World(SceneRoot)->World(InGame)->View->Projection transformation
        curMwvp = viewProj * tranform * this->meshIndex2meshTransform[i]; 
        curMwvp = viewProj * tranform;

        //{ // debugging render 
        //    std::cout << "ViewProj Mat: " << std::endl << viewProj << std::endl;
        //    std::cout << "GameWorld Mat: " << std::endl << tranform << std::endl;
        //    std::cout << "MeshTransform Mat: " << std::endl << this->meshIndex2meshTransform[i] << std::endl;
        //    std::cout << "MWVP Array" << std::endl << curMwvp << std::endl; 
        //    glm::vec4 temp; 
        //    for (const glm::vec3& position : positions) {
        //        temp = glm::vec4(position, 1); 
        //        temp = curMwvp * temp
        //        std::cout << "Current Vert Pos: " << position << std::endl; 
        //        std::cout << "After Projection: " << temp << std::endl;
        //    std::string msg = "IndexNum in Currently drawn mesh: " + std::to_string(meshes[i].NumIndices);
        //    log(msg);
        //    }
        //}
        // // debugging

        // get TextureIndex 
        // curTextureIndex = this->textures[i].GetTextureIndex();
        curTextureIndex = ptrTexture->GetTextureIndex();

        // set MWVP transformation
        glUniformMatrix4fv(mwvpLoc, 1, GL_FALSE, glm::value_ptr(curMwvp));

        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, curTextureIndex);
        glUniform1i(textureLoc, 0);

        

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), // Specifies a pointer to the location where the indices are stored 
            meshes[i].BaseVertex
        ); // Specifies a constant that should be added to each element of indices when chosing elements from the enabled vertex arrays.
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0); 
    glUseProgram(shaderProgram);
}

void RiggedMesh::Render(CameraObject& cameraObj, glm::mat4& tranform, Texture* ptrTexture)
{
    if (!sharedShaderProgram) {
        std::cout << "Object ??? Missing sharedShaderProgram" << std::endl;
    }
    GLuint shaderProgram = this->sharedShaderProgram;

    glm::mat4 curMwvp = glm::mat4(1);
    glm::mat4 viewProj = cameraObj.GetviewProjMat();

    glm::vec3 cameraGlobalPos = cameraObj.GetGlobalCameraPosition();


    GLuint curTextureIndex = 0;

    GLuint mwvpLoc = glGetUniformLocation(shaderProgram, "mwvp");
    GLuint textureLoc = glGetUniformLocation(shaderProgram, "textureDiffuse");
    GLuint cameraGlobalPositionLoc = glGetUniformLocation(shaderProgram, "gCameraPos");

    { // set light
        DirectionalLight& directionalLight = this->GetDirectionalLight(); // IDK what the end sign does
        GLint location;

        // Set BaseLight fields
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.Color");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.base.color));

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.AmbientIntensity");
        glUniform1f(location, directionalLight.base.ambientIntensity);

        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Base.DiffuseIntensity");
        glUniform1f(location, directionalLight.base.diffuseIntensity);

        // Set Direction field
        location = glGetUniformLocation(shaderProgram, "gDirectionalLight.Direction");
        glUniform3fv(location, 1, glm::value_ptr(directionalLight.direction));
    }

    // set cameraPosition
    glUniform3f(
        cameraGlobalPositionLoc,
        cameraGlobalPos[0],
        cameraGlobalPos[1],
        cameraGlobalPos[2]
    );

    // get Mesh->World(SceneRoot)->World(InGame)->View->Projection transformation
    curMwvp = viewProj * this->model2WorldTransform.GetTransformMatrix() * tranform;

    // set MWVP transformation
    glUniformMatrix4fv(mwvpLoc, 1, GL_FALSE, glm::value_ptr(curMwvp));

    glBindVertexArray(VAO);

    for (unsigned int i = 0; i < meshes.size(); i++) {
        unsigned int MaterialIndex = meshes[i].MaterialIndex;

        // assert(MaterialIndex < m_Materials.size()); todo: how do I assert?

        //if (m_Materials[MaterialIndex].pDiffuse) { todo: material is currently ignored
        //    m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        //}

        //if (m_Materials[MaterialIndex].pSpecularExponent) {
        //    m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
        //}

        // get TextureIndex 
        // curTextureIndex = this->textures[i].GetTextureIndex();
        curTextureIndex = ptrTexture->GetTextureIndex();

        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, curTextureIndex);
        glUniform1i(textureLoc, 0);

        glDrawElementsBaseVertex(GL_TRIANGLES,
            meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * meshes[i].BaseIndex),
            meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}

//const Material& RiggedMesh::GetMaterial()
//{
//    for (unsigned int i = 0; i < m_Materials.size(); i++) {
//        if (m_Materials[i].AmbientColor != Vector4f(0.0f, 0.0f, 0.0f, 0.0f)) {
//            return m_Materials[i];
//        }
//    }
//
//    return m_Materials[0];
//}


uint32_t RiggedMesh::FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void RiggedMesh::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    uint32_t PositionIndex = this->FindPosition(AnimationTimeTicks, pNodeAnim);
    uint32_t NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
    float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}


uint32_t RiggedMesh::FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
        float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void RiggedMesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    uint32_t RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
    uint32_t NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
    float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out.Normalize();
}


uint32_t RiggedMesh::FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
        float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void RiggedMesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    uint32_t ScalingIndex = FindScaling(AnimationTimeTicks, pNodeAnim);
    uint32_t NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float t1 = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
    float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}




void StaticMesh::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const glm::mat4& ParentTransform)
{
    // Get the node name and its local transformation
    std::string NodeName(pNode->mName.data);

    // Node transformation in local space
    glm::mat4 NodeTransformation(ConvertToGlmMat4(pNode->mTransformation));

    //// todo: not implemented animation not available
    //const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

    //if (pNodeAnim) { 
    //    // Interpolate scaling and generate scaling transformation matrix
    //    aiVector3D Scaling;
    //    CalcInterpolatedScaling(Scaling, AnimationTimeTicks, pNodeAnim);
    //    Matrix4f ScalingM;
    //    ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

    //    // Interpolate rotation and generate rotation transformation matrix
    //    aiQuaternion RotationQ;
    //    CalcInterpolatedRotation(RotationQ, AnimationTimeTicks, pNodeAnim);
    //    glm::mat4 RotationM = Matrix4f(RotationQ.GetMatrix());

    //    // Interpolate translation and generate translation transformation matrix
    //    aiVector3D Translation;
    //    CalcInterpolatedPosition(Translation, AnimationTimeTicks, pNodeAnim);
    //    Matrix4f TranslationM;
    //    TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

    //    // Combine the above transformations
    //    NodeTransformation = TranslationM * RotationM * ScalingM;
    //}

    // Combine with parent transformation to get global transformation
    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation; 

    // Check if this node references any meshes
    for (uint32_t i = 0; i < pNode->mNumMeshes; i++) {
        uint32_t MeshIndex = pNode->mMeshes[i]; // Get the mesh index

        // Apply the global transformation to the mesh
        this->meshIndex2meshTransform[MeshIndex] = GlobalTransformation; 
    }

    // Recursively process child nodes
    for (uint32_t i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation);
    }
}




void RiggedMesh::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const glm::mat4& ParentTransform)
{
    // Read the current Node and Set tranformation for each corresponding bone
    // recursively call children below it 

    std::string NodeName(pNode->mName.data);

    const aiAnimation* pAnimation = ptrScene->mAnimations[0];

    glm::mat4 NodeTransformation(ConvertToGlmMat4(pNode->mTransformation));


    //// todo: not implemented animation not available
    //const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

    //if (pNodeAnim) { 
    //    // Interpolate scaling and generate scaling transformation matrix
    //    aiVector3D Scaling;
    //    CalcInterpolatedScaling(Scaling, AnimationTimeTicks, pNodeAnim);
    //    Matrix4f ScalingM;
    //    ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

    //    // Interpolate rotation and generate rotation transformation matrix
    //    aiQuaternion RotationQ;
    //    CalcInterpolatedRotation(RotationQ, AnimationTimeTicks, pNodeAnim);
    //    glm::mat4 RotationM = Matrix4f(RotationQ.GetMatrix());

    //    // Interpolate translation and generate translation transformation matrix
    //    aiVector3D Translation;
    //    CalcInterpolatedPosition(Translation, AnimationTimeTicks, pNodeAnim);
    //    Matrix4f TranslationM;
    //    TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

    //    // Combine the above transformations
    //    NodeTransformation = TranslationM * RotationM * ScalingM;
    //}

    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (this->boneName2IndexMap.find(NodeName) != this->boneName2IndexMap.end()) {
        uint32_t BoneIndex = this->boneName2IndexMap[NodeName];


        this->boneIndex2BoneInfo[BoneIndex].bone2GlobalCoord = 
            this->GlobalInverseTransform * GlobalTransformation 
            * this->boneIndex2BoneInfo[BoneIndex].boneOffset;
    }

    for (uint32_t i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation);
    }
}


void RiggedMesh::GetBoneTransforms(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
{
    glm::mat4 Identity = glm::mat4(1.0);

    float TicksPerSecond = (float)(ptrScene->mAnimations[0]->mTicksPerSecond != 0 ? ptrScene->mAnimations[0]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTimeTicks = fmod(TimeInTicks, (float)ptrScene->mAnimations[0]->mDuration);

    ReadNodeHierarchy(AnimationTimeTicks, ptrScene->mRootNode, Identity);
    Transforms.resize(this->boneIndex2BoneInfo.size());

    for (uint32_t i = 0; i < this->boneIndex2BoneInfo.size(); i++) {
        Transforms[i] = this->boneIndex2BoneInfo[i].bone2GlobalCoord;
    }
}


//const aiNodeAnim* RiggedMesh::FindNodeAnim(const aiAnimation* pAnimation, const string& NodeName)
//{
//    for (uint i = 0; i < pAnimation->mNumChannels; i++) {
//        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
//
//        if (string(pNodeAnim->mNodeName.data) == NodeName) {
//            return pNodeAnim;
//        }
//    }
//
//    return NULL;
//}
