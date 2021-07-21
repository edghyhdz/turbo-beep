#ifndef CLIENT
#define CLIENT
#include "socket_creator.h"

class Client {
public:
  Client(const char *&serverIp, const char *&port);
  void connectToServer();
  void connectToPeer();

private:
  Socket _clientSocket;

};

#endif