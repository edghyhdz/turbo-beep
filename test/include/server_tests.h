#ifndef SERVER_TEST
#define SERVER_TEST
#include "server.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class TestServer : public Server {
public:
  // Made them public to test
  void findPeerInfo(std::string &buffer, int sock) {
    _findPeerInformation(buffer, sock);
  };
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
  TestServer server;
};

#endif