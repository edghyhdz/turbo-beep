#include "peer.h"
// #include "file_handler.h"
#include "messages.h"
#include "vendor/base64.h"
#include <iostream>
#include <math.h>
#include <socket/socket_utils.h>
#include <string.h>
#include <sys/socket.h>

using namespace turbobeep;

// Connect and hash keys and so
p2p::Peer::Peer(char *&ipAddress, char *&portNum, std::string flag, std::string pathKeyPair,
         std::string pathPeerPublicKey)
    : _hasAdvertisedFirst(false), _needsAuth(true) {

  _messageHandler = std::make_unique<messages::MessageHandler>(pathKeyPair, pathPeerPublicKey);
  _myInfo.userName = ""; 
  _myInfo.peerName = "";
  _myInfo.myHash = _messageHandler->sha1(_messageHandler->publicKey(),
                                         _messageHandler->publicKey().length());
  _myInfo.peerHash =
      _messageHandler->sha1(_messageHandler->peerPublicKey(),
                            _messageHandler->peerPublicKey().length());
  std::cout << "My hashed key: " << _myInfo.myHash << std::endl;
  std::cout << "Peer hashed key: " << _myInfo.peerHash << std::endl;

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
p2p::Peer::Peer(char *&ipAddress, char *&portNum, std::string userName,
               std::string peerName)
    : _hasAdvertisedFirst(false), _needsAuth(false){

  _messageHandler = std::make_unique<messages::MessageHandler>(); 
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
p2p::Peer::~Peer() {
  if (_t.joinable())
   _t.join(); 
}

void p2p::Peer::close(){
  // Close server connection
  ::close(_sockFD); 
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
void p2p::Peer::_listenToServer() {
  // If it does not need to authenticate
  if (!_needsAuth) {
    do {
      std::string recvMessage;
      if (!_messageHandler->receiveMessage(_sockFD, &recvMessage)) {
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
    if (!_messageHandler->authenticate(_sockFD)){
      throw std::runtime_error("Failed to authenticate"); 
    }

    // Wait for peer info to be sent
    payload::packet packet; 
    if (!_messageHandler->receiveMessage(_sockFD, &packet)){
      throw std::runtime_error("Error receiving peer information"); 
    } 
    
    auto *payload = packet.mutable_payload(); 
    auto *otherPeerInfo = payload->mutable_otherpeerinfo(); 
    _peerIpAddress = otherPeerInfo->peeripaddress();
    _peerPort = otherPeerInfo->peerport();

    _hasAdvertisedFirst = otherPeerInfo->hasadvertisedfirst(); 
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
void p2p::Peer::_setIpAddress() {
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
void p2p::Peer::_setPort() { socket_utils::getAvailablePort(&_myInfo.myPort); }

/**
 * Binds to port that will be sent to the mediator and later from the mediator
 * the the peer. It uses `_setPort()` to get an available port and assign it to
 * `_myPort`
 */
void p2p::Peer::_bindToPort() {
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
void p2p::Peer::_connectToServer() {

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
  _t = std::thread(&Peer::_listenToServer, this);
}

/**
 * Public method implementation of _connectToServer.
 * Including also `_cond` to wait for _listenToServer() to retrieve peer port
 * and ip address. 
 * After that this method will return
 */
void p2p::Peer::connectToServer(payload::packet::MessageTypes &mType) {
  payload::packet packet; 
  _connectToServer();

  // After connecting to the server, send message
  int size; 

  // Add information to the packet and send message
  _messageHandler->addUserInfo(&size, &packet, myInfo(), mType);
  _messageHandler->sendMessage(size, _sockFD, packet);

  // wait until other client has connected
  std::unique_lock<std::mutex> lck(_mutex);
  _cond.wait(lck);

  std::cout << "[PEER]: Retrieved peer info: " << _peerIpAddress << ":" << _peerPort
            << ". Ready to connect to peer" << std::endl;
}

/**
 * Connects to peer.
 *
 * @param peerPort - Peer's port to connect to
 * @param peerIp - Peer's ip address to connect to
 */
bool p2p::Peer::connectToPeer() {
  char buffer[128];
  char bufferSend[128];

  // Change _peerip in case both peers are in the same network
  if (_peerIpAddress == ipAddress()) {
    _peerIpAddress = "127.0.0.1";
  }

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
      if (j >= 1000)
        return false;
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      j++;
    } else
      break;
  }

  std::cout << "[PEER]: Connected to: " << _peerIpAddress << ":" << _peerPort
            << ". Authenticating..." << std::endl;

  // Close server socket
  ::close(_sockFD);

  // If it does not require authentication
  if (!_needsAuth)
    return true;

  // Peer authentication depending on what peer advertised first to the server
  if (_hasAdvertisedFirst) {
    return _messageHandler->authenticate(_connFD);
  }
  // If not, then verify authentication
  std::string peerKey = _messageHandler->peerPublicKey();
  return _messageHandler->verify(_connFD, peerKey);
}

/**
 * Listen to other peer for messages
 */
void p2p::Peer::listenToPeer(){
  while (true) {
    std::cout << "> ";
    payload::packet packet;

    if (!_messageHandler->receiveMessage(_connFD, &packet)){
      throw std::runtime_error("Could not receive message."); 
    }

    // packet
    auto *payload = packet.mutable_payload();
    auto *crypto = payload->mutable_crypto();

    std::string encrypted_msg = crypto->encryptedmsg();

    // Since there are two ways to use this client. We need to seee if user
    // needs auth to check if we encrypt/decrypt messages or not
    std::string decrypted_msg =
        _needsAuth ? _messageHandler->decryptWithPublicKey(
                         encrypted_msg, _messageHandler->peerPublicKey())
                   : encrypted_msg;
    std::cout << "[" << _peerIpAddress << "]: " << decrypted_msg << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

/**
 * Handles messages comming from peer
 */
void p2p::Peer::peerMessageHandler() {
  std::string userInput;
  payload::packet packet;
  auto *payload = packet.mutable_payload(); 
  auto *crypto = payload->mutable_crypto(); 
  int size;

  while (true) {
    std::getline(std::cin, userInput);
    packet.set_time_stamp(messages::MessageHandler::getTimeStamp());
    payload->set_type(payload::packet_MessageTypes_SUCCESS);

    // Since there are two ways to use this client. We need to seee if user
    // needs auth to check if we encrypt/decrypt messages or not
    userInput = _needsAuth ? _messageHandler->signString(userInput) : userInput;
    crypto->set_encryptedmsg(userInput);
    size = packet.ByteSize() + 4;

    _messageHandler->sendMessage(size, _connFD, packet); 
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

/**
 * Sends encrypted file to other peer
 * @param filePath well, it is the file path of the file to send indeed
 */
bool p2p::Peer::sendFile(std::string &filePath){
  return _messageHandler->sendFile(_connFD, filePath); 
}

/**
 * Receive file
 */
bool p2p::Peer::receiveFile(){
  return _messageHandler->receiveFile(_connFD);
}