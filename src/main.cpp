#include "peer.h"
#include <iostream>

using namespace turbobeep; 

int main(int argc, char **argv) {
  auto &ipAddress = argv[1];
  auto &portNum = argv[2];
  if (argc == 5) {
    std::string userName = argv[3];
    std::string theirUserName = argv[4];
    auto mTypePI = payload::packet_MessageTypes_PEER_INFO;
    auto peer = p2p::Peer(ipAddress, portNum, userName, theirUserName);
    // Wait for method to return - waiting for other peer
    peer.connectToServer(mTypePI);
    // After other peer has connected -> start trying to connect to peer
    peer.connectToPeer();
    peer.listenToPeer();
    
  } else if (argc == 6) {
    std::string flag = argv[3];
    std::string keyPairPath = argv[4];
    std::string peerPublicKey = argv[5]; 
    auto mTypeAdv = payload::packet_MessageTypes_ADVERTISE;
    auto peer = p2p::Peer(ipAddress, portNum, flag, keyPairPath, peerPublicKey);

    // Wait for method to return - waiting for other peer
    peer.connectToServer(mTypeAdv);
    // After other peer has connected -> start trying to connect to peer
    peer.connectToPeer();
    std::thread tListen(&p2p::Peer::listenToPeer, &peer);
    std::thread tMsgHandler(&p2p::Peer::peerMessageHandler, &peer);

    tListen.join();
    tMsgHandler.join();

  } else {
    throw std::runtime_error("Not enough arguments to start ..."); 
  }
}