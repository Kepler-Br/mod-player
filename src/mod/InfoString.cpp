#include "InfoString.h"

#include <iomanip>
#include <sstream>

#include "fmt/format.h"

namespace mod {

std::string InfoString::toString(const Note& note) {
  return fmt::format(
      "Effect parameter: {}\nEffect number: {}\n"
      "Sample period frequency: {}\nSample number: {}",
      note.effectParameter, note.effectNumber, note.samplePeriodFrequency,
      note.sampleIndex);
}

std::string InfoString::toString(const Row& row) {
  return fmt::format("Channels: {}", row.getChannels());
}

std::string InfoString::toString(const Pattern& pattern) {
  return fmt::format("Channels: {}\nTotal rows: {}\nOccupied rows: {}",
                     pattern.getChannels(), pattern.getTotalRows(),
                     pattern.getOccupiedRows());
}

std::string InfoString::toString(const Sample& sample) {
  return fmt::format(
      "Repeat length: {}\nRepeat point: {}\nVolume: {}\nFinetune: {}\n"
      "Length: {}\nName: {}",
      sample.getRepeatLength(), sample.getRepeatPoint(), sample.getVolume(),
      sample.getFinetune(), sample.getLength(), sample.getName());
}

std::string InfoString::toString(const Mod& mod) {
  return fmt::format(
      "Name: {}\nChannels: {}\nSong length: {}\nSamples: {}\nPatterns: {}",
      mod.getName(), mod.getChannels(), mod.getSongLength(),
      mod.getSampleCount(), mod.getPatternCount());
}

std::string InfoString::intToNote(int i) {
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

InfoString::UnixColors InfoString::colors{};
InfoString::UnixColorsBackground InfoString::background{};

std::string InfoString::fancyRow(const Row& row) {
  constexpr size_t totalNotes = 12;
  std::stringstream stringstream;

  static const int originalPeriods[] = {
      1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 906,
      856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480, 453,
      428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240, 226,
      214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120, 113,
      107,  101,  95,   90,   85,   80,   75,   71,   67,   63,   60,  56};

  stringstream << "|";
  const std::string filler = colors.brightWhite + "..." + colors.blue + ".." +
                             colors.green + ".." + colors.magenta + "." +
                             colors.yellow + ".." + colors.reset;
  const std::streampos fillerLen = (std::streampos)filler.size();
  const auto originalPeriodsSize = sizeof(originalPeriods) / sizeof(int);

  for (const auto& note : row.getNotes()) {
    std::streampos cur = stringstream.tellp();
    stringstream << colors.brightWhite;
    if (note.sampleIndex != 0) {
      std::string noteString = "???";

      for (size_t i = 0; i < originalPeriodsSize; i++) {
        if (originalPeriods[i] == note.samplePeriodFrequency) {
          noteString = intToNote(i % totalNotes) + (char)('0' + (i / 12) + 2);
          break;
        }
      }

      stringstream << noteString;
    } else {
      stringstream << "...";
    }

    stringstream << colors.blue;

    if (note.sampleIndex != 0) {
      stringstream << std::setfill('0') << std::setw(2);
      stringstream << note.sampleIndex;
    } else {
      stringstream << "..";
    }

    stringstream << colors.green;
    stringstream << "..";

    stringstream << colors.magenta;
    stringstream << std::hex;

    if (note.effectNumber != 0x0) {
      stringstream << note.effectNumber;
    } else {
      stringstream << ".";
    }

    stringstream << colors.yellow;

    if (note.effectNumber != 0x0) {
      stringstream << std::hex << std::setfill('0') << std::setw(2);
      stringstream << note.effectParameter;
    } else {
      stringstream << "..";
    }

    stringstream << std::dec;

    stringstream << colors.reset;

    std::size_t written = stringstream.tellp() - cur;
    if (written <= filler.size()) {
      stringstream << std::string(filler.size() - written, '.');
    }
    stringstream << "|";
  }

  return stringstream.str();
}

}  // namespace mod
