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
  RSA(std::string keyPairPath, std::string peerPublicKeyPath); 
  // RSA(std::string myKey, std::string peerKey);
  std::string const publicKey();
  std::string const signString(std::string const message);
  std::string const privateKey(){ return _secretKey; }
  std::string const generateNonce();

private:
  void _loadKeys(std::string &keyPairPath, std::string &peerKeyPath);
  std::string _secretKey;
  std::string _publicKey;
  std::string _peerPublicKey;
};
} // namespace crypto
} // namespace turbobeep
#endif