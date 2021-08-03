#include "socket_creator.h"
#include <iostream>

using namespace turbobeep; 

int main(int argc, char **argv) {
  auto &ipAddress = argv[1];
  auto &portNum = argv[2];
  if (argc == 5) {
    std::string userName = argv[3];
    std::string theirUserName = argv[4];
    auto mTypePI = payload::packet_MessageTypes_PEER_INFO;
    auto socket = p2p::Socket(ipAddress, portNum, userName, theirUserName);
    // Wait for method to return - waiting for other peer
    socket.connectToServer(mTypePI);

    // After other peer has connected -> start trying to connect to peer
    socket.connectToPeer();
    
  } else if (argc == 6) {
    std::string flag = argv[3];
    std::string keyPairPath = argv[4];
    std::string peerPublicKey = argv[5]; 
    auto mTypeAdv = payload::packet_MessageTypes_ADVERTISE;
    auto socket = p2p::Socket(ipAddress, portNum, flag, keyPairPath, peerPublicKey);

    // Wait for method to return - waiting for other peer
    socket.connectToServer(mTypeAdv);

  } else {
    throw std::runtime_error("Not enough arguments to start ..."); 
  }
}