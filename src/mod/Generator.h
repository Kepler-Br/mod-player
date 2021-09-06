#pragma once

#include "Mod.h"

namespace mod {

enum class GeneratorState {
  Playing = 0,
  Paused,
};

class Generator {
 private:
  struct ChannelState {
    size_t sampleTime = 0;
    size_t sampleIndex = 0;
    float volume = 1.0f;
    float pitch = 1.0f;
  };

  Mod _mod;
  std::vector<ChannelState> _channelsStates;
  std::vector<bool> _mutedChannels;
  std::vector<float> _buffer;

  //  std::vector<size_t> _perChannelTime;
  //  std::vector<size_t> _previousSampleIndex;
  //  std::vector<const Note *> _previousNotEmptyNote;
  bool _rowPlayed = false;
  size_t _timePassed = 0;
  //  size_t _timePerRow = 1024*3.0f * (230.0f / 3072.0f);
  //  size_t _timePerRow = 230.0f / (276.0f/3072.0f);
  //  size_t _timePerRow = 427*6;
  size_t _timePerRow = 440 * 6;
  size_t _currentOrderIndex = 0;
  size_t _currentRowIndex = 0;
  size_t _bytesInEncoding = 1;

  GeneratorState _generatorState = GeneratorState::Playing;
  Encoding _audioDataEncoding = Encoding::Unknown;

  void (*_convertor)(const float &value, uint8_t *target) = nullptr;

  static void convertToU8(const float &value, uint8_t *target);
  static void convertToS8(const float &value, uint8_t *target);
  static void convertToU16(const float &value, uint8_t *target);
  static void convertToS16(const float &value, uint8_t *target);

  inline static void mix(const float &src, float &dest);

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

 public:
  /**
   * @param mod
   * @param audioDataEncoding
   * @throw std::invalid_argument
   */
  explicit Generator(Mod mod, Encoding audioDataEncoding);

  Generator() = default;

  void setMod(Mod mod);

  void stop();
  void restart();
  void pause();
  void start();

  /**
   * @param data
   * @param size
   * @throw std::logic_error
   */
  void generate(uint8_t *data, size_t size, float volume);

  /**
   * @param audioDataEncoding
   * @throw std::invalid_argument
   */
  void setEncoding(Encoding audioDataEncoding);

  Encoding getAudioDataEncoding() const;

  GeneratorState getState() const;

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