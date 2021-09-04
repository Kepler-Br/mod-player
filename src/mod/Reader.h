#pragma once

#include <string>
#include <vector>

#include "Mod.h"
#include "Pattern.h"
#include "Sample.h"
#include "Encoding.h"

namespace mod {

class Reader {
 private:
  static float convertS8ToU8(uint8_t value);
  static float convertS8ToU16(uint8_t value);
  static float convertS8ToS16(uint8_t value);

  /**
   * @param stream
   * @throw std::runtime_error
   * @return
   */
  [[nodiscard]] static size_t getChannels(std::ifstream &stream);
  /**
   * @param stream
   * @throw std::runtime_error
   */
  [[nodiscard]] static std::vector<Sample> readSamples(std::ifstream &stream, Encoding audioDataEncoding);
  /**
   * @param stream
   * @throw std::runtime_error
   */
  [[nodiscard]] static std::vector<Pattern> readPatterns(std::ifstream &stream,
                                                         size_t patternsNumber);
  /**
   * @param stream
   * @throw std::runtime_error
   */
  static void readSamplesAudioData(std::ifstream &stream,
                                   std::vector<Sample> &samples, Encoding audioDataEncoding);
  /**
   * @param stream
   * @throw std::runtime_error
   */
  [[nodiscard]] static std::vector<int> readOrders(std::ifstream &stream);
  /**
   * @param stream
   * @throw std::runtime_error
   */
  [[nodiscard]] static std::string readName(std::ifstream &stream);

 public:
  Reader() = delete;

  static Mod read(const std::string &path, Encoding encoding);
};

}  // namespace mod
