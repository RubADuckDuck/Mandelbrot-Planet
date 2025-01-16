#include <unordered_map>
#include <string>

#include "NetworkMessage.h"
#include "../PlayerDirection.h"
#include "../LOG.h"
#include "../RidableObject.h"

// ... (your enum definition) ...

std::unordered_map<MessageType, std::string> messageType2string = {
    {MessageType::PLAYER_INPUT, "PLAYER_INPUT"},
    {MessageType::INTERACTION_INFO, "INTERACTION_INFO"},
    {MessageType::ADD_GAMEOBJECT, "ADD_GAMEOBJECT"},
    {MessageType::REMOVE_GAMEOBJECT, "REMOVE_GAMEOBJECT"},
    {MessageType::GAMEOBJECT_POSITION, "GAMEOBJECT_POSITION"},
    {MessageType::GAMEOBJECT_PARENT_OBJECT, "GAMEOBJECT_PARENT_OBJECT"},
    {MessageType::AUTHENTICATION, "AUTHENTICATION"},
    {MessageType::UDP_VERIFICATION, "UDP_VERIFICATION"},
    {MessageType::FULL_GAME_STATE, "FULL_GAME_STATE"}
    // Add more entries as you add new message types
};

// Helper method to handle incoming UDP/TCP messages


// Encode a message to bytes

void NetworkCodec::log(LogLevel level, std::string text) {
    LOG(level, GetName() + "::" + text);
}

std::string NetworkCodec::GetName() { return "TcpConnection"; }

std::vector<uint8_t> NetworkCodec::Encode(const INetworkMessage* message) {
    return message->Serialize();
}

// Decode bytes to a message

std::unique_ptr<INetworkMessage> NetworkCodec::Decode(const std::vector<uint8_t>& data) {
    return MessageFactory::CreateMessage(data);
}

// Helper method to handle incoming UDP/TCP messages

void NetworkCodec::HandleNetworkData(const std::vector<uint8_t>& data, GameState& gameState) {
    log(LOG_INFO, "Handling Network Data");
    try {
        std::unique_ptr<INetworkMessage> message = Decode(data);
        // Process message : Message -> Command
        std::unique_ptr<IGameCommand> command = GameMessageProcessor::GetInstance().ProcessMessage(*message);

        // command: GameState -> GameState 
        command->Execute(gameState);
    }
    catch (const std::exception& e) {
        // Log error and handle gracefully
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
} 


GameMessageProcessor& GameMessageProcessor::GetInstance() {
    static GameMessageProcessor instance;
    return instance;
}

std::unique_ptr<IGameCommand> GameMessageProcessor::ProcessMessage(const INetworkMessage& message) {
    switch (message.GetType()) {
    case MessageType::UDP_VERIFICATION: {
        const auto& ipt_msg = static_cast<const UdpVerificationMessage&>(message);
        return std::make_unique<UdpVerificationCommand>(
            ipt_msg.session_id, ipt_msg.verification_code, ipt_msg.timestamp
        );
    }
    case MessageType::PLAYER_INPUT: {
        const auto& ipt_msg = static_cast<const PlayerInputMessage&>(message);
        return std::make_unique<PlayerInputCommand>(
            ipt_msg.playerDirection, ipt_msg.playerID
        );
    }
    case MessageType::ADD_GAMEOBJECT: {
        const auto& add_msg = static_cast<const AddGameObjectMessage&>(message);
        return std::make_unique<AddGameObjectCommand>(
            add_msg.gameObjectTypeID, add_msg.gameObjectID
        );
    }
    case MessageType::REMOVE_GAMEOBJECT: {
        const auto& remove_msg = static_cast<const RemoveGameObjectMessage&>(message);
        return std::make_unique<RemoveGameObjectCommand>(
            remove_msg.gameObjectID
        );
    }
    case MessageType::GAMEOBJECT_POSITION: {
        const auto& pos_msg = static_cast<const GameObjectPositionMessage&>(message);
        return std::make_unique<GameObjectPositionCommand>(
            pos_msg.y, pos_msg.x, pos_msg.gameObjectID
        );
    }
    case MessageType::GAMEOBJECT_PARENT_OBJECT: {
        const auto& parent_msg = static_cast<const GameObjectParentObjectMessage&>(message);
        return std::make_unique<GameObjectParentCommand>(
            parent_msg.parentObjectID, parent_msg.gameObjectID
        );
    }
    case MessageType::AUTHENTICATION:
        LOG(LOG_WARNING, "Authenication message cannot be handled by a MessageProcessor."); 
        return nullptr; 

        // v1.0 

    case MessageType::ADD_RIDABLE_OBJECT: {
        const auto& add_msg = static_cast<const AddRidableObjectMessage&>(message);
        return std::make_unique<AddRidableObjectCommand>(
            add_msg.objID_, add_msg.meshID_, add_msg.textureID_,
            add_msg.gridHeight_, add_msg.gridWidth
        );
    }
    case MessageType::WALK_ON_RIDABLE_OBJECT: {
        const auto& walk_msg = static_cast<const WalkOnRidableObjectMessage&>(message);
        return std::make_unique<WalkOnRidableObjectCommand>(
            walk_msg.walkerID_, walk_msg.direction
        );
    }
    case MessageType::RIDE_ON_RIDABLE_OBJECT: {
        const auto& ride_msg = static_cast<const RideOnRidableObjectMessage&>(message);
        return std::make_unique<RideOnRidableObjectCommand>(
            ride_msg.vehicleID, ride_msg.riderID, ride_msg.rideAt
        );
    }
    default:
        throw std::runtime_error("Unknown message type: " + messageType2string[message.GetType()]);
    }
}

void INetworkMessage::add_int(std::vector<uint8_t>& buffer, int val) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&val);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(int));
}

