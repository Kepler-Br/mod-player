#pragma once

#include <functional>
#include <utility>

#include "Mod.h"

namespace mod {

enum class GeneratorState {
  Playing = 0,
  Paused,
};

class Generator {
 private:
  struct ChannelState {
    size_t sampleIndex = 0;
    float sampleTime = 0.0f;
    float volume = 1.0f;
    float pitch = 1.0f;
  };

  Mod _mod;
  std::vector<ChannelState> _channelsStates;
  std::vector<bool> _mutedChannels;
  std::vector<float> _buffer;

  bool _rowPlayed = false;
  size_t _timePassed = 0;
  size_t _timePerRow = 440 * 6;
  size_t _currentOrderIndex = 0;
  size_t _currentRowIndex = 0;
  size_t _bytesInEncoding = 1;
  float _volume = 1.0f;
  float _frequency = 22050.0f;

  std::function<void (Generator &, size_t, size_t)> _nextRowCallback = nullptr;
  std::function<void (Generator &, size_t, size_t, size_t, size_t)> _nextOrderCallback = nullptr;
  std::function<void (Generator &, GeneratorState, GeneratorState)> _stateChangedCallback = nullptr;

  GeneratorState _generatorState = GeneratorState::Playing;
  Encoding _audioDataEncoding = Encoding::Unknown;

  void (*_convertor)(const float &value, uint8_t *target) = nullptr;

  static void convertToU8(const float &value, uint8_t *target);
  static void convertToS8(const float &value, uint8_t *target);
  static void convertToU16(const float &value, uint8_t *target);
  static void convertToS16(const float &value, uint8_t *target);

  /**
   * Advance current order and current row indexes.
   * @return true if end reached, false if not.
   */
  bool advanceIndexes();

  void generateByChannel(std::vector<float> &data, size_t start, size_t end,
                         const Row &row, size_t channelIndex);

  void resetState() {
    for (auto &state : this->_channelsStates) {
      state = {};
    }
  }

  void _setState(GeneratorState newState) {
    if (this->_stateChangedCallback != nullptr) {
      this->_stateChangedCallback(*this, this->_generatorState, newState);
    }

    this->_generatorState = newState;
  }

  void _setRowIndex(size_t newRowIndex) {
    if (this->_nextRowCallback != nullptr) {
      this->_nextRowCallback(*this, this->_currentRowIndex, newRowIndex);
    }

    this->_currentRowIndex = newRowIndex;
  }

  void _setOrderIndex(size_t newOrderIndex) {
    if (this->_nextOrderCallback != nullptr) {
      const std::vector<int> orders = this->_mod.getOrders();

      const int &oldPatternIndex = orders[this->_currentOrderIndex];
      const int &newPatternIndex = orders[newOrderIndex];
      this->_nextOrderCallback(*this, this->_currentOrderIndex, newOrderIndex, oldPatternIndex, newPatternIndex);
    }

    this->_currentOrderIndex = newOrderIndex;
  }

 public:
  /**
   * @param mod
   * @param audioDataEncoding
   * @throw std::invalid_argument
   */
  explicit Generator(Mod mod, Encoding audioDataEncoding);

  Generator() = default;

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setNextRowCallback(std::function<void (Generator &, size_t oldIndex, size_t newIndex)> callback) {
    this->_nextRowCallback = std::move(callback);
  }

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setNextOrderCallback(std::function<void (mod::Generator &generator, size_t oldOrderIndex, size_t newOrderIndex, size_t oldPatternIndex, size_t newPatternIndex)> callback) {
    this->_nextOrderCallback = std::move(callback);
  }

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setStateChangedCallback(std::function<void (Generator &, GeneratorState oldState, GeneratorState newState)> callback) {
    this->_stateChangedCallback = std::move(callback);
  }

  void setVolume(float volume);

  /**
   * @throws std::invalid_argument
   * @param frequency
   */
  void setFrequency(float frequency);

  void setMod(Mod mod);

  Mod &getMod() {
    return this->_mod;
  }

  [[nodiscard]] const Mod &getMod() const {
    return this->_mod;
  }

  /**
   * @throws std::out_of_range
   * @param index
   * @return
   */
  Row &getRow(size_t index) {
    Pattern &pattern = this->getCurrentPattern();

    return pattern.getRow(index);
  }

  /**
   * @throws std::out_of_range
   * @param index
   * @return
   */
  [[nodiscard]] const Row &getRow(size_t index) const {
    const Pattern &pattern = this->getCurrentPattern();

    return pattern.getRow(index);
  }

  Row &getCurrentRow() {
    Pattern &pattern = this->getCurrentPattern();

    return pattern.getRow(this->_currentRowIndex);
  }

  [[nodiscard]] const Row &getCurrentRow() const {
    const Pattern &pattern = this->getCurrentPattern();

    return pattern.getRow(this->_currentRowIndex);
  }

  Pattern &getCurrentPattern() {
    const int &patternIndex = this->_mod.getOrders()[this->_currentOrderIndex];

    return this->_mod.getPatterns()[patternIndex];
  }

  [[nodiscard]] const Pattern &getCurrentPattern() const {
    const int &patternIndex = this->_mod.getOrders()[this->_currentOrderIndex];

    return this->_mod.getPatterns()[patternIndex];
  }

  void stop();
  void restart();
  void pause();
  void start();

  /**
   * @param data
   * @param size
   * @throw std::logic_error
   */
  void generate(uint8_t *data, size_t size);

  /**
   * @param audioDataEncoding
   * @throw std::invalid_argument
   */
  void setEncoding(Encoding audioDataEncoding);

  [[nodiscard]] Encoding getAudioDataEncoding() const;

  [[nodiscard]] GeneratorState getState() const;

  /**
   * @param index
   * @throw std::out_of_range
   */
  void setCurrentOrder(size_t index);

  /**
   * @param index
   * @throw std::out_of_range
   */
  void setCurrentRow(size_t index);

  /**
   * @param channelIndex
   * @throw std::out_of_range
   */
  void solo(size_t channelIndex);

  /**
   * @param channelIndex
   * @throw std::out_of_range
   */
  void unmute(size_t channelIndex);

  /**
   * @param channelIndex
   * @throw std::out_of_range
   */
  void mute(size_t channelIndex);

  void unmuteAll();

  void muteAll();

  /**
   * @param channelIndex
   * @throw std::out_of_range
   */
  bool isMuted(size_t channelIndex);
};

}  // namespace mod