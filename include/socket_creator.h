/**
 * Socket class declaration
 */

#ifndef SOCKET_CREATOR
#define SOCKET_CREATOR
#include <arpa/inet.h>
#include <memory>
#include <netdb.h>
#include <unistd.h>
#include <thread>

class Socket {
public:
  Socket(char *&ipAddress, char *&portNum, std::string userName, std::string peerName);
  ~Socket(); 
  void bindToPort();
  void connectToPeer(std::uint16_t const peerPort, std::string const peerIp);
  void close();
  std::string ipAddress() const { return _myIpAddress; }
  std::uint16_t port() const { return _myPort; }

private:
  addrinfo _hints, *_p;
  struct sockaddr_in _myAddr, _peerAddr;
  void _setIpAddress();
  void _setPort(); 
  void _connectToServer();
  void _listenToServer(); 
  void _connectToPeer(std::uint16_t const peerPort, std::string const peerIp);
  
  std::string _userName; 
  int _sockFD; 
  int _addrInfo; 
  std::string _myIpAddress;
  std::uint16_t _myPort;
  bool _connectionOpen;
  std::thread _t; 
  std::string _peerName; 
};

#endif