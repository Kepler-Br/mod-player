#pragma once

#include <string>

#include "Mod.h"
#include "Note.h"
#include "Pattern.h"
#include "Row.h"
#include "Sample.h"

namespace mod {

class InfoString {
 public:
  InfoString() = delete;

  static std::string toString(const Note &note);

  static std::string toString(const Row &row);

  static std::string toString(const Pattern &pattern);

  static std::string toString(const Sample &sample);

  static std::string toString(const Mod &mod);
};

}  // namespace mod