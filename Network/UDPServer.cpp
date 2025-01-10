#include "UDPServer.h" 
#include "NetworkMessage.h"
#include "GameState.h"



using pointer = std::shared_ptr<GameServer::TcpConnection>;

std::string GameServer::GetName() const { return "GameServer"; }

void GameServer::log(LogLevel level, std::string text) {
    LOG(level, GetName() + "::" + text);
}

void GameServer::InitGameState() {
    LOG(LOG_INFO, "GameServer::Initializing Game State"); 
    game_state = new GameState(this);
}

void GameServer::stop_accepting_connections() {
    should_accept_new_connections = false;
}


// Register GameServer As listener to data type event messages 


// Event Related 

void GameServer::handle_events(const std::vector<uint8_t> data) {
    // broadcast to other clients 
    this->broadcast_data_through_udp(data); 

    this->handle_data(data); 
}

void GameServer::register_to_dispatcher() {
    Listener* dataListener = new Listener([this](const std::vector<uint8_t> data) {
        log(LOG_INFO, "DataListener Triggered");
        this->handle_events(data);
        });

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();

    dispatcher.Subscribe(dataListener);
    dispatcher.Subscribe(Tag::USER_INPUT, dataListener);
    log(LOG_INFO, "Subscribing GameServer as Listener");
}

GameServer::GameServer(asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port)
    : tcp_acceptor_(io_context, tcp::endpoint(tcp::v4(), tcp_port)),
    udp_socket_(io_context, udp::endpoint(udp::v4(), udp_port)),
    io_context_(io_context) 
{
    InitGameState();

    // register listener to dispatcher. 
    register_to_dispatcher(); 

    // Start Network
    start_tcp_accept();  
    start_udp_receive();  
}

// Broadcast a message to all connected clients
void GameServer::broadcast_message(const INetworkMessage* message) {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    std::vector<uint8_t> data = network_codec->Encode(message);

    broadcast_data_through_udp(data);

    //// Determine if this message should go through TCP or UDP based on its type
    //if (message.requires_reliable_delivery()) {
    //    broadcast_tcp_message(message);
    //}
    //else {
    //    broadcast_udp_message(message);
    //}
}

void GameServer::set_game_state(GameState* gs) {
    game_state = gs;
}

void GameServer::set_network_codec(NetworkCodec* nc) {
    network_codec = nc;
}


// Private constructor - forces use of create() method

std::string GameServer::TcpConnection::GetName() const
{
    return "GameServer::TcpConnection";
}

void GameServer::TcpConnection::log(LogLevel level, std::string text) {
    LOG(level, GetName() + "::" + text);
}

GameServer::TcpConnection::TcpConnection(asio::io_context& io_context, GameServer* ptrServer)
    : socket_(io_context), server(ptrServer) {
}

void GameServer::TcpConnection::do_enqueue_message(const std::vector<uint8_t>& data) {
    bool start_sending = false;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        message_queue_.push(data);
        if (!is_writing_) {
            is_writing_ = true;
            start_sending = true;
        }
    }

    if (start_sending) {
        do_write();
    }
}

void GameServer::TcpConnection::do_write() {
    auto self = shared_from_this();
    std::vector<uint8_t> current_message;

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (message_queue_.empty()) {
            is_writing_ = false;
            return;
        }
        current_message = message_queue_.front();
    }

    // Prepare message with size prefix
    // length of message to size 
    uint32_t size = static_cast<uint32_t>(current_message.size());
    complete_message_.clear(); 
    // reserve 4byte(32bit) + (message size in bytes)
    complete_message_.reserve(sizeof(size) + current_message.size());

    // Add size prefix
    const uint8_t* size_ptr = reinterpret_cast<const uint8_t*>(&size);
    // I am a bit surprized that the end doesn't care about the reserved spaces 
    complete_message_.insert(complete_message_.end(), size_ptr, size_ptr + sizeof(size));

    // Add message content
    complete_message_.insert(complete_message_.end(),
        current_message.begin(),
        current_message.end());

    // Send the message
    asio::async_write(socket_,
        asio::buffer(complete_message_),
        [self](const asio::error_code& ec, std::size_t /*length*/) {
            if (!ec) {
                {
                    std::lock_guard<std::mutex> lock(self->queue_mutex_);
                    self->message_queue_.pop();
                }
                self->do_write();  // Process next message if any
            }
            else {
                // Handle error
                std::cerr << "Write error: " << ec.message() << std::endl;
                self->is_writing_ = false;
            }
        });
}

