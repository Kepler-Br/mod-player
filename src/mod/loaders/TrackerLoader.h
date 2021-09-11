#pragma once

#include <istream>
#include <memory>

#include "mod/Mod.h"

namespace mod {

class TrackerLoader {
 public:
  virtual ~TrackerLoader() = default;

  virtual std::shared_ptr<Mod> load(std::istream &stream) = 0;
  virtual std::shared_ptr<Mod> load(const std::string &path) = 0;
};

}  // namespace mod
