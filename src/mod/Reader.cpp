#include "Reader.h"

#include <fmt/format.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "Serializer.h"

namespace mod {

uint8_t Reader::convertS8ToU8(uint8_t value) {
  constexpr uint8_t halfByte = 128;

  int8_t signedByte = *((int8_t *)&value);

  signedByte += halfByte;

  return *((uint8_t *)&signedByte);
}

uint16_t Reader::convertS8ToU16(uint8_t value) {
  uint16_t converted = convertS8ToU8(value);

  return converted << 8;
}

int16_t Reader::convertS8ToS16(uint8_t value) {
  return (int16_t)(*(int8_t *)&value) * (int16_t)0x100;
}

size_t Reader::getChannels(std::ifstream &stream) {
  if (!stream) {
    throw std::runtime_error("File stream bad.");
  }

  std::ifstream::pos_type pos = stream.tellg();
  stream.seekg(1080, std::ios_base::beg);

  std::string type(4, '\0');

  stream.read(type.data(), 4);
  stream.seekg(pos, std::ios_base::beg);

  if (!stream) {
    throw std::runtime_error("File stream gone bad.");
  }

  if (type == "M.K.") {
    return 4;
  } else if (type == "6CHN") {
    return 6;
  } else if (type == "8CHN") {
    return 8;
  }

  try {
    if (std::strcmp(type.data() + 2, "CH") == 0) {
      return std::stoi(type);
    }
  } catch (std::invalid_argument &e) {
    // Will throw anyway.
  }

  throw std::runtime_error(fmt::format("Unknown mod type format: '{}'", type));
}

std::vector<Sample> Reader::readSamples(std::ifstream &stream, Encoding audioDataEncoding) {
  if (!stream) {
    throw std::runtime_error("Samples reading error: stream bad.");
  }

  std::vector<Sample> samples;

  constexpr size_t samplesTotal = 31;
  samples.reserve(samplesTotal);

  for (int i = 0; i < samplesTotal; i++) {
    Sample sample = Serializer::sample(stream, audioDataEncoding);

    if (sample.getLength() != 0) {
      samples.push_back(sample);
    }
  }

  return samples;
}

std::vector<Pattern> Reader::readPatterns(std::ifstream &stream,
                                          size_t patternsNumber) {
  if (!stream) {
    throw std::runtime_error("Patterns reading error: stream bad.");
  }

  std::vector<Pattern> patterns;
  size_t channels = Reader::getChannels(stream);

  patterns.reserve(patternsNumber);

  for (auto i = 0; i < patternsNumber; i++) {
    Pattern pattern = Serializer::pattern(stream, channels, 64);

    patterns.push_back(pattern);
  }

  return patterns;
}

void Reader::readSamplesAudioData(std::ifstream &stream,
                                  std::vector<Sample> &samples,
                                  Encoding audioDataEncoding) {
  if (!stream) {
    throw std::runtime_error("Sample audio data reading error: stream bad.");
  }

  for (auto &sample : samples) {
    sample.reserveData();

    if (audioDataEncoding == Encoding::Signed8) {
      std::vector<uint8_t> &sampleData = sample.getData();

      stream.read((char *)sampleData.data(), sample.getLength());
    } else {
      std::vector<uint8_t> readData(sample.getLength());

      stream.read((char *)readData.data(), sample.getLength());

      std::vector<uint8_t> &sampleData = sample.getData();

      size_t index = 0;
      for (auto byteRead : readData) {
        if (audioDataEncoding == Encoding::Unsigned8) {
          sampleData[index] = Reader::convertS8ToU8(byteRead);

          index++;
        } else if (audioDataEncoding == Encoding::Unsigned16) {
          uint16_t converted = Reader::convertS8ToU16(byteRead);

          sampleData[index] = ((uint8_t *)&converted)[0];
          sampleData[index+1] = ((uint8_t *)&converted)[1];

          index += 2;
        } else if (audioDataEncoding == Encoding::Signed16) {
          int16_t converted = Reader::convertS8ToS16(byteRead);

          sampleData[index] = ((uint8_t *)&converted)[0];
          sampleData[index+1] = ((uint8_t *)&converted)[1];

          index += 2;
        }
      }
    }

    if (!stream) {
      throw std::runtime_error(
          "Sample audio data reading error: stream gone bad.");
    }
  }
}

std::vector<int> Reader::readOrders(std::ifstream &stream) {
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

std::string Reader::readName(std::ifstream &stream) {
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

Mod Reader::read(const std::string &path, Encoding encoding) {
  std::ifstream stream(path);

  if (!stream) {
    throw std::runtime_error("Mod reading error: cannot open file: " + path);
  }

  std::string name = Reader::readName(stream);
  std::vector<Sample> samples = Reader::readSamples(stream, encoding);

  uint8_t byte;

  stream.read((char *)(&byte), 1);
  size_t songLength = (int)byte;

  uint8_t unused;
  stream.read((char *)(&unused), 1);

  std::vector<int> orders = Reader::readOrders(stream);

  stream.seekg(4, std::ios_base::cur);

  int patternsCount = 0;

  for (auto order : orders) {
    if (order > patternsCount) {
      patternsCount = order;
    }
  }
//  patternsCount++;

  std::vector<Pattern> patterns = Reader::readPatterns(stream, patternsCount);

  Reader::readSamplesAudioData(stream, samples, encoding);

  return Mod(name, songLength, encoding, std::move(samples),
             std::move(patterns), std::move(orders));
}

}  // namespace mod
