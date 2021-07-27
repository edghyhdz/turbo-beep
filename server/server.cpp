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

/*****************************************************************************
 * Server class member definitions
 */

using namespace turbobeep; 

mediator::Server::Server(std::uint16_t port) : _serverPort(port) { this->initServer(); }

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

void mediator::Server::_findPeerInformation(std::string &buffer, int sock){
  std::string delimiter = "::";

  if (buffer.find(delimiter) != std::string::npos) {
    size_t pos = 0;
    std::vector<std::string> params;

    while ((pos = buffer.find(delimiter)) != std::string::npos) {
      params.push_back(buffer.substr(0, pos));
      buffer.erase(0, pos + delimiter.length());
    }
    
    params.push_back(buffer);
    
    if (_userDescriptor.find(params[2]) == _userDescriptor.end()) {
      // If username not found -> add it to map
      userInfo newUser;
      newUser.ipAddress = params[0];
      newUser.port = stoi(params[1]); 
      newUser.name = params[2];
      newUser.socket = sock;
      newUser.peerInfo.name = params[3];
      _userDescriptor.insert(std::make_pair(params[2], newUser));
      _findPeer(params[2], params[3]);
    }
  }
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
        char buf[4096];
        memset(buf, 0, 4096);
        // Receive message
        int bytesIn = recv(sock, buf, 4096, 0);
        if (bytesIn <= 0) {
          // drop client
          close(sock);
          FD_CLR(sock, &_master);
          _removePeer(sock);
        } else {

          std::ostringstream ss;
          ss << buf;
          std::string s = ss.str();

          _findPeerInformation(s, sock); 
        }
        // Check if both peers are connected and ready to start a P2P connection
        _readyToP2P(sock);
      }
    }
  }
}
