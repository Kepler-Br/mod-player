#pragma once

#include <cstdint>

namespace mod::dataconvertors {

float convertFromU8(const uint8_t *value);
float convertFromS8(const uint8_t *value);
float convertFromU16(const uint8_t *value);
float convertFromS16(const uint8_t *value);

void convertFromU8(const uint8_t *value, float &target);
void convertFromS8(const uint8_t *value, float &target);
void convertFromU16(const uint8_t *value, float &target);
void convertFromS16(const uint8_t *value, float &target);

void convertToU8(const float &value, uint8_t *target);
void convertToS8(const float &value, uint8_t *target);
void convertToU16(const float &value, uint8_t *target);
void convertToS16(const float &value, uint8_t *target);

void swapEndian(uint16_t *value);
void swapEndian(uint32_t *value);
void swapEndian(uint64_t *value);

uint16_t swapEndian(uint16_t value);
uint32_t swapEndian(uint32_t value);
uint64_t swapEndian(uint64_t value);

}  // namespace mod::dataconvertors
