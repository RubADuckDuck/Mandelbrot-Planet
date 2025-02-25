#include "Network/GameStateManager.h"
#include "Network/NetworkMessage.h"
#include "Core/Event.h"

//void GameStateManager::setupNetworking() {
//    // Create message processor
//    auto processor = std::make_shared<GameMessageProcessor>();
//
//    // Create network codec with callback to this manager
//    m_networkCodec = std::make_unique<NetworkCodec>(
//        processor,
//        [this](std::unique_ptr<IGameCommand> command) {
//            this->ExecuteCommand(std::move(command));
//        }
//    );
//
//    // Subscribe to network messages
//    EventDispatcher& dispatcher = EventDispatcher::GetInstance();
//
//    /*dispatcher.Subscribe(Tag::UDP, [this](const std::vector<uint8_t>& msg) {
//        m_networkCodec->HandleNetworkMessage(msg);
//        });
//
//    dispatcher.Subscribe(Tag::TCP, [this](const std::vector<uint8_t>& msg) {
//        m_networkCodec->HandleNetworkMessage(msg);
//        });*/
//}
