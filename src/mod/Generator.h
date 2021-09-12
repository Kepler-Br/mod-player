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
  T oldValue;
  T newValue;

  ChangedValue(T inOldValue, T inNewValue)
      : oldValue(inOldValue), newValue(inNewValue) {}

  explicit ChangedValue(T value) : oldValue(value), newValue(value) {}
};

struct ChangedRowEvent {
  ChangedValue<size_t> row;
  ChangedValue<size_t> order;
  ChangedValue<size_t> pattern;

  ChangedRowEvent(ChangedValue<size_t> inRow, ChangedValue<size_t> inOrder,
                  ChangedValue<size_t> inPattern)
      : row(inRow), order(inOrder), pattern(inPattern) {}
};

struct ChangedOrderEvent {
  ChangedValue<size_t> row;
  ChangedValue<size_t> order;
  ChangedValue<size_t> pattern;

  ChangedOrderEvent(ChangedValue<size_t> inRow, ChangedValue<size_t> inOrder,
                    ChangedValue<size_t> inPattern)
      : row(inRow), order(inOrder), pattern(inPattern) {}
};

struct ChangedStateEvent {
  ChangedValue<GeneratorState> state;

  explicit ChangedStateEvent(ChangedValue<GeneratorState> inState)
      : state(inState) {}
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
  size_t _timePassed = 0.0f;
  size_t _timePerRow = 440.0f * 6.0f;
  size_t _currentOrderIndex = 0;
  size_t _currentRowIndex = 0;
  size_t _bytesInEncoding = 1;
  float _volume = 1.0f;
  float _frequency = 22050.0f;

  std::function<void(Generator &, ChangedRowEvent event)> _nextRowCallback =
      nullptr;
  std::function<void(Generator &, ChangedOrderEvent event)> _nextOrderCallback =
      nullptr;
  std::function<void(Generator &, ChangedStateEvent event)>
      _stateChangedCallback = nullptr;

  GeneratorState _generatorState = GeneratorState::Playing;
  Encoding _audioDataEncoding = Encoding::Unknown;

  void (*_convertor)(const float &value, uint8_t *target) = nullptr;

  /**
   * Advance current order and current row indexes.
   * @return true if end reached, false if not.
   */
  bool advanceIndexes();

  void generateByChannel(std::vector<float> &data, size_t start, size_t end,
                         const Row &row, size_t channelIndex);

  void resetState();

  size_t calculateTimePerRow(float frequency, float speed);

  void _setState(GeneratorState newState);

  void _setRowIndex(size_t newRowIndex);

  void _setOrderIndex(size_t newOrderIndex);

  void _setOrderAndRowIndex(size_t newOrderIndex, size_t newRowIndex);

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
      std::function<void(Generator &, ChangedRowEvent event)> callback);

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setNextOrderCallback(
      std::function<void(mod::Generator &generator, ChangedOrderEvent event)>
          callback);

  /**
   * @param callback If nullptr, then callback would not be called.
   */
  void setStateChangedCallback(
      std::function<void(Generator &, ChangedStateEvent event)>
          callback);

  void setVolume(float volume);

  /**
   * @param frequency
   * @throws invalid_argument If frequency is <= 1.0f.
   */
  void setFrequency(float frequency);

  void setMod(std::shared_ptr<Mod> mod);

  std::shared_ptr<Mod> getMod();

  [[nodiscard]] std::shared_ptr<const Mod> getMod() const;

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