void INetworkMessage::add_float(std::vector<uint8_t>& buffer, float val) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&val);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(int));
}

// Constructor for creating new auth requests

AuthRequestMessage::AuthRequestMessage(uint32_t version, const uint32_t& id, uint16_t udp_port, const std::string& token)
    : protocol_version(version)
    , client_id(id)
    , client_udp_port(udp_port)
    , auth_token(token) {}

// Default constructor initializes client_id array to zeros

AuthRequestMessage::AuthRequestMessage()
    : protocol_version(0)
    , client_udp_port(0) {
    client_id = (0);
}

MessageType AuthRequestMessage::GetType() const {
    return MessageType::AUTHENTICATION;
}

size_t AuthRequestMessage::GetSize() const {
    // Now we have a fixed size for client_id instead of variable length
    return sizeof(uint8_t) +              // Message type
        sizeof(uint32_t) +             // Protocol version
        sizeof(uint32_t) +             // Fixed-length client ID
        sizeof(uint16_t) +             // UDP port
        sizeof(uint32_t) +             // Auth token length
        auth_token.length();           // Auth token string
}

std::vector<uint8_t> AuthRequestMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    // Add message type
    buffer.push_back(static_cast<uint8_t>(GetType()));

    // Add protocol version
    add_to_buffer<uint32_t>(buffer, protocol_version);

    // Add client ID 
    add_to_buffer<uint32_t>(buffer, client_id);

    // Add UDP port
    add_to_buffer<uint16_t>(buffer, client_udp_port);

    // Add auth token (length + string)
    add_to_buffer<uint32_t>(buffer, static_cast<uint32_t>(auth_token.length()));
    buffer.insert(buffer.end(), auth_token.begin(), auth_token.end());

    return buffer;
}

void AuthRequestMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Auth request message too short");
    }

    size_t offset = 1; // Skip message type byte

    // Extract protocol version
    protocol_version = extract_from_data<uint32_t>(data, offset);

    // Extract client id
    client_id = extract_from_data<uint32_t>(data, offset);

    // Extract UDP port
    client_udp_port = extract_from_data<uint16_t>(data, offset);

    // Extract auth token
    uint32_t token_length = extract_from_data<uint32_t>(data, offset);
    if (offset + token_length > data.size()) {
        throw std::runtime_error("Invalid auth token length");
    }
    auth_token = std::string(data.begin() + offset,
        data.begin() + offset + token_length);
}

