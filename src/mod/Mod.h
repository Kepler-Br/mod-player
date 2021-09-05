#pragma once

#include <utility>
#include <vector>

#include "Encoding.h"
#include "Pattern.h"
#include "Sample.h"

namespace mod {

class Mod {
 private:
  std::string _name;
  size_t _songLength = 0;
  size_t _channels = 0;

  std::vector<Sample> _samples;
  std::vector<Pattern> _patterns;
  std::vector<int> _orders;

 public:
  Mod(std::string name, size_t songLength,
      std::vector<Sample> samples, std::vector<Pattern> patterns,
      std::vector<int> orders);

  Mod() = default;

  [[nodiscard]] size_t getChannels() const;

  [[nodiscard]] size_t getSongLength() const;

  [[nodiscard]] size_t getSampleCount() const;

  [[nodiscard]] size_t getPatternCount() const;

  [[nodiscard]] std::vector<Sample> &getSamples();

  [[nodiscard]] std::vector<int> &getOrders();

  [[nodiscard]] std::vector<Pattern> &getPatterns();

  [[nodiscard]] const std::vector<Sample> &getSamples() const;

  [[nodiscard]] const std::vector<int> &getOrders() const;

  [[nodiscard]] const std::vector<Pattern> &getPatterns() const;

  [[nodiscard]] const std::string &getName() const;
};

}  // namespace mod