// tcp connection process #1 
void GameServer::TcpConnection::start_connection_process() {
    // Handle new TCP connection
    auto client_info = std::make_shared<ClientInfo>();

    this->client_info = client_info;

    client_info->state = ClientInfo::State::CONNECTING;
    log(LOG_INFO, "Client Connecting..."); 

    // Start authentication process
    begin_authentication(client_info);
}

// tcp connection process #5
// register client to server and start syncing gameState Information  
std::shared_ptr<ClientInfo> GameServer::TcpConnection::handle_udp_establishment_and_get_client() {
    log(LOG_INFO, "UDP established. Syncing GameState..."); 
    // Start to send Map info 
    client_info->state = ClientInfo::State::SYNCHRONIZING;

    begin_state_sync(this->client_info);

    return client_info;
}

// tcp connection process #2
// begin authentication process  
void GameServer::TcpConnection::begin_authentication(std::shared_ptr<ClientInfo> client) {
    log(LOG_INFO, "Waiting for authentication message...");
    // First read size
    asio::async_read(socket_,
        asio::buffer(read_buffer_, sizeof(uint32_t)),
        [this, client](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                uint32_t message_size;
                std::memcpy(&message_size, read_buffer_.data(), sizeof(uint32_t));

                // Then read message
                asio::async_read(socket_,
                    asio::buffer(read_buffer_, message_size),
                    [this, client](std::error_code ec, std::size_t length) {
                        log(LOG_INFO, "Message Arrived!");
                        if (!ec) {
                            std::vector<uint8_t> data(read_buffer_.begin(), read_buffer_.begin() + length);
                            handle_auth_request(client, data, length);
                        }
                        else {
                            handle_error(ec, client);
                        }
                    });
            }
            else {
                handle_error(ec, client);
            }
        });
}

void GameServer::TcpConnection::handle_error(const std::error_code& ec, std::shared_ptr<ClientInfo> client) {
    if (ec.value() == 10054) {
        LOG(LOG_INFO, "Client disconnected normally");
    }
    else {
        LOG(LOG_INFO, "Error: " + ec.message() + " (" + std::to_string(ec.value()) + ")");
    }
}

bool GameServer::TcpConnection::validate_auth_request(AuthRequestMessage* auth_msg) {
    // to do 

    return true;
}

// tcp connection process #3 
void GameServer::TcpConnection::handle_auth_request(std::shared_ptr<ClientInfo> client, const std::vector<uint8_t>& data, std::size_t length) {
    // Process authentication data
    std::unique_ptr<INetworkMessage> message = std::move(MessageFactory::CreateMessage(data));

    if (auto auth_msg = dynamic_cast<AuthRequestMessage*>(message.get())) {
        // safely received auth message  
        if (validate_auth_request(auth_msg)) {
            log(LOG_INFO, "Authentication Successful"); 
            client->state = ClientInfo::State::ESTABLISHING;
            client->client_id = auth_msg->client_id; 

            auto it = server->clients.find(client->client_id); 

            if (it == server->clients.end()) {
                begin_udp_establishment(client);
            }
            else {
                log(LOG_WARNING, "That id already exists."); 

                // We do nothing, The client will try again. 
            }
        }
        else {
            log(LOG_INFO, "Authentication Failed"); 
        }
    }
    else {
        log(LOG_INFO, "Client did not send a auth message. Retry connection"); 
        // try again 
        begin_authentication(client); 
    }
}

// tcp connection process #4 
// Send UDP verification code 
void GameServer::TcpConnection::begin_udp_establishment(std::shared_ptr<ClientInfo> client) {
    log(LOG_INFO, "Begin UDP establishment"); 

    // Create verification message
    UdpVerificationMessage verify_msg = UdpVerificationMessage();

    // Store the verification data
    client->session_id = verify_msg.session_id;

    // Store the pending verification
    server->pendingVerification[verify_msg.verification_code] = shared_from_this();

    // Send the verification message over TCP (secure channel)
    send_udp_verification(client, verify_msg.Serialize());

    //// Start the verification timeout timer
    //start_verification_timeout(client->client_id);
}

void GameServer::TcpConnection::send_udp_verification(std::shared_ptr<ClientInfo> client, std::vector<uint8_t> data) {
    log(LOG_INFO, "UDP verification message sent through TCP"); 
    this->send_tcp_message(data);
}

// tcp connection process #6 
// sync gameState
void GameServer::TcpConnection::begin_state_sync(std::shared_ptr<ClientInfo> client)
{
    std::unique_ptr<INetworkMessage> gameStateCaptureMsg = this->server->game_state->CaptureGameState();
}

