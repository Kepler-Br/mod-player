#include "InfoString.h"

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

}  // namespace mod