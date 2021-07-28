#include "tests.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>

// Helper function to add information to struct myInfo
void setMyInfo(p2p::myInfo *myInfo, std::string &&ipAddress,
               std::uint16_t &&port, std::string &&userName,
               std::string &&peerName) {
  myInfo->myIpAddress = ipAddress;
  myInfo->myPort = port;
  myInfo->userName = userName;
  myInfo->peerName = peerName;
}

void ServerFixture::SetUp() {
  int size; 
  payload::packet packetFirstPeer, packetSecondPeer; 
  std::string buffer;
  p2p::myInfo firstPeer, secondPeer; 

  // Add both peer information to struct
  setMyInfo(&firstPeer, "127.0.0.1", 1234, "FirstPeer", "SecondPeer");
  setMyInfo(&secondPeer, "195.0.0.1", 4321, "SecondPeer", "FirstPeer");

  // Add user information to packet
  messages::UserInfo::addUserInfo(&size, &packetFirstPeer, firstPeer);
  messages::UserInfo::addUserInfo(&size, &packetSecondPeer, secondPeer);

  // Get payload from peerinfo
  auto * payloadP1 = packetFirstPeer.mutable_payload();
  auto * peerInfoP1 = payloadP1->mutable_peerinfo(); 
  auto * payloadP2 = packetSecondPeer.mutable_payload();
  auto * peerInfoP2 = payloadP2->mutable_peerinfo();

  // PeerInfo contains all information that normally is sent to the server by
  // the peers
  server.findPeerInfo(*peerInfoP1, 1);
  server.findPeerInfo(*peerInfoP2, 2);
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
