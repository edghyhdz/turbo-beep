#include "socket_creator.h"
#include <iostream>

int main(int argc, char **argv) {
  auto &ipAddress = argv[1];
  auto &portNum = argv[2];
  std::string userName = argv[3]; 
  std::string theirUserName = argv[4]; 

  auto socket = Socket(ipAddress, portNum, userName, theirUserName);

  // Wait for method to return - waiting for other peer
  socket.connectToServer(); 

  // After other peer has connected -> start trying to connect to peer
  socket.connectToPeer(); 

}