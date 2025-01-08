#include "NetworkMessage.h"
#include "UDPClient.h"

#include <limits> 
#include <random>

using pointer = std::shared_ptr<TcpConnection>;

pointer TcpConnection::create(asio::io_context& io_context, GameClient* client) {
    return pointer(new TcpConnection(io_context, client));
}

tcp::socket& TcpConnection::socket() { return socket_; }


// Implements the same message queuing system as the server

void TcpConnection::send_tcp_message(const std::vector<uint8_t>& data) {
    auto self = shared_from_this();
    asio::post(socket_.get_executor(),
        [self, data]() {
            self->do_enqueue_message(data);
        });
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
            is_writing_ = false;
            return;
        }
        current_message = message_queue_.front();
    }

    uint32_t size = static_cast<uint32_t>(current_message.size());
    std::vector<uint8_t> complete_message;
    complete_message.reserve(sizeof(size) + current_message.size());

    const uint8_t* size_ptr = reinterpret_cast<const uint8_t*>(&size);
    complete_message.insert(complete_message.end(), size_ptr, size_ptr + sizeof(size));
    complete_message.insert(complete_message.end(),
        current_message.begin(),
        current_message.end());

    asio::async_write(socket_,
        asio::buffer(complete_message),
        [self](const asio::error_code& ec, std::size_t /*length*/) {
            if (!ec) {
                std::lock_guard<std::mutex> lock(self->queue_mutex_);
                self->message_queue_.pop();
                self->do_write();
            }
            else {
                std::cerr << "Write error: " << ec.message() << std::endl;
                self->is_writing_ = false;
            }
        });
}

void TcpConnection::handle_message(std::size_t length) {
    // Create a vector from the received data
    std::vector<uint8_t> data(read_buffer_.begin(),
        read_buffer_.begin() + length);

    // Decode the message and handle it appropriately
    std::unique_ptr<INetworkMessage> message =
        client->Decode(data);

    // Handle different message types


    if (auto verify_msg =
        dynamic_cast<UdpVerificationMessage*>(message.get())) {
        // on receiving verification message
        client->handle_udp_verification(*verify_msg);
    }
    // Add handlers for other message types...
}

GameClient::GameClient()
    : tcp_connection_(nullptr)
    , udp_socket_(io_context_)
    , state_(ClientState::DISCONNECTED) {
}

bool GameClient::connect(const std::string& address, unsigned short tcp_port) {
    try {
        // Create TCP connection
        tcp_connection_ = TcpConnection::create(io_context_, this);

        // Resolve and connect to server
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(address, std::to_string(tcp_port));
        asio::connect(tcp_connection_->socket(), endpoints);

        // Start network thread
        state_ = ClientState::CONNECTING;
        network_thread_ = std::thread([this]() { io_context_.run(); });

        // Send authentication request
        send_authentication();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

void GameClient::send_authentication() {
    if (!this->id_has_been_set) {
        // Generate a random uint32_t
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
        this->client_id = dis(gen);

        this->id_has_been_set = true;
    }

    uint32_t version = 1; 

    // Create and send authentication message
    AuthRequestMessage auth_msg(
        version,  // Protocol version
        client_id,
        udp_socket_.local_endpoint().port()
    );

    std::vector<uint8_t> auth_data = network_codec->Encode(&auth_msg);
    tcp_connection_->send_tcp_message(auth_data);

    // Start receiving messages
    tcp_connection_->start_read();
}

void GameClient::handle_udp_verification(const UdpVerificationMessage& msg) {
    // Store session ID and send verification response via UDP
    session_id_ = msg.session_id;

    // Create verification response
    UdpVerificationMessage response = msg;
    response.timestamp = get_current_timestamp();

    std::vector<uint8_t> response_data = network_codec->Encode(&response);

    // Send response through UDP
    udp_socket_.async_send_to(
        asio::buffer(response_data),
        server_udp_endpoint_,
        [](const asio::error_code& ec, std::size_t /*bytes_sent*/) {
            if (ec) {
                std::cerr << "UDP verification failed: " << ec.message() << std::endl;
            }
        });
}

void GameClient::send_message(INetworkMessage* msg) {
    std::vector<uint8_t> data = network_codec->Encode(msg);

    // Send response through UDP
    udp_socket_.async_send_to(
        asio::buffer(data),
        server_udp_endpoint_,
        [](const asio::error_code& ec, std::size_t /*bytes_sent*/) {
            if (ec) {
                std::cerr << "UDP verification failed: " << ec.message() << std::endl;
            }
        });
}

std::unique_ptr<INetworkMessage> GameClient::Decode(std::vector<uint8_t> data) {
    return network_codec->Decode(data);
}
