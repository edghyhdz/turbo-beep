/**
 * Deals with sending and receiving files, as well as encrypting the file
 */
#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#define MAX_FILE_SIZE 7000000

#include <string>
#include "vendor/base64.h"

namespace turbobeep {
namespace messages {

class FileHandler {
public:
  FileHandler(){};
  bool saveFile(std::string &filePath, std::string &file); 
  bool sendFile(int sock, const char *fileData, int dataSize);
  bool receiveFile(int sock, std::string *receivedFile);
  bool loadFile(std::string filePath, std::string *fileName,
                 std::string *encodedFile);

  std::string base64decode(std::string &toDecode) {
    return base64_decode(toDecode);
  }
};
} // namespace messages
} // namespace turbobeep

#endif
