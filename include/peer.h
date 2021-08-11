/**
 * Peer class declaration
 */

#ifndef PEER_H
#define PEER_H
#include "payload.pb.h"
#include <arpa/inet.h>
#include <condition_variable>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <memory>
#include <mutex>
#include <netdb.h>
#include <thread>
#include <unistd.h>

typedef google::protobuf::io::CodedOutputStream output_stream;
typedef google::protobuf::io::ArrayOutputStream array_output_stream;

namespace turbobeep {
namespace messages {
class MessageHandler; // Forward declaration
// class FileHandler; // Forward declaration
}

namespace p2p {

struct myInfo {
  std::string myIpAddress; 
  std::uint16_t myPort; 
  std::string userName; 
  std::string myHash;
  std::string peerName; 
  std::string peerHash; 
}; 

class Peer{
public:
  Peer(char *&ipAddress, char *&portNum, std::string flag, std::string pathKeyPair,
         std::string pathPeerPublicKey);

  Peer(char *&ipAddress, char *&portNum, std::string userName,
         std::string peerName);
  ~Peer();
  void connectToServer(payload::packet::MessageTypes &mType);
  bool connectToPeer();
  void listenToPeer(); 
  void peerMessageHandler(); 
  bool sendFile(std::string &filePath); 
  bool receiveFile(); 

  void close();
  p2p::myInfo myInfo() const & { return _myInfo; }
  std::string ipAddress() const { return _myInfo.myIpAddress; }
  std::uint16_t port() const { return _myInfo.myPort; }
  std::string userName() const { return _myInfo.userName; }
  std::string peerName() const { return _myInfo.peerName; }
  std::string hashedKey() const { return _myInfo.myHash; }
  std::string peerHashedKey() const { return _myInfo.peerHash; }

private:
  std::unique_ptr<messages::MessageHandler> _messageHandler; 
  addrinfo _hints, *_p;
  struct sockaddr_in _myAddr, _peerAddr;
  void _setIpAddress();
  void _setPort();
  void _bindToPort();
  void _connectToServer();
  void _listenToServer();

  p2p::myInfo _myInfo; 

  int _sockFD;
  int _connFD;
  int _addrInfo;
  bool _needsAuth; 
  bool _hasAdvertisedFirst;
  std::thread _t;
  std::string _peerName;
  std::uint16_t _peerPort;
  std::string _peerIpAddress;
  std::mutex _mutex;
  std::condition_variable _cond;
};
} // namespace p2p
} // namespace turbobeep
#endif