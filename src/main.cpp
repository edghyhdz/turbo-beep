#include "peer.h"
#include <iostream>

using namespace turbobeep;

/**
 * Entry point of code. There are three ways in which one can run this program.
 * They all share the first two arguments, `ipAddress` and `portNum`.
 *
 * 1. Connect to a peer just by providing his/her username. The additional
 * arguments are `userName` and `peerUserName`
 *
 * 2. Connect to a peer by providing your public key, and his/her public key,
 * additional arguments are your public key and his/her public key
 *
 * 3. Send/Receive a file. If you are the sender, then the last argument shall
 * be the path to the file to send. Else, the flag "-r" should be given, to let
 * the script know that you are the receiver peer. The arguments are the same as
 * in the mode 2. with the exception of either providing the "-r" flag or the
 * path/to/file
 *
 * @param ipAddress server's ip address
 * @param portNum server's port number
 * @param userName your user name, like rob or edgar
 * @param peersUserName peer's user name, like edgar or rob
 * @param keyPairPath path to your key pair, only the hashed public key will be
 * sent to the server
 * @param peerPublicKey path to the peer's public key
 */
int main(int argc, char **argv) {
  auto &ipAddress = argv[1];
  auto &portNum = argv[2];
  
  // Providing user name and peer user name. This has no encryption at all
  if (argc == 5) {
    std::string userName = argv[3];
    std::string theirUserName = argv[4];
    auto mTypePI = payload::packet_MessageTypes_PEER_INFO;
    auto peer = p2p::Peer(ipAddress, portNum, userName, theirUserName);

    // Wait for method to return - waiting for other peer
    peer.connectToServer(mTypePI);

    // After other peer has connected -> start trying to connect to peer
    if (!peer.connectToPeer())
      throw std::runtime_error("Could not connect");

    std::thread tListen(&p2p::Peer::listenToPeer, &peer);
    std::thread tMsgHandler(&p2p::Peer::peerMessageHandler, &peer);

    tListen.join();
    tMsgHandler.join();
    return 0; 

  } else if (argc == 6) {
    // For this way, there is encryption between messages
    std::string flag = argv[3];
    std::string keyPairPath = argv[4];
    std::string peerPublicKey = argv[5];
    auto mTypeAdv = payload::packet_MessageTypes_ADVERTISE;
    auto peer = p2p::Peer(ipAddress, portNum, flag, keyPairPath, peerPublicKey);

    // Wait for method to return - waiting for other peer
    peer.connectToServer(mTypeAdv);
    // After other peer has connected -> start trying to connect to peer
    if (!peer.connectToPeer())
      throw std::runtime_error("Could not connect");

    std::thread tListen(&p2p::Peer::listenToPeer, &peer);
    std::thread tMsgHandler(&p2p::Peer::peerMessageHandler, &peer);

    tListen.join();
    tMsgHandler.join();
    return 0; 
  }
  // Sending/Receiving a file. Not encrypted file.
  else if (argc == 7) {
    std::string flag = argv[3];
    std::string keyPairPath = argv[4];
    std::string peerPublicKey = argv[5];
    std::string filePath = argv[6];

    auto mTypeAdv = payload::packet_MessageTypes_ADVERTISE;
    auto peer = p2p::Peer(ipAddress, portNum, flag, keyPairPath, peerPublicKey);

    // Wait for method to return - waiting for other peer
    peer.connectToServer(mTypeAdv);
    // After other peer has connected -> start trying to connect to peer
    if (!peer.connectToPeer())
      throw std::runtime_error("Could not connect");
    
    bool success; 
    if (filePath == "-r") {
      success = peer.receiveFile();
    } else {
      success = peer.sendFile(filePath);
      std::cout << "File sent" << std::endl;
    }

    return success ? 0 : 1;

  } else {
    throw std::runtime_error("Not enough arguments to start ...");
  }
}