#include "tests.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

P2PMessage::P2PMessage() {
  _ipAddress = new char[10];
  _port = new char[6];
  std::string ipStr = "127.0.0.1";
  std::string portStr = "54002";
  strcpy(_ipAddress, ipStr.c_str());
  strcpy(_port, portStr.c_str());
  std::string userName{"peerOne"};
  std::string theirUserName{"peerTwo"};

  _socket = new p2p::Socket(_ipAddress, _port, userName, theirUserName);
}

P2PMessage::~P2PMessage() {}

void P2PMessage::SetUp() {
  messages::ProtoBuf::addUserInfo(&_size, &_packet, _socket->myInfo());
}

void P2PMessage::TearDown() {
  delete[] _ipAddress;
  delete[] _port;
}

TEST_F(P2PMessage, AddPeerDataToProtobuffer) {
  auto *crypto = _packet.mutable_payload();
  auto *peerInfo = crypto->mutable_peerinfo();

  ASSERT_EQ(peerInfo->ipaddress(), _socket->ipAddress());
  ASSERT_EQ(peerInfo->port(), _socket->port());
  ASSERT_EQ(peerInfo->username(), _socket->userName());
}

TEST_F(P2PMessage, BufferByteSizeTest) {
  payload::packet tempPacket;

  // Get the bytecount to compare with that returned by serializeMessage()
  auto *payload = tempPacket.mutable_payload();
  auto *crypto = payload->mutable_crypto(); 
  auto *peerInfo = payload->mutable_peerinfo();

  payload->set_type(tempPacket.ADVERTISE);
  peerInfo->set_port(_socket->port());
  peerInfo->set_ipaddress(_socket->ipAddress());
  peerInfo->set_username(_socket->userName());
  peerInfo->set_peername(_socket->peerName());
  const auto timeStamp = std::chrono::system_clock::now();
  long tS = std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();

  tempPacket.set_time_stamp(tS);

  crypto->set_hashedkey(_socket->hashedKey());
  crypto->set_peerhashedkey(_socket->peerHashedKey()); 

  // 4-byte "magic number" that helps use identify size of the packet
  ASSERT_EQ(_size, tempPacket.ByteSize() + 4);
}

TEST_F(P2PMessage, ProtobufferSerializationTestNoThrow) {
  char *pkt = new char[_size];
  array_output_stream aos(pkt, _size);
  output_stream *coded_output =
      new google::protobuf::io::CodedOutputStream(&aos);

  // Should serialize the message correctly
  ASSERT_NO_THROW((void)messages::ProtoBuf::serializeMessage(coded_output, _packet));

  // Remove overhead from protobuffer and add the 4-byte "magic number" that helps
  // use identify size of the packet
  ASSERT_EQ(coded_output->ByteCount(), _size - 4 + 1);

  std::cout << "Send message and free up them memory" << std::endl;
  delete[] pkt;
  delete coded_output;
  google::protobuf::ShutdownProtobufLibrary();
}