#ifndef P2P_MESSAGE_TEST
#define P2P_MESSAGE_TEST
#include "server_tests.h"
#include "socket_creator.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class P2PMessage : public testing::Test {
public:
  P2PMessage();
  ~P2PMessage();
  void SetUp() override;
  void TearDown() override;
  static void SetUpTestCase(){};
  static void TearDownTestCase(){};

protected:
  Socket *_socket;
  int _size; 
  payload::packet _packet;
  char *_ipAddress;
  char *_port;
};

#endif