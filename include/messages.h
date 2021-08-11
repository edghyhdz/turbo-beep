#ifndef MESSAGES_H
#define MESSAGES_H

#include "crypto.h"
#include "file_handler.h"
#include "payload.pb.h"
#include "peer.h"
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

class MessageHandler : public crypto::RSA {
public:
  MessageHandler();
  MessageHandler(std::string keyPair, std::string peerPublicKey);

  static long getTimeStamp(); 
  void addUserInfo(int *size, payload::packet *packet,
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
  bool authenticate(int sock); 
  bool verify(int sock, std::string &key);

  bool sendFile(int sock, std::string &filePath);  
  bool receiveFile(int sock); 

  payload::packet setPeerData(int *size, std::string &peerIpAddress,
                              std::uint16_t &peerPort,
                              bool &hasAdvertisedFirst);

private:
  std::unique_ptr<messages::FileHandler> _fileHandler;
  std::string _publicKey;
  std::string _privateKey;
  std::string _peerPublicKey;
};

} // namespace messages
} // namespace turbobeep

#endif