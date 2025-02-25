#pragma once
#include <memory>
#include "Command.h"


//// Modified GameStateManager
//class GameStateManager {
//public:
//    GameStateManager() {
//        setupNetworking();
//    }
//
//    void ExecuteCommand(std::unique_ptr<IGameCommand> command) {
//        command->Execute(m_gameState);
//    }
//
//private:
//    void setupNetworking();
//
//    GameState m_gameState;
//    std::unique_ptr<NetworkCodec> m_networkCodec;
//};