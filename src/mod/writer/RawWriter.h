#pragma once

#include "ModWriter.h"

namespace mod {

class RawWriter : public ModWriter {
 public:
  RawWriter() = default;
  ~RawWriter() override = default;

  void write(Generator &generator, std::ostream &stream) override;
};

}  // namespace mod
