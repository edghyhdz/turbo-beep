#ifndef MESSAGES_H
#define MESSAGES_H

#include "payload.pb.h"
#include "socket_creator.h"
#include "crypto.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <memory>

typedef google::protobuf::io::CodedOutputStream output_stream;
typedef google::protobuf::io::ArrayOutputStream array_output_stream;
typedef google::protobuf::uint32 uint32g;
typedef google::protobuf::io::CodedInputStream input_stream;
typedef google::protobuf::io::ArrayInputStream array_input_stream;

namespace turbobeep {
namespace messages {

class ProtoBuf : public crypto::RSA {
public:
  ProtoBuf(){};
  ProtoBuf(std::string keyPair, std::string peerPublicKey)
      : RSA(keyPair, peerPublicKey){};
    
  static long getTimeStamp(); 
  static void addUserInfo(int *size, payload::packet *packet,
                          p2p::myInfo const &myInfo,
                          payload::packet::MessageTypes &mType);
  void serializeMessage(output_stream *coded_output, payload::packet &packet);

  uint32g readHeader(char *buffer);
  bool deserializeMessage(payload::packet *packet, char *buffer,
                                 uint32g size);
  
  bool readBody(int sock, uint32g size, payload::packet *packet);
  void sendMessage(int size, int sock, payload::packet &packet);
  bool receiveMessage(int sock, payload::packet *packet); 
  bool receiveMessage(int sock, std::string *recvMsg);

  payload::packet setPeerData(int *size, std::string &peerIpAddress,
                               std::uint16_t &peerPort);

private:
  std::string _publicKey;
  std::string _privateKey;
  std::string _peerPublicKey;
};

} // namespace messages
} // namespace turbobeep

#endif