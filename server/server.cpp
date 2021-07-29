#include "server.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <thread>

/*****************************************************************************
 * Server class member definitions
 */

using namespace turbobeep; 

mediator::Server::Server(std::uint16_t port) : _serverPort(port) {
  _protoHandle = std::make_unique<messages::ProtoBuf>(); 
  this->initServer(); 
}

mediator::Server::~Server() {
  FD_CLR(_listening, &_master);
  close(_listening);
}

int mediator::Server::initServer() {
  this->_listening = socket(AF_INET, SOCK_STREAM, 0);
  int value{1};
  if (_listening == -1) {
    throw std::runtime_error("Can't create a socket"); 
  }

  // Bind the socket to an IP / Port
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(_serverPort);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  setsockopt(_listening, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

  if (bind(_listening, (sockaddr *)&hint, sizeof(hint)) == -1) {
    throw std::runtime_error("Can't bind to IP/Port");
  }
  // Mark socket for listening in
  if (listen(_listening, SOMAXCONN) == -1) {
    throw std::runtime_error("Can't listen"); 
  }

  FD_ZERO(&_master);
  // Add to set
  FD_SET(_listening, &_master);
}

/**
 * Updates peer information (`_userDescriptor`) with other's peer information (ipAddress and port).
 *
 * @param[in, out] user current users' name
 * @param[in] peer Client's peer to connect to
 */
void mediator::Server::_updatePeerInfo(std::string const &user, std::string const &peer){
  _userDescriptor.at(user).canConnect = true;
  _userDescriptor.at(user).peerInfo.ipAddress = _userDescriptor.at(peer).ipAddress;
  _userDescriptor.at(user).peerInfo.port = _userDescriptor.at(peer).port;;

}

/**
 * Finds whether if peer's peer is already connected.
 * Peer that connects first will be consired the client, and thus
 * `isClient = true` will be assigned.
 *
 * @param[in, out] user current users' name
 * @param[in] peer Client's peer to connect to
 */
void mediator::Server::_findPeer(std::string const &user, std::string const &peer) {
  if (_userDescriptor.find(peer) == _userDescriptor.end()) {
    _userDescriptor.at(user).isClient = true;
  } else {
    // Both are now connected
    _userDescriptor.at(user).isClient = false;
    _updatePeerInfo(user, peer);
    _updatePeerInfo(peer, user);
  }
}

/**
 * Remove peer from `_userDescriptor` once peer has disconnected from server
 *
 * @param[in] socket disconnected user's socket
 */
void mediator::Server::_removePeer(int const &socket) {
  for (auto key_val : _userDescriptor) {
    if (socket == key_val.second.socket) {
      _userDescriptor.erase(key_val.second.name);
      break;
    }
  }
}

/**
 * Checks if both peers have `canConnect`=true, this only happens once both
 * peers have successfully connected to the server
 *
 * @param[in] socket disconnected user's socket
 */
void mediator::Server::_readyToP2P(int const &socket) {

  for (auto key_val : _userDescriptor) {
    if (socket == key_val.second.socket) {
      if (key_val.second.canConnect) {

        // Send message to both peers
        std::string peerName = key_val.second.peerInfo.name;
        int peerSocket = _userDescriptor.at(peerName).socket;

        std::string ipAddressPeer =
            _userDescriptor.at(peerName).peerInfo.ipAddress + ":" +
            std::to_string(_userDescriptor.at(peerName).peerInfo.port);

        std::string ipAddressCl =
            key_val.second.peerInfo.ipAddress + ":" +
            std::to_string(key_val.second.peerInfo.port);

        // Send peer information to other peer
        send(peerSocket, ipAddressPeer.c_str(), ipAddressPeer.length() + 1, 0);
        send(socket, ipAddressCl.c_str(), ipAddressCl.length() + 1, 0);
        break;
      }
    }
  }
}

/**
 * 
 * @param peerInfo protobuf packet
 * @param sock peer socket
 */
void mediator::Server::_findPeerInformation(payload::packet_Payload &payload, int sock){
  auto *peerInfo = payload.mutable_peerinfo();
  
  if (payload::packet::PEER_INFO == payload.type()) {
    if (_userDescriptor.find(peerInfo->username()) == _userDescriptor.end()) {
      // If username not found -> add it to map
      userInfo newUser;
      newUser.ipAddress = peerInfo->ipaddress();
      newUser.port = peerInfo->port(); 
      newUser.name = peerInfo->username();
      newUser.socket = sock;
      newUser.peerInfo.name = peerInfo->peername(); 
      _userDescriptor.insert(std::make_pair(peerInfo->username(), newUser));
      _findPeer(peerInfo->username(), peerInfo->peername());
    }
  }
  else {
    std::cout << "Different type of message" << std::endl; 
  }
}

/**
 * Read protobuf message body.
 * Deserializes sent package from client
 * 
 * @param sock peer socket
 * @param size size of the message as given by messages::Receive::readHeader()
 * @param[in, out] packet protobuf packet, containing payload to be deserialized
 */
void mediator::Server::readBody(int sock, uint32g size, payload::packet *packet){
  int bytecount;
  char buffer[size + 4];
  bytecount = recv(sock, (void *)buffer, 4 + size, 0);
  _protoHandle->deserializeMessage(packet, buffer, size);
}

void mediator::Server::runServer() {
  while (true) {
    // copies all
    auto copy = _master;
    int socketCount = select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);

    for (int sock = 0; sock <= FD_SETSIZE - 1; ++sock) {
      if (!FD_ISSET(sock, &copy))
        continue;

      if (sock == _listening) {
        // Accept new connection
        auto client = accept(_listening, nullptr, nullptr);
        // Add new connection to the list of connected clients
        FD_SET(client, &_master);
      } else {
        // Accept new message
        char buffer[4];
        int bytesIn;
        memset(buffer, '\0', 4);

        // Peek into the socket and get the packet size
        if ((bytesIn = recv(sock, buffer, 4, MSG_PEEK)) <= 0) {
          close(sock);
          FD_CLR(sock, &_master);
          _removePeer(sock);
        } else {
          if (bytesIn > 0) {
            payload::packet packet;
            (void)readBody(sock, _protoHandle->readHeader(buffer), &packet);
            
            auto *payload = packet.mutable_payload();
            _findPeerInformation(*payload, sock);
          }
        }
        _readyToP2P(sock);
      }
    }
  }
}
