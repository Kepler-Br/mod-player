#include "RawWriter.h"

namespace mod {

void RawWriter::write(Generator& generator, std::ostream& stream) {
  generator.restart();

  std::vector<uint8_t> buffer(256);

  while(generator.getState() != GeneratorState::Paused) {
    generator.generate(buffer.data(), buffer.size());

    stream.write((char *)buffer.data(), (std::streamsize)buffer.size());
  }
}

}
