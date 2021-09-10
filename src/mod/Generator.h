#pragma once

#include <functional>
#include <memory>
#include <utility>

#include "Mod.h"

namespace mod {

enum class GeneratorState {
  Playing = 0,
  Paused,
};

template <class T>
struct ChangedValue {
  T &oldValue;
  T &newValue;
};

class Generator {
 private:
  struct ChannelState {
    size_t sampleIndex = 0;
    float sampleTime = 0.0f;
    float volume = 1.0f;
    float pitch = 1.0f;
  };

  std::shared_ptr<Mod> _mod = nullptr;
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

  std::function<void(Generator &, size_t, size_t)> _nextRowCallback = nullptr;
  std::function<void(Generator &, size_t, size_t, size_t, size_t)>
      _nextOrderCallback = nullptr;
  std::function<void(Generator &, GeneratorState, GeneratorState)>
      _stateChangedCallback = nullptr;

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

  void resetState();

  void _setState(GeneratorState newState);

  void _setRowIndex(size_t newRowIndex);

  void _setOrderIndex(size_t newOrderIndex);

 public:
  /**
   * @param mod
   * @param audioDataEncoding
   * @throws invalid_argument
   */
  Generator(std::shared_ptr<Mod> mod, Encoding audioDataEncoding);

  Generator() = default;

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setNextRowCallback(
      std::function<void(Generator &, size_t oldIndex, size_t newIndex)>
          callback);

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setNextOrderCallback(
      std::function<void(mod::Generator &generator, size_t oldOrderIndex,
                         size_t newOrderIndex, size_t oldPatternIndex,
                         size_t newPatternIndex)>
          callback);

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setStateChangedCallback(
      std::function<void(Generator &, GeneratorState oldState,
                         GeneratorState newState)>
          callback);

  void setVolume(float volume);

  /**
   * @param frequency
   * @throws invalid_argument If frequency is <= 1.0f.
   */
  void setFrequency(float frequency);

  void setMod(std::shared_ptr<Mod> mod);

  std::shared_ptr<Mod> getMod();

  [[nodiscard]] const std::shared_ptr<Mod> getMod() const;

  /**
   * @throws out_of_range If index is out of range for current pattern.
   * @throws BadStateException If mod file was not set.
   * @param index
   * @return
   */
  Row &getRow(size_t index);

  /**
   * @throws out_of_range If index is out of range for current pattern.
   * @throws BadStateException If mod file was not set.
   * @param index
   * @return
   */
  [[nodiscard]] const Row &getRow(size_t index) const;

  /**
   * @throws BadStateException If mod file was not set.
   * @return
   */
  Row &getCurrentRow();

  /**
   * @throws BadStateException If mod file was not set.
   * @return
   */
  [[nodiscard]] const Row &getCurrentRow() const;

  /**
   * @throws BadStateException If mod file was not set.
   * @return
   */
  Pattern &getCurrentPattern();

  /**
   * @throws BadStateException If mod file was not set.
   * @return
   */
  [[nodiscard]] const Pattern &getCurrentPattern() const;

  void stop();
  void restart();
  void pause();
  void start();

  /**
   * @param data
   * @param size
   * @throws BadStateException If encoding or mod was not set.
   */
  void generate(uint8_t *data, size_t size);

  /**
   * @param audioDataEncoding
   * @throws invalid_argument If passed unsupported encoding.
   */
  void setEncoding(Encoding audioDataEncoding);

  [[nodiscard]] Encoding getAudioDataEncoding() const;

  [[nodiscard]] GeneratorState getState() const;

  /**
   * @param index
   * @throws out_of_range
   * @throws BadStateException If encoding or mod was not set.
   */
  void setCurrentOrder(size_t index);

  /**
   * @param index
   * @throws out_of_range
   * @throws BadStateException If encoding or mod was not set.
   */
  void setCurrentRow(size_t index);

  /**
   * @param channelIndex
   * @throws out_of_range
   * @throws BadStateException If encoding or mod was not set.
   */
  void solo(size_t channelIndex);

  /**
   * @param channelIndex
   * @throws out_of_range
   * @throws BadStateException If encoding or mod was not set.
   */
  void unmute(size_t channelIndex);

  /**
   * @param channelIndex
   * @throws out_of_range
   * @throws BadStateException If encoding or mod was not set.
   */
  void mute(size_t channelIndex);

  /**
   * @throws BadStateException If encoding or mod was not set.
   */
  void unmuteAll();

  /**
   * @throws BadStateException If encoding or mod was not set.
   */
  void muteAll();

  /**
   * @param channelIndex
   * @throws out_of_range
   * @throws BadStateException If encoding or mod was not set.
   */
  bool isMuted(size_t channelIndex);
};

}  // namespace mod