#pragma once

#include "ModWriter.h"

namespace mod {

class RawWriter : public ModWriter {
 public:
  RawWriter() = default;

  void write(Generator &generator, std::ostream &stream) override;
};

}  // namespace mod
