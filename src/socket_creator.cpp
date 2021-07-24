#include "socket_creator.h"
#include <iostream>
#include <socket/socket_utils.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>

// Connect to the server upon socket creation
Socket::Socket(char *&ipAddress, char *&portNum) : _connectionOpen(false) {

  // Get own ip address
  _setIpAddress();

  std::cout << ipAddress << ":" << portNum << std::endl;
  memset(&this->_hints, 0, sizeof(_hints));
  _hints.ai_family = AF_UNSPEC;
  _hints.ai_socktype = SOCK_STREAM;
  _hints.ai_flags = AI_PASSIVE;
  _addrInfo = getaddrinfo(ipAddress, portNum, &_hints, &_p);

  _connectToServer();
  
}

/**
 * Gets user's ip address from given endpoint on socket_utils::getIpAddress.
 * It might be a bit unreliable, since that endpoint migth not exists at some
 * point in time
 *
 * @param[out] httpCode the response code from http request
 * @param[out] readBuffer the response ip address from http request
 * @ref [socket_utils::getIpAddress]
 */
void Socket::_setIpAddress() {
  long httpCode;
  socket_utils::getIpAddress(&httpCode, &_myIpAddress);

  if (httpCode != 200) {
    throw std::runtime_error("Could not get my IP Address");
  }
  return;
}

/**
 * Sets `_myPort` to my current used port
 *
 * @param _myPort private member to be passed as reference to getAvailablePort
 */
void Socket::_setPort() { socket_utils::getAvailablePort(&_myPort); }

/**
 * Connects to server as given by the args `ipAddress` and `portNum`
 */
void Socket::_connectToServer() {
  unsigned int value = 1;
  this->_sockFD = socket(_p->ai_family, _p->ai_socktype, _p->ai_protocol);
  setsockopt(_sockFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
  int status = connect(_sockFD, _p->ai_addr, _p->ai_addrlen);
  std::cout << "Status: " << status << std::endl;

  // Upon connection, send my IP address to the server
  send(_sockFD, this->ipAddress().c_str(), this->ipAddress().length() + 1, 0);

  _connectionOpen = true;
}

/**
 * Connects to peer. 
 * 
 * @param peerPort - Peer's port to connect to 
 * @param peerIp - Peer's ip address to connect to
 */
void Socket::_connectToPeer(std::uint16_t const peerPort,
                            std::string const peerIp) {}
