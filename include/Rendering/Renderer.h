#pragma once
#include "Utils/LOG.h" 
#include "Network/GameState.h"
#include <stack>
#include "Rendering/MeshTextureLoader.h"



class Renderer {
public:
    std::string GetName() const;


private:
    void log(LogLevel level, std::string text);

    struct ParentWithGridIndex {
        GameObject* ptrParent = nullptr;
        uint8_t gridIndex = 0;
    };

    using GameObjectChain = std::vector<ParentWithGridIndex>;

    GameObjectChain previousGameObjectChain; 

    GameState* gameState_; 
    // (mesh & texture id) -> (mesh & texture)

    std::unique_ptr<CameraObject> ptrCamera_; 
     
    std::unique_ptr<Type2MeshAndTexture<uint32_t>> id2MeshAndTexture;

public: 
    Renderer(GameState* gameState);

    void SetTransformationsForEachGameObject();

    void SetTransformationChainForEachGameObject();

    void DrawDepth(uint32_t objID, uint8_t descendDepth);

    void DrawRespectTo(uint32_t objID, uint8_t ascendLevels, uint8_t descendDepth);

private:

    void DrawMesh(uint32_t meshID, uint32_t textureID, glm::mat4 transfromMatrix);

};