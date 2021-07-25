/*
References:
Part of this code was taken from user Sloan Kelly, and adapted to this script to
run on linux and work with Ncurses link to repo:
https://bitbucket.org/sloankelly/youtube-source-repository/src/master/cpp/networking/MultipleClientsBarebonesServer/MultipleClientsBarebonesServer/

-----------------------
Server class definition
*/

#ifndef SERVER_MULT
#define SERVER_MULT
#define PORT_SERVER 54700

#include <map>
#include <set>

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
  Server();
  ~Server();
  int initServer();
  void runServer();

private:
  void _updatePeerInfo(std::string const &user, std::string const &peer); 
  void _findPeer(std::string const &user, std::string const &peer);
  void _removePeer(int const &socket);
  void _readyToP2P(int const &socket);
  void _findPeerInformation(std::string &buffer, int sock);
  int _listening;
  fd_set _master;
  std::map<std::string, userInfo> _userDescriptor; 
};

#endif