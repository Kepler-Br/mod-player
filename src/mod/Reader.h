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
  static float convertFromU8(const uint8_t *value);
  static float convertFromS8(const uint8_t *value);
  static float convertFromU16(const uint8_t *value);
  static float convertFromS16(const uint8_t *value);

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
  [[nodiscard]] static std::vector<Sample> readSamples(std::ifstream &stream);
  /**
   * @param stream
   * @throw std::runtime_error
   */
  [[nodiscard]] static std::vector<Pattern> readPatterns(std::ifstream &stream,
                                                         size_t patternsNumber);
  /**
   * @param stream
   * @throw std::runtime_error
   * @throw std::invalid_argument
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

  static Mod read(const std::string &path);
};

}  // namespace mod