void GameServer::TcpConnection::start_read() {
    auto self(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(read_buffer_),
        [this, self](std::error_code ec, std::size_t length) {
            if (!ec) {
                // Process the received data
                handle_read(length);
                start_read();  // Continue reading
            }
        });
}

void GameServer::TcpConnection::handle_read(std::size_t length) {
    // Process the TCP message
    // Implementation depends on your message format
}


// Static creation method that ensures proper shared_ptr management

pointer GameServer::TcpConnection::create(asio::io_context& io_context, GameServer* ptrServer) { 
    return pointer(new TcpConnection(io_context, ptrServer)); 
}  

// The main send interface 

void GameServer::TcpConnection::send_tcp_message(const std::vector<uint8_t>& data) {
    // We can safely use shared_from_this() here because the object
    // is guaranteed to be managed by a shared_ptr when this method is called
    auto self = shared_from_this();

    // Post to IO context to ensure thread safety
    asio::post(socket_.get_executor(),
        [self, data]() {
            self->do_enqueue_message(data);
        });
}

 
void GameServer::start_tcp_accept() {
    if (!should_accept_new_connections) {
        return;
    }

    // creating new tcp connection
    auto new_connection = TcpConnection::create(io_context_, this); // creates a shared pointer

    LOG(LOG_INFO, "Start TCP Accept");  

    tcp_acceptor_.async_accept(
        new_connection->socket(), // the socket made by the connection goes here.
        [this, new_connection](std::error_code ec) {
            LOG(LOG_INFO, "Accepting new Connection");

            if (!ec) {
                // Register the new client
                std::lock_guard<std::mutex> lock(clients_mutex_);
                tcp_clients_.insert(new_connection);
                new_connection->start_connection_process();
            }

            // Continue accepting new connections
            start_tcp_accept();
        });
}

void GameServer::start_udp_receive() {

    LOG(LOG_INFO, "Start UDP Receive");

    udp_socket_.async_receive_from(
        asio::buffer(udp_receive_buffer_), udp_remote_endpoint_,
        [this](std::error_code ec, std::size_t bytes_received) {
            if (!ec) {
                handle_udp_receive(bytes_received);
                start_udp_receive();  // Continue receiving
            }
        });
}

void GameServer::handle_udp_receive(std::size_t bytes_received) {
    std::vector<uint8_t> data(
        udp_receive_buffer_.begin(),
        udp_receive_buffer_.begin() + bytes_received
    );

    this->handle_data(data); 
} 

void GameServer::handle_data(const std::vector<uint8_t> data) {
    NetworkCodec::HandleNetworkData(data, *(this->game_state)); 
}

void GameServer::verify_pending_udp_connection(uint64_t verification_code) 
{
    log(LOG_INFO, "Verifying pending udp connection");  

    // when the message is a udp_verification, search from the pending verifications
    auto it = pendingVerification.find(verification_code);

    if (it != pendingVerification.end()) {
        log(LOG_INFO, "Valid verification code.");
        std::shared_ptr<ClientInfo> newClient = it->second->handle_udp_establishment_and_get_client();


        // set udp endpoint of client 
        newClient->udp_endpoint = udp_remote_endpoint_;

        // register client  
        register_client(newClient);
    }
    else {
        // none valid verification code
        log(LOG_INFO, "Not a Valid verification code.");
        return;
    }
}

// broadcast data to all tcp clients 
void GameServer::broadcast_data_through_tcp(const std::vector<uint8_t> data) {
    for (const auto& tcpConnection : tcp_clients_) {
        tcpConnection.get()->send_tcp_message(data);
    }
}

// broadcast data to all existing clients 

void GameServer::broadcast_data_through_udp(const std::vector<uint8_t> data) {
    udp::endpoint curUdpEndpoint;

    for (const auto& pair : clients) {
        curUdpEndpoint = pair.second->udp_endpoint;
        this->send_data_to_specific_client_by_udp(curUdpEndpoint, data);
    }
}

// send to specific client by udp_endpoint

void GameServer::send_data_to_specific_client_by_udp(udp::endpoint udp_endpoint, const std::vector<uint8_t> data) {
    udp_socket_.async_send_to(
        asio::buffer(data), udp_endpoint,
        [](std::error_code ec, std::size_t /*bytes_sent*/) {
            if (ec) {
                // Handle error
            }
        }
    );
}

// clients are registered after connections are established

void GameServer::register_client(std::shared_ptr<ClientInfo> newClient) {
    uint32_t& newID = newClient->client_id;

    log(LOG_INFO, "Registering new client of id: " + std::to_string(newID));

    clients[newID] = newClient;
}
