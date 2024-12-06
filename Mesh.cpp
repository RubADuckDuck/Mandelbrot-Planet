

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags 
#include <cassert>
#include "Mesh.h"


// Function to convert aiMatrix4x4 to glm::mat4
glm::mat4 ConvertToGlmMat4(const aiMatrix4x4& aiMat) {
    glm::mat4 glmMat;

    glmMat[0][0] = aiMat.a1; glmMat[0][1] = aiMat.b1; glmMat[0][2] = aiMat.c1; glmMat[0][3] = aiMat.d1;
    glmMat[1][0] = aiMat.a2; glmMat[1][1] = aiMat.b2; glmMat[1][2] = aiMat.c2; glmMat[1][3] = aiMat.d2;
    glmMat[2][0] = aiMat.a3; glmMat[2][1] = aiMat.b3; glmMat[2][2] = aiMat.c3; glmMat[2][3] = aiMat.d3;
    glmMat[3][0] = aiMat.a4; glmMat[3][1] = aiMat.b4; glmMat[3][2] = aiMat.c4; glmMat[3][3] = aiMat.d4;

    return glmMat;
}

int SceneLoader::GetBoneID(const aiBone* pBone)
{
    int bone_id = 0;
    std::string bone_name(pBone->mName.C_Str());

    // each bone has to have a different name to assign the ID
    if (bone_name_to_index_map.find(bone_name) == bone_name_to_index_map.end()) { 
        // Allocate an index for a new bone
        bone_id = (int)bone_name_to_index_map.size();
        bone_name_to_index_map[bone_name] = bone_id;
    }
    else {
        bone_id = bone_name_to_index_map[bone_name];
    }

    return bone_id;
}

void SceneLoader::ParseSingleBone(int mesh_index, const aiBone* pBone)
{
    printf("      Bone '%s': num vertices affected by this bone: %d\n", pBone->mName.C_Str(), pBone->mNumWeights);

    int bone_id = this->GetBoneID(pBone);
    // printf("bone id %d\n", bone_id); 

    

    for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
        if (i == 0) printf("\n");
        const aiVertexWeight& vw = pBone->mWeights[i]; // vw has mVertexId and mWeight as attribute 

        uint32_t global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId;  // calculate global id
        printf("Vertex id %d ", global_vertex_id);

        // for this to be true if bone1 refers to vertices in Mesh1 and Mesh2 but the bone is under Mesh1
        // Vertices on mesh2 should not be here.
        assert(global_vertex_id < vertexID2BoneDatas.size()); 
        vertexID2BoneDatas[global_vertex_id].AddBoneData(bone_id, vw.mWeight);
    }

    printf("\n");
}


void SceneLoader::ParseMeshBones(int mesh_index, const aiMesh* pMesh)
{
    for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
        ParseSingleBone(mesh_index, pMesh->mBones[i]);
    }
}


void SceneLoader::ParseMeshes(const aiScene* pScene)
{
    printf("*******************************************************\n");
    printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    mesh_base_vertex.resize(pScene->mNumMeshes);

    for (unsigned int i = 0; i < pScene->mNumMeshes; i++) { const aiMesh* pMesh = pScene->mMeshes[i];
        int num_vertices = pMesh->mNumVertices;
        int nuindices = pMesh->mNumFaces * 3;
        int nubones = pMesh->mNumBones;
        printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, nuindices, nubones);
        total_vertices += num_vertices;
        total_indices += nuindices;
        total_bones += nubones;

        vertexID2BoneDatas.resize(total_vertices);

        if (pMesh->HasBones()) {
            this->ParseMeshBones(i ,pMesh);
        }

        printf("\n");
    }

    printf("\nTotal vertices %d total indices %d total bones %d\n", total_vertices, total_indices, total_bones);
}


void SceneLoader::ParseScene(const aiScene* pScene)
{
    this->ParseMeshes(pScene);
}


