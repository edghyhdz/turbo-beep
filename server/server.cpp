/**
 * Server class declaration
 */
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

/**
 * Updates peer information with other's peer information (ipAddress and port).
 *
 * @param[in, out] uD `userDescriptor` connecting's peer information
 * @param[in, out] user current users' name
 * @param[in] peer Client's peer to connect to
 */
void updatePeerInfo(std::map<std::string, userInfo> *uD,
                    std::string const &user, std::string const &peer) {
  uD->at(user).canConnect = true;
  uD->at(user).peerInfo.ipAddress = uD->at(peer).ipAddress;
  uD->at(user).peerInfo.port = uD->at(peer).port;;
}

/**
 * Finds whether if peer's peer is already connected.
 * Peer that connects first will be consired the client, and thus
 * `isClient = true` will be assigned.
 *
 * @param[in, out] userDescriptor connecting's peer information
 * @param[in, out] user current users' name
 * @param[in] peer Client's peer to connect to
 */
void findPeer(std::map<std::string, userInfo> *userDescriptor,
              std::string const &user, std::string const &peer) {
  if (userDescriptor->find(peer) == userDescriptor->end()) {
    userDescriptor->at(user).isClient = true;
  } else {
    // Both are now connected
    userDescriptor->at(user).isClient = false;
    updatePeerInfo(userDescriptor, user, peer);
    updatePeerInfo(userDescriptor, peer, user);
  }
}

/**
 * Remove peer from `userDescriptor` once peer has disconnected from server
 *
 * @param[in, out] uD `userDescriptor` map containing user's information
 * @param[in] socket disconnected user's socket
 */
void removePeer(std::map<std::string, userInfo> *uD, int const &socket) {
  if (!uD->empty()) {
    for (auto key_val : *uD) {
      if (socket == key_val.second.socket) {
        std::cout << "Erased: " << key_val.second.name << std::endl;
        uD->erase(key_val.second.name);
        break;
      }
    }
  }
}

/**
 * Checks if both peers have `canConnect`=true, this only happens once both
 * peers have successfully connected to the server
 *
 * @param[in, out] uD `userDescriptor` map containing user's information
 * @param[in] socket disconnected user's socket
 */
void readyToP2P(std::map<std::string, userInfo> const &uD, int const &socket) {

  for (auto key_val : uD) {
    if (socket == key_val.second.socket) {
      if (key_val.second.canConnect) {
        // Send message to both peers
        std::string peerName = key_val.second.peerInfo.name;
        int peerSocket = uD.at(peerName).socket;

        std::string ipAddressPeer =
            uD.at(peerName).peerInfo.ipAddress + ":" +
            std::to_string(uD.at(peerName).peerInfo.port);

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

void findPeerInformation(std::string &buffer, std::map<std::string, userInfo> *uD, int sock){
  std::string delimiter = "::";

  if (buffer.find(delimiter) != std::string::npos) {
    size_t pos = 0;
    std::vector<std::string> params;

    while ((pos = buffer.find(delimiter)) != std::string::npos) {
      params.push_back(buffer.substr(0, pos));
      buffer.erase(0, pos + delimiter.length());
    }

    params.push_back(buffer);

    if (uD->find(params[2]) == uD->end()) {
      // If username not found -> add it to map
      userInfo newUser;
      newUser.ipAddress = params[0];
      newUser.port = stoi(params[1]); 
      newUser.name = params[2];
      newUser.socket = sock;
      newUser.peerInfo.name = params[3];
      uD->insert(std::make_pair(params[2], newUser));
      findPeer(uD, params[2], params[3]);
    }

    for (auto const &key_val : *uD) {
      std::cout << key_val.first
                << " has address: " << key_val.second.ipAddress << ":"
                << key_val.second.port
                << " with socket: " << key_val.second.socket
                << " and peer to connect to: " << key_val.second.peerInfo.name
                << std::endl;
    }
  }
}

/*****************************************************************************
 * Class member methods start here
 */

Server::Server() { this->initServer(); }

Server::~Server() {
  FD_CLR(_listening, &_master);
  close(_listening);
}

int Server::initServer() {
  this->_listening = socket(AF_INET, SOCK_STREAM, 0);

  if (_listening == -1) {
    throw std::runtime_error("Can't create a socket"); 
  }

  // Bind the socket to an IP / Port
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(PORT_SERVER);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

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

void Server::runServer() {
  std::map<int, int> socketToUser;
  std::map<std::string, userInfo> userDescriptor;

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
          removePeer(&userDescriptor, sock);
        } else {

          std::ostringstream ss;
          ss << buf;
          std::string s = ss.str();

          findPeerInformation(s, &userDescriptor, sock); 
        }
        // Check if both peers are connected and ready to start a P2P connection
        std::cout << "Sock " << sock << std::endl;
        readyToP2P(userDescriptor, sock);
      }
    }
  }
}
