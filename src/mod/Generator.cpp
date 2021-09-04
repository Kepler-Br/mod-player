#include "Generator.h"

#include <iostream>
#include <sstream>

#include "InfoString.h"

namespace mod {

bool Generator::advanceIndexes() {
  if (this->_generatorState == GeneratorState::Paused) {
    return false;
  }

  const std::vector<Pattern> &patterns = this->_mod.getPatterns();
  const std::vector<int> &orders = this->_mod.getOrders();

  if (this->_currentOrderIndex > this->_mod.getSongLength()) {
    return true;
  }

  int currentOrder = orders[this->_currentOrderIndex];
  const Pattern *currentPattern = &patterns[currentOrder];

  this->_currentRowIndex++;

  if (this->_currentRowIndex >= currentPattern->getTotalRows()) {
    this->_currentRowIndex = 0;
    this->_currentOrderIndex++;

    if (this->_currentOrderIndex > this->_mod.getSongLength()) {
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
    if (note.sampleNumber == 0) {
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
    stringstream << "." << note.sampleNumber;
    stringstream << "|";
  }
  return stringstream.str();
}

void Generator::generateByChannel(uint8_t *data, size_t size, const Row &row, size_t channelIndex) {
  const Note &note = row.getNote(channelIndex);

  if (note.sampleNumber != 0) {
    this->_perChannelTime[channelIndex] = 0;
    this->_previousSampleIndex[channelIndex] = note.sampleNumber;
  }

  //  std::cout << note.sampleNumber << std::endl;

  if (this->_previousSampleIndex[channelIndex] != 0) {
    const auto &sampleIndex = this->_previousSampleIndex[channelIndex];
    const Sample &sample = this->_mod.getSamples()[sampleIndex];
    //    const Sample &sample = this->_mod.getSamples()[0];
    const auto &sampleTime = this->_perChannelTime[channelIndex];

    const std::vector<uint8_t> &sampleData = sample.getData();
    for (auto i = 0; i < size; i++) {
      if ((sampleTime + i) < sampleData.size()) {
        data[i] = sampleData[sampleTime + i];
      }
    }

    if (sampleTime + size >= sampleData.size()) {
      this->_previousSampleIndex[channelIndex] = 0;
      this->_perChannelTime[channelIndex] = 0;
    }
  } else {
    for (auto i = 0; i < size; i++) {
      data[i] = 0x80;
    }
  }

  this->_perChannelTime[channelIndex] += size;
}

Generator::Generator(Mod mod) : _mod(std::move(mod)) {
  this->_perChannelTime.resize(this->_mod.getChannels());
  this->_previousSampleIndex.resize(this->_mod.getChannels());
}

void Generator::setMod(Mod mod) {
  if (mod.getAudioDataEncoding() == Encoding::Unknown) {
    throw std::invalid_argument("Mod with invalid encoding: " +
                                encodingToString(mod.getAudioDataEncoding()));
  }

  this->_mod = std::move(mod);
  this->_perChannelTime.resize(this->_mod.getChannels());
  this->_previousSampleIndex.resize(this->_mod.getChannels());
}

void Generator::stop() {
  this->_currentOrderIndex = 0;
  this->_currentRowIndex = 0;
  this->_timePassed = 0;
  this->_generatorState = GeneratorState::Paused;
}

void Generator::generate(uint8_t *data, size_t size) {
  if (this->_mod.getAudioDataEncoding() == Encoding::Unknown ||
      this->_generatorState == GeneratorState::Paused) {
    return;
  }

  const std::vector<Pattern> &patterns = this->_mod.getPatterns();
  const std::vector<int> &orders = this->_mod.getOrders();

  int currentOrder = orders[this->_currentOrderIndex];
  const Pattern &currentPattern = patterns[currentOrder];

  const Row &currentRow = currentPattern.getRow(this->_currentRowIndex);

  this->generateByChannel(data, size, currentRow, 1);

  if (((this->_timePassed % this->_timePerRow) + size + 1) >
      this->_timePerRow) {
    std::cout << this->fancyRow(currentRow) << std::endl;
    if (this->advanceIndexes()) {
      this->stop();
      return;
    }
  }

  this->_timePassed += size;


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

GeneratorState Generator::getState() const { return this->_generatorState; }

}  // namespace mod