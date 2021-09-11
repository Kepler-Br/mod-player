#include "WavWriter.h"

#include "mod/loaders/StreamUtils.h"

namespace mod {

void WavWriter::writeHeader(std::ostream& stream, uint32_t dataSize) {
  if (!stream) {
    throw std::runtime_error("writeHeader: stream is bad.");
  }

  stream << "RIFF";
  // Chunksize
  streamutils::writeU32(stream, dataSize + 44 - 8);
  stream << "WAVE";
  stream << "fmt ";
  // Subchunk 1 size
  streamutils::writeU32(stream, (uint32_t)(16));
  // Audio format 1=PCM,6=mulaw,7=alaw,257=IBM
  // Mu-Law, 258=IBM A-Law, 259=ADPCM
  streamutils::writeU16(stream, (uint16_t)(1));
  // Number of channels
  streamutils::writeU16(stream, (uint16_t)(1));
  // Sampling freq in Hz
  streamutils::writeU32(stream, (uint32_t)(11025*2));
  // Bytes per second
  streamutils::writeU32(stream, (uint32_t)(11025*2 * 1));
  // 2=16-bit mono, 4=16-bit stereo
  streamutils::writeU16(stream, (uint16_t)(1));
  // Number of bits per sample
  streamutils::writeU16(stream, (uint16_t)(8));
  stream << "data";
  // Data chunk length
  streamutils::writeU32(stream, dataSize);

  if (!stream) {
    throw std::runtime_error("writeHeader: stream gone bad.");
  }
}

void WavWriter::write(Generator& generator, std::ostream& stream) {
  WavWriter::writeHeader(stream, 0);

  std::streampos currentPosition = stream.tellp();

  generator.restart();

  std::vector<uint8_t> buffer(440 * 6);

  while(generator.getState() != GeneratorState::Paused) {
    generator.generate(buffer.data(), buffer.size());

    stream.write((char *)buffer.data(), (std::streamsize)buffer.size());
  }

  std::streampos endPosition = stream.tellp();

  auto dataWrote = (uint32_t)(endPosition - currentPosition);

  stream.seekp(0, std::ios_base::beg);
  this->writeHeader(stream, dataWrote);
}
}
