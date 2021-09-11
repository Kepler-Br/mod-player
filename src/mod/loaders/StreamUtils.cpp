#include "StreamUtils.h"

#include "DataConvertors.h"

namespace mod::streamutils {

#pragma region read swapped

uint16_t readU16Swapped(std::istream& stream) {
  uint16_t value = readU16(stream);

  return dataconvertors::swapEndian(value);
}

uint32_t readU32Swapped(std::istream& stream) {
  uint32_t value = readU32(stream);

  return dataconvertors::swapEndian(value);
}

uint64_t readU64Swapped(std::istream& stream) {
  uint64_t value = readU64(stream);

  return dataconvertors::swapEndian(value);
}

#pragma endregion

#pragma region read

uint8_t readU8(std::istream& stream) {
  uint8_t value;

  stream.read((char*)&value, sizeof(uint8_t));

  return value;
}

int8_t readS8(std::istream& stream) {
  int8_t value;

  stream.read((char*)&value, sizeof(int8_t));

  return value;
}

uint16_t readU16(std::istream& stream) {
  uint16_t value;

  stream.read((char*)&value, sizeof(uint16_t));

  return value;
}

int16_t readS16(std::istream& stream) {
  int16_t value;

  stream.read((char*)&value, sizeof(int16_t));

  return value;
}

uint32_t readU32(std::istream& stream) {
  uint32_t value;

  stream.read((char*)&value, sizeof(uint32_t));

  return value;
}

int32_t readS32(std::istream& stream) {
  int32_t value;

  stream.read((char*)&value, sizeof(int32_t));

  return value;
}

uint64_t readU64(std::istream& stream) {
  uint64_t value;

  stream.read((char*)&value, sizeof(uint64_t));

  return value;
}

int64_t readS64(std::istream& stream) {
  int64_t value;

  stream.read((char*)&value, sizeof(int64_t));

  return value;
}

#pragma endregion

#pragma region write

void writeU8(std::ostream& stream, uint8_t value) {
  stream.write((char *)&value, sizeof(uint8_t));
}

void writeS8(std::ostream& stream, int8_t value) {
  stream.write((char *)&value, sizeof(int8_t));
}

void writeU16(std::ostream& stream, uint16_t value) {
  stream.write((char *)&value, sizeof(uint16_t));
}

void writeS16(std::ostream& stream, int16_t value) {
  stream.write((char *)&value, sizeof(int16_t));
}

void writeU32(std::ostream& stream, uint32_t value) {
  stream.write((char *)&value, sizeof(uint32_t));
}

void writeS32(std::ostream& stream, int32_t value) {
  stream.write((char *)&value, sizeof(int32_t));
}

void writeU64(std::ostream& stream, uint64_t value) {
  stream.write((char *)&value, sizeof(uint64_t));
}

void writeS64(std::ostream& stream, int64_t value) {
  stream.write((char *)&value, sizeof(int64_t));
}

#pragma endregion

}  // namespace mod::streamutilities
