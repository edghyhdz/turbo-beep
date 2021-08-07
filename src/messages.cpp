#include "messages.h"
#include <sys/socket.h>
#include <iostream>
#include <sstream>

using namespace turbobeep;


// Generates current timestamp
long messages::ProtoBuf::getTimeStamp(){
  const auto timeStamp = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();
}

/**
 * Serializes message packet after having added the data with
 * peerInfoToPayload() method
 *
 * @param[in, out] coded_output serialized object to send to peer
 * @param[in] packet packet object needed to serialize coded_output
 * @param[in] myInfo struct containing user information (port, ip, name)
 */
void messages::ProtoBuf::addUserInfo(int *size, payload::packet *packet,
                                     p2p::myInfo const &myInfo,
                                     payload::packet::MessageTypes &mType) {

  auto *payload = packet->mutable_payload();
  auto *crypto = payload->mutable_crypto(); 
  auto *peerInfo = payload->mutable_peerinfo();

  const auto timeStamp = std::chrono::system_clock::now();
  long tS = std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();

  packet->set_time_stamp(tS);
  payload->set_type(mType);
  peerInfo->set_port(myInfo.myPort);
  peerInfo->set_ipaddress(myInfo.myIpAddress);
  peerInfo->set_username(myInfo.userName);
  peerInfo->set_peername(myInfo.peerName);

  crypto->set_hashedkey(myInfo.myHash);
  crypto->set_peerhashedkey(myInfo.peerHash); 
  
  // 4-byte "magic number" to help use identify size of the packet
  *size = packet->ByteSize() + 4;
}

/**
 * Serializes message packet after having added the data with
 * peerInfoToPayload() method
 *
 * @param[in, out] coded_output serialized object to send to peer
 * @param[in] packet packet object needed to serialize coded_output
 */
void messages::ProtoBuf::serializeMessage(output_stream *coded_output,
                                          payload::packet &packet) {

  coded_output->WriteVarint32(packet.ByteSize());
  packet.SerializeToCodedStream(coded_output);
}

/**
 * Reads headers from packet
 *
 * @param buffer packet buffer, to be later deserialized by
 * ProtoBuf::deserializeMessage
 * @returns the size of the message
 */
uint32g messages::ProtoBuf::readHeader(char *buffer) {
  uint32g size;
  array_input_stream ais(buffer, 4);
  input_stream coded_input(&ais);
  coded_input.ReadVarint32(&size); // Decode the HDR and get the size
  return size;
}

/**
 * Deserializes protobuf packet
 * 
 * @param packet protobuf packet to pass buffer data
 * @param buffer buffer containing data
 * @param size message size
 */
bool messages::ProtoBuf::deserializeMessage(payload::packet *packet, char *buffer, uint32g size){
  // Assign ArrayInputStream with enough memory
  array_input_stream ais(buffer, size + 4);
  input_stream coded_input(&ais);
  // Read an unsigned integer with Varint encoding, truncating to 32 bits.
  coded_input.ReadVarint32(&size);
  // After the message's length is read, PushLimit() is used to prevent the
  // CodedInputStream from reading beyond that length.Limits are used when
  // parsing length-delimited embedded messages
  input_stream::Limit msgLimit = coded_input.PushLimit(size);

  // De-Serialize
  if (!packet->ParseFromCodedStream(&coded_input)) {
    return false;
  }
  // Once the embedded message has been parsed, PopLimit() is called to undo the
  // limit
  coded_input.PopLimit(msgLimit);
  return true;
}

/**
 * Read protobuf message body.
 * Deserializes sent package from client
 * 
 * @param sock peer socket
 * @param size size of the message as given by messages::Receive::readHeader()
 * @param[in, out] packet protobuf packet, containing payload to be deserialized
 */
bool messages::ProtoBuf::readBody(int sock, uint32g size, payload::packet *packet){
  int bytecount;
  char buffer[size + 4];
  bytecount = recv(sock, (void *)buffer, 4 + size, 0);
  return this->deserializeMessage(packet, buffer, size);
}

/**
 * Sends protobuf message
 *
 * @param size buffer message size plus 4 bytes (magic number to help get all
 * message size)
 * @param sock user socket to send message to
 * @param packet protobuf payload packet
 */
void messages::ProtoBuf::sendMessage(int size, int sock, payload::packet &packet){

  char *pkt = new char[size];
  array_output_stream aos(pkt, size);
  output_stream *coded_output = new output_stream(&aos);

  // Serialize the message
  this->serializeMessage(coded_output, packet);

  // Send serialized packet
  send(sock, (void *)pkt, coded_output->ByteCount(), 0);

  delete[] pkt;
  delete coded_output;
}

/**
 * Receives protobuf message.
 * 
 * @param sock user socket from which we receive message
 * @param[in, out] packet protobuf payload packet to write incoming message to
 */
bool messages::ProtoBuf::receiveMessage(int sock, payload::packet *packet){
  char buffer[4];
  int bytesIn; 
  memset(buffer, '\0', 4);

  // Peek into the socket and get the packet size
  if ((bytesIn = recv(sock, buffer, 4, MSG_PEEK)) <= 0) {
    return false;
  }
  return this->readBody(sock, this->readHeader(buffer), packet);
}

/**
 * Receives a non serialized message
 */
bool messages::ProtoBuf::receiveMessage(int socket, std::string *recvMsg) {
  char buffer[128];
  memset(buffer, 0, 128);
  int bytesReceived = recv(socket, buffer, 128, 0);

  if (bytesReceived == 0) {
    return false;
  }
  std::ostringstream ss;
  ss << buffer;
  *recvMsg = ss.str();
  return true; 
}

/**
 * Add data from other peer. 
 * 
 * @param[in, out] size byte size of message
 * @param peerIpAddress other peer ip address
 * @param peerPort other peer port
 * @param hasAdvertisedFirst who initiated the connection - useful for authentication
 */
payload::packet messages::ProtoBuf::setPeerData(int *size,
                                                std::string &peerIpAddress,
                                                std::uint16_t &peerPort,
                                                bool &hasAdvertisedFirst) {
  payload::packet packet;
  packet.set_time_stamp(this->getTimeStamp());
  auto *payload = packet.mutable_payload();
  payload->set_type(payload::packet_MessageTypes_SUCCESS);

  auto *otherPeerInfo = payload->mutable_otherpeerinfo();
  otherPeerInfo->set_peeripaddress(peerIpAddress);
  otherPeerInfo->set_peerport(peerPort);
  otherPeerInfo->set_hasadvertisedfirst(hasAdvertisedFirst); 
  // Get packet size - including magic number
  *size = packet.ByteSize() + 4;

  return packet; 
}