#pragma once

#include <array>
#include <iostream>
#include <iomanip>
#include <sstream>


#include <asio.hpp>
#include <unordered_map>
#include <memory>
#include <queue>
#include <mutex>
#include <unordered_set>


#include "LOG.h"
#include "NetworkConfig.h"


using namespace asio::ip;

std::string make_daytime_string();

using asio::ip::tcp;
using asio::ip::udp;

class AuthRequestMessage;
class NetworkCodec;
class GameState;
class INetworkMessage;


struct ClientInfo {
    enum class State {
        CONNECTING,      // Initial TCP handshake
        AUTHENTICATING,  // Verifying client credentials
        ESTABLISHING,    // Setting up UDP connection
        SYNCHRONIZING,   // Sending initial game state
        CONNECTED,       // Fully connected and playing
        DISCONNECTED    // Connection lost or closed
    };


    uint32_t client_id;
    State state;
    tcp::endpoint tcp_endpoint;
    udp::endpoint udp_endpoint;
    uint32_t session_id;
    uint64_t last_heartbeat;
};




class GameServer {

private: 
    NetworkCodec* network_codec;
    GameState* game_state;
public:
    GameServer(asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port);

    // Broadcast a message to all connected clients
    void broadcast_message(const INetworkMessage* message);

    void set_game_state(GameState* gs) {
        game_state = gs; 
    } 

    void set_network_codec(NetworkCodec* nc) {
        network_codec = nc; 
    }

private:
    // TCP connection handling
    class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    private: 
        std::queue<std::vector<uint8_t>> message_queue_;
        std::mutex queue_mutex_;
        bool is_writing_ = false;


        // Private constructor - forces use of create() method
        TcpConnection(asio::io_context& io_context, GameServer* ptrServer)
            : socket_(io_context), server(ptrServer){
        }

    public:

        using pointer = std::shared_ptr<TcpConnection>; 

        // Static creation method that ensures proper shared_ptr management
        static pointer create(asio::io_context& io_context, GameServer* ptrServer) {
            return pointer(new TcpConnection(io_context, ptrServer));
        }

        tcp::socket& socket() { return socket_; }

        // The main send interface
        void send_tcp_message(const std::vector<uint8_t>& data) {
            // We can safely use shared_from_this() here because the object
            // is guaranteed to be managed by a shared_ptr when this method is called
            auto self = shared_from_this();

            // Post to IO context to ensure thread safety
            asio::post(socket_.get_executor(),
                [self, data]() {
                    self->do_enqueue_message(data);
                });
        }

    private:
        void do_enqueue_message(const std::vector<uint8_t>& data) {
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

        void do_write() {
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
            uint32_t size = static_cast<uint32_t>(current_message.size());
            std::vector<uint8_t> complete_message;
            complete_message.reserve(sizeof(size) + current_message.size());

            // Add size prefix
            const uint8_t* size_ptr = reinterpret_cast<const uint8_t*>(&size);
            complete_message.insert(complete_message.end(), size_ptr, size_ptr + sizeof(size));

            // Add message content
            complete_message.insert(complete_message.end(),
                current_message.begin(),
                current_message.end());

            // Send the message
            asio::async_write(socket_,
                asio::buffer(complete_message),
                [self](const asio::error_code& ec, std::size_t /*length*/) {
                    if (!ec) {
                        std::lock_guard<std::mutex> lock(self->queue_mutex_);
                        self->message_queue_.pop();
                        self->do_write();  // Process next message if any
                    }
                    else {
                        // Handle error
                        std::cerr << "Write error: " << ec.message() << std::endl;
                        self->is_writing_ = false;
                    }
                });
        }

    public:
        void start_connection_process() {
            // Handle new TCP connection
            auto client_info = std::make_shared<ClientInfo>();

            this->client_info = client_info; 

            client_info->state = ClientInfo::State::CONNECTING;

            // Start authentication process
            begin_authentication(client_info);
        }

        std::shared_ptr<ClientInfo> handle_udp_establishment_and_get_client() { 
            // Start to send Map info 
            client_info->state = ClientInfo::State::SYNCHRONIZING; 

            begin_state_sync(this->client_info); 

            return client_info;
        }

    private:
        void begin_authentication(std::shared_ptr<ClientInfo> client) {
            asio::async_read(
                this->socket_, asio::buffer(read_buffer_),
                [this, client](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::vector<uint8_t> data(read_buffer_.begin(), read_buffer_.begin() + length);
                        handle_auth_request(client, data, length);
                    }
                });
        }

