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
#include <algorithm>
#include <thread>
#include "vars.h"

/*****************************************************************************
 * Server class member definitions
 */

using namespace turbobeep;

mediator::Server::Server(std::uint16_t port) : _serverPort(port) {
  _msgHandler = std::make_unique<messages::ProtoBuf>(); 
  this->initServer(); 
}

mediator::Server::~Server() {
  FD_CLR(_listening, &_master);
  close(_listening);
}

void mediator::Server::initServer() {
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
void mediator::Server::_readyToP2P(int const &socket, bool &isProto) {

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
        // Check if its a serialized protobuffer message
        if (isProto){
          int sizeP1, sizeP2;
          payload::packet packetP1 = _msgHandler->setPeerData(
              &sizeP1, _userDescriptor.at(peerName).peerInfo.ipAddress,
              _userDescriptor.at(peerName).peerInfo.port,
              _userDescriptor.at(peerName).isClient);

          payload::packet packetP2 = _msgHandler->setPeerData(
              &sizeP2, key_val.second.peerInfo.ipAddress,
              key_val.second.peerInfo.port, key_val.second.isClient);

          // Send both peers the other peer's information
          _msgHandler->sendMessage(sizeP1, peerSocket, packetP1);
          _msgHandler->sendMessage(sizeP2, socket, packetP2);

        } else {
          // Non serialized data
          send(peerSocket, ipAddressPeer.c_str(), ipAddressPeer.length() + 1,
               0);
          send(socket, ipAddressCl.c_str(), ipAddressCl.length() + 1, 0);
        }
        break;
      }
    }
  }
}

/**
 * Finds peer information. Checks whether peer has already connected to the
 * mediator and is ready to connect to peer. If not then user is added to the
 * userInfo struct
 *
 * @param peerInfo protobuf packet
 * @param sock peer socket
 */
void mediator::Server::_findPeerInformation(payload::packet_Payload &payload, int sock){
  auto *peerInfo = payload.mutable_peerinfo();
  auto *crypto = payload.mutable_crypto();
  auto mType = payload.type(); 

  if (_userDescriptor.find(peerInfo->username()) == _userDescriptor.end()) {
    userInfo newUser;
    std::string userName, peerName;

    // Message type can only be PEER_INFO or ADVERTISE at this point
    // Depending mType, userName and peerName will be either the provided name
    // or hashed keys
    userName = mType == payload::packet::ADVERTISE ? crypto->hashedkey()
                                                   : peerInfo->username();
    peerName = mType == payload::packet::ADVERTISE ? crypto->peerhashedkey()
                                                   : peerInfo->peername();

    newUser.ipAddress = peerInfo->ipaddress();
    newUser.port = peerInfo->port();
    newUser.socket = sock;
    newUser.name = userName;
    newUser.peerInfo.name = peerName;
    _userDescriptor.insert(std::make_pair(userName, newUser));
    _findPeer(userName, peerName);
  }
}

/**
 * Run server waiting for new peers to connect.
 * 
 * There are two accepted cases:
 * 
 *  1. Either the peer provides the name of the peer
 *  to connect to, in which case the server will just wait and find the other
 *  name to connect.
 *
 *  2. Peer advertises including its own hashed public key plus that of the peer
 *  it intends to connect to.
 *
 * In both cases the server will try and find the peer to connect to
 */
void mediator::Server::runServer() {
  while (true) {
    // copies all
    auto copy = _master;
    int socketCount = select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);

    for (int sock = 0; sock <= FD_SETSIZE - 1; ++sock) {
      if (!FD_ISSET(sock, &copy))
        continue;

      // Accept new connection
      if (sock == _listening) {
        auto client = accept(_listening, nullptr, nullptr);
        FD_SET(client, &_master);
      } else {
        bool isProto = false; 
        payload::packet packet;
        // Check if there is a message
        if (_msgHandler->receiveMessage(sock, &packet)) {
          auto *payload = packet.mutable_payload();
          bool checkForPeer = false;
          if (payload->type() == payload::packet::ADVERTISE) {
            // If authentication fails, close connection with client
            isProto = true;
            auto *crypto = payload->mutable_crypto();
            std::string hashedKey = crypto->hashedkey();

            // Get public key. NOTE: Here I use a really simple way to store
            // public keys just for testing purposes
            std::string key = this->getPublicKey(hashedKey); 
            checkForPeer = _msgHandler->verify(sock, key);
          } else if (payload->type() == payload::packet::PEER_INFO) {
            // PEER_INFO requires no authentication
            checkForPeer = true;
          }
          if (checkForPeer) {
            _findPeerInformation(*payload, sock);
          } else {
            close(sock);
            FD_CLR(sock, &_master);
            _removePeer(sock);
          }
        } else {
          // No message/ or peer not using official client
          close(sock);
          FD_CLR(sock, &_master);
          _removePeer(sock);
        }
        _readyToP2P(sock, isProto);
      }
    }
  }
}
std::string mediator::Server::getPublicKey(std::string &hashedKey){
  // NOTE
  // ------------------------------------------------------------------
  // This is just for testing. You may want to add another way to
  // fetch peer's public keys rather than by calling a folder with the
  // public keys
  std::string pathCert = CERTIFICATES_PATH;
  std::string keyPath = pathCert + hashedKey + "/public.pem";

  return _msgHandler->loadPublicKey(keyPath); 
}

bool mediator::Server::_isAuthenticating(int sock){
  std::lock_guard<std::mutex> lck(_mutex);
  return std::find(_authenticating.begin(), _authenticating.end(), sock) !=
         _authenticating.end();
}
