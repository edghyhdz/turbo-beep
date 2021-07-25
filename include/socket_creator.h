/**
 * Socket class declaration
 */

#ifndef SOCKET_CREATOR
#define SOCKET_CREATOR
#include <arpa/inet.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <netdb.h>
#include <thread>
#include <unistd.h>

class Socket {
public:
  Socket(char *&ipAddress, char *&portNum, std::string userName,
         std::string peerName);
  ~Socket();
  void connectToServer();
  void connectToPeer();
  void close();
  std::string ipAddress() const { return _myIpAddress; }
  std::uint16_t port() const { return _myPort; }

private:
  addrinfo _hints, *_p;
  struct sockaddr_in _myAddr, _peerAddr;
  void _setIpAddress();
  void _setPort();
  void _bindToPort();
  void _connectToServer();
  void _listenToServer();

  std::string _userName;
  int _sockFD;
  int _connFD;
  int _addrInfo;
  std::string _myIpAddress;
  std::uint16_t _myPort;
  bool _connectionOpen;
  std::thread _t;
  std::string _peerName;
  std::uint16_t _peerPort;
  std::string _peerIpAddress;
  std::mutex _mutex;
  std::condition_variable _cond; 
};

#endif