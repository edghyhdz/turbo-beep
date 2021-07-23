#ifndef CLIENT
#define CLIENT
#include "socket_creator.h"

class Client {
public:
  Client(char *&serverIp, char *&port);
  void connectToServer();
  void connectToPeer();

private:
  Socket _clientSocket;

};

#endif