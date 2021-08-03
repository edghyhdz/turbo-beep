#include "tests.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

P2PFixture::P2PFixture() {
  std::uint16_t serverPort{54002};
  _server = std::make_shared<mediator::Server>(serverPort);
}

P2PFixture::~P2PFixture() {}

void P2PFixture::SetUp() {
  _ipAddress = new char[10];
  _port = new char[6];
  std::string ipStr = "127.0.0.1";
  std::string portStr = "54002";
  strcpy(_ipAddress, ipStr.c_str());
  strcpy(_port, portStr.c_str());
  std::string userName{"peerOne"};
  std::string theirUserName{"peerTwo"};
  _peerOne = std::make_shared<p2p::Socket>(_ipAddress, _port, userName, theirUserName); 
  _peerTwo = std::make_shared<p2p::Socket>(_ipAddress, _port, theirUserName, userName); 
}

void P2PFixture::TearDown() {
  _tPeer.join();
  _tServer.detach();
  delete[] _ipAddress;
  delete[] _port;
}

// Wrapper function to return 1 upon successful connection of both peers to the
// server
int P2PFixture::peerWrapper() {

  std::mutex m;
  std::condition_variable cv;
  int retval{0};

  std::thread t([&cv, &retval, this]() {
    auto mTypePI = payload::packet_MessageTypes_PEER_INFO; 
    _peerTwo->connectToServer(mTypePI);
    retval = 1;
    cv.notify_one();
  });
  t.detach();
  {
    std::unique_lock<std::mutex> lck(m);
    if (cv.wait_for(lck, std::chrono::seconds(2)) == std::cv_status::timeout) {
      if (!retval) {
        return retval;
      }
    }
  }
  return retval;
}

TEST_F(P2PFixture, SuccessfulDiscconectFromServerAferBothPeersOnline) {
    auto mTypePI = payload::packet_MessageTypes_PEER_INFO; 

  _tServer = std::thread(&mediator::Server::runServer, _server);
  _tPeer = std::thread(&p2p::Socket::connectToServer, _peerOne,  std::ref(mTypePI));

  // Wait for wrapper to return
  int retval = this->peerWrapper();

  // If both clients connected to the server successfully
  // They will both return, the wrapper with a value of one
  ASSERT_EQ(retval, 1);
  _server.reset();
}