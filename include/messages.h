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

namespace turbobeep {
namespace messages {

class PeerInfo {
public:
  static void addPeerInfo(int *size, payload::packet *packet,
                          p2p::myInfo const &myInfo);
  static void serializeMessage(output_stream *coded_output,
                               payload::packet &packet);
};

} // namespace messages
} // namespace turbobeep

#endif