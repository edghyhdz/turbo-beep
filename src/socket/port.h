
#ifndef PORT_FETCH
#define PORT_FETCH

#define TCP_FILE "/proc/net/tcp"
#define MIN_PORT 50000
#define MAX_PORT 60000

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace socket_utils {

/**
 * Helper function to socket_utils::getAvailablePort(uint16_t port).
 *
 * @param[in] line `TCP_FILE` line, port info at the begining of the line
 * @param[in, out] counter used to ignore headers from `TCP_FILE`
 */
std::uint16_t parseLine(std::string const &line, int *counter) {
  std::istringstream sline(line);
  std::uint16_t port{0};

  // Ignore file headers
  if (*counter > 0) {
    char c;
    sline >> port >> c;
  }
  (*counter)++;
  return port;
}

/**

 * Checks on /proc/net/tcp for already in use ports and chooses randomly from a
 * range of 50000 to 60000, which is not included on the net/tcp list
 * @note Only linux implementation.
 *
 * @param[in, out] port the port to which the client will be bound to
 * @ref socket_utils::getIpAddress(long *, string *)
 */
inline void getAvailablePort(std::uint16_t *port) {
  std::ifstream myFile(TCP_FILE);
  std::vector<std::uint16_t> ports;
  int counter{0};
  if (myFile) {
    std::string line;
    while (getline(myFile, line)) {
      ports.push_back(parseLine(line, &counter));
    }
  }

  // Choose a random port between MIN_PORT and MAX_PORT
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(MIN_PORT, MAX_PORT);

  *port = distr(eng);

  while (std::find(ports.begin(), ports.end(), *port) != ports.end()) {
    *port = distr(eng);
  }
}
} // namespace socket_utils
#endif