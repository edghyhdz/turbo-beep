/**
 * Socket class declaration
 */

#ifndef SOCKET_CREATOR
#define SOCKET_CREATOR
#include <arpa/inet.h>
#include <memory>
#include <netdb.h>
#include <unistd.h>

class Socket {
public:
  Socket(char *&ipAddress, char *&portNum);
  // Socket(const Socket &) = delete;
  // ~Socket();

  void bindToPort();
  void connectToPeer(std::uint16_t const peerPort, std::string const peerIp);
  void connectToServer(std::uint16_t const serverPort,
                       std::string const serverIp);
  void close();

  std::string ipAddress() const { return _myIpAddress; }
  std::uint16_t port() const { return _myPort; }

private:
  addrinfo _hints, *_p;
  struct sockaddr_in _myAddr, peerAddr;
  void _setIpAddress();
  void _setPort(); 
  void _connectToServer();
  void _listenToServer(); 
  void _connectToPeer(std::uint16_t const peerPort, std::string const peerIp);
  
  int _sockFD; 
  int _addrInfo; 
  std::string _myIpAddress;
  std::uint16_t _myPort;
  bool _connectionOpen;
};

#endif