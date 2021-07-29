/*
Reference: https://www.programmersought.com/article/37955188510/
Reference certificates server
https://stackoverflow.com/questions/11705815/client-and-server-communication-using-ssl-c-c-ssl-protocol-dont-works
*/

#ifndef RSA_H
#define RSA_H

#include "openssl/rsa.h"
#include <string>

/*
Class definition
*/
namespace turbobeep {
namespace crypto {
class RSA {
public:
  RSA(std::string myKey, std::string peerKey);

private:
  std::string _secretKey;
  std::string _publicKey;
  std::string _peerPublicKey;
};
} // namespace crypto
} // namespace turbobeep
#endif