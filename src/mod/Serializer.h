#pragma once

#include <cstring>
#include <istream>
#include <stdexcept>

#include "Note.h"
#include "Pattern.h"
#include "Sample.h"

namespace mod {

class Serializer {
 private:
  static const size_t noteDataSize;

  static uint16_t swapEndian(uint16_t val);

  static uint16_t readU16(std::istream &stream);

  static uint16_t readU16Big(std::istream &stream);

  static uint8_t readU8(std::istream &stream);

 public:
  Serializer() = delete;

  /**
   *
   * @param stream
   * @throw std::runtime_error
   * @return
   */
  static Note note(std::istream &stream);

  /**
   *
   * @param stream
   * @throw std::runtime_error
   * @return
   */
  static Row row(std::istream &stream, size_t channels);

  /**
   *
   * @param stream
   * @param channels
   * @param totalRows
   * @throw std::runtime_error
   * @return
   */
  static Pattern pattern(std::istream &stream, size_t channels,
                         size_t totalRows);

  /**
   *
   * @param stream
   * @throw std::runtime_error
   * @return
   */
  static Sample sample(std::istream &stream);
};

}  // namespace mod