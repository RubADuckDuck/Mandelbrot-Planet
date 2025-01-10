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
#include <thread>
#include "../LOG.h"

using asio::ip::tcp;
using asio::ip::udp; 

class NetworkCodec; 
class GameState;
class GameClient;
class INetworkMessage;
class AuthRequestMessage; 
class UdpVerificationMessage;
class TcpConnection;


// The GameClient class handles all network communication with the game server.
// Just like the server, it maintains both TCP and UDP connections and manages
// the complete connection lifecycle from initial handshake to gameplay.
class GameClient {
public:
    std::string GetName() const;

private:
    void log(LogLevel level, std::string text);  

    NetworkCodec* network_codec;
    GameState* game_state;

public:
    GameClient(asio::io_context* io_context, unsigned short tcp_port, unsigned short udp_port); 

    void register_to_dispatcher();

    void handle_events(std::vector<uint8_t> data);

    bool connect(asio::io_context* io_context, const std::string& address);

    void set_udp_endpoint(asio::io_context* io_context, const std::string& address); 

public:
    void send_authentication_and_wait_for_verification(); 

    void handle_udp_verification(const UdpVerificationMessage& msg); 


public:
    void send_message(INetworkMessage* msg, bool using_udp);

    std::unique_ptr<INetworkMessage> Decode(std::vector<uint8_t> data);
private:
    enum class ClientState {
        DISCONNECTED,
        CONNECTING,
        AUTHENTICATING,
        ESTABLISHING_UDP,
        CONNECTED
    };

    std::shared_ptr<TcpConnection> tcp_connection_;

    udp::socket udp_socket_;
    udp::endpoint remote_udp_endpoint_;

    std::vector<uint8_t> current_udp_message_; 

    unsigned short tcp_port;  
    unsigned short udp_port; 

    ClientState state_;
    uint32_t session_id_; 
    std::mutex state_mutex_;

    uint32_t client_id;
    bool id_has_been_set = false; 


};



// The TcpConnection class handles reliable communication with the server
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    std::string GetName() const;

private:
    void log(LogLevel level, std::string text);

private:
    tcp::socket socket_;
    asio::steady_timer read_timer_;

    std::queue<std::vector<uint8_t>> message_queue_;
    std::mutex queue_mutex_;
    bool is_writing_ = false;
    std::array<uint8_t, 1024> read_buffer_;
    std::array<uint8_t, 1024> write_buffer_;
    GameClient* client;

    bool is_waiting_for_udp_verification_code = true;

    // Just like in the server, we use a private constructor to ensure proper shared_ptr usage
    TcpConnection(asio::io_context& io_context, GameClient* cli);

public:
    NetworkCodec* network_codec;
    GameState* game_state;

    void SetToWaitForAuth();

    void SendAuthenticationAndWaitForUDPVerification(AuthRequestMessage* ptrMsg);

private:
    void check_if_udp_verification_code_has_arrived_if_not_resend_authentication_message(AuthRequestMessage* ptrMsg);

public:
    using pointer = std::shared_ptr<TcpConnection>;

    static pointer create(asio::io_context& io_context, GameClient* client);

    tcp::socket& socket();

    // Implements the same message queuing system as the server
    void send_tcp_message(INetworkMessage* msg);

    void send_tcp_data(std::vector<uint8_t> data); 

    void start_read();

    void set_game_state(GameState* gs);

    void set_network_codec(NetworkCodec* nc);

private:
    // Message queuing implementation (same as server)
    void do_enqueue_message(const std::vector<uint8_t>& data);

    void do_write();

    void handle_message(std::size_t length);
};

