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
  _peer = std::make_shared<p2p::Peer>(_ipAddress, _port, userName,
                                          theirUserName);
  _messageHandler = std::make_shared<messages::MessageHandler>();                                           
}

P2PMessage::~P2PMessage() {}

void P2PMessage::SetUp() {
  auto mTypePI = payload::packet_MessageTypes_PEER_INFO;
  _messageHandler->addUserInfo(&_size, &_packet, _peer->myInfo(), mTypePI);
}

void P2PMessage::TearDown() {
  delete[] _ipAddress;
  delete[] _port;
}

TEST_F(P2PMessage, AddPeerDataToProtobuffer) {
  auto *crypto = _packet.mutable_payload();
  auto *peerInfo = crypto->mutable_peerinfo();

  ASSERT_EQ(peerInfo->ipaddress(), _peer->ipAddress());
  ASSERT_EQ(peerInfo->port(), _peer->port());
  ASSERT_EQ(peerInfo->username(), _peer->userName());
}

TEST_F(P2PMessage, BufferByteSizeTest) {
  payload::packet tempPacket;

  // Get the bytecount to compare with that returned by serializeMessage()
  auto *payload = tempPacket.mutable_payload();
  auto *crypto = payload->mutable_crypto(); 
  auto *peerInfo = payload->mutable_peerinfo();

  payload->set_type(tempPacket.ADVERTISE);
  peerInfo->set_port(_peer->port());
  peerInfo->set_ipaddress(_peer->ipAddress());
  peerInfo->set_username(_peer->userName());
  peerInfo->set_peername(_peer->peerName());
  const auto timeStamp = std::chrono::system_clock::now();
  long tS = std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();

  tempPacket.set_time_stamp(tS);

  crypto->set_hashedkey(_peer->hashedKey());
  crypto->set_peerhashedkey(_peer->peerHashedKey()); 

  // 4-byte "magic number" that helps use identify size of the packet
  ASSERT_EQ(_size, tempPacket.ByteSize() + 4);
}

TEST_F(P2PMessage, ProtobufferSerializationTestNoThrow) {
  char *pkt = new char[_size];
  array_output_stream aos(pkt, _size);
  output_stream *coded_output =
      new google::protobuf::io::CodedOutputStream(&aos);

  // Should serialize the message correctly
  ASSERT_NO_THROW(
      (void)_messageHandler->serializeMessage(coded_output, _packet));

  // Remove overhead from protobuffer and add the 4-byte "magic number" that
  // helps use identify size of the packet
  ASSERT_EQ(coded_output->ByteCount(), _size - 4 + 1);

  delete[] pkt;
  delete coded_output;
  // google::protobuf::ShutdownProtobufLibrary();
}