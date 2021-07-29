#ifndef MESSAGES
#define MESSAGES

#include "payload.pb.h"
#include "socket_creator.h"
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

class ProtoBuf {
public:
  ProtoBuf(){}; 
  ProtoBuf(std::string keyPair, std::string peerPublicKey);
  static void addUserInfo(int *size, payload::packet *packet,
                          p2p::myInfo const &myInfo);
  static void serializeMessage(output_stream *coded_output,
                               payload::packet &packet);

  uint32g readHeader(char *buffer);
  void deserializeMessage(payload::packet *packet, char *buffer,
                                 uint32g size);

private:
  std::string _publicKey;
  std::string _privateKey;
  std::string _peerPublicKey;
};

} // namespace messages
} // namespace turbobeep

#endif