bool RiggedMesh::LoadMesh(const std::string& filename) {
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
        GlobalInverseTransform = ptrScene->mRootNode->mTransformation; 
        GlobalInverseTransform = GlobalInverseTransform.Inverse(); // todo 
        Ret = this->InitFromScene(ptrScene, filename); 

    } else {
        printf("Error parsing '%s': '%s'\n", filename.c_str(), Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}

bool RiggedMesh::InitFromScene(const aiScene* ptrScene, const std::string& filename) {
    this->meshes.resize(ptrScene->mNumMeshes); 
   
    unsigned int numVertices = 0; 
    unsigned int numIndices = 0; 

    this->CountVerticesAndIndices(ptrScene, numVertices, numIndices); 

    this->ReserveSpace(numVertices, numIndices); 

    InitAllMeshes(ptrScene); 

    //if (!this->InitMaterials(pScene, Filename)) {
    //    return false;
    //}

    this->PopulateBuffers(); 

    return GLCheckError(); // todo
}

void RiggedMesh::CountVerticesAndIndices(const aiScene* pScene, unsigned int& numVertices, unsigned int& numIndices)
{
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].MaterialIndex = this->ptrScene->mMeshes[i]->mMaterialIndex;
        meshes[i].NumIndices = this->ptrScene->mMeshes[i]->mNumFaces * 3;
        meshes[i].BaseVertex = numVertices; // baseVertex is the Global index of the first vertex in mesh 
        meshes[i].BaseIndex = numIndices;   // goes same for Indices 

        numVertices += pScene->mMeshes[i]->mNumVertices;
        numIndices += meshes[i].NumIndices;
    }
} 

void RiggedMesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    this->positions.reserve(NumVertices);
    this->normals.reserve(NumVertices);
    this->texCoords.reserve(NumVertices);
    this->indices.reserve(NumIndices);
    this->bones.resize(NumVertices);
} 

void RiggedMesh::InitAllMeshes(const aiScene* ptrScene)
{
    for (unsigned int i = 0; i < this->meshes.size(); i++) {
        const aiMesh* paiMesh = ptrScene->mMeshes[i];
        InitSingleMesh(i, paiMesh); 
    }
} 

void RiggedMesh::InitSingleMesh(uint32_t MeshIndex, const aiMesh* ptraiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < ptraiMesh->mNumVertices; i++) {

        const aiVector3D& pPos = ptraiMesh->mVertices[i];
        positions.push_back(glm::vec3(pPos.x, pPos.y, pPos.z));

        if (ptraiMesh->mNormals) {
            const aiVector3D& pNormal = ptraiMesh->mNormals[i];
            normals.push_back(glm::vec3(pNormal.x, pNormal.y, pNormal.z));
        }
        else {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            normals.push_back(glm::vec3(Normal.x, Normal.y, Normal.z));
        }

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
    for (uint32_t i = 0; i < ptrMesh->mNumBones; i++) {
        this->LoadSingleBone(MeshIndex, ptrMesh->mBones[i]); 
    }
}

