#include "tests.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

CryptoFixture::CryptoFixture() {}

CryptoFixture::~CryptoFixture() {}

void CryptoFixture::SetUp() {
  std::string keyPairPath{"./mykeypair.pem"};
  std::string peerPublicKeyPath{"./peer.pem"};
  _crypto = std::make_shared<crypto::RSA>(keyPairPath, peerPublicKeyPath);
}
void CryptoFixture::TearDown() {}

TEST_F(CryptoFixture, TestSigningAndDecryptingNonce) {
  auto nonce = _crypto->generateNonce();
  std::cout << "Nonce: " << nonce << std::endl;
  std::string encrypted_nonce = _crypto->signString(nonce);
  std::string decrypted_nonce = _crypto->decryptWithPublicKey(encrypted_nonce, _crypto->publicKey()); 

  ASSERT_EQ(nonce, decrypted_nonce);
}
