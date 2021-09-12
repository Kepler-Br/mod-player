#include "DataConvertors.h"

namespace mod::dataconvertors {

constexpr float maxSigned8 = (float)0x80 - 1.0f;
constexpr float maxSigned16 = (float)0x8000 - 1.0f;

#pragma region convert from

float convertFromU8(const uint8_t *value) {
  return ((float)*value - maxSigned8) / maxSigned8;
}

float convertFromS8(const uint8_t *value) {
  const auto *convertedValue = (const int8_t *)value;

  return (float)*convertedValue / maxSigned8;
}

float convertFromU16(const uint8_t *value) {
  const auto *convertedValue = (const uint16_t *)value;

  return ((float)*convertedValue - maxSigned16) / maxSigned16;
}

float convertFromS16(const uint8_t *value) {
  const auto *convertedValue = (const int16_t *)value;

  return (float)*convertedValue / maxSigned16;
}

void convertFromU8(const uint8_t *value, float &target) {
  target = ((float)*value - maxSigned8) / maxSigned8;
}

void convertFromS8(const uint8_t *value, float &target) {
  const auto *convertedValue = (const int8_t *)value;

  target = (float)*convertedValue / maxSigned8;
}

void convertFromU16(const uint8_t *value, float &target) {
  const auto *convertedValue = (const uint16_t *)value;

  target = ((float)*convertedValue - maxSigned16) / maxSigned16;
}

void convertFromS16(const uint8_t *value, float &target) {
  const auto *convertedValue = (const int16_t *)value;

  target = (float)*convertedValue / maxSigned16;
}

#pragma endregion

#pragma region convert to

void convertToU8(const float &value, uint8_t *target) {
  *target = (uint8_t)((value * maxSigned8) + maxSigned8);
}

void convertToS8(const float &value, uint8_t *target) {
  auto *convertedPointer = (int8_t *)target;

  *convertedPointer = (int8_t)(value * maxSigned8);
}

void convertToU16(const float &value, uint8_t *target) {
  auto *convertedPointer = (uint16_t *)target;

  *convertedPointer = (uint16_t)(value * maxSigned16 + maxSigned16);
}

void convertToS16(const float &value, uint8_t *target) {
  auto *convertedPointer = (int16_t *)target;

  *convertedPointer = (int16_t)(value * maxSigned16);
}

#pragma endregion

#pragma region swap endian

void swapEndian(uint16_t *value) {
  *value = (uint16_t)(*value >> 8) | (uint16_t)(*value << 8);
}

void swapEndian(uint32_t *value) {
  *value = ((((*value) & 0xff000000) >> 24) |  // .
            (((*value) & 0x00ff0000) >> 8) |   // .
            (((*value) & 0x0000ff00) << 8) |   // .
            (((*value) & 0x000000ff) << 24));
}

void swapEndian(uint64_t *value) {
  *value = ((((*value) & 0xff00000000000000ULL) >> 56) |  // .
            (((*value) & 0x00ff000000000000ULL) >> 40) |  // .
            (((*value) & 0x0000ff0000000000ULL) >> 24) |  // .
            (((*value) & 0x000000ff00000000ULL) >> 8) |   // .
            (((*value) & 0x00000000ff000000ULL) << 8) |   // .
            (((*value) & 0x0000000000ff0000ULL) << 24) |  // .
            (((*value) & 0x000000000000ff00ULL) << 40) |  // .
            (((*value) & 0x00000000000000ffULL) << 56));
}

uint16_t swapEndian(uint16_t value) {
  swapEndian(&value);

  return value;
}

uint32_t swapEndian(uint32_t value) {
  swapEndian(&value);

  return value;
}

uint64_t swapEndian(uint64_t value) {
  swapEndian(&value);

  return value;
}

#pragma endregion

}  // namespace mod::dataconvertors
