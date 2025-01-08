#include "UDPServer.h" 
#include "NetworkMessage.h"
#include "GameState.h"

GameServer::GameServer(asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port)
    : tcp_acceptor_(io_context, tcp::endpoint(tcp::v4(), tcp_port)),
    udp_socket_(io_context, udp::endpoint(udp::v4(), udp_port)),
    io_context_(io_context) {

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

void GameServer::TcpConnection::handle_auth_request(
    std::shared_ptr<ClientInfo> client,
    const std::vector<uint8_t>& data,
    std::size_t length
) {
    // Process authentication data
    std::unique_ptr<INetworkMessage> message = std::move(MessageFactory::CreateMessage(data));

    if (auto auth_msg = dynamic_cast<AuthRequestMessage*>(message.get())) {
        // safely received auth message  
        if (validate_auth_request(auth_msg)) {
            client->state = ClientInfo::State::ESTABLISHING;
            client->client_id = auth_msg->client_id;
            begin_udp_establishment(client);
        }
    }
    else {
        LOG(LOG_ERROR, "Client did not send a auth message. ");
    }
}

void  GameServer::TcpConnection::begin_udp_establishment(std::shared_ptr<ClientInfo> client) {
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

void GameServer::TcpConnection::begin_state_sync(std::shared_ptr<ClientInfo> client)
{
    std::unique_ptr<INetworkMessage> gameStateCaptureMsg = this->server->game_state->CaptureGameState();
}


// send to specific client by client_id
void GameServer::send_data_to_specific_client_by_udp(
    uint32_t client_id,
    const std::vector<uint8_t> data
) {
    ClientInfo* curClient = clients[client_id].get();
    udp::endpoint curUdpEndpoint = curClient->udp_endpoint;

    this->send_data_to_specific_client_by_udp(curUdpEndpoint, data);
}

void GameServer::handle_udp_receive(std::size_t bytes_received) {
    std::vector<uint8_t> data(
        udp_receive_buffer_.begin(),
        udp_receive_buffer_.begin() + bytes_received
    );
    std::unique_ptr<INetworkMessage> receivedMessage = MessageFactory::CreateMessage(data);

    if (auto verificationMsg = dynamic_cast<UdpVerificationMessage*>(receivedMessage.get())) {
        // when the message is a udp_verification, search from the pending verifications
        auto it = pendingVerification.find(verificationMsg->verification_code);

        if (it != pendingVerification.end()) {
            std::shared_ptr<ClientInfo> newClient = it->second->handle_udp_establishment_and_get_client();

            // set udp endpoint of client 
            newClient->udp_endpoint = udp_remote_endpoint_;

            // register client  
            register_client(newClient);
        }
        else {
            // none valid verification code
            return;
        }
    }

    // add logic for altering gameState
}