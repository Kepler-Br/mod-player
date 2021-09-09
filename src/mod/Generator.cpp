#include "Generator.h"

#include <fmt/format.h>

namespace mod {

void Generator::convertToU8(const float &value, uint8_t *target) {
  constexpr int maxSigned = 0x80 - 1;
  constexpr float maxSignedFloat = 0x80 - 1;

  *target = (uint8_t)((int)(value * maxSignedFloat) + maxSigned);
}

void Generator::convertToS8(const float &value, uint8_t *target) {
  constexpr float maxSignedFloat = 0x80 - 1;

  *(int8_t *)target = (int8_t)((int)(value * maxSignedFloat));
}

void Generator::convertToU16(const float &value, uint8_t *target) {
  constexpr int maxSigned16 = 0x8000 - 1;
  constexpr float maxSigned16Float = 0x8000 - 1;

  *(uint16_t *)target =
      (uint16_t)((int)(value * maxSigned16Float) + maxSigned16);
}

void Generator::convertToS16(const float &value, uint8_t *target) {
  constexpr float maxSigned16Float = 0x8000 - 1;

  *(int16_t *)target = (int16_t)((int)(value * maxSigned16Float));
}

bool Generator::advanceIndexes() {
  if (this->_generatorState == GeneratorState::Paused) {
    return false;
  }

  const std::vector<Pattern> &patterns = this->_mod.getPatterns();
  const std::vector<int> &orders = this->_mod.getOrders();

  if (this->_currentOrderIndex >= this->_mod.getSongLength()) {
    return true;
  }

  int currentOrder = orders[this->_currentOrderIndex];
  const Pattern *currentPattern = &patterns[currentOrder];

  if (this->_currentRowIndex + 1 >= currentPattern->getTotalRows()) {
    if (this->_currentOrderIndex + 1 >= this->_mod.getSongLength()) {
      return true;
    }

    this->_setOrderIndex(this->_currentOrderIndex + 1);
    this->_setRowIndex(0);
  } else {
    this->_setRowIndex(this->_currentRowIndex + 1);
  }

  return false;
}

void Generator::generateByChannel(std::vector<float> &data, size_t start,
                                  size_t end, const Row &row,
                                  size_t channelIndex) {
  const Note &note = row.getNote(channelIndex);
  ChannelState &channelState = this->_channelsStates[channelIndex];

  if (note.sampleIndex != 0 && !this->_rowPlayed) {
    channelState = {};
    channelState.sampleIndex = note.sampleIndex;
    if (note.samplePeriodFrequency == 0) {
      channelState.pitch = 1.0f;
    } else {
      channelState.pitch = 7093789.2f /
                           ((float)(note.samplePeriodFrequency) * 2.0f) /
                           (this->_frequency);
    }
  }

  const auto &sampleIndex = channelState.sampleIndex;

  if (sampleIndex == 0) {
    return;
  }

  const Sample &sample = this->_mod.getSamples()[sampleIndex - 1];
  const std::vector<float> &sampleData = sample.getData();

  size_t dataIndex2 = 0;

  if (note.effectNumber == 0xC) {
    channelState.volume = (float)note.effectParameter / 64.0f;
  }

  float sampleVolume = (float)sample.getVolume() / 64.0f * channelState.volume;

  for (auto i = start; i < end; i++) {
    auto sampleDataIndex = (size_t)(channelState.sampleTime +
                                    (float)dataIndex2 * channelState.pitch);

    if (sampleDataIndex >= sampleData.size()) {
      if (sample.getRepeatLength() == 0) {
        channelState = {};
        break;
      } else {
        channelState.sampleTime = (float)sample.getRepeatPoint();
        // Todo
        dataIndex2 = 0;
        sampleDataIndex = (size_t)(channelState.sampleTime +
                                   (float)dataIndex2 * channelState.pitch);
      }
    }

    data[i] += sampleData[sampleDataIndex] * sampleVolume /
               (float)this->_mod.getChannels();

    dataIndex2++;
  }

  if (sampleIndex == 0) {
    return;
  }

  channelState.sampleTime += (float)dataIndex2 * channelState.pitch;
}

Generator::Generator(Mod mod, Encoding audioDataEncoding)
    : _mod(std::move(mod)), _audioDataEncoding(audioDataEncoding) {
  this->_channelsStates.resize(this->_mod.getChannels());
  this->_mutedChannels.resize(this->_mod.getChannels(), false);

  this->setEncoding(audioDataEncoding);
}

void Generator::setVolume(float volume) { this->_volume = volume; }

void Generator::setFrequency(float frequency) {
  if (frequency <= 0.0f) {
    throw std::invalid_argument(
        fmt::format("Frequency cannot be less than 0. Have {}", frequency));
  }

  this->_frequency = frequency;
}

void Generator::setMod(Mod mod) {
  this->_mod = std::move(mod);
  this->_channelsStates.resize(this->_mod.getChannels());

  this->resetState();
}

void Generator::stop() {
  this->_setState(GeneratorState::Paused);
  this->_setOrderIndex(0);
  this->_setRowIndex(0);
  this->_timePassed = 0;
  this->_rowPlayed = false;
}

void Generator::restart() {
  this->stop();
  this->start();
}

void Generator::pause() { this->_setState(GeneratorState::Paused); }

void Generator::start() { this->_setState(GeneratorState::Playing); }

void Generator::generate(uint8_t *data, size_t size) {
  if (this->_convertor == nullptr) {
    throw std::logic_error("generate: Audio encoding was not set.");
  }

  if (this->_generatorState == GeneratorState::Paused) {
    for (uint8_t *ptr = data; ptr < (data + size);
         ptr += this->_bytesInEncoding) {
      this->_convertor(0.0f, ptr);
    }
    return;
  }

  if (this->_buffer.size() != (size / this->_bytesInEncoding)) {
    this->_buffer.resize(size / this->_bytesInEncoding);
  }

  for (size_t current = 0; current < this->_buffer.size();) {
    //        size_t next = std::min(current + this->_timePerRow,
    //        this->_buffer.size());
    size_t next =
        std::min(this->_timePerRow - this->_timePassed % this->_timePerRow,
                 this->_buffer.size());

    const std::vector<Pattern> &patterns = this->_mod.getPatterns();
    const std::vector<int> &orders = this->_mod.getOrders();

    int currentOrder = orders[this->_currentOrderIndex];
    const Pattern &currentPattern = patterns[currentOrder];

    const Row &currentRow = currentPattern.getRow(this->_currentRowIndex);

    if (!this->_rowPlayed) {
      for (const auto &note : currentRow.getNotes()) {
        if (note.effectNumber == 0xF) {
          this->_timePerRow = 440 * note.effectParameter;
        }
      }
    }

    for (auto channelIndex = 0; channelIndex < this->_mod.getChannels();
         channelIndex++) {
      if (this->_mutedChannels[channelIndex]) {
        continue;
      }

      this->generateByChannel(this->_buffer, current, next, currentRow,
                              channelIndex);
    }

    this->_rowPlayed = true;

    if ((this->_timePassed % this->_timePerRow) + (next - current) >=
        this->_timePerRow) {
      this->_rowPlayed = false;
      if (this->advanceIndexes()) {
        this->stop();
        break;
      }
    }
    this->_timePassed += next - current;

    current = next;
  }

  uint8_t *dataPtr = data;
  for (const float &i : this->_buffer) {
    this->_convertor(std::min(1.0f, std::max(-1.0f, i * this->_volume)),
                     dataPtr);
    dataPtr += this->_bytesInEncoding;
  }

  std::memset(this->_buffer.data(), 0, this->_buffer.size() * sizeof(float));
}

void Generator::setEncoding(Encoding audioDataEncoding) {
  switch (audioDataEncoding) {
    case Encoding::Signed16:
      this->_convertor = &Generator::convertToS16;
      this->_bytesInEncoding = bytesInEncoding(audioDataEncoding);
      break;
    case Encoding::Unsigned16:
      this->_convertor = &Generator::convertToU16;
      this->_bytesInEncoding = bytesInEncoding(audioDataEncoding);
      break;
    case Encoding::Signed8:
      this->_convertor = &Generator::convertToS8;
      this->_bytesInEncoding = bytesInEncoding(audioDataEncoding);
      break;
    case Encoding::Unsigned8:
      this->_convertor = &Generator::convertToU8;
      this->_bytesInEncoding = bytesInEncoding(audioDataEncoding);
      break;
    default:
      throw std::invalid_argument("setEncoding: unknown encoding: " +
                                  encodingToString(audioDataEncoding));
  }

  this->_audioDataEncoding = audioDataEncoding;
}

Encoding Generator::getAudioDataEncoding() const {
  return this->_audioDataEncoding;
}

GeneratorState Generator::getState() const { return this->_generatorState; }

void Generator::setCurrentOrder(size_t index) {
  if (index >= this->_mod.getOrders().size()) {
    const std::string message = fmt::format(
        "setCurrentOrder: Tried to set order out of range: {}. Total orders: "
        "{}",
        index, this->_mod.getOrders().size());

    throw std::out_of_range(message);
  }

  this->resetState();
  this->_setOrderIndex(index);
}

void Generator::setCurrentRow(size_t index) {
  const auto &patternIndex = this->_mod.getOrders()[this->_currentOrderIndex];
  const auto &pattern = this->_mod.getPatterns()[patternIndex];

  if (index >= pattern.getTotalRows()) {
    const std::string message = fmt::format(
        "setCurrentRow: Tried to set row out of range: {}. Total rows: "
        "{}",
        index, pattern.getTotalRows());

    throw std::out_of_range(message);
  }

  this->resetState();
  this->_setRowIndex(index);
}

void Generator::solo(size_t channelIndex) {
  if (channelIndex >= this->_mutedChannels.size()) {
    const std::string message = fmt::format(
        "solo: Tried to set channel solo out of range: {}. Total channels: "
        "{}",
        channelIndex, this->_mutedChannels.size());

    throw std::out_of_range(message);
  }

  for (auto &&b : this->_mutedChannels) {
    b = true;
  }

  this->_mutedChannels[channelIndex] = false;
}

void Generator::unmute(size_t channelIndex) {
  if (channelIndex >= this->_mutedChannels.size()) {
    const std::string message = fmt::format(
        "unmute: Tried to set channel unmute out of range: {}. Total channels: "
        "{}",
        channelIndex, this->_mutedChannels.size());

    throw std::out_of_range(message);
  }

  this->_mutedChannels[channelIndex] = false;
}

void Generator::mute(size_t channelIndex) {
  if (channelIndex >= this->_mutedChannels.size()) {
    const std::string message = fmt::format(
        "mute: Tried to set channel mute out of range: {}. Total channels: "
        "{}",
        channelIndex, this->_mutedChannels.size());

    throw std::out_of_range(message);
  }

  this->_mutedChannels[channelIndex] = true;
}

void Generator::unmuteAll() {
  for (auto &&b : this->_mutedChannels) {
    b = false;
  }
}

void Generator::muteAll() {
  for (auto &&b : this->_mutedChannels) {
    b = true;
  }
}

bool Generator::isMuted(size_t channelIndex) {
  if (channelIndex >= this->_mutedChannels.size()) {
    const std::string message = fmt::format(
        "mute: Tried to set channel mute out of range: {}. Total channels: "
        "{}",
        channelIndex, this->_mutedChannels.size());

    throw std::out_of_range(message);
  }

  return this->_mutedChannels[channelIndex];
}

}  // namespace mod