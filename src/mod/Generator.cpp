#include "Generator.h"

#include <fmt/format.h>

#include <bitset>
#include <iostream>
#include <sstream>

#include "InfoString.h"

namespace mod {

void Generator::convertToU8(const float &value, uint8_t *target) {
  constexpr int maxSigned = 0x80 - 1;
  constexpr float maxSignedFloat = 0x80 - 1;

  *target = (uint8_t)((int)(value * maxSignedFloat) + maxSigned);
}

void Generator::convertToS8(const float &value, uint8_t *target) {
  constexpr float maxSignedFloat = 0x80 - 1;

  *(uint8_t *)target = (int8_t)((int)(value * maxSignedFloat));
}

void Generator::convertToU16(const float &value, uint8_t *target) {
  constexpr int maxSigned16 = 0x8000 - 1;
  constexpr float maxSigned16Float = 0x8000 - 1;

  *(uint16_t *)target =
      (uint16_t)((int)(value * maxSigned16Float) + maxSigned16);
}

void Generator::convertToS16(const float &value, uint8_t *target) {
  constexpr float maxSigned16Float = 0x8000 - 1;

  *(uint16_t *)target = (int16_t)((int)(value * maxSigned16Float));
}

void Generator::mix(const float &src, float &dest) {
  //  dest = src + dest;
  //
  //  if (dest > 1.0f) {
  //    dest = 1.0f;
  //  }
  //  if (dest < -1.0f) {
  //    dest = -1.0f;
  //  }

  //  dest = (src + dest) - src * dest * std::signum(src + dest)
  const float sum = src + dest;

  dest =
      (sum)-src * dest * ((sum > 0.0f) ? 1.0f : ((sum < 0.0f) ? -1.0f : 0.0f));
  //  dest = sum;
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

  this->_currentRowIndex++;

  if (this->_currentRowIndex >= currentPattern->getTotalRows()) {
    this->_currentRowIndex = 0;
    this->_currentOrderIndex++;

    if (this->_currentOrderIndex >= this->_mod.getSongLength()) {
      return true;
    }
  }

  return false;
}

std::string intToNote(int i) {
  switch (i) {
    case 0:
      return "C-";
    case 1:
      return "C#";
    case 2:
      return "D-";
    case 3:
      return "D#";
    case 4:
      return "E-";
    case 5:
      return "F-";
    case 6:
      return "F#";
    case 7:
      return "G-";
    case 8:
      return "G#";
    case 9:
      return "A-";
    case 10:
      return "A#";
    case 11:
      return "B-";
    default:
      return "??";
  }
}

std::string Generator::fancyRow(const Row &row) const {
  constexpr size_t totalNotes = 12;
  std::stringstream stringstream;
  static const int originalPeriods[] = {
      1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 906,
      856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480, 453,
      428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240, 226,
      214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120, 113,
      107,  101,  95,   90,   85,   80,   75,   71,   67,   63,   60,  56};

  stringstream << "|";
  for (const auto &note : row.getNotes()) {
    if (note.sampleIndex == 0) {
      stringstream << ".....|";
      continue;
    }

    std::string noteString = "??";
    for (size_t i = 0; i < sizeof(originalPeriods); i++) {
      if (originalPeriods[i] == note.samplePeriodFrequency) {
        noteString = intToNote(i % totalNotes) + (char)('0' + (i / 12) + 2);
        break;
      }
    }

    stringstream << noteString;
    stringstream << "." << note.sampleIndex;
    stringstream << "|";
  }
  return stringstream.str();
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
                           (11025.0f * 2.0f);
    }
  }

  const auto &sampleIndex = channelState.sampleIndex;

  if (sampleIndex == 0) {
    return;
  }

  const Sample &sample = this->_mod.getSamples()[sampleIndex - 1];
  auto &sampleTime = channelState.sampleTime;
  const std::vector<float> &sampleData = sample.getData();

  size_t dataIndex2 = 0;

  if (note.effectNumber == 0xC) {
    channelState.volume = (float)note.effectParameter / 64.0f;
  }

  float sampleVolume = (float)sample.getVolume() / 64.0f * channelState.volume;

  for (auto i = start; i < end; i++) {
    size_t sampleDataIndex =
        sampleTime + (size_t)((float)dataIndex2 * channelState.pitch);

    if (sampleDataIndex >= sampleData.size()) {
      if (sample.getRepeatLength() == 0) {
        channelState = {};
        break;
      } else {
        channelState.sampleTime = sample.getRepeatPoint();
        // Todo
        dataIndex2 = 0;
        sampleDataIndex =
            sampleTime + (size_t)((float)dataIndex2 * channelState.pitch);
      }
    }


    data[i] += sampleData[sampleDataIndex] * sampleVolume / 4;

    dataIndex2++;
  }

  if (sampleIndex == 0) {
    return;
  }

  //    sampleTime += sampleTimeIncrement;
  sampleTime += (size_t)((float)dataIndex2 * channelState.pitch);

  //  this->_perChannelTime[channelIndex] += data.size();
}

