#include "messages.h"

using namespace turbobeep;

// messages::ProtoBuf::ProtoBuf(std::string keyPair, std::string peerPublicKey) {}

/**
 * Serializes message packet after having added the data with
 * peerInfoToPayload() method
 *
 * @param[in, out] coded_output serialized object to send to peer
 * @param[in] packet packet object needed to serialize coded_output
 * @param[in] myInfo struct containing user information (port, ip, name)
 */
void messages::ProtoBuf::addUserInfo(int *size, payload::packet *packet,
                                     p2p::myInfo const &myInfo) {

  auto *payload = packet->mutable_payload();
  auto *crypto = payload->mutable_crypto(); 
  auto *peerInfo = payload->mutable_peerinfo();

  const auto timeStamp = std::chrono::system_clock::now();
  long tS = std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();

  packet->set_time_stamp(tS);
  payload->set_type(packet->ADVERTISE);
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
  // std::cout << "size of payload is " << size << std::endl;
  return size;
}

/**
 * Deserializes protobuf packet
 * 
 * @param packet protobuf packet to pass buffer data
 * @param buffer buffer containing data
 * @param size message size
 */
void messages::ProtoBuf::deserializeMessage(payload::packet *packet, char *buffer, uint32g size){
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
  packet->ParseFromCodedStream(&coded_input);
  // Once the embedded message has been parsed, PopLimit() is called to undo the
  // limit
  coded_input.PopLimit(msgLimit);
}
