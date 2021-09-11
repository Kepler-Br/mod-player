#pragma once

#include <cstdint>
#include <istream>

namespace mod::streamutils {

uint16_t readU16Swapped(std::istream &stream);
uint32_t readU32Swapped(std::istream &stream);
uint64_t readU64Swapped(std::istream &stream);

uint8_t readU8(std::istream &stream);
int8_t readS8(std::istream &stream);
uint16_t readU16(std::istream &stream);
int16_t readS16(std::istream &stream);
uint32_t readU32(std::istream &stream);
int32_t readS32(std::istream &stream);
uint64_t readU64(std::istream &stream);
int64_t readS64(std::istream &stream);

void writeU8(std::ostream &stream, uint8_t value);
void writeS8(std::ostream &stream, int8_t value);
void writeU16(std::ostream &stream, uint16_t value);
void writeS16(std::ostream &stream, int16_t value);
void writeU32(std::ostream &stream, uint32_t value);
void writeS32(std::ostream &stream, int32_t value);
void writeU64(std::ostream &stream, uint64_t value);
void writeS64(std::ostream &stream, int64_t value);

}  // namespace mod::streamutilities
