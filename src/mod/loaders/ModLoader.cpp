#include "ModLoader.h"

#include <fmt/format.h>

#include <array>
#include <fstream>

#include "DataConvertors.h"
#include "StreamUtils.h"

namespace mod {

#pragma region private static

Note ModLoader::serializeNote(std::istream &stream) {
  constexpr size_t noteDataSize = 4;

  std::array<uint8_t, noteDataSize> data{};

  if (!stream) {
    throw std::runtime_error("Cannot serialize note: stream bad");
  }

  stream.read((char *)data.data(), data.size());

  if (!stream) {
    const std::string message = fmt::format(
        "Cannot serialize note: stream gone bad after trying to read {} bytes",
        noteDataSize);

    throw std::runtime_error(message);
  }

  int sampleNumber = (data[0] & 0xF0) | (data[2] >> 4);
  int samplePeriodFrequency = ((data[0] & 0xF) << 8) | data[1];
  int effectNumber = (data[2] & 0xF);
  int effectParameter = data[3];

  Note outNote{};

  outNote.sampleIndex = sampleNumber;
  outNote.samplePeriodFrequency = samplePeriodFrequency;
  outNote.effectNumber = effectNumber;
  outNote.effectParameter = effectParameter;

  return outNote;
}

Row ModLoader::serializeRow(std::istream &stream, size_t channels) {
  if (!stream) {
    throw std::runtime_error("Cannot serialize row: stream bad");
  }

  Row outRow(channels);

  for (auto i = 0; i < channels; i++) {
    Note readNote = ModLoader::serializeNote(stream);

    outRow.addNote(readNote);
  }

  return outRow;
}

Pattern ModLoader::serializePattern(std::istream &stream, size_t channels,
                                    size_t totalRows) {
  if (!stream) {
    throw std::runtime_error("Cannot serialize pattern: stream bad");
  }

  Pattern outPattern(channels, totalRows);

  for (auto i = 0; i < totalRows; i++) {
    Row readRow = ModLoader::serializeRow(stream, channels);

    outPattern.addRow(readRow);
  }

  return outPattern;
}

Sample ModLoader::serializeSample(std::istream &stream) {
  if (!stream) {
    throw std::runtime_error("Cannot serialize sample: stream bad");
  }

  constexpr size_t nameLength = 22;
  std::string sampleName(nameLength + 1, '\0');

  stream.read(sampleName.data(), nameLength);
  sampleName.resize(sampleName.find('\0'));

  int length = streamutils::readU16Swapped(stream);

  length *= 2;

  uint8_t finetune = streamutils::readU8(stream);

  uint8_t volume = streamutils::readU8(stream);

  int repeatPoint = streamutils::readU16Swapped(stream);
  repeatPoint *= 2;

  int repeatLength = streamutils::readU16Swapped(stream);

  if (repeatLength == 1) {
    repeatLength = 0;
  } else {
    repeatLength *= 2;
  }

  if (!stream) {
    throw std::runtime_error("Cannot serialize sample: stream gone bad");
  }

  return {sampleName,  length,       finetune, volume,
          repeatPoint, repeatLength, 8363.0f};
}

size_t ModLoader::getChannels(std::istream &stream) {
  if (!stream) {
    throw std::runtime_error("File stream bad.");
  }

  std::ifstream::pos_type pos = stream.tellg();

  stream.seekg(1080, std::ios_base::beg);

  std::string type(5, '\0');

  stream.read(type.data(), 4);
  stream.seekg(pos, std::ios_base::beg);

  if (!stream) {
    throw std::runtime_error("File stream gone bad.");
  }

  if (std::strcmp(type.data(), "M.K.") == 0 ||
      std::strcmp(type.data(), "FLT4") == 0) {
    return 4;
  } else if (std::strcmp(type.data(), "6CHN") == 0) {
    return 6;
  } else if (std::strcmp(type.data(), "8CHN") == 0) {
    return 8;
  }

  try {
    if (std::strcmp(type.data() + 2, "CH") == 0) {
      return std::stoi(type);
    }
  } catch (const std::invalid_argument &) {
    // Will throw anyway, but sonar won't shut up about ignored exception.
    throw std::runtime_error(
        fmt::format("Unknown mod type format: '{}'", type));
  }

  throw std::runtime_error(fmt::format("Unknown mod type format: '{}'", type));
}

std::vector<Sample> ModLoader::readSamples(std::istream &stream) {
  if (!stream) {
    throw std::runtime_error("Samples reading error: stream bad.");
  }

  std::vector<Sample> samples;

  constexpr size_t samplesTotal = 31;
  samples.reserve(samplesTotal);

  for (int i = 0; i < samplesTotal; i++) {
    Sample sample = ModLoader::serializeSample(stream);

    samples.push_back(sample);
  }

  return samples;
}

std::vector<Pattern> ModLoader::readPatterns(std::istream &stream,
                                             size_t patternsNumber) {
  if (!stream) {
    throw std::runtime_error("Patterns reading error: stream bad.");
  }

  std::vector<Pattern> patterns;
  size_t channels = ModLoader::getChannels(stream);

  patterns.reserve(patternsNumber);

  for (auto i = 0; i < patternsNumber; i++) {
    Pattern pattern = ModLoader::serializePattern(stream, channels, 64);

    patterns.push_back(pattern);
  }

  return patterns;
}

void ModLoader::readSamplesAudioData(std::istream &stream,
                                     std::vector<Sample> &samples,
                                     Encoding audioDataEncoding) {
  if (!stream) {
    throw std::runtime_error("Sample audio data reading error: stream bad.");
  }

  void (*convertor)(const uint8_t *, float &);

  if (audioDataEncoding == Encoding::Signed8) {
    convertor = &dataconvertors::convertFromS8;
  } else if (audioDataEncoding == Encoding::Unsigned8) {
    convertor = &dataconvertors::convertFromU8;
  } else if (audioDataEncoding == Encoding::Signed16) {
    convertor = &dataconvertors::convertFromS16;
  } else if (audioDataEncoding == Encoding::Unsigned16) {
    convertor = &dataconvertors::convertFromU16;
  } else {
    throw std::invalid_argument(
        "readSamplesAudioData: unknown audio data encoding: " +
        encodingToString(audioDataEncoding));
  }

  for (auto &sample : samples) {
    sample.reserveData();

    std::vector<uint8_t> readData(sample.getLength());

    stream.read((char *)readData.data(), sample.getLength());

    if (!stream) {
      throw std::runtime_error(
          "Sample audio data reading error: stream gone bad.");
    }

    std::vector<float> &sampleData = sample.getData();

    for (auto i = 0; i < sample.getLength(); i++) {
      convertor(&readData[i], sampleData[i]);
    }
  }
}

std::vector<int> ModLoader::readOrders(std::istream &stream) {
  if (!stream) {
    throw std::runtime_error("Orders reading error: stream bad.");
  }

  constexpr size_t totalOrders = 128;

  std::vector<uint8_t> orders;

  orders.resize(totalOrders);
  stream.read((char *)orders.data(), totalOrders);

  if (!stream) {
    throw std::runtime_error("Orders reading error: stream gone bad.");
  }

  std::vector<int> convertedOrders;
  convertedOrders.reserve(totalOrders);

  for (auto order : orders) {
    convertedOrders.push_back((int)order);
  }

  return convertedOrders;
}

std::string ModLoader::readName(std::istream &stream) {
  if (!stream) {
    throw std::runtime_error("Mod name reading error: stream is bad.");
  }

  std::string name(21, '\0');

  stream.read(name.data(), 20);

  if (!stream) {
    throw std::runtime_error("Mod name reading error: stream gone bad.");
  }

  name.resize(name.find('\0'));

  return name;
}

#pragma endregion

std::shared_ptr<Mod> ModLoader::load(std::istream &stream) {
  if (!stream) {
    throw std::runtime_error("Mod reading error: stream bad");
  }

  std::string name = ModLoader::readName(stream);
  std::vector<Sample> samples = ModLoader::readSamples(stream);

  uint8_t byte;

  stream.read((char *)(&byte), 1);
  size_t songLength = (int)byte;

  uint8_t unused;
  stream.read((char *)(&unused), 1);

  std::vector<int> orders = ModLoader::readOrders(stream);

  stream.seekg(4, std::ios_base::cur);

  int patternsCount = 0;

  for (auto order : orders) {
    if (order > patternsCount) {
      patternsCount = order;
    }
  }
  patternsCount++;

  std::vector<Pattern> patterns =
      ModLoader::readPatterns(stream, patternsCount);

  ModLoader::readSamplesAudioData(stream, samples, Encoding::Signed8);

  return std::make_shared<Mod>(name, songLength, std::move(samples),
                               std::move(patterns), std::move(orders));
}

std::shared_ptr<Mod> ModLoader::load(const std::string &path) {
  std::ifstream stream(path);

  return this->load(stream);
}

}  // namespace mod