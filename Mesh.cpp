

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags 
#include <cassert>
#include "Mesh.h"


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
        int num_indices = pMesh->mNumFaces * 3;
        int num_bones = pMesh->mNumBones;
        printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);
        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

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