UdpVerificationMessage::UdpVerificationMessage(const uint32_t& session, uint64_t code)
    : session_id(session)
    , verification_code(code)
    , timestamp(get_current_timestamp()) {}


// Default constructor for deserialization - Randomly init verification code and session_id

UdpVerificationMessage::UdpVerificationMessage()
    : verification_code(0)
    , timestamp(0) {
    session_id = (0);

    verification_code = generate_verification_code();
    this->set_random_session_id();
}

MessageType UdpVerificationMessage::GetType() const {
    return MessageType::UDP_VERIFICATION;
}

size_t UdpVerificationMessage::GetSize() const {
    return sizeof(uint8_t) +                // Message type
        sizeof(uint32_t) +              // Fixed-length session ID
        sizeof(uint64_t) +               // Verification code
        sizeof(uint64_t);                // Timestamp
}

std::vector<uint8_t> UdpVerificationMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    // Add message type
    buffer.push_back(static_cast<uint8_t>(GetType()));

    // Add session ID directly - no length needed since it's fixed
    add_to_buffer<uint32_t>(buffer, session_id);

    // Add verification code
    add_to_buffer<uint64_t>(buffer, verification_code);

    // Add timestamp
    add_to_buffer<uint64_t>(buffer, timestamp);

    return buffer;
}

void UdpVerificationMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1; // Skip message type byte

    // Extract session id 
    session_id = extract_from_data<uint32_t>(data, offset);

    // Extract verification code
    verification_code = extract_from_data<uint64_t>(data, offset);

    // Extract timestamp
    timestamp = extract_from_data<uint64_t>(data, offset);
}

void UdpVerificationMessage::set_random_session_id() {
    // Generate a random uint32_t
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
    session_id = dis(gen);
}

PlayerInputMessage::PlayerInputMessage(Direction direction, uint32_t id)
    : playerDirection(direction), playerID(id) {}

PlayerInputMessage::PlayerInputMessage()
    : playerDirection(Direction::IDLE), playerID(0) {}

MessageType PlayerInputMessage::GetType() const {
    return MessageType::PLAYER_INPUT;
}

size_t PlayerInputMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(uint8_t) + sizeof(uint32_t);
}

std::vector<uint8_t> PlayerInputMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    // Add message type
    buffer.push_back(static_cast<uint8_t>(GetType()));

    // add Player direction 
    INetworkMessage::add_to_buffer<uint8_t>(buffer, static_cast<uint8_t>(playerDirection));

    // add PlayerID
    INetworkMessage::add_to_buffer<uint32_t>(buffer, playerID);

    return buffer;
}

void PlayerInputMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1; // Skip message type byte

    // Extract Player Direction 
    playerDirection = static_cast<Direction>(extract_from_data<uint8_t>(data, offset));

    // Extract player ID
    playerID = extract_from_data<uint32_t>(data, offset);
}

AddGameObjectMessage::AddGameObjectMessage(uint8_t typeID, uint32_t objID)
    : gameObjectTypeID(typeID), gameObjectID(objID) {}

AddGameObjectMessage::AddGameObjectMessage()
    : gameObjectTypeID(0), gameObjectID(0) {}

MessageType AddGameObjectMessage::GetType() const {
    return MessageType::ADD_GAMEOBJECT;
}

size_t AddGameObjectMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(uint8_t) + sizeof(uint32_t);
}

std::vector<uint8_t> AddGameObjectMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    // Add message type
    buffer.push_back(static_cast<uint8_t>(GetType()));

    // add GameObjectTypeID  
    INetworkMessage::add_to_buffer<uint8_t>(buffer, gameObjectTypeID);

    // add GameObjectID
    INetworkMessage::add_to_buffer<uint32_t>(buffer, gameObjectID);

    return buffer;
}

void AddGameObjectMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1; // Skip message type byte

    // Extract Player Direction 
    gameObjectTypeID = extract_from_data<uint8_t>(data, offset);

    // Extract player ID
    gameObjectID = extract_from_data<uint32_t>(data, offset);
}

