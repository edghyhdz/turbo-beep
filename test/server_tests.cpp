#include "tests.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>

void ServerFixture::SetUp() {
  std::string buffer;
  int sock{1};

  buffer = "127.0.0.1::1234::FirstPeer::SecondPeer";
  server.findPeerInfo(buffer, sock);

  buffer = "195.0.0.1::4321::SecondPeer::FirstPeer";
  server.findPeerInfo(buffer, ++sock);
}

TEST_F(ServerFixture, findPeerInfo_FirstPeer) {
  auto users = server.testUsers();

  ASSERT_EQ(users.at("FirstPeer").name, "FirstPeer");
  ASSERT_EQ(users.at("FirstPeer").ipAddress, "127.0.0.1");
  ASSERT_EQ(users.at("FirstPeer").port, 1234);
  ASSERT_EQ(users.at("FirstPeer").socket, 1);
  ASSERT_EQ(users.at("FirstPeer").peerInfo.name, "SecondPeer");
}

TEST_F(ServerFixture, findPeerInfo_FirstPeer_findPeer_isClient_True) {
  auto users = server.testUsers();
  ASSERT_EQ(users.at("FirstPeer").isClient, true);
}

TEST_F(ServerFixture, findPeerInfo_SecondPeer_findPeer_isClient_False) {
  auto users = server.testUsers();
  ASSERT_EQ(users.at("SecondPeer").isClient, false);
}

TEST_F(ServerFixture, findPeerInfo_SecondPeer) {
  auto users = server.testUsers();

  ASSERT_EQ(users.at("SecondPeer").name, "SecondPeer");
  ASSERT_EQ(users.at("SecondPeer").ipAddress, "195.0.0.1");
  ASSERT_EQ(users.at("SecondPeer").port, 4321);
  ASSERT_EQ(users.at("SecondPeer").socket, 2);
  ASSERT_EQ(users.at("SecondPeer").peerInfo.name, "FirstPeer");
  ASSERT_EQ(users.at("SecondPeer").isClient, false);
}

TEST_F(ServerFixture, ComparePeerInformation) {
  auto users = server.testUsers();

  ASSERT_EQ(users.at("FirstPeer").peerInfo.name, users.at("SecondPeer").name);
  ASSERT_EQ(users.at("FirstPeer").peerInfo.port, users.at("SecondPeer").port);
  ASSERT_EQ(users.at("FirstPeer").peerInfo.ipAddress,
            users.at("SecondPeer").ipAddress);
}

TEST_F(ServerFixture, PeersReadyToConnect) {
  auto users = server.testUsers();
  ASSERT_EQ(users.at("FirstPeer").canConnect,
            users.at("SecondPeer").canConnect);
}

TEST_F(ServerFixture, RemoveFirstPeerFromPeerInfoMap) {
  // Remove firstPeer
  server.removePeer(1);
  auto users = server.testUsers();

  ASSERT_THROW(users.at("FirstPeer"), std::exception);
  // Second peer should still be here
  ASSERT_NO_THROW(users.at("SecondPeer"));
}
