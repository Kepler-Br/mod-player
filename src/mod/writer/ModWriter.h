#pragma once

#include <ostream>

#include "mod/Generator.h"

namespace mod {

class ModWriter {
 public:
  virtual void write(Generator &generator, std::ostream &stream) = 0;
};

}  // namespace mod
