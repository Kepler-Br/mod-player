#include "Reader.h"

#include <fmt/format.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "Serializer.h"

namespace mod {

float Reader::convertFromU8(const uint8_t *value) {
  constexpr int maxSignedByte = 0x80 - 1;
  constexpr float maxSignedByteFloat = 0x80 - 1;

  return (float)((int)(*value) - maxSignedByte) / maxSignedByteFloat;
}

float Reader::convertFromS8(const uint8_t *value) {
  constexpr float maxSignedByte = 0x80;

  return (float)(*(int8_t *)value) / maxSignedByte;
}

float Reader::convertFromU16(const uint8_t *value) {
  constexpr int maxSigned16 = 0x8000;
  constexpr float maxSigned16Float = 0x8000;

  const int unsignedValue = *(uint16_t *)value;

  return (float)(unsignedValue - maxSigned16) / maxSigned16Float;
}

float Reader::convertFromS16(const uint8_t *value) {
  constexpr float maxSigned16Float = 0x8000;

  const int signedValue = *(int16_t *)value;

  return (float)(signedValue) / maxSigned16Float;
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

  if (type == "M.K." || type == "FLT4") {
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

std::vector<Sample> Reader::readSamples(std::ifstream &stream) {
  if (!stream) {
    throw std::runtime_error("Samples reading error: stream bad.");
  }

  std::vector<Sample> samples;

  constexpr size_t samplesTotal = 31;
  samples.reserve(samplesTotal);

  for (int i = 0; i < samplesTotal; i++) {
    Sample sample = Serializer::sample(stream);

    samples.push_back(sample);
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

  float (*convertor)(const uint8_t *);

  if (audioDataEncoding == Encoding::Signed8) {
    convertor = &Reader::convertFromS8;
  } else if (audioDataEncoding == Encoding::Unsigned8) {
    convertor = &Reader::convertFromU8;
  } else if (audioDataEncoding == Encoding::Signed16) {
    convertor = &Reader::convertFromS16;
  } else if (audioDataEncoding == Encoding::Unsigned16) {
    convertor = &Reader::convertFromU16;
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
      sampleData[i] = convertor(&readData[i]);
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

Mod Reader::read(const std::string &path) {
  std::ifstream stream(path);

  if (!stream) {
    throw std::runtime_error("Mod reading error: cannot open file: " + path);
  }

  std::string name = Reader::readName(stream);
  std::vector<Sample> samples = Reader::readSamples(stream);

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
  patternsCount++;

  std::vector<Pattern> patterns = Reader::readPatterns(stream, patternsCount);

  Reader::readSamplesAudioData(stream, samples, Encoding::Signed8);

  return Mod(name, songLength, std::move(samples), std::move(patterns),
             std::move(orders));
}

}  // namespace mod
