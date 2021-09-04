#pragma once

#include "Mod.h"

namespace mod {

enum class GeneratorState {
  Playing = 0,
  Paused,
};

class Generator {
 private:
  Mod _mod;
  std::vector<size_t> _perChannelTime;
  std::vector<size_t> _previousSampleIndex;
  size_t _timePassed = 0;
  size_t _timePerRow = 1024;
  size_t _currentOrderIndex = 0;
  size_t _currentRowIndex = 0;
  GeneratorState _generatorState = GeneratorState::Playing;

  /**
   * Advance current order and current row indexes.
   * @return true if end reached, false if not.
   */
  bool advanceIndexes();

  std::string fancyRow(const Row &row) const;

  void generateByChannel(uint8_t *data, size_t size, const Row &row, size_t channelIndex);

 public:
  explicit Generator(Mod mod);

  Generator() = default;

  void setMod(Mod mod);

  void stop();

  void generate(uint8_t *data, size_t size);

  GeneratorState getState() const;
};

}  // namespace mod