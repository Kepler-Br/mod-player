#pragma once

#include "ModWriter.h"
#include <iostream>

namespace mod {

class WavWriter : public ModWriter {
 private:

  void write16(std::ostream &stream, uint16_t value) {
    stream.write((char *)&value, sizeof(uint16_t));
  }

  void write16(std::ostream &stream, int16_t value) {
    stream.write((char *)&value, sizeof(int16_t));
  }

  void write32(std::ostream &stream, uint32_t value) {
    stream.write((char *)&value, sizeof(uint32_t));
  }

  void write32(std::ostream &stream, int32_t value) {
    stream.write((char *)&value, sizeof(int32_t));
  }

  /**
   * @param stream
   * @throw std::runtime_error
   */
  void writeHeader(std::ostream &stream, uint32_t dataSize) {
    if (!stream) {
      throw std::runtime_error("writeHeader: stream is bad.");
    }

    stream << "RIFF";
    // Chunksize
    this->write32(stream, dataSize + 44 - 8);
    stream << "WAVE";
    stream << "fmt ";
    // Subchunk 1 size
    this->write32(stream, (uint32_t)(16));
    // Audio format 1=PCM,6=mulaw,7=alaw,257=IBM
    // Mu-Law, 258=IBM A-Law, 259=ADPCM
    this->write16(stream, (uint16_t)(1));
    // Number of channels
    this->write16(stream, (uint16_t)(1));
    // Sampling freq in Hz
    this->write32(stream, (uint32_t)(11025*2));
    // Bytes per second
    this->write32(stream, (uint32_t)(11025*2 * 1));
    // 2=16-bit mono, 4=16-bit stereo
    this->write16(stream, (uint16_t)(1));
    // Number of bits per sample
    this->write16(stream, (uint16_t)(8));
    stream << "data";
    // Data chunk length
    this->write32(stream, (uint32_t)(dataSize));

    if (!stream) {
      throw std::runtime_error("writeHeader: stream gone bad.");
    }
  }

 public:
  WavWriter() = default;

  void write(Generator &generator, std::ostream &stream) override {
    this->writeHeader(stream, 0);

    std::streampos currentPosition = stream.tellp();

    generator.restart();

    std::vector<uint8_t> buffer(440 * 6);

    while(generator.getState() != GeneratorState::Paused) {
      generator.generate(buffer.data(), buffer.size());

      stream.write((char *)buffer.data(), (std::streamsize)buffer.size());
    }

    std::streampos endPosition = stream.tellp();

    uint32_t dataWrote = endPosition - currentPosition;

    stream.seekp(0, std::ios_base::beg);
    this->writeHeader(stream, dataWrote);
  }
};

}  // namespace mod