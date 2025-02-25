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
#include <string>

#include "Utils/LOG.h"
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
public:
    std::string GetName() const;

private: 
    void log(LogLevel level, std::string text);

    NetworkCodec* network_codec;
    GameState* game_state; 

    bool should_accept_new_connections = true;

public:
    void InitGameState();

    void stop_accepting_connections();

private: 
    // Event Related 
    void handle_events(const std::vector<uint8_t> data);

    // Register GameServer As listener to data type event messages 
    void register_to_dispatcher();

public:
    GameServer(asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port);

    // Broadcast a message to all connected clients
    void broadcast_message(const INetworkMessage* message);

    void set_game_state(GameState* gs);

    void set_network_codec(NetworkCodec* nc);

public:
    // TCP connection handling
    class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    public:
        std::string GetName() const;

    private:
        void log(LogLevel level, std::string text);

    private: 
        std::queue<std::vector<uint8_t>> message_queue_;
        std::mutex queue_mutex_; 

        std::vector<uint8_t> complete_message_; 

        bool is_writing_ = false;


        // Private constructor - forces use of create() method
        TcpConnection(asio::io_context& io_context, GameServer* ptrServer);

    public:

        using pointer = std::shared_ptr<TcpConnection>; 

        // Static creation method that ensures proper shared_ptr management
        static pointer create(asio::io_context& io_context, GameServer* ptrServer);

        tcp::socket& socket() { return socket_; }

        // The main send interface
        void send_tcp_message(const std::vector<uint8_t>& data);

    private:
        void do_enqueue_message(const std::vector<uint8_t>& data);

        void do_write();

    public:
        void start_connection_process();

        std::shared_ptr<ClientInfo> handle_udp_establishment_and_get_client();

    private:
        void begin_authentication(std::shared_ptr<ClientInfo> client);

        void handle_error(const std::error_code& ec, std::shared_ptr<ClientInfo> client);

        bool validate_auth_request(AuthRequestMessage* auth_msg);

        void handle_auth_request(
            std::shared_ptr<ClientInfo> client, 
            const std::vector<uint8_t>& data, 
            std::size_t length 
        );

        void begin_udp_establishment(std::shared_ptr<ClientInfo> client);

        void send_udp_verification(std::shared_ptr<ClientInfo> client, std::vector<uint8_t> data);

        void begin_state_sync(std::shared_ptr<ClientInfo> client);


    private:

        void start_read();

        void handle_read(std::size_t length);

        tcp::socket socket_;
        std::array<uint8_t, 1024> read_buffer_; 

        std::shared_ptr<ClientInfo> client_info; 
        GameServer* server; 
    };

    // -------------------------------------------------------------------------------------------

    void start_tcp_accept();

    void start_udp_receive();

    // handle input 
    void handle_udp_receive(std::size_t bytes_received); 
    void handle_data(const std::vector<uint8_t> data);

    // verify udp connection 
    void verify_pending_udp_connection(uint64_t verification_code);

    // broadcast data to all tcp clients 
    void broadcast_data_through_tcp(const std::vector<uint8_t> data);

    // broadcast data to all existing clients 
    void broadcast_data_through_udp(const std::vector<uint8_t> data);

    // send to specific client by client_id
    void send_data_to_specific_client_by_udp(
        uint32_t client_id,
        const std::vector<uint8_t> data
    ) {
        ClientInfo* curClient = clients[client_id].get();
        udp::endpoint curUdpEndpoint = curClient->udp_endpoint;

        this->send_data_to_specific_client_by_udp(curUdpEndpoint, data);
    }

    // send to specific client by udp_endpoint
    void send_data_to_specific_client_by_udp(udp::endpoint udp_endpoint, const std::vector<uint8_t> data);

    // clients are registered after connections are established
    void register_client(std::shared_ptr<ClientInfo> newClient);

    // TCP server components  
    tcp::acceptor tcp_acceptor_;
    std::unordered_set<typename TcpConnection::pointer> tcp_clients_;

    // UDP server components   
    udp::socket udp_socket_;
    udp::endpoint udp_remote_endpoint_;
    std::array<uint8_t, 1024> udp_receive_buffer_;  
    std::vector<uint8_t> udp_send_buffer_; 

    // Shared components  
    asio::io_context& io_context_;
    std::mutex clients_mutex_; 

    // Registered clients 
    std::unordered_map<uint32_t,std::shared_ptr<ClientInfo>> clients;

    // TCP connections waiting for UDP Verification to arrive  
    // f: verification code -> Client's TCP Connection
    std::unordered_map<uint64_t, std::shared_ptr<TcpConnection>> pendingVerification;
};