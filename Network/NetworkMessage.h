#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <array>
#include <stdexcept>
#include <iostream> 
#include <chrono>
#include <random>

#include "Command.h"
#include "NetworkConfig.h"


// Forward declarations
class PlayerPosition;
class PlayerInput;
class GameState;

class GameMessageProcessor;


// Get current timestamp in milliseconds since epoch
inline uint64_t get_current_timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

// Generate a cryptographically secure random number for verification
inline uint64_t generate_verification_code() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

// Message type identifiers
enum class MessageType : uint8_t {
    PLAYER_INPUT, // only message client sends to server
    ADD_GAMEOBJECT, 
    REMOVE_GAMEOBJECT, 
    GAMEOBJECT_POSITION, 
    GAMEOBJECT_PARENT_OBJECT,

    AUTHENTICATION, 
    UDP_VERIFICATION,
    FULL_GAME_STATE

    // Add more message types as needed
};

// Messages serialize and deserialize by themselves 
// Serialize: Message -> Data 
// Deserialize: Data -> Message

// Big picture would be 
// Client::Input -> Client::Data -> Server::Data -> Server::Input 
// Server::Input : state1 -> state2  results in altered state. 
// When a Game object is altered. 
// Server::msg = Server::GameObect::CreateMessage 
// Server::msg -> Server::Data -> Client::Data -> Client::Message -> Client::Command 
// Client::Command : state1 -> state2 

// DeltaState -> Message <-> Data <-> Message -> Command -> DeltaState 

// Base interface for all network messages
class INetworkMessage {
public:
    virtual ~INetworkMessage() = default;
    virtual MessageType GetType() const = 0;
    virtual size_t GetSize() const = 0;  
    virtual std::vector<uint8_t> Serialize() const = 0;
    virtual void Deserialize(const std::vector<uint8_t>& data) = 0;

    static void add_int(std::vector<uint8_t>& buffer, int val);

    static void add_float(std::vector<uint8_t>& buffer, float val);

    template <typename T> 
    static void add_to_buffer(std::vector<uint8_t>& buffer, T val) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&val);
        buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
    }

    template <typename T> 
    static T extract_from_data(const std::vector<uint8_t>& data, size_t& offset) {
        T value;
        std::memcpy(&value, data.data() + offset, sizeof(T));
        offset += sizeof(T);
        return value;
    }
}; 

class AuthRequestMessage : public INetworkMessage {
public:
    // Protocol version to ensure compatibility between client and server
    uint32_t protocol_version;

    uint32_t client_id;

    // UDP port that the client will use for game updates
    uint16_t client_udp_port;

    // Authentication token - we'll keep this variable length since
    // tokens often need to be flexible in size
    std::string auth_token;

    // Constructor for creating new auth requests
    AuthRequestMessage(
        uint32_t version,
        const uint32_t& id,
        uint16_t udp_port,
        const std::string& token = "");

    // Default constructor initializes client_id array to zeros
    AuthRequestMessage();

    MessageType GetType() const override;

    size_t GetSize() const override;

    std::vector<uint8_t> Serialize() const override;

    void Deserialize(const std::vector<uint8_t>& data) override;
};

class UdpVerificationMessage : public INetworkMessage {
public:

    uint32_t session_id;

    // Keeping the 64-bit verification code
    uint64_t verification_code;

    // Keeping the 64-bit timestamp
    uint64_t timestamp;

    UdpVerificationMessage(const uint32_t& session, uint64_t code);

    // Default constructor for deserialization - Randomly init verification code and session_id
    UdpVerificationMessage();

    MessageType GetType() const override;

    size_t GetSize() const override;

    std::vector<uint8_t> Serialize() const override;

    void Deserialize(const std::vector<uint8_t>& data) override;

    void set_random_session_id();
};

// -------------------------------------------------------------------------------------------------------

class PlayerInputMessage : public INetworkMessage {
public: 
    Direction playerDirection;  

    uint32_t playerID = 0; 

    PlayerInputMessage(Direction direction, uint32_t id);

    PlayerInputMessage();

    MessageType GetType() const override;

    size_t GetSize() const override;

    std::vector<uint8_t> Serialize() const override;

    void Deserialize(const std::vector<uint8_t>& data) override;
}; 

class AddGameObjectMessage : public INetworkMessage {
public: 
    uint8_t gameObjectTypeID = 0;  

    uint32_t gameObjectID = 0;  

    AddGameObjectMessage(uint8_t typeID, uint32_t objID);

    AddGameObjectMessage();

    MessageType GetType() const override;

    size_t GetSize() const override;

    std::vector<uint8_t> Serialize() const override;

