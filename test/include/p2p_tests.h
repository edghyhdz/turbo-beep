#ifndef P2P_TEST
#define P2P_TEST
#include "server_tests.h"
#include "socket_creator.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

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
  Socket *_socket;
  Socket *_peerTwo;
  std::shared_ptr<Server> _server;

  char *_ipAddress;
  char *_port;

  std::thread _tPeer;
  std::thread _tServer;
};

#endif