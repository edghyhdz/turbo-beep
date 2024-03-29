/*
Reference: https://www.programmersought.com/article/37955188510/
Reference certificates server
https://stackoverflow.com/questions/11705815/client-and-server-communication-using-ssl-c-c-ssl-protocol-dont-works
*/

#ifndef RSA_H
#define RSA_H

#include "openssl/rsa.h"
#include <string>

#define ALPHANUM                                                               \
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define LEN_NONCE 25

/*
Class definition
*/
namespace turbobeep {
namespace crypto {
class RSA {
public:
  RSA(){};
  RSA(std::string keyPairPath, std::string peerPublicKeyPath); 
  std::string const publicKey() { return _publicKey; }
  std::string const peerPublicKey() { return _peerPublicKey; }
  std::string const signString(const std::string &message);
  std::string decryptWithPublicKey(const std::string &message, const std::string & pubKey); 
  std::string const privateKey(){ return _secretKey; }
  std::string const generateNonce();
  std::string const sha1(const std::string &input, unsigned long length);

  // Included pretty much for testing mediator
  std::string const loadPublicKey(std::string &keyPath); 

private:
  void _loadKeys(std::string &keyPairPath, std::string &peerKeyPath);
  std::string _secretKey;
  std::string _publicKey;
  std::string _peerPublicKey;
};
} // namespace crypto
} // namespace turbobeep
#endif