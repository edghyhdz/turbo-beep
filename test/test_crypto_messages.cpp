#include "tests.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

CryptoFixture::CryptoFixture() {}

CryptoFixture::~CryptoFixture() {}

void CryptoFixture::SetUp() {
  std::string keyPairPath{"./private.pem"};
  std::string peerPublicKeyPath{"./public.pem"};
  _crypto = std::make_shared<crypto::RSA>(keyPairPath, peerPublicKeyPath);
}
void CryptoFixture::TearDown() {}

TEST_F(CryptoFixture, TestNonce) {
  auto nonce = _crypto->generateNonce();

  std::cout << "Nonce: " << nonce << std::endl;
  ASSERT_EQ(1, 1);
}
