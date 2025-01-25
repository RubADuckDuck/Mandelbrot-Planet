#pragma once
#include "LOG.h" 
#include "Network/GameState.h"
#include <stack>
#include "MeshTextureLoader.h"

class Renderer {
public:
    std::string GetName() const;

private:
    void log(LogLevel level, std::string text);

    GameState* gameState_; 
    // (mesh & texture id) -> (mesh & texture)

    std::unique_ptr<CameraObject> ptrCamera_; 
     
    std::unique_ptr<Type2MeshAndTexture<uint32_t>> id2MeshAndTexture;

public: 
    Renderer(GameState* gameState);

    void DrawRespectTo(uint32_t objID, uint8_t ascendLevels, uint8_t descendDepth);

private:

    void DrawMesh(uint32_t meshID, uint32_t textureID, glm::mat4 transfromMatrix);

};