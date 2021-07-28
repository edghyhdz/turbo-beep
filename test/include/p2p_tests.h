#ifndef P2P_TEST
#define P2P_TEST
#include "server_tests.h"
#include "socket_creator.h"
#include <gtest/gtest.h>

using namespace turbobeep; 

class P2PFixture : public testing::Test {
public:
  P2PFixture();
  ~P2PFixture();
  void SetUp() override;
  void TearDown() override;
  static void SetUpTestCase(){};
  static void TearDownTestCase(){};
  int peerWrapper();

protected:
  std::shared_ptr<p2p::Socket> _peerOne; 
  std::shared_ptr<p2p::Socket> _peerTwo; 
  std::shared_ptr<mediator::Server> _server;

  char *_ipAddress;
  char *_port;

  std::thread _tPeer;
  std::thread _tServer;
};

#endif