#pragma once


// ----------------------------------------------------------------------------
// Common
// ----------------------------------------------------------------------------

typedef struct tfaesecret* pfaesecret;
typedef std::array<std::uint8_t, 32> tfaesecret;

typedef struct tfaekey* pfaekey;
typedef std::array<std::uint8_t, 32> tfaekey;

typedef struct tfaechallenge* pfaechallenge;
struct tfaechallenge {
  std::array<std::uint8_t, 32> ephkey;
  tfaesecret encryptedsecret;
};

typedef struct tfaeresponse* pfaeresponse;
struct tfaeresponse {
  std::array<std::uint8_t, 32> gamekey;
  std::array<char, 32> gameversion;
  int64 validuntil;    // LE unix timestamp
  std::uint32_t faebuild; // LE
  std::uint32_t status; // LE
  std::array<char, 32> statusstr;
};

typedef struct tfaeresponsebox* pfaeresponsebox;
struct tfaeresponsebox {
  std::uint32_t outerstatus;
  std::uint32_t reserved;
  std::array<std::uint8_t, 16> mac;
  array<1,sizeof(tfaeresponse),std::uint8_t> ciphertext;
};

