#include "Serializer.h"

#include <array>

#include "fmt/format.h"

namespace mod {

const size_t Serializer::noteDataSize = 4;

uint16_t Serializer::swapEndian(uint16_t val) {
  return (val >> 8) | (val << 8);
}

uint16_t Serializer::readU16(std::istream &stream) {
  uint16_t value;

  stream.read((char *)&value, sizeof(uint16_t));

  return value;
}

uint16_t Serializer::readU16Big(std::istream &stream) {
  uint16_t value = Serializer::readU16(stream);

  return Serializer::swapEndian(value);
}

uint8_t Serializer::readU8(std::istream &stream) {
  uint8_t value;

  stream.read((char *)&value, sizeof(uint8_t));

  return value;
}

Note Serializer::note(std::istream &stream) {
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

Row Serializer::row(std::istream &stream, size_t channels) {
  if (!stream) {
    throw std::runtime_error("Cannot serialize row: stream bad");
  }

  Row outRow(channels);

  for (auto i = 0; i < channels; i++) {
    Note readNote = Serializer::note(stream);

    outRow.addNote(readNote);
  }

  return outRow;
}

Pattern Serializer::pattern(std::istream &stream, size_t channels,
                            size_t totalRows) {
  if (!stream) {
    throw std::runtime_error("Cannot serialize pattern: stream bad");
  }

  Pattern outPattern(channels, totalRows);

  for (auto i = 0; i < totalRows; i++) {
    Row readRow = Serializer::row(stream, channels);

    outPattern.addRow(readRow);
  }

  return outPattern;
}

Sample Serializer::sample(std::istream &stream) {
  if (!stream) {
    throw std::runtime_error("Cannot serialize sample: stream bad");
  }

  constexpr size_t nameLength = 22;
  std::string sampleName(nameLength + 1, '\0');

  stream.read(sampleName.data(), nameLength);
  sampleName.resize(sampleName.find('\0'));

  int length = Serializer::readU16Big(stream);
  length *= 2;

  uint8_t finetune = readU8(stream);

  uint8_t volume = readU8(stream);

  int repeatPoint = Serializer::readU16Big(stream);
  repeatPoint *= 2;

  int repeatLength = Serializer::readU16Big(stream);
  if (repeatLength == 1) {
    repeatLength = 0;
  } else {
    repeatLength *= 2;
  }

  if (!stream) {
    throw std::runtime_error("Cannot serialize sample: stream gone bad");
  }

  return {sampleName, length, finetune, volume, repeatPoint, repeatLength, 8363.0f};
}

}  // namespace mod
