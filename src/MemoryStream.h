#pragma once

#include "MemoryBuffer.h"

class MemoryStream : public std::istream {
 private:
  MemoryBuffer _buffer;

 public:
  MemoryStream(const char *p, size_t l)
      : std::istream(&_buffer), _buffer(p, l) {
    rdbuf(&_buffer);
  }
};