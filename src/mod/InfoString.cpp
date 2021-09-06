#include "InfoString.h"

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
//  const std::string filler = "..........";
  const std::string filler = colors.brightWhite + "..." + colors.blue + ".." + colors.green + ".." + colors.magenta + "." + colors.yellow + ".." + colors.reset;
  const std::streampos fillerLen = (std::streampos)filler.size();

  for (const auto& note : row.getNotes()) {
    if (note.sampleIndex == 0) {
      stringstream << filler << "|";
      continue;
    }

    std::string noteString = "??";
    for (size_t i = 0; i < sizeof(originalPeriods); i++) {
      if (originalPeriods[i] == note.samplePeriodFrequency) {
        noteString = intToNote(i % totalNotes) + (char)('0' + (i / 12) + 2);
        break;
      }
    }
    size_t notVisibleSymbols = 0;

    std::streampos cur = stringstream.tellp();
//    stringstream << background.brightWhite;
    stringstream << colors.brightWhite;
    notVisibleSymbols += colors.brightWhite.size();
    stringstream << noteString;

    stringstream << colors.blue;
    notVisibleSymbols += colors.blue.size();
    if (note.sampleIndex < 10) {
      stringstream << '.';
    }
    stringstream << note.sampleIndex;

    stringstream << colors.green;
    notVisibleSymbols += colors.green.size();
    stringstream << "..";

    stringstream << colors.magenta;
    notVisibleSymbols += colors.magenta.size();
    std::hex(stringstream);

    if (note.effectNumber != 0x0) {
      stringstream << note.effectParameter;
    } else {
      stringstream << ".";
    }

    stringstream << colors.yellow;
    notVisibleSymbols += colors.yellow.size();

    if (note.effectParameter != 0x0) {
      if (note.effectParameter <= 0xF) {
        stringstream << "0";
      }
      stringstream << note.effectParameter;
    } else {
      stringstream << "..";
    }

    std::dec(stringstream);

    stringstream << colors.reset;
    notVisibleSymbols += colors.reset.size();
    std::size_t written = stringstream.tellp() - cur;
    if (written <= filler.size()) {
      stringstream << std::string(filler.size() - written, '.');
    }
    stringstream << "|";
  }

  return stringstream.str();
}

}  // namespace mod