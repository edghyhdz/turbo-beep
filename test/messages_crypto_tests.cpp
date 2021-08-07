#include "tests.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

CryptoFixture::CryptoFixture() : _certPath(CERTIFICATES_PATH) {}

CryptoFixture::~CryptoFixture() {}

void CryptoFixture::SetUp() {
  std::string keyPairPath = _certPath + "peer1/mykeypair.pem";
  std::string peerPublicKeyPath = _certPath + "peer1/peer.pem";
  _crypto = std::make_shared<crypto::RSA>(keyPairPath, peerPublicKeyPath);
}
void CryptoFixture::TearDown() {}

TEST_F(CryptoFixture, TestSigningAndDecryptingNonce) {
  auto nonce = _crypto->generateNonce();
  std::string encrypted_nonce = _crypto->signString(nonce);
  std::string decrypted_nonce = _crypto->decryptWithPublicKey(encrypted_nonce, _crypto->publicKey()); 

  ASSERT_EQ(nonce, decrypted_nonce);
}

TEST_F(CryptoFixture, TestHashingSHA1){
  std::string pubKey = _crypto->publicKey();
  std::string hashed = _crypto->sha1(pubKey, pubKey.length());
  
  ASSERT_EQ("78ed774f7871ab4e631031374fec8211e0cfb006", hashed); 
}