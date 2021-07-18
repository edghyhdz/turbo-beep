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

#define PORT_SERVER 54000

#include <map>
#include <set>

class Server {
public:
  Server();
  ~Server();
  int initServer();
  void runServer();

private:
  int _listening;
  fd_set _master;
};

#endif