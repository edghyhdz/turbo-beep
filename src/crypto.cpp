#include "crypto.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <openssl/pem.h>
#include <unistd.h>

using namespace turbobeep;

crypto::RSA::RSA(std::string keyPairPath, std::string peerPublicKeyPath) {
  // Load both keys into private member variables
  _loadKeys(keyPairPath, peerPublicKeyPath);
}

/**
 * Reference : https://stackoverflow.com/a/440240
 * Generate nonce that will be later signed by party that wants to authenticate
 */
std::string const crypto::RSA::generateNonce() {
  std::string tmp_s;
  const auto timeStamp = std::chrono::system_clock::now();

  long tS = std::chrono::duration_cast<std::chrono::milliseconds>(
                timeStamp.time_since_epoch())
                .count();

  srand((unsigned)time(&tS) * getpid());
  tmp_s.reserve(LEN_NONCE);

  for (int i = 0; i < LEN_NONCE; ++i)
    tmp_s += ALPHANUM[rand() % (sizeof(ALPHANUM) - 1)];

  return tmp_s;
}

/**
 * Lad private and public keys from pem file. It is assumed that the private key
 * comes before than the public key.
 *
 * @param keyPath path to own key pair
 * @param peerPath path to peer's public key
 */
void crypto::RSA::_loadKeys(std::string &keyPath, std::string &peerPath) {
  std::ifstream sKeyRaw(keyPath);   // My own key pair file
  std::ifstream pKeyRaw(peerPath);  // Peer public key file
  std::string peerKey, key;
  std::string delimiter{"-----END RSA PRIVATE KEY-----"};
  bool finishedSecretKey{false};

  if (sKeyRaw) {
    std::string line;
    while (getline(sKeyRaw, line)) {
      key += line + "\n";
      if (line.find(delimiter) != std::string::npos) {
        // Remove trailing space
        key = key.substr(0, key.size() - 1);
        this->_secretKey = key;
        finishedSecretKey = true;
      }

      if (finishedSecretKey) {
        key = "";
        finishedSecretKey = false;
      }
    }
    key = key.substr(0, key.size() - 1);
    this->_publicKey = key;
  }

  // Load peer's public key
  if (pKeyRaw) {
    std::string line;
    while (getline(pKeyRaw, line)) {
      peerKey += line + "\n";
    }
    // Remove trailing space
    peerKey = peerKey.substr(0, peerKey.size() - 1);
    this->_peerPublicKey = peerKey;
  }
}