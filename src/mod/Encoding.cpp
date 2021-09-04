#include "Encoding.h"
#include <stdexcept>

namespace mod {

bool isSignedEncoding(Encoding encoding) {
  return ((int)encoding >> 8) & unsignedEncoding;
}

size_t bytesInEncoding(Encoding encoding) {
  int field = (int)encoding >> 8;

  if (field & encoding8) {
    return 1;
  }

  if (field & encoding16) {
    return 2;
  }

  if (field & encoding32) {
    return 4;
  }

  if (field & encoding64) {
    return 8;
  }

  throw std::invalid_argument("bytesInEncoding: unknown encoding.");
}

std::string encodingToString(Encoding value) {
  switch(value) {
    case Encoding::Unknown:
      return "Unknown";
    case Encoding::Unsigned8:
      return "Unsigned8";
    case Encoding::Signed8:
      return "Signed8";
    case Encoding::Unsigned16:
      return "Unsigned16";
    case Encoding::Signed16:
      return "Signed16";
  }
}

}