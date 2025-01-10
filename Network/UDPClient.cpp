#include "NetworkMessage.h"
#include "UDPClient.h"

#include <limits> 
#include <random>
#include "../Event.h"

using pointer = std::shared_ptr<TcpConnection>;

std::string TcpConnection::GetName() const { return "TcpConnection"; }

void TcpConnection::log(LogLevel level, std::string text) {
    LOG(level, GetName() + "::" + text);
}

// Just like in the server, we use a private constructor to ensure proper shared_ptr usage

TcpConnection::TcpConnection(asio::io_context& io_context, GameClient* cli)
    : socket_(io_context), client(cli), read_timer_(io_context) {
}

void TcpConnection::SetToWaitForAuth() { is_waiting_for_udp_verification_code = true; }

pointer TcpConnection::create(asio::io_context& io_context, GameClient* client) {
    return pointer(new TcpConnection(io_context, client));
}

tcp::socket& TcpConnection::socket() { return socket_; }


// Implements the same message queuing system as the server

void TcpConnection::send_tcp_data(std::vector<uint8_t> data) {  // Take by value
    log(LOG_INFO, "Posting Message through tcp");
    auto self = shared_from_this();
    asio::post(socket_.get_executor(),
        [self, data = std::move(data)]() {  // Move the parameter into lambda
            // self->log(LOG_INFO, "Sending tcp message"); 

            // Log in lambda before enqueue
            self->log(LOG_INFO, "In lambda before enqueue, data size: " + std::to_string(data.size()));
            self->do_enqueue_message(data);

            // self->log(LOG_INFO, "After sending tcp message");
        });
}

void TcpConnection::send_tcp_message(INetworkMessage* msg) {  // Take by value
    std::vector<uint8_t>data = network_codec->Encode(msg); 

    this->send_tcp_data(data); 
}

void TcpConnection::start_read() {
    auto self = shared_from_this();


    // First read the message size (4 bytes)
    asio::async_read(socket_,
        asio::buffer(read_buffer_, sizeof(uint32_t)),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                // Extract message size
                uint32_t message_size;
                std::memcpy(&message_size, read_buffer_.data(), sizeof(uint32_t));

                log(LOG_INFO, "Reading incomming message of length:" + message_size); 

                // Now read the actual message
                asio::async_read(socket_,
                    asio::buffer(read_buffer_, message_size),
                    [this, self](std::error_code ec, std::size_t length) {
                        if (!ec) {
                            handle_message(length);
                            start_read(); // Continue reading
                        }
                    });
            }
        });
}

void TcpConnection::handle_message(std::size_t length) {
    log(LOG_INFO, "Handling Received Message");

    // Create a vector from the received data
    std::vector<uint8_t> data(read_buffer_.begin(),
        read_buffer_.begin() + length);

    // Decode the message and handle it appropriately
    std::unique_ptr<INetworkMessage> message =
        client->Decode(data);

    // Handle different message types 

    switch (message->GetType()) {
    case MessageType::UDP_VERIFICATION:
        is_waiting_for_udp_verification_code = false;  // not anymore!
        auto verify_msg = dynamic_cast<UdpVerificationMessage*>(message.get());

        log(LOG_INFO, "UDP verification code: " + std::to_string(verify_msg->verification_code)); 

        client->handle_udp_verification(*verify_msg);
        break;

        // Add handlers for other message types...
    }
}


void TcpConnection::set_game_state(GameState* gs) {
    this->game_state = gs;
}

void TcpConnection::set_network_codec(NetworkCodec* nc) {
    this->network_codec = nc;
}


// Message queuing implementation (same as server)

void TcpConnection::do_enqueue_message(const std::vector<uint8_t>& data) {
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

void TcpConnection::do_write() {
    auto self = shared_from_this();
    std::vector<uint8_t> current_message;

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        if (message_queue_.empty()) { 
            log(LOG_INFO, "Empty queue, ending do_write calls."); 
            is_writing_ = false;
            return;
        }
        current_message = message_queue_.front();
    }

    uint32_t size = static_cast<uint32_t>(current_message.size());
    size_t total_size = sizeof(size) + current_message.size();

    // Check if message fits in buffer
    if (total_size > write_buffer_.size()) {
        std::cerr << "Message too large for buffer" << std::endl;
        return;
    }

    // Copy size into buffer
    std::memcpy(write_buffer_.data(), &size, sizeof(size));
    // Copy message after size
    std::memcpy(write_buffer_.data() + sizeof(size), current_message.data(), current_message.size());

    asio::async_write(socket_,
        asio::buffer(write_buffer_.data(), total_size),
        [self](const asio::error_code& ec, std::size_t /*length*/) {
            self->log(LOG_INFO, "Async Write to server.");
            if (!ec) {
                self->log(LOG_INFO, "Successfully sent message.");
                {
                    std::lock_guard<std::mutex> lock(self->queue_mutex_);

                    self->message_queue_.pop();
                }
                
                self->do_write();

            }
            else {
                std::cerr << "Write error: " << ec.message() << std::endl;
                self->is_writing_ = false;
            }
        });
}


    
std::string GameClient::GetName() const { return "GameClient"; }

void GameClient::log(LogLevel level, std::string text) {
    LOG(level, GetName() + "::" + text);
}

GameClient::GameClient(asio::io_context* io_context, unsigned short tcp_port, unsigned short udp_port)
    : tcp_connection_(nullptr), udp_socket_(*io_context), state_(ClientState::DISCONNECTED),
    tcp_port(tcp_port), udp_port(udp_port)
{
    this->register_to_dispatcher(); 
}