    void Deserialize(const std::vector<uint8_t>& data) override;
};

class RemoveGameObjectMessage : public INetworkMessage {
public: 
    uint32_t gameObjectID = 0; 

    RemoveGameObjectMessage(uint32_t objID);

    RemoveGameObjectMessage();

    MessageType GetType() const override;

    size_t GetSize() const override;

    std::vector<uint8_t> Serialize() const override;

    void Deserialize(const std::vector<uint8_t>& data) override;
};

class GameObjectPositionMessage : public INetworkMessage {
public: 
    int y = 0; 
    int x = 0;  

    uint32_t gameObjectID = 0;  

    GameObjectPositionMessage(int y, int x, uint32_t id);

    GameObjectPositionMessage();

    MessageType GetType() const override;

    size_t GetSize() const override;

    std::vector<uint8_t> Serialize() const override;

    void Deserialize(const std::vector<uint8_t>& data) override;
}; 

class GameObjectParentObjectMessage : public INetworkMessage {
public:
    uint32_t parentObjectID = 0; 

    uint32_t gameObjectID = 0; 

    GameObjectParentObjectMessage(uint32_t parentID, uint32_t objID);

    GameObjectParentObjectMessage();

    MessageType GetType() const override;

    size_t GetSize() const override;

    std::vector<uint8_t> Serialize() const override;

    void Deserialize(const std::vector<uint8_t>& data) override;
}; 


//// Example message implementation
//class PlayerPositionMessage : public INetworkMessage {
//public:
//    float x = 0.0f;
//    float y = 0.0f;
//    float z = 0.0f;
//    uint32_t player_id = 0;
//
//    MessageType GetType() const override {
//        return MessageType::PLAYER_POSITION;
//    }
//
//    std::vector<uint8_t> Serialize() const override {
//        std::vector<uint8_t> buffer;
//        buffer.reserve(sizeof(MessageType) + sizeof(float) * 3 + sizeof(uint32_t));
//
//        // Add message type
//        buffer.push_back(static_cast<uint8_t>(GetType()));
//
//        // Add position data
//        auto addFloat = [&buffer](float value) {
//            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
//            buffer.insert(buffer.end(), bytes, bytes + sizeof(float));
//        };
//
//        addFloat(x);
//        addFloat(y);
//        addFloat(z);
//
//        // Add player ID
//        const uint8_t* id_bytes = reinterpret_cast<const uint8_t*>(&player_id);
//        buffer.insert(buffer.end(), id_bytes, id_bytes + sizeof(uint32_t));
//
//        return buffer;
//    }
//
//    void Deserialize(const std::vector<uint8_t>& data) override {
//        if (data.size() < sizeof(MessageType) + sizeof(float) * 3 + sizeof(uint32_t)) {
//            throw std::runtime_error("Invalid message size");
//        }
//
//        size_t offset = 1; // Skip message type byte
//
//        // Extract position data
//        auto readFloat = [&data, &offset]() {
//            float value;
//            std::memcpy(&value, data.data() + offset, sizeof(float));
//            offset += sizeof(float);
//            return value;
//        };
//
//        x = readFloat();
//        y = readFloat();
//        z = readFloat();
//
//        // Extract player ID
//        std::memcpy(&player_id, data.data() + offset, sizeof(uint32_t));
//    }
//};


// Message factory for creating messages from raw data

class MessageFactory { // Data -> Message
public:
    static std::unique_ptr<INetworkMessage> CreateMessage(const std::vector<uint8_t>& data);
};

// Network codec class that handles encoding and decoding
// NetworkCodec::Encode : Message -> Data 
// NetworkCodec::Decode : Data -> Message

class IGameCommand;

// NetworkCodec::HandleNetworkMessage : Data x GameState -> GameState
class NetworkCodec { 
private:
public:
    // Encode a message to bytes
    static std::vector<uint8_t> Encode(const INetworkMessage* message);

    // Decode bytes to a message
    static std::unique_ptr<INetworkMessage> Decode(const std::vector<uint8_t>& data);

    // Helper method to handle incoming UDP/TCP messages
    static void HandleNetworkMessage(const std::vector<uint8_t>& data, GameState& gameState);
};


// Message processor that converts network messages to game commands
// Message -> Command
class IMessageProcessor {
public:
    virtual ~IMessageProcessor() = default;
};

// Concrete message processor implementation
// Message -> Command
class GameMessageProcessor : public IMessageProcessor {
private:

public:
    static GameMessageProcessor& GetInstance();

    static std::unique_ptr<IGameCommand> ProcessMessage(const INetworkMessage& message);
};

