#include <fmt/format.h>

#include <iostream>
#include <utility>

#include "Generator.h"
#include "exceptions/BadStateException.h"
#include "loaders/DataConvertors.h"

namespace mod {

#pragma region private

bool Generator::advanceIndexes() {
  if (this->_generatorState == GeneratorState::Paused) {
    return false;
  }

  const std::vector<Pattern> &patterns = this->_mod->getPatterns();
  const std::vector<int> &orders = this->_mod->getOrders();

  if (this->_currentOrderIndex >= this->_mod->getSongLength()) {
    return true;
  }

  int currentOrder = orders[this->_currentOrderIndex];
  const Pattern *currentPattern = &patterns[currentOrder];

  if (this->_currentRowIndex + 1 >= currentPattern->getTotalRows()) {
    if (this->_currentOrderIndex + 1 >= this->_mod->getSongLength()) {
      return true;
    }

    this->_setOrderAndRowIndex(this->_currentOrderIndex + 1, 0);
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

  const Sample &sample = this->_mod->getSamples()[sampleIndex - 1];
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
               (float)this->_mod->getChannels();

    dataIndex2++;
  }

  if (sampleIndex == 0) {
    return;
  }

  channelState.sampleTime += (float)dataIndex2 * channelState.pitch;
}

void Generator::resetState() {
  for (auto &state : this->_channelsStates) {
    state = {};
  }
}

size_t Generator::calculateTimePerRow(float frequency, float speed) {
  return (size_t)(441.5f * speed / (11025.0f * 2.0f) * frequency);
}

void Generator::_setState(GeneratorState newState) {
  if (newState == this->_generatorState) {
    return;
  }

  if (this->_stateChangedCallback != nullptr) {
    ChangedValue<GeneratorState> state(this->_generatorState, newState);

    this->_stateChangedCallback(*this, ChangedStateEvent(state));
  }

  this->_generatorState = newState;
}

void Generator::_setRowIndex(size_t newRowIndex) {
  if (this->_nextRowCallback != nullptr) {
    const std::vector<int> &orders = this->_mod->getOrders();
    const int &patternIndex = orders[this->_currentOrderIndex];
    ChangedValue<size_t> changedRow(this->_currentRowIndex, newRowIndex);
    ChangedValue<size_t> changedOrder(this->_currentOrderIndex);
    ChangedValue<size_t> changedPattern(patternIndex);
    ChangedRowEvent event(changedRow, changedOrder, changedPattern);

    this->_currentRowIndex = newRowIndex;

    this->_nextRowCallback(*this, event);
  } else {
    this->_currentRowIndex = newRowIndex;
  }
}

void Generator::_setOrderIndex(size_t newOrderIndex) {
  if (this->_nextOrderCallback != nullptr) {
    const std::vector<int> &orders = this->_mod->getOrders();

    const int &oldPatternIndex = orders[this->_currentOrderIndex];
    const int &newPatternIndex = orders[newOrderIndex];
    ChangedValue<size_t> changedRow(this->_currentRowIndex);
    ChangedValue<size_t> changedOrder(this->_currentOrderIndex, newOrderIndex);
    ChangedValue<size_t> changedPattern(oldPatternIndex, newPatternIndex);
    ChangedOrderEvent event(changedRow, changedOrder, changedPattern);

    this->_currentOrderIndex = newOrderIndex;

    this->_nextOrderCallback(*this, event);
  } else {
    this->_currentOrderIndex = newOrderIndex;
  }
}

void Generator::_setOrderAndRowIndex(size_t newOrderIndex, size_t newRowIndex) {
  if (this->_nextOrderCallback != nullptr || this->_nextRowCallback != nullptr) {
    const std::vector<int> &orders = this->_mod->getOrders();

    const int &oldPatternIndex = orders[this->_currentOrderIndex];
    const int &newPatternIndex = orders[newOrderIndex];

    ChangedValue<size_t> changedRow(this->_currentRowIndex, newRowIndex);
    ChangedValue<size_t> changedOrder(this->_currentOrderIndex, newOrderIndex);
    ChangedValue<size_t> changedPattern(oldPatternIndex, newPatternIndex);

    this->_currentOrderIndex = newOrderIndex;
    this->_currentRowIndex = newRowIndex;

    if (this->_nextOrderCallback != nullptr) {
      ChangedOrderEvent event(changedRow, changedOrder, changedPattern);

      this->_nextOrderCallback(*this, event);
    }

    if (this->_nextRowCallback != nullptr) {
      ChangedRowEvent event(changedRow, changedOrder, changedPattern);

      this->_nextRowCallback(*this, event);
    }
  } else {
    this->_currentOrderIndex = newOrderIndex;
    this->_currentRowIndex = newRowIndex;
  }
}

#pragma endregion

#pragma region public constructor

Generator::Generator(std::shared_ptr<Mod> mod, Encoding audioDataEncoding)
    : _mod(std::move(mod)), _audioDataEncoding(audioDataEncoding) {
  this->_channelsStates.resize(this->_mod->getChannels());
  this->_mutedChannels.resize(this->_mod->getChannels(), false);

  this->setEncoding(audioDataEncoding);
}

#pragma endregion

#pragma region public

void Generator::setNextRowCallback(
    std::function<void(Generator &, ChangedRowEvent event)>
        callback) {
  this->_nextRowCallback = std::move(callback);
}

void Generator::setNextOrderCallback(
    std::function<void(mod::Generator &generator, ChangedOrderEvent event)>
        callback) {
  this->_nextOrderCallback = std::move(callback);
}

void Generator::setStateChangedCallback(
    std::function<void(Generator &, ChangedStateEvent event)>
        callback) {
  this->_stateChangedCallback = std::move(callback);
}

void Generator::setVolume(float volume) { this->_volume = volume; }

void Generator::setFrequency(float frequency) {
  if (frequency <= 1.0f) {
    throw std::invalid_argument(
        fmt::format("Frequency cannot be less than 0. Have {}", frequency));
  }

  this->_timePerRow = this->calculateTimePerRow(frequency, 6.0f);
//  this->_timePerRow = (size_t)(450.0f * 6.0f / (11025 * 2.0f) * frequency);
  //  this->_timePerRow = frequency;

  this->_frequency = frequency;
}

void Generator::setMod(std::shared_ptr<Mod> mod) {
  this->_mod = std::move(mod);
  this->_channelsStates.resize(this->_mod->getChannels());

  this->resetState();
}

std::shared_ptr<Mod> Generator::getMod() { return this->_mod; }

std::shared_ptr<const Mod> Generator::getMod() const { return this->_mod; }

Row &Generator::getRow(size_t index) {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  Pattern &pattern = this->getCurrentPattern();

  return pattern.getRow(index);
}

const Row &Generator::getRow(size_t index) const {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  const Pattern &pattern = this->getCurrentPattern();

  return pattern.getRow(index);
}

Row &Generator::getCurrentRow() {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  Pattern &pattern = this->getCurrentPattern();

  return pattern.getRow(this->_currentRowIndex);
}

const Row &Generator::getCurrentRow() const {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  const Pattern &pattern = this->getCurrentPattern();

  return pattern.getRow(this->_currentRowIndex);
}

Pattern &Generator::getCurrentPattern() {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  const int &patternIndex = this->_mod->getOrders()[this->_currentOrderIndex];

  return this->_mod->getPatterns()[patternIndex];
}

const Pattern &Generator::getCurrentPattern() const {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  const int &patternIndex = this->_mod->getOrders()[this->_currentOrderIndex];

  return this->_mod->getPatterns()[patternIndex];
}

void Generator::stop() {
  this->_setState(GeneratorState::Paused);
  this->_setOrderAndRowIndex(0, 0);
  this->_timePassed = 0;
  this->_rowPlayed = false;
}

void Generator::restart() {
  this->_setState(GeneratorState::Playing);
  this->_setOrderAndRowIndex(0, 0);
  this->_timePassed = 0;
  this->_rowPlayed = false;
}

void Generator::pause() { this->_setState(GeneratorState::Paused); }

void Generator::start() { this->_setState(GeneratorState::Playing); }

void Generator::generate(uint8_t *data, size_t size) {
  if (this->_convertor == nullptr) {
    throw BadStateException("generate: Audio encoding was not set.");
  }

  if (this->_mod == nullptr) {
    throw BadStateException("generate: Mod was not set.");
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
    size_t next;
    if (this->_timePassed % this->_timePerRow == 0) {
      next = std::min(current + this->_timePerRow, this->_buffer.size());
    } else {
      next = std::min(this->_timePerRow - this->_timePassed % this->_timePerRow,
                      this->_buffer.size());
    }

    const std::vector<Pattern> &patterns = this->_mod->getPatterns();
    const std::vector<int> &orders = this->_mod->getOrders();

    int currentOrder = orders[this->_currentOrderIndex];
    const Pattern &currentPattern = patterns[currentOrder];

    const Row &currentRow = currentPattern.getRow(this->_currentRowIndex);

    if (!this->_rowPlayed) {
      for (const auto &note : currentRow.getNotes()) {
        if (note.effectNumber == 0xF) {
          this->_timePerRow = this->calculateTimePerRow(this->_frequency, note.effectParameter);
        }
      }
    }

    for (auto channelIndex = 0; channelIndex < this->_mod->getChannels();
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
        this->pause();
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
    //    this->_convertor(0.5f,
    //                     dataPtr);
    dataPtr += this->_bytesInEncoding;
  }

  std::memset(this->_buffer.data(), 0, this->_buffer.size() * sizeof(float));
}

void Generator::setEncoding(Encoding audioDataEncoding) {
  switch (audioDataEncoding) {
    case Encoding::Signed16:
      this->_convertor = &dataconvertors::convertToS16;
      this->_bytesInEncoding = bytesInEncoding(audioDataEncoding);
      break;
    case Encoding::Unsigned16:
      this->_convertor = &dataconvertors::convertToU16;
      this->_bytesInEncoding = bytesInEncoding(audioDataEncoding);
      break;
    case Encoding::Signed8:
      this->_convertor = &dataconvertors::convertToS8;
      this->_bytesInEncoding = bytesInEncoding(audioDataEncoding);
      break;
    case Encoding::Unsigned8:
      this->_convertor = &dataconvertors::convertToU8;
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
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  if (index >= this->_mod->getOrders().size()) {
    const std::string message = fmt::format(
        "setCurrentOrder: Tried to set order out of range: {}. Total orders: "
        "{}",
        index, this->_mod->getOrders().size());

    throw std::out_of_range(message);
  }

  this->resetState();
  this->_setOrderIndex(index);
}

void Generator::setCurrentRow(size_t index) {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  const auto &patternIndex = this->_mod->getOrders()[this->_currentOrderIndex];
  const auto &pattern = this->_mod->getPatterns()[patternIndex];

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
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

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
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

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
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

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
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  for (auto &&b : this->_mutedChannels) {
    b = false;
  }
}

void Generator::muteAll() {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  for (auto &&b : this->_mutedChannels) {
    b = true;
  }
}

bool Generator::isMuted(size_t channelIndex) {
  if (this->_mod == nullptr) {
    throw BadStateException("Mod was not set.");
  }

  if (channelIndex >= this->_mutedChannels.size()) {
    const std::string message = fmt::format(
        "mute: Tried to set channel mute out of range: {}. Total channels: "
        "{}",
        channelIndex, this->_mutedChannels.size());

    throw std::out_of_range(message);
  }

  return this->_mutedChannels[channelIndex];
}

#pragma endregion

}  // namespace mod