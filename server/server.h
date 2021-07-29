/*
References:
Part of this code was taken from user Sloan Kelly, and adapted to this script to
run on linux and work with Ncurses link to repo:
https://bitbucket.org/sloankelly/youtube-source-repository/src/master/cpp/networking/MultipleClientsBarebonesServer/MultipleClientsBarebonesServer/

-----------------------
Server class declarations
*/

#ifndef SERVER_MULT
#define SERVER_MULT
#define PORT_SERVER 54700
#include <map>
#include <set>
#include "messages_receive.h"

namespace turbobeep {
namespace mediator {

class TestServer;

// Used to keep track of connecting peer info as well as peer to connect to
struct peerInfo {
  std::string name;
  std::string ipAddress;
  uint16_t port;
};

struct userInfo {
  bool isClient;
  bool canConnect{false};
  std::string name;
  std::string ipAddress;
  uint16_t port;
  struct peerInfo peerInfo;
  int socket;
};

// Server class declaration
class Server {
public:
  Server(std::uint16_t port = 54700);
  ~Server();
  int initServer();
  void runServer();
  void readBody(int sock, uint32g size,payload::packet *packet); 

private:
  void _updatePeerInfo(std::string const &user, std::string const &peer);
  void _findPeer(std::string const &user, std::string const &peer);
  void _removePeer(int const &socket);
  void _readyToP2P(int const &socket);
  void _findPeerInformation(payload::packet_Payload& payload, int sock);

  int _listening;
  fd_set _master;
  std::map<std::string, userInfo> _userDescriptor;
  std::uint16_t _serverPort;
  std::unique_ptr<messages::Receive> _recvHandle; 

  friend mediator::TestServer;
};
} // namespace mediator
} // namespace turbobeep
#endif