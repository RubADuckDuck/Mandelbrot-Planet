#include "HostLobbyMode.h"
#include "Network/NetworkMessage.h"

HostLobbyMode::HostLobbyMode(GameEngine* engine) : GameMode(engine) {}

void HostLobbyMode::Enter() {
    unsigned short tcp_port = 10429; 
    unsigned short udp_port = 20429;


    LOG(LOG_INFO, "HostLobbyMode::Initializing Gameserver");
    // Initialize the server when entering host mode
    server = std::make_unique<GameServer>(*(gameEngine->GetIOContext()), tcp_port, udp_port);

    LOG(LOG_INFO, "HostLobbyMode::Run IO Context on IO Thread"); 

    gameEngine->RunIOContextOnIOThread();  


    {
        this->TestRendering(); 
    }
}

void HostLobbyMode::Update() {
    // Check for new connections
    // server->Update();

    // If player clicks "Start Game" and we have players connected
    if (/* start game button clicked */ false && !connectedPlayers.empty()) {
        // Switch to PlayingMode
        gameEngine->GetModeController()->SwitchMode(GameModeType::PLAYING);
    }

    
}

void HostLobbyMode::Update(float delta_time)
{
    server->GetGameState()->UpdateGameState(delta_time);
}

void HostLobbyMode::Draw() {
    // Draw the lobby UI
    // You'll need to implement actual rendering
    // Show connected players
    // Show "Start Game" button when ready

    this->server->GetGameState()->Draw();
}

void HostLobbyMode::Exit() {
    // Clean up server when leaving
    server.reset();
    connectedPlayers.clear();
}

void HostLobbyMode::TestRendering()
{        
    // test purpose 

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();

    AddRidableObjectMessage* cur_aro_msg; 
    RideOnRidableObjectMessage* cur_ror_msg; 

    std::vector<uint8_t> data; 

    // spawn 8 objects 
    for (uint8_t i = 0; i < 25; i++) {
        cur_aro_msg= new AddRidableObjectMessage(); 
        cur_aro_msg->gridHeight_ = 2;  

        data = cur_aro_msg->Serialize();  
        delete cur_aro_msg; 

        dispatcher.Publish(data);  
    }

    //----------------------------------------------------------
    cur_ror_msg = new RideOnRidableObjectMessage(); 
    cur_ror_msg->vehicleID = 1;
    for (uint8_t i = 2; i < 26; i++) {
        cur_ror_msg->riderID = i;
        cur_ror_msg->rideAt = i - 2;
        data = cur_ror_msg->Serialize();

        dispatcher.Publish(data);
    }

    
}
