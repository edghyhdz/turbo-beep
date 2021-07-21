/**
 * Socket class declaration
 */

#ifndef SOCKET_CREATOR
#define SOCKET_CREATOR
#include <arpa/inet.h>
#include <memory>

class Socket {
public:
  Socket(const char *&ipAddress, const char *&portNum);
  Socket(const Socket &) = delete;
  ~Socket();

  void bindToPort();
  void connectToPeer(std::uint16_t const peerPort, std::string const peerIp);
  void connectToServer(std::uint16_t const serverPort,
                       std::string const serverIp);
  void close();

  std::string ipAddress() const { return _myIpAddress; }
  std::uint16_t port() const { return _myPort; }

private:
  struct sockaddr_in _hints;
  std::string _myIpAddress;
  std::uint16_t _myPort;
  bool _connectionOpen;
};

#endif