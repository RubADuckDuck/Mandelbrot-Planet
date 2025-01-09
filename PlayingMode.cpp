#include "PlayingMode.h"
#include "GameEngine.h"
#include "TerrainManager.h"

PlayingMode::PlayingMode(GameEngine* engine) : GameMode(engine), terrainManager(nullptr), isNetworked(false) {}

PlayingMode::~PlayingMode() {
    if (terrainManager) {
        delete terrainManager;
    }
}

void PlayingMode::Enter() {
    // This is called when we switch to playing mode
    std::string objPath = "E:\\repos\\[DuckFishing]\\model\\duck.obj";
    std::string texturePath = "E:/repos/[DuckFishing]/model/texture/duck.png";

    terrainManager = new TerrainManager();
    terrainManager->SetGameEngine(gameEngine);
    terrainManager->CreateAndAddPlayer(objPath, texturePath);
    terrainManager->SubscribeItemListener();

    // Initialize some game objects
    terrainManager->CreateAndAddDroppedItemAt(1, 1, ItemType::PYTHON);
    terrainManager->BuildFactoryAt(FactoryType::Bakery, 5, 5, 2, 1);

    gameEngine->DirectlyAddGameObject(terrainManager);
}

void PlayingMode::Update() {
    // Your existing game update logic from MainLoop

}

void PlayingMode::Draw() {
    // Your existing game rendering logic

}

void PlayingMode::Exit() {
    // Clean up when leaving playing mode

}
