#pragma once

#include <streambuf>

class MemoryBuffer : public std::basic_streambuf<char> {
 private:
  size_t _size;

 public:
  MemoryBuffer(char *pointer, size_t size);

  pos_type seekoff(off_type pos, std::ios_base::seekdir dir,
                   std::ios_base::openmode mode) override;
};
