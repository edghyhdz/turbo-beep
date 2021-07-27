#include "socket_creator.h"
#include <iostream>
#include <socket/socket_utils.h>
#include <string.h>
#include <sys/socket.h>

using namespace turbobeep;

// Connect to the server upon socket creation
p2p::Socket::Socket(char *&ipAddress, char *&portNum, std::string userName,
               std::string peerName)
    : _connectionOpen(false), _userName(userName), _peerName(peerName) {

  // Get own ip address
  _setIpAddress();
  std::cout << ipAddress << ":" << portNum << std::endl;
  memset(&this->_hints, 0, sizeof(_hints));
  _hints.ai_family = AF_UNSPEC;
  _hints.ai_socktype = SOCK_STREAM;
  _hints.ai_flags = AI_PASSIVE;
  _addrInfo = getaddrinfo(ipAddress, portNum, &_hints, &_p);
  _bindToPort();
}

// Destructor
p2p::Socket::~Socket() { _t.join(); }

/**
 * Add peer information, _myPort, myIpAddress, myName, timestamp and message
 * type to the protobuffer
 *
 * @param[in, out] size size of the serialized payload to send
 * @param[in, out] packet packet object
 */
void p2p::Socket::peerInfoToPayload(int *size, payload::packet *packet) {

  auto *payload = packet->mutable_payload();
  auto *peerInfo = payload->mutable_peerinfo(); 

  const auto timeStamp = std::chrono::system_clock::now();
  long tS = std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();

  packet->set_time_stamp(tS); 
  payload->set_type(packet->PEER_INFO);
  peerInfo->set_port(this->port()); 
  peerInfo->set_ipaddress(this->ipAddress());
  peerInfo->set_username(this->userName()); 

  *size = packet->ByteSize() + 4;
}

/**
 * Serializes message packet after having added the data with
 * peerInfoToPayload() method
 *
 * @param[in, out] coded_output serialized object to send to peer
 * @param[in] packet packet object needed to serialize coded_output
 */
void p2p::Socket::serializeMessage(output_stream *coded_output,
                              payload::packet &packet) {

  coded_output->WriteVarint32(packet.ByteSize()); 
  packet.SerializeToCodedStream(coded_output); 
}

// TODO: This implementation will change, since this is just a feasibility
// example
/**
 * Listens to server, started on a thread.
 * Waits for instructions from the server, related to info of peer to connect to
 */
void p2p::Socket::_listenToServer() {
  int res;
  char buffer[128];

  do {
    memset(buffer, 0, 128);
    int bytesReceived = recv(_sockFD, buffer, 128, 0);

    if (bytesReceived == 0) {
      throw std::runtime_error(
          "Error receiving message. Connection closed by server");
    } else {
      std::ostringstream ss;
      ss << buffer;
      std::string s = ss.str();
      std::cout << "Server msg: " << s << std::endl;

      // Server sent peer's ip and port
      std::string delimiter{":"};
      if (s.find(delimiter) != std::string::npos) {
        size_t pos = 0;

        while ((pos = s.find(delimiter)) != std::string::npos) {
          _peerIpAddress = s.substr(0, pos);
          s.erase(0, pos + delimiter.length());
        }
        _peerPort = stoi(s);
        break;
      }
    }
  } while (true);

  // Notify so that connectToServer() can return
  std::lock_guard<std::mutex> lck(_mutex);
  this->_cond.notify_one();
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
void p2p::Socket::_setIpAddress() {
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
void p2p::Socket::_setPort() { socket_utils::getAvailablePort(&_myPort); }

/**
 * Binds to port that will be sent to the mediator and later from the mediator
 * the the peer. It uses `_setPort()` to get an available port and assign it to
 * `_myPort`
 */
void p2p::Socket::_bindToPort() {
  // Get available port
  _setPort();
  unsigned int value = 1;
  _connFD = socket(AF_INET, SOCK_STREAM, 0);
  this->_myAddr.sin_family = AF_INET;
  this->_myAddr.sin_port = htons(_myPort);
  this->_myAddr.sin_addr.s_addr = INADDR_ANY;

  setsockopt(_connFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

  if (bind(_connFD, (struct sockaddr *)&_myAddr, sizeof(_myAddr)) == -1) {
    std::cerr << "Can't bind to IP/Port";
    throw std::runtime_error("Can't bind to IP/Port");
  }
}

/**
 * Connects to server as given by the args `ipAddress` and `portNum`
 */
void p2p::Socket::_connectToServer() {

  unsigned int value = 1;
  this->_sockFD = socket(_p->ai_family, _p->ai_socktype, _p->ai_protocol);
  setsockopt(_sockFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
  int status = connect(_sockFD, _p->ai_addr, _p->ai_addrlen);

  // Start listening to server after connection
  if (status == -1) {
    throw std::runtime_error("Error connecting to server");
  }

  // Start a thread to wait for response of server confirming
  // peer has connected
  _t = std::thread(&Socket::_listenToServer, this);

  // Send message containing this peer info
  std::string message = ipAddress() + "::" + std::to_string(port()) +
                        "::" + _userName + "::" + _peerName;

  // Upon connection, send my IP address to the server
  send(_sockFD, message.c_str(), message.length() + 1, 0);

  _connectionOpen = true;
}

/**
 * Public method implementation of _connectToServer.
 * Including also `_cond` to wait for _listenToServer() to retrieve peer port
 * and ip address. 
 * After that this method will return
 */
void p2p::Socket::connectToServer() {
  _connectToServer();

  // wait until other client has connected
  std::unique_lock<std::mutex> lck(_mutex);
  _cond.wait(lck);

  std::cout << "Retrieved peer info: " << _peerIpAddress << ":" << _peerPort
            << ". Ready to connect to peer" << std::endl;
}

/**
 * Connects to peer.
 *
 * @param peerPort - Peer's port to connect to
 * @param peerIp - Peer's ip address to connect to
 */
void p2p::Socket::connectToPeer() {
  char buffer[128];
  char bufferSend[128];

  _peerAddr.sin_family = AF_INET;
  _peerAddr.sin_addr.s_addr = INADDR_ANY;
  _peerAddr.sin_port = htons(_peerPort);
  _peerAddr.sin_addr.s_addr = inet_addr(_peerIpAddress.c_str());

  // Try to connect to the other peer at the same time
  int res;
  while (1) {
    static int j = 1;
    res = connect(_connFD, (struct sockaddr *)&_peerAddr, sizeof(_peerAddr));
    if (res == -1) {
      if (j >= 10)
        throw std::runtime_error("can't connect to the other client\n");
      std::cout << "Connection timed out... trying again." << std::endl;
      sleep(5);
    } else
      break;
  }

  std::cout << "Connected ..." << std::endl;
  ::close(_sockFD);
  std::string message =
      "Hello world from " + _peerIpAddress + ":" + std::to_string(_peerPort);
  strcpy(bufferSend, message.c_str());

  // Send and receive messages from the connected peer
  while (1) {
    res = send(_connFD, bufferSend, strlen(bufferSend) + 1, 0);
    if (res <= 0)
      throw std::runtime_error("write error");
    sleep(1);

    res = recv(_connFD, buffer, 4096, 0);
    if (res <= 0)
      throw std::runtime_error("read error");
    std::cout << "Received message: " << buffer << std::endl;
  }
}
