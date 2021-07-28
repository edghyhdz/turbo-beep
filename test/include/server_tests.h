#ifndef SERVER_TEST
#define SERVER_TEST
#include "server.h"
#include <gtest/gtest.h>

using namespace turbobeep; 

class mediator::TestServer : public mediator::Server {
public:
  // Made them public to test
  void findPeerInfo(payload::packet_PeerInfo &peerInfo, int sock){
    _findPeerInformation(peerInfo, sock);
  }

  void removePeer(const int &socket) { _removePeer(socket); }

  std::map<std::string, userInfo> testUsers() { return _userDescriptor; }
};


class ServerFixture : public testing::Test {
public:
  ServerFixture(){};
  virtual ~ServerFixture(){};
  void SetUp() override;
  void TearDown() override{};
  static void SetUpTestCase(){};
  static void TearDownTestCase(){};

protected:
  mediator::TestServer server;
};

#endif