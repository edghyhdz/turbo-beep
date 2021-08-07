#ifndef P2P_AUTHENTICATION_TEST
#define P2P_AUTHENTICATION_TEST
#include "server_tests.h"
#include "peer.h"
#include <gtest/gtest.h>

using namespace turbobeep; 

class P2PAuthenticationFixture : public testing::Test {
public:
  P2PAuthenticationFixture();
  ~P2PAuthenticationFixture();
  void SetUp() override;
  void TearDown() override;
  static void SetUpTestCase(){};
  static void TearDownTestCase(){};
  int peerWrapper();
  std::string const getPath(std::string &&pathName); 

private:
  std::string _certPath;

protected:
  std::shared_ptr<p2p::Peer> _peerOne; 
  std::shared_ptr<p2p::Peer> _peerTwo; 
  std::shared_ptr<mediator::Server> _server;

  char *_ipAddress;
  char *_port;

  std::thread _tPeer;
  std::thread _tServer;
};

#endif