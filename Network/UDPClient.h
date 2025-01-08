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

using asio::ip::tcp;
using asio::ip::udp; 

class NetworkCodec; 
class GameState;
class GameClient;
class INetworkMessage;
class UdpVerificationMessage;
class TcpConnection;

// The GameClient class handles all network communication with the game server.
// Just like the server, it maintains both TCP and UDP connections and manages
// the complete connection lifecycle from initial handshake to gameplay.
class GameClient {

public:
    GameClient();

    bool connect(const std::string& address, unsigned short tcp_port);

public:
    void send_authentication();

    void handle_udp_verification(const UdpVerificationMessage& msg);
public:
    void send_message(INetworkMessage* msg);

    std::unique_ptr<INetworkMessage> Decode(std::vector<uint8_t> data);
private:
    enum class ClientState {
        DISCONNECTED,
        CONNECTING,
        AUTHENTICATING,
        ESTABLISHING_UDP,
        CONNECTED
    };

    asio::io_context io_context_;
    std::thread network_thread_;
    std::shared_ptr<TcpConnection> tcp_connection_;
    udp::socket udp_socket_;
    udp::endpoint server_udp_endpoint_;

    ClientState state_;
    uint32_t session_id_;
    NetworkCodec* network_codec;
    std::mutex state_mutex_;

    uint32_t client_id;
    bool id_has_been_set = false;
};



// The TcpConnection class handles reliable communication with the server
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
private:
    tcp::socket socket_;
    std::queue<std::vector<uint8_t>> message_queue_;
    std::mutex queue_mutex_;
    bool is_writing_ = false;
    std::array<uint8_t, 1024> read_buffer_;
    GameClient* client;

    // Just like in the server, we use a private constructor to ensure proper shared_ptr usage
    TcpConnection(asio::io_context& io_context, GameClient* cli)
        : socket_(io_context), client(cli) {
    }

public:
    NetworkCodec* network_codec;
    GameState* game_state;



public:
    using pointer = std::shared_ptr<TcpConnection>;

    static pointer create(asio::io_context& io_context, GameClient* client);

    tcp::socket& socket();

    // Implements the same message queuing system as the server
    void send_tcp_message(const std::vector<uint8_t>& data);

    void start_read();

    void set_game_state(GameState* gs);

    void set_network_codec(NetworkCodec* nc);

private:
    // Message queuing implementation (same as server)
    void do_enqueue_message(const std::vector<uint8_t>& data);

    void do_write();

    void handle_message(std::size_t length);
};

