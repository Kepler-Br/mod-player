#pragma once

#include <string>

#include "Mod.h"
#include "Note.h"
#include "Pattern.h"
#include "Row.h"
#include "Sample.h"

namespace mod {

class InfoString {
 private:
  struct UnixColorsBackground {
    std::string black         = "\u001b[40m";
    std::string red           = "\u001b[41m";
    std::string green         = "\u001b[42m";
    std::string yellow        = "\u001b[43m";
    std::string blue          = "\u001b[44m";
    std::string magenta       = "\u001b[45m";
    std::string cyan          = "\u001b[46m";
    std::string white         = "\u001b[47m";

    std::string brightBlack   = "\u001b[40;1m";
    std::string brightRed     = "\u001b[41;1m";
    std::string brightGreen   = "\u001b[42;1m";
    std::string brightYellow  = "\u001b[43;1m";
    std::string brightBlue    = "\u001b[44;1m";
    std::string brightMagenta = "\u001b[45;1m";
    std::string brightCyan    = "\u001b[46;1m";
    std::string brightWhite   = "\u001b[47;1m";

    std::string reset         = "\u001b[0m";
  };

  struct UnixColors {
    std::string black         = "\u001b[30m";
    std::string red           = "\u001b[31m";
    std::string green         = "\u001b[32m";
    std::string yellow        = "\u001b[33m";
    std::string blue          = "\u001b[34m";
    std::string magenta       = "\u001b[35m";
    std::string cyan          = "\u001b[36m";
    std::string white         = "\u001b[37m";

    std::string brightBlack   = "\u001b[30;1m";
    std::string brightRed     = "\u001b[31;1m";
    std::string brightGreen   = "\u001b[32;1m";
    std::string brightYellow  = "\u001b[33;1m";
    std::string brightBlue    = "\u001b[34;1m";
    std::string brightMagenta = "\u001b[35;1m";
    std::string brightCyan    = "\u001b[36;1m";
    std::string brightWhite   = "\u001b[37;1m";

    std::string reset         = "\u001b[0m";
  };

  static UnixColorsBackground background;

  static UnixColors colors;

  static std::string intToNote(int i);

 public:
  InfoString() = delete;

  static std::string toString(const Note &note);

  static std::string toString(const Row &row);

  static std::string toString(const Pattern &pattern);

  static std::string toString(const Sample &sample);

  static std::string toString(const Mod &mod);

  static std::string fancyRow(const Row &row);
};

}  // namespace mod