RemoveGameObjectMessage::RemoveGameObjectMessage(uint32_t objID)
    : gameObjectID(objID) { }

RemoveGameObjectMessage::RemoveGameObjectMessage()
    : gameObjectID(0) { }

MessageType RemoveGameObjectMessage::GetType() const {
    return MessageType::REMOVE_GAMEOBJECT;
}

size_t RemoveGameObjectMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(uint32_t);
}

std::vector<uint8_t> RemoveGameObjectMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    // Add message type
    buffer.push_back(static_cast<uint8_t>(GetType()));

    // add GameObjectID
    INetworkMessage::add_to_buffer<uint32_t>(buffer, gameObjectID);

    return buffer;
}

void RemoveGameObjectMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1; // Skip message type byte

    // Extract player ID
    gameObjectID = extract_from_data<uint32_t>(data, offset);
}

GameObjectPositionMessage::GameObjectPositionMessage(int y, int x, uint32_t id)
    : y(y), x(x), gameObjectID(id) {}

GameObjectPositionMessage::GameObjectPositionMessage()
    : y(0), x(0), gameObjectID(0) {}

MessageType GameObjectPositionMessage::GetType() const {
    return MessageType::GAMEOBJECT_POSITION;
}

size_t GameObjectPositionMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(int) * 2 + sizeof(uint32_t);
}

std::vector<uint8_t> GameObjectPositionMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    // Add message type
    buffer.push_back(static_cast<uint8_t>(GetType()));

    //// Add position data
    //auto addInt = [&buffer](int value) {
    //    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    //    buffer.insert(buffer.end(), bytes, bytes + sizeof(int));
    //};

    INetworkMessage::add_int(buffer, y);
    INetworkMessage::add_int(buffer, x);

    // Add player ID
    INetworkMessage::add_to_buffer<uint32_t>(buffer, gameObjectID);

    return buffer;
}

void GameObjectPositionMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1; // Skip message type byte

    //// Extract position data
    //auto readFloat = [&data, &offset]() {
    //    float value;
    //    std::memcpy(&value, data.data() + offset, sizeof(float));
    //    offset += sizeof(int);
    //    return value;
    //};

    // Extract player position 
    y = extract_from_data<int>(data, offset);
    x = extract_from_data<int>(data, offset);

    // Extract player ID
    gameObjectID = extract_from_data<uint32_t>(data, offset);
}

GameObjectParentObjectMessage::GameObjectParentObjectMessage(uint32_t parentID, uint32_t objID)
    : parentObjectID(parentID), gameObjectID(objID) {}

GameObjectParentObjectMessage::GameObjectParentObjectMessage()
    : parentObjectID(0), gameObjectID(0) {}

MessageType GameObjectParentObjectMessage::GetType() const {
    return MessageType::GAMEOBJECT_PARENT_OBJECT;
}

size_t GameObjectParentObjectMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(uint32_t) * 2;
}

std::vector<uint8_t> GameObjectParentObjectMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    // Add message type
    buffer.push_back(static_cast<uint8_t>(GetType()));

    // Add parentObjectID
    INetworkMessage::add_to_buffer<uint32_t>(buffer, parentObjectID);

    // Add gameObjectID
    INetworkMessage::add_to_buffer<uint32_t>(buffer, gameObjectID);

    return buffer;
}

void GameObjectParentObjectMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1; // Skip message type byte

    // Extract parentObjectID
    parentObjectID = extract_from_data<uint32_t>(data, offset);

    // Extract player ID
    gameObjectID = extract_from_data<uint32_t>(data, offset);
}