Generator::Generator(Mod mod, Encoding audioDataEncoding)
    : _mod(std::move(mod)), _audioDataEncoding(audioDataEncoding) {
  this->_channelsStates.resize(this->_mod.getChannels());
  this->_mutedChannels.resize(this->_mod.getChannels(), false);

  this->setEncoding(audioDataEncoding);
}

void Generator::setMod(Mod mod) {
  this->_mod = std::move(mod);
  this->_channelsStates.resize(this->_mod.getChannels());

  this->resetState();
}

void Generator::stop() {
  this->_currentOrderIndex = 0;
  this->_currentRowIndex = 0;
  this->_timePassed = 0;
  this->_generatorState = GeneratorState::Paused;
  this->_rowPlayed = false;
}

void Generator::restart() {
  this->stop();
  this->start();
}

void Generator::pause() { this->_generatorState = GeneratorState::Paused; }

void Generator::start() { this->_generatorState = GeneratorState::Playing; }

void Generator::generate(uint8_t *data, size_t size, float volume) {
  if (this->_convertor == nullptr) {
    throw std::logic_error("generate: Audio encoding was not set.");
  }

  if (this->_generatorState == GeneratorState::Paused) {
    return;
  }

  if (this->_buffer.size() != (size / this->_bytesInEncoding)) {
    this->_buffer.resize(size / this->_bytesInEncoding);
  }

  for (size_t current = 0; current < size;) {
    size_t next = std::min(current + this->_timePerRow, size);

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

    current = next;

    if (((this->_timePassed % this->_timePerRow) + current) >=
        this->_timePerRow) {
      this->_rowPlayed = false;
      std::cout << this->fancyRow(currentRow) << std::endl;
      if (this->advanceIndexes()) {
        this->stop();
        break;
      }
    }
  }

  uint8_t *dataPtr = data;
  for (float &i : this->_buffer) {
    this->_convertor(i * volume, dataPtr);
    dataPtr += this->_bytesInEncoding;
  }

  this->_timePassed += size;

  std::memset(this->_buffer.data(), 0, this->_buffer.size() * sizeof(float));

//  size_t byteCount = ((char *)&this->_buffer[this->_buffer.size() - 1] - (char *)&this->_buffer[0]);



  //  std::cout << "Passed: " << this->_timePassed
  //            << "; Wrapped: " << (this->_timePassed % this->_timePerRow)
  //            << "; Row: " << this->_currentRowIndex
  //            << "; Order: " << this->_currentOrderIndex << std::endl;
  //  std::cout << "Prev: " << this->_previousSampleIndex[channelIndex]
  //            << "; Chan time: " << this->_perChannelTime[channelIndex]
  //            << std::endl;
  //  std::cout << InfoString::toString(note)
  //            << std::endl;
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
  this->_currentOrderIndex = index;
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
  this->_currentRowIndex = index;
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