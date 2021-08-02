#ifndef CRYPTO_TEST
#define CRYPTO_TEST
#include "crypto.h"
#include <gtest/gtest.h>
#include <memory>

using namespace turbobeep;

class CryptoFixture : public testing::Test {
public:
  CryptoFixture();
  ~CryptoFixture();
  void SetUp() override;
  void TearDown() override;
  static void SetUpTestCase(){};
  static void TearDownTestCase(){};

protected:
  std::shared_ptr<crypto::RSA> _crypto; 
};

#endif