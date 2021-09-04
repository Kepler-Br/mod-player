#pragma once

#include <string>

namespace mod {

constexpr int unsignedEncoding = 0x0;
constexpr int signedEncoding = 0x1;
constexpr int encoding8 = 0x1 << 1;
constexpr int encoding16 = 0x1 << 2;
constexpr int encoding32 = 0x1 << 3;
constexpr int encoding64 = 0x1 << 4;

enum class Encoding {
  Unknown = 0,
  Unsigned8 = ((unsignedEncoding | encoding8) << 8) | 0,
  Signed8 = ((signedEncoding | encoding8) << 8) | 1,
  Unsigned16 = ((unsignedEncoding | encoding16) << 8) | 2,
  Signed16 = ((signedEncoding | encoding16) << 8) | 3,
};

bool isSignedEncoding(Encoding encoding);

size_t bytesInEncoding(Encoding encoding);

std::string encodingToString(Encoding value);

}