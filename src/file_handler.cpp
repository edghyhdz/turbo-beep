#include "file_handler.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <strings.h>
#include <sys/socket.h>

using namespace turbobeep;

/**
 * Load file up to MAX_FILE_SIZE bytes
 *
 * @param filePath path to file
 * @param[in, out] fileName interpreted by the input filePath
 * @param[in, out] loadedFile loaded file
 * @param[in, out] readChars size of loaded file
 */
bool messages::FileHandler::loadFile(std::string filePath,
                                      std::string *fileName,
                                      std::string *encodedFile) {

  char buffer[MAX_FILE_SIZE];
  std::ifstream infile(filePath);
  size_t readChars;

  if (!(infile.read(buffer, sizeof(buffer)))) {
    if (!infile.eof()) {
      return false;
    }
  }
  *fileName = filePath.substr(filePath.find_last_of("/") + 1);
  readChars = infile.gcount();
  const unsigned char *lodadedFile =
      reinterpret_cast<const unsigned char *>(buffer);

  *encodedFile= base64_encode(lodadedFile, readChars);
  bool needs_padding = (*encodedFile).length() % 4 != 0;

  if (needs_padding) {
    for (int i = 0; i < (*encodedFile).length() % 4; i++) {
      *encodedFile += "=";
    }
  }

  return true;
}

bool messages::FileHandler::saveFile(std::string &filePath, std::string &file) {
  try {
    std::ofstream outputFile;
    outputFile.open(filePath, std::ios::out);
    outputFile.write(file.c_str(), file.length());
    outputFile.close();
  } catch (...) {
    return false;
  }
  return true;
}

/**
 * Send the file
 * @param sock socket to send data to
 * @param[in] fileData data to consume and send
 * @param data size of data
 */
bool messages::FileHandler::sendFile(int sock, const char *fileData, int dataSize) {
  int bytes_sent;

  while (dataSize > 0) {
    bytes_sent = send(sock, fileData, dataSize, 0);
    if (bytes_sent == -1){
      return false;
    }
    fileData += bytes_sent;
    dataSize -= bytes_sent;
  }
  return true; 
}

/**
 * Receive file
 * @param sock
 * @param[in, out] file received file
 */
bool messages::FileHandler::receiveFile(int sock, std::string *file) {
  int n, counter{0}, received{0};
  while (true) {
    // buffer for storing incoming data
    char buf[5000];
    bzero(buf, 5000);
    n = recv(sock, buf, 5000, 0);
    received += n;
    std::cout << "\rReceived: [" << received << " kBs]" << std::flush;
    *file += buf;

    if (n == 0) {
      counter++;
      if (counter > 5)
        break;
    }
  }
  std::cout << std::endl;
  return true;
}
