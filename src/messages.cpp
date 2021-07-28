#include "messages.h"

using namespace turbobeep;


/**
 * Serializes message packet after having added the data with
 * peerInfoToPayload() method
 *
 * @param[in, out] coded_output serialized object to send to peer
 * @param[in] packet packet object needed to serialize coded_output
 * @param[in] myInfo struct containing user information (port, ip, name)
 */
void messages::UserInfo::addUserInfo(int *size, payload::packet *packet,
                                     p2p::myInfo const &myInfo) {

  auto *payload = packet->mutable_payload();
  auto *peerInfo = payload->mutable_peerinfo();

  const auto timeStamp = std::chrono::system_clock::now();
  long tS = std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();

  packet->set_time_stamp(tS);
  payload->set_type(packet->PEER_INFO);
  peerInfo->set_port(myInfo.myPort);
  peerInfo->set_ipaddress(myInfo.myIpAddress);
  peerInfo->set_username(myInfo.userName);
  peerInfo->set_peername(myInfo.peerName);

  *size = packet->ByteSize() + 4;
}

/**
 * Serializes message packet after having added the data with
 * peerInfoToPayload() method
 *
 * @param[in, out] coded_output serialized object to send to peer
 * @param[in] packet packet object needed to serialize coded_output
 */
void messages::UserInfo::serializeMessage(output_stream *coded_output,
                                          payload::packet &packet) {

  coded_output->WriteVarint32(packet.ByteSize());
  packet.SerializeToCodedStream(coded_output);
}
