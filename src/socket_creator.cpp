#include "socket_creator.h"
#include "messages.h"
#include <iostream>
#include <socket/socket_utils.h>
#include <string.h>
#include <sys/socket.h>

using namespace turbobeep;

// Connect and hash keys and so
p2p::Socket::Socket(char *&ipAddress, char *&portNum, std::string flag, std::string pathKeyPair,
         std::string pathPeerPublicKey)
    : _connectionOpen(false), _needsAuth(true) {

  _protoHandle = std::make_unique<messages::ProtoBuf>(pathKeyPair, pathPeerPublicKey);
  _myInfo.userName = ""; 
  _myInfo.peerName = ""; 
  _myInfo.myHash = _protoHandle->sha1(_protoHandle->publicKey(),
                                        _protoHandle->publicKey().length());
  _myInfo.peerHash = _protoHandle->sha1(_protoHandle->peerPublicKey(),
                                        _protoHandle->peerPublicKey().length());

  // Get own ip address
  _setIpAddress();
  memset(&this->_hints, 0, sizeof(_hints));
  _hints.ai_family = AF_UNSPEC;
  _hints.ai_socktype = SOCK_STREAM;
  _hints.ai_flags = AI_PASSIVE;
  _addrInfo = getaddrinfo(ipAddress, portNum, &_hints, &_p);
  _bindToPort();
}

// Connect to the server upon socket creation
p2p::Socket::Socket(char *&ipAddress, char *&portNum, std::string userName,
               std::string peerName)
    : _connectionOpen(false), _needsAuth(false){

  _protoHandle = std::make_unique<messages::ProtoBuf>(); 
  _myInfo.userName = userName; 
  _myInfo.peerName = peerName; 

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
p2p::Socket::~Socket() {
  if (_t.joinable())
   _t.join(); 
}

/**
 * Listens to server, started on a thread.
 * Waits for instructions from the server, related to info of peer to connect to
 *
 * It can either send basic information depending on how the peer wants to
 * connect to the other peer. Either by providing the name, or the hashed key.
 * If peer provides the hashed key, then it will follow up a challenge/response
 * authentication step with the server
 */
void p2p::Socket::_listenToServer() {
  // If it does not need to authenticate
  if (!_needsAuth) {
    do {
      std::string recvMessage;
      if (!_protoHandle->receiveMessage(_sockFD, &recvMessage)) {
        throw std::runtime_error(
            "Error receiving message. Connection closed by server");
      } else {
        // Server sent peer's ip and port
        std::string delimiter{":"};
        if (recvMessage.find(delimiter) != std::string::npos) {
          size_t pos = 0;

          while ((pos = recvMessage.find(delimiter)) != std::string::npos) {
            _peerIpAddress = recvMessage.substr(0, pos);
            recvMessage.erase(0, pos + delimiter.length());
          }
          _peerPort = stoi(recvMessage);
          break;
        }
      }
    } while (true);

  } else {
    // Authentication needed, challenge/response authentication steps
    int size;
    payload::packet packet;
    if (!_protoHandle->receiveMessage(_sockFD, &packet)) {
      throw std::runtime_error(
          "Error receiving message. Connection closed by server");
    }
    auto *payload = packet.mutable_payload();
    auto *crypto = payload->mutable_crypto();

    // Message should contain a type CHALLENGE and the nonce to encrypt
    std::string encryptedNonce = _protoHandle->signString(crypto->nonce());

    // Respond to challenge
    packet.set_time_stamp(_protoHandle->getTimeStamp());
    payload->set_type(payload::packet_MessageTypes_RESPONSE);
    crypto->set_encryptednonce(encryptedNonce);
    size = packet.ByteSize() + 4;

    // Send encrypted nonce for server to verify
    _protoHandle->sendMessage(size, _sockFD, packet);

    // Upon successfully authenticating, server will keep connection open
    // Wait for server to send peer information
    if (!_protoHandle->receiveMessage(_sockFD, &packet)) {
      throw std::runtime_error("Connection closed by server");
    }

    auto *success = packet.mutable_payload(); 

    if (success->type() != payload::packet_MessageTypes_SUCCESS){
      throw std::runtime_error("Could not successfully authenticate"); 
    }

    auto *otherPeerInfo = success->mutable_otherpeerinfo(); 
    _peerIpAddress = otherPeerInfo->peeripaddress();
    _peerPort = otherPeerInfo->peerport();
  }

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
  socket_utils::getIpAddress(&httpCode, &_myInfo.myIpAddress);

  if (httpCode != 200) {
    throw std::runtime_error("Could not get my IP Address");
  }
  return;
}

/**
 * Sets `_myPort` to my current used port
 *
 * @param _myInfo.myPort private member to be passed as reference to getAvailablePort
 */
void p2p::Socket::_setPort() { socket_utils::getAvailablePort(&_myInfo.myPort); }

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
  this->_myAddr.sin_port = htons(port());
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

  // Start a thread to wait for response of server comfirming
  // peer has connected
  _t = std::thread(&Socket::_listenToServer, this);
  _connectionOpen = true;
}

/**
 * Public method implementation of _connectToServer.
 * Including also `_cond` to wait for _listenToServer() to retrieve peer port
 * and ip address. 
 * After that this method will return
 */
void p2p::Socket::connectToServer(payload::packet::MessageTypes &mType) {
  payload::packet packet; 
  _connectToServer();

  // After connecting to the server, send message
  int size; 

  // Add information to the packet and send message
  messages::ProtoBuf::addUserInfo(&size, &packet, myInfo(), mType);
  _protoHandle->sendMessage(size, _sockFD, packet);

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
