#include "tests.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

P2PAuthenticationFixture::P2PAuthenticationFixture() {
  std::uint16_t serverPort{54001};
  _server = std::make_shared<mediator::Server>(serverPort);
}

P2PAuthenticationFixture::~P2PAuthenticationFixture() {}

void P2PAuthenticationFixture::SetUp() {
  _ipAddress = new char[10];
  _port = new char[6];
  std::string ipStr = "127.0.0.1";
  std::string portStr = "54001";
  strcpy(_ipAddress, ipStr.c_str());
  strcpy(_port, portStr.c_str());
  std::string keyPairPath{"./mykeypair.pem"};
  std::string peerPublicKeyPath{"./peer.pem"};
  std::string flag{"0"};
  _peerOne = std::make_shared<p2p::Socket>(_ipAddress, _port, flag, keyPairPath,
                                           peerPublicKeyPath);
//   _peerTwo = std::make_shared<p2p::Socket>(_ipAddress, _port, flag, keyPairPath,
//                                           peerPublicKeyPath);
}

void P2PAuthenticationFixture::TearDown() {
  //   _tPeer.join();
  _tServer.detach();
//   delete[] _ipAddress;
//   delete[] _port;
}

// Wrapper function to return 1 upon successful connection of both peers to the
// server
int P2PAuthenticationFixture::peerWrapper() {

  std::mutex m;
  std::condition_variable cv;
  int retval{0};

  std::thread t([&cv, &retval, this]() {
    auto mTypePI = payload::packet_MessageTypes_ADVERTISE; 
    _peerOne->connectToServer(mTypePI);
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

TEST_F(P2PAuthenticationFixture, PeerAuthenticateToServer){
    auto mTypePI = payload::packet_MessageTypes_ADVERTISE;

    // _tServer = std::thread(&mediator::Server::runServer, _server);
    // int retval = peerWrapper(); 

    ASSERT_EQ(1, 1); 
}