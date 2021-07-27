/**
 * Socket class declaration
 */

#ifndef SOCKET_CREATOR
#define SOCKET_CREATOR
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
namespace p2p {

struct myInfo {
  std::string myIpAddress; 
  std::uint16_t myPort; 
  std::string userName; 
}; 

class Socket {
public:
  Socket(char *&ipAddress, char *&portNum, std::string userName,
         std::string peerName);
  ~Socket();
  void connectToServer();
  void connectToPeer();
  void close();
  p2p::myInfo myInfo() const & { return _myInfo; }
  std::string ipAddress() const { return _myInfo.myIpAddress; }
  std::uint16_t port() const { return _myInfo.myPort; }
  std::string userName() const { return _myInfo.userName; }

private:
  addrinfo _hints, *_p;
  struct sockaddr_in _myAddr, _peerAddr;
  void _sendMessage(); 
  void _setIpAddress();
  void _setPort();
  void _bindToPort();
  void _connectToServer();
  void _listenToServer();

  p2p::myInfo _myInfo; 

  int _sockFD;
  int _connFD;
  int _addrInfo;
  bool _connectionOpen;
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