        bool validate_auth_request(AuthRequestMessage* auth_msg) {
            // to do 

            return true; 
        }

        void handle_auth_request(
            std::shared_ptr<ClientInfo> client,
            const std::vector<uint8_t>& data,
            std::size_t length
        );

        void begin_udp_establishment(std::shared_ptr<ClientInfo> client);

        void send_udp_verification(std::shared_ptr<ClientInfo> client, std::vector<uint8_t> data) {
            this->send_tcp_message(data); 
        } 

        void begin_state_sync(std::shared_ptr<ClientInfo> client);


    private:
        TcpConnection(asio::io_context& io_context, GameServer* server)
            : socket_(io_context), server(server) {}

        void start_read() {
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

        void handle_read(std::size_t length) {
            // Process the TCP message
            // Implementation depends on your message format
        }

        tcp::socket socket_;
        std::array<uint8_t, 1024> read_buffer_; 

        std::shared_ptr<ClientInfo> client_info; 
        GameServer* server; 
    };

    // -------------------------------------------------------------------------------------------

    void start_tcp_accept() {
        // creating new tcp connection
        auto new_connection = TcpConnection::create(io_context_, this); // creates a shared pointer

        tcp_acceptor_.async_accept( 
            new_connection->socket(), // the socket made by the connection goes here.
            [this, new_connection](std::error_code ec) {
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

    void start_udp_receive() {
        udp_socket_.async_receive_from(
            asio::buffer(udp_receive_buffer_), udp_remote_endpoint_,
            [this](std::error_code ec, std::size_t bytes_received) {
                if (!ec) {
                    handle_udp_receive(bytes_received);
                    start_udp_receive();  // Continue receiving
                }
            });
    }

    void handle_udp_receive(std::size_t bytes_received); 

    // broadcast data to all tcp clients 
    void broadcast_data_through_tcp(const std::vector<uint8_t> data) {
        for (const auto& tcpConnection: tcp_clients_) {
            tcpConnection.get()->send_tcp_message(data); 
        }
    }

    // broadcast data to all existing clients 
    void broadcast_data_through_udp(const std::vector<uint8_t> data) {
        udp::endpoint curUdpEndpoint; 
        
        for (const auto& pair : clients) {
            curUdpEndpoint = pair.second->udp_endpoint; 
            this->send_data_to_specific_client_by_udp(curUdpEndpoint, data);
        }
    }

    // send to specific client by client_id
    void send_data_to_specific_client_by_udp(
        uint32_t client_id,
        const std::vector<uint8_t> data
    );

    // send to specific client by udp_endpoint
    void send_data_to_specific_client_by_udp(udp::endpoint udp_endpoint, const std::vector<uint8_t> data) {
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
    void register_client(std::shared_ptr<ClientInfo> newClient) {
        uint32_t& newID = newClient->client_id;
        
        clients[newID] = newClient; 
    }

    // TCP server components
    tcp::acceptor tcp_acceptor_;
    std::unordered_set<typename TcpConnection::pointer> tcp_clients_;

    // UDP server components
    udp::socket udp_socket_;
    udp::endpoint udp_remote_endpoint_;
    std::array<uint8_t, 1024> udp_receive_buffer_;

    // Shared components
    asio::io_context& io_context_;
    std::mutex clients_mutex_; 

    // clients
    std::unordered_map<uint32_t,std::shared_ptr<ClientInfo>> clients;

    // f: verification code -> Client's TCP Connection
    std::unordered_map<uint64_t, std::shared_ptr<TcpConnection>> pendingVerification;
};