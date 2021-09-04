#pragma once

class MemoryBuffer : public std::basic_streambuf<char> {
 public:
  MemoryBuffer(const char *p, size_t l) {
    setg((char *)p, (char *)p, (char *)p + l);
  }
};