void GameClient::register_to_dispatcher()
{
    Listener* dataListener = new Listener([this](const std::vector<uint8_t> data) {
        log(LOG_INFO, "DataListener Triggered");
        this->handle_events(data);
    });

    EventDispatcher& dispatcher = EventDispatcher::GetInstance();

    dispatcher.Subscribe(dataListener);
    dispatcher.Subscribe(Tag::USER_INPUT, dataListener);
    log(LOG_INFO, "Subscribing GameServer as Listener");
}

void GameClient::handle_events(std::vector<uint8_t> data)
{
    log(LOG_INFO, "hadling event"); 

    // for now...  
    std::unique_ptr<INetworkMessage> curMessage = NetworkCodec::Decode(data);   

    MessageType mt = curMessage->GetType();  

    switch (mt) {
    case MessageType::PLAYER_INPUT: 
        log(LOG_INFO, "Player Input event Triggered, Sending to Sever"); 
        auto pi_msg = dynamic_cast<PlayerInputMessage*>(curMessage.get()); 

        pi_msg->playerID = client_id; 

        this->send_message(pi_msg, true);
    }

    
} 

bool GameClient::connect(asio::io_context* io_context ,const std::string& address) {
    try {
        
        // Create TCP connection
        tcp_connection_ = TcpConnection::create(*io_context, this);

        // Resolve and connect to server
        tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve(address, std::to_string(tcp_port));

        log(LOG_INFO, "Attempting to Connect to server");
        asio::connect(tcp_connection_->socket(), endpoints); 
        log(LOG_INFO, "Right after TCP connection attempt"); 

        // Start network thread 
        state_ = ClientState::CONNECTING; 

        set_udp_endpoint(io_context, address);

        // Send authentication request
        send_authentication_and_wait_for_verification();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl; 
        return false;
    }
}

void GameClient::set_udp_endpoint(asio::io_context* io_context, const std::string& address)
{
    log(LOG_INFO, "Setting UDP endpoint");  
    udp::resolver resolver(*io_context);  
    remote_udp_endpoint_ = *(resolver.resolve(address, std::to_string(udp_port)).begin());  

    udp_socket_.open(asio::ip::udp::v4());  
    udp_socket_.bind(asio::ip::udp::endpoint(
        asio::ip::address_v4::any(),
        0
    )); 
}

void GameClient::send_authentication_and_wait_for_verification() {
    log(LOG_INFO, "Sending Authentication Message"); 
    
    if (!this->id_has_been_set) {
        // Generate a random uint32_t
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
        this->client_id = dis(gen); 

        log(LOG_INFO, "Generated Client id: " + std::to_string(client_id)); 

        this->id_has_been_set = true;
    }

    uint32_t version = 1; 

    // Create and send authentication message
    AuthRequestMessage* ptrMsg = new AuthRequestMessage(
        version,  // Protocol version
        client_id,
        udp_socket_.local_endpoint().port()
    );

    tcp_connection_->SendAuthenticationAndWaitForUDPVerification(ptrMsg); 
}

void TcpConnection::SendAuthenticationAndWaitForUDPVerification(AuthRequestMessage* ptrMsg) {
    // Send authentication message 
    this->send_tcp_message(ptrMsg);  

    check_if_udp_verification_code_has_arrived_if_not_resend_authentication_message(ptrMsg);  

    this->start_read(); 
}

void TcpConnection::check_if_udp_verification_code_has_arrived_if_not_resend_authentication_message(AuthRequestMessage* ptrMsg) {
    // is waiting for udp verification 
    // add a time limit for how much we chould wait for no response
    // when the time is up, send authentication message again.

    auto self = shared_from_this();

    // Set timeout
    read_timer_.expires_after(std::chrono::seconds(3));
    read_timer_.async_wait([self, ptrMsg](const std::error_code& ec) {
        if (!ec) {
            self->log(LOG_INFO, "verification time out");

            // after 3 seconds 
            if (self->is_waiting_for_udp_verification_code) {
                self->log(LOG_INFO, "Resending Authentication message");
                // if the authentication message doesn't arrive
                self->send_tcp_message(ptrMsg); // resend and wait for authentication again. 
                self->check_if_udp_verification_code_has_arrived_if_not_resend_authentication_message(ptrMsg);
            }
            else {
                self->log(LOG_INFO, "verification has been complete");
            }
        }
        });
}

void GameClient::handle_udp_verification(const UdpVerificationMessage& msg) {
    log(LOG_INFO, "Handling UDP verification Message" );

    // Store session ID and send verification response via UDP
    session_id_ = msg.session_id;

    // Create verification response
    UdpVerificationMessage response = msg;
    response.timestamp = get_current_timestamp(); 


    // you can't send this. this vector will get destoryed as soon as we leave the scope.
    current_udp_message_ = network_codec->Encode(&response);


    log(LOG_INFO, "Sending back verification code Through UDP");
    // Send response through UDP
    udp_socket_.async_send_to(
        asio::buffer(current_udp_message_),
        remote_udp_endpoint_,
        [](const asio::error_code& ec, std::size_t /*bytes_sent*/) {
            if (ec) {
                std::cerr << "UDP verification failed: " << ec.message() << std::endl;
            }
        });
}

void GameClient::send_message(INetworkMessage* msg, bool using_udp=true) {
    current_udp_message_ = network_codec->Encode(msg);

    if (using_udp) {
        // Send response through UDP
        udp_socket_.async_send_to(
            asio::buffer(current_udp_message_),
            remote_udp_endpoint_,
            [](const asio::error_code& ec, std::size_t /*bytes_sent*/) {
                if (ec) {
                    std::cerr << "UDP verification failed: " << ec.message() << std::endl;
                }
            });
    }
    else {
        tcp_connection_->send_tcp_message(msg);
    }

}

std::unique_ptr<INetworkMessage> GameClient::Decode(std::vector<uint8_t> data) {
    return network_codec->Decode(data);
}