std::unique_ptr<INetworkMessage> MessageFactory::CreateMessage(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        throw std::runtime_error("Empty message data");
    }

    MessageType type = static_cast<MessageType>(data[0]);
    std::unique_ptr<INetworkMessage> message;

    switch (type) {
    case MessageType::GAMEOBJECT_POSITION:
        message = std::make_unique<GameObjectPositionMessage>();
        break;
    case MessageType::GAMEOBJECT_PARENT_OBJECT:
        message = std::make_unique<GameObjectParentObjectMessage>();
        break;
    case MessageType::PLAYER_INPUT:
        message = std::make_unique<PlayerInputMessage>();
        break;
    case MessageType::ADD_GAMEOBJECT:
        message = std::make_unique<AddGameObjectMessage>();
        break;
    case MessageType::REMOVE_GAMEOBJECT:
        message = std::make_unique<RemoveGameObjectMessage>();
        break;
    case MessageType::AUTHENTICATION:
        message = std::make_unique<AuthRequestMessage>();
        break;
    case MessageType::UDP_VERIFICATION:
        message = std::make_unique<UdpVerificationMessage>();  
        break; 

        // add more 

    default:
        throw std::runtime_error("Unknown message type: " + messageType2string[message->GetType()]);
    }

    message->Deserialize(data);
    return message;
}

// Get current timestamp in milliseconds since epoch
uint64_t get_current_timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

// Generate a cryptographically secure random number for verification
uint64_t generate_verification_code() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

MessageType AddRidableObjectMessage::GetType() const {
    return MessageType::ADD_RIDABLE_OBJECT;
}

size_t AddRidableObjectMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(uint32_t) * 3 + sizeof(uint8_t) * 2;
}

std::vector<uint8_t> AddRidableObjectMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    buffer.push_back(static_cast<uint8_t>(GetType()));

    INetworkMessage::add_to_buffer<uint32_t>(buffer, objID_);
    INetworkMessage::add_to_buffer<uint32_t>(buffer, meshID_);
    INetworkMessage::add_to_buffer<uint32_t>(buffer, textureID_);
    INetworkMessage::add_to_buffer<uint8_t>(buffer, gridHeight_);
    INetworkMessage::add_to_buffer<uint8_t>(buffer, gridWidth);

    return buffer;
}

void AddRidableObjectMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1;

    objID_ = extract_from_data<uint32_t>(data, offset);
    meshID_ = extract_from_data<uint32_t>(data, offset);
    textureID_ = extract_from_data<uint32_t>(data, offset);
    gridHeight_ = extract_from_data<uint8_t>(data, offset);
    gridWidth = extract_from_data<uint8_t>(data, offset); 
}


MessageType WalkOnRidableObjectMessage::GetType() const {
    return MessageType::WALK_ON_RIDABLE_OBJECT;
}

size_t WalkOnRidableObjectMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(uint32_t) + sizeof(Direction);
}

std::vector<uint8_t> WalkOnRidableObjectMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    buffer.push_back(static_cast<uint8_t>(GetType()));
    INetworkMessage::add_to_buffer<uint32_t>(buffer, walkerID_);
    INetworkMessage::add_to_buffer<Direction>(buffer, direction);

    return buffer;
}

void WalkOnRidableObjectMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1;
    walkerID_ = extract_from_data<uint32_t>(data, offset);
    direction = extract_from_data<Direction>(data, offset);
}

MessageType RideOnRidableObjectMessage::GetType() const {
    return MessageType::RIDE_ON_RIDABLE_OBJECT;
}

size_t RideOnRidableObjectMessage::GetSize() const {
    return sizeof(MessageType) + sizeof(uint32_t) * 2 + sizeof(uint8_t);
}

std::vector<uint8_t> RideOnRidableObjectMessage::Serialize() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(GetSize());

    buffer.push_back(static_cast<uint8_t>(GetType()));
    INetworkMessage::add_to_buffer<uint32_t>(buffer, vehicleID);
    INetworkMessage::add_to_buffer<uint32_t>(buffer, riderID);
    INetworkMessage::add_to_buffer<uint8_t>(buffer, rideAt);

    return buffer;
}

void RideOnRidableObjectMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < GetSize()) {
        throw std::runtime_error("Invalid message size");
    }

    size_t offset = 1;
    vehicleID = extract_from_data<uint32_t>(data, offset);
    riderID = extract_from_data<uint32_t>(data, offset);
    rideAt = extract_from_data<uint8_t>(data, offset);
}