void RiggedMesh::LoadSingleBone(uint32_t MeshIndex, const aiBone* ptrBone) {
    int BoneId = this->GetBoneId(ptrBone);

    if (BoneId == this->boneID2BoneInfo.size()) {
        aiMatrix4x4 curOffsetMat = ptrBone->mOffsetMatrix;

        BoneInfo bi(ConvertToGlmMat4(curOffsetMat));
        boneID2BoneInfo.push_back(bi);
    }

    for (uint32_t i = 0; i < ptrBone->mNumWeights; i++) {
        const aiVertexWeight& vw = ptrBone->mWeights[i];
        // globalVertexID = (index of first vertex in mesh) + (local Vertex ID)
        uint32_t GlobalVertexID = meshes[MeshIndex].BaseVertex + ptrBone->mWeights[i].mVertexId; 
        bones[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
    }
}

//------------------


int RiggedMesh::GetBoneId(const aiBone* ptrBone) {
    int BoneIndex = 0;
    std::string BoneName(ptrBone->mName.C_Str());

    if (this->boneName2IndexMap.find(BoneName) == this->boneName2IndexMap.end()) {
        // Allocate an index for a new bone
        BoneIndex = (int)this->boneName2IndexMap.size();
        this->boneName2IndexMap[BoneName] = BoneIndex;
    }
    else {
        BoneIndex = this->boneName2IndexMap[BoneName];
    }

    return BoneIndex;
}


//bool RiggedMesh::InitMaterials(const aiScene* pScene, const string& Filename)
//{
//    string Dir = GetDirFromFilename(Filename);
//
//    bool Ret = true;
//
//    printf("Num materials: %d\n", pScene->mNumMaterials);
//
//    // Initialize the materials
//    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
//        const aiMaterial* pMaterial = pScene->mMaterials[i];
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
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
        (const GLvoid*)(MAX_NUbones_PER_VERTEX * sizeof(int32_t)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
}


// Introduced in youtube tutorial #18
void RiggedMesh::Render()
{
    glBindVertexArray(VAO);

    for (unsigned int i = 0; i < meshes.size(); i++) {
        unsigned int MaterialIndex = meshes[i].MaterialIndex;

        // assert(MaterialIndex < m_Materials.size()); todo: how do I assert?

        if (m_Materials[MaterialIndex].pDiffuse) {
            m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_Materials[MaterialIndex].pSpecularExponent) {
            m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
            meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * meshes[i].BaseIndex),
            meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


const Material& RiggedMesh::GetMaterial()
{
    for (unsigned int i = 0; i < m_Materials.size(); i++) {
        if (m_Materials[i].AmbientColor != Vector4f(0.0f, 0.0f, 0.0f, 0.0f)) {
            return m_Materials[i];
        }
    }

    return m_Materials[0];
}


uint RiggedMesh::FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    for (uint i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
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

    uint PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
    uint NextPositionIndex = PositionIndex + 1;
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


uint RiggedMesh::FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
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

    uint RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
    uint NextRotationIndex = RotationIndex + 1;
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


uint RiggedMesh::FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
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

    uint ScalingIndex = FindScaling(AnimationTimeTicks, pNodeAnim);
    uint NextScalingIndex = ScalingIndex + 1;
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


void RiggedMesh::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform)
{
    string NodeName(pNode->mName.data);

    const aiAnimation* pAnimation = pScene->mAnimations[0];

    Matrix4f NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        aiVector3D Scaling;
        CalcInterpolatedScaling(Scaling, AnimationTimeTicks, pNodeAnim);
        Matrix4f ScalingM;
        ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        CalcInterpolatedRotation(RotationQ, AnimationTimeTicks, pNodeAnim);
        Matrix4f RotationM = Matrix4f(RotationQ.GetMatrix());

        // Interpolate translation and generate translation transformation matrix
        aiVector3D Translation;
        CalcInterpolatedPosition(Translation, AnimationTimeTicks, pNodeAnim);
        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(Translation.x, Translation.y, Translation.z);

        // Combine the above transformations
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (this->boneName2IndexMap.find(NodeName) != this->boneName2IndexMap.end()) {
        uint BoneIndex = this->boneName2IndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation);
    }
}


void RiggedMesh::GetBoneTransforms(float TimeInSeconds, vector<Matrix4f>& Transforms)
{
    Matrix4f Identity;
    Identity.InitIdentity();

    float TicksPerSecond = (float)(pScene->mAnimations[0]->mTicksPerSecond != 0 ? pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTimeTicks = fmod(TimeInTicks, (float)pScene->mAnimations[0]->mDuration);

    ReadNodeHierarchy(AnimationTimeTicks, pScene->mRootNode, Identity);
    Transforms.resize(m_BoneInfo.size());

    for (uint i = 0; i < m_BoneInfo.size(); i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}


const aiNodeAnim* RiggedMesh::FindNodeAnim(const aiAnimation* pAnimation, const string& NodeName)
{
    for (uint i = 0; i < pAnimation->mNumChannels; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}
