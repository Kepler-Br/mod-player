#pragma once

#include "ModWriter.h"
#include <iostream>

namespace mod {

class WavWriter : public ModWriter {
 private:

  /**
   * @param stream
   * @throw std::runtime_error
   */
  static void writeHeader(std::ostream &stream, uint32_t dataSize);

 public:
  WavWriter() = default;
  ~WavWriter() override = default;

  void write(Generator &generator, std::ostream &stream) override;
};

}  // namespace mod