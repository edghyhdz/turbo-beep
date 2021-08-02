#include "crypto.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <openssl/pem.h>
#include <string.h>
#include <unistd.h>

using namespace turbobeep;

crypto::RSA::RSA(std::string keyPairPath, std::string peerPublicKeyPath) {
  // Load both keys into private member variables
  _loadKeys(keyPairPath, peerPublicKeyPath);
}

/**
 * Decrypt with public key
 * Reference: https://www.programmersought.com/article/37955188510/
 * 
 * @param message encrypted message
 * @param pubKey public key to use to decrypt message
 */
std::string crypto::RSA::decryptWithPublicKey(const std::string &message,
                                              const std::string &pubKey) {
  std::string decrypt_text;
  BIO *keybio = BIO_new_mem_buf((unsigned char *)pubKey.c_str(), -1);
  ::RSA *rsa = RSA_new();

  rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);

  if (!rsa) {
    BIO_free_all(keybio);
    return "";
  }

  // Get the maximum length of RSA single processing
  int len = RSA_size(rsa);
  char *sub_text = new char[len + 1];
  memset(sub_text, 0, len + 1);
  int ret = 0;
  std::string sub_str;
  int pos = 0;

  int counter = 0;
  // Decrypt the ciphertext in segments
  while (pos < message.length()) {
    sub_str = message.substr(pos, len);
    memset(sub_text, 0, len + 1);
    ret = RSA_public_decrypt(sub_str.length(),
                             (const unsigned char *)sub_str.c_str(),
                             (unsigned char *)sub_text, rsa, RSA_PKCS1_PADDING);
    if (ret >= 0) {
      decrypt_text.append(std::string(sub_text, ret));
      pos += len;
    }
    counter++;
    if (counter > 5000) {
      break;
    }
  }

  // release memory
  delete sub_text;
  BIO_free_all(keybio);
  RSA_free(rsa);

  return decrypt_text;
}

std::string const crypto::RSA::signString(const std::string &message) {
  	std::string encrypt_text;
	BIO *keybio = BIO_new_mem_buf((unsigned char *)_secretKey.c_str(), -1);
	::RSA* rsa = RSA_new();
	rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	if (!rsa)
	{
		BIO_free_all(keybio);
		return "";
	}
 
	 // Get the maximum length of the data block that RSA can process at a time
	int key_len = RSA_size(rsa);
	 int block_len = key_len-11; // Because the filling method is RSA_PKCS1_PADDING, so you need to subtract 11 from the key_len
 
	 // Apply for memory: store encrypted ciphertext data
	char *sub_text = new char[key_len + 1];
	memset(sub_text, 0, key_len + 1);
	int ret = 0;
	int pos = 0;
	std::string sub_str;
	 // Encrypt the data in segments (the return value is the length of the encrypted data)
	while (pos < message.length()) {
		sub_str = message.substr(pos, block_len);
		memset(sub_text, 0, key_len + 1);
		ret = RSA_private_encrypt(sub_str.length(), (const unsigned char*)sub_str.c_str(), (unsigned char*)sub_text, rsa, RSA_PKCS1_PADDING);
		if (ret >= 0) {
			encrypt_text.append(std::string(sub_text, ret));
		}
		pos += block_len;
	}
	
	 // release memory  
	delete sub_text;
	BIO_free_all(keybio);
	RSA_free(rsa);
 
	return encrypt_text;
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
  std::ifstream sKeyRaw(keyPath);  // My own key pair file
  std::ifstream pKeyRaw(peerPath); // Peer public key file
  std::string peerKey, key;
  std::string delimiter{"-----END RSA PRIVATE KEY-----"};
  bool finishedSecretKey{false};

  if (sKeyRaw) {
    std::string line;
    while (getline(sKeyRaw, line)) {
      if (line == "")
        continue;
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