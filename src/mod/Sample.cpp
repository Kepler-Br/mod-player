#include "Sample.h"

#include <fmt/format.h>

#include <stdexcept>

namespace mod {

Sample::Sample(std::string name, int length, int finetune, int volume,
               int repeatPoint, int repeatLength, Encoding dataEncoding)
    : _name(std::move(name)),
      _length(length),
      _finetune(finetune),
      _volume(volume),
      _repeatPoint(repeatPoint),
      _repeatLength(repeatLength),
      _dataEncoding(dataEncoding) {
  this->_expectedDataLength = length * bytesInEncoding(dataEncoding);
}

const std::string& Sample::getName() const { return this->_name; }

int Sample::getLength() const { return this->_length; }

int Sample::getFinetune() const { return this->_finetune; }

int Sample::getVolume() const { return this->_volume; }

int Sample::getRepeatPoint() const { return this->_repeatPoint; }

int Sample::getRepeatLength() const { return this->_repeatLength; }

void Sample::reserveData() { this->_data.resize(this->_expectedDataLength); }

std::vector<uint8_t>& Sample::getData() {
  if (this->_data.size() != this->_expectedDataLength) {
    throw std::runtime_error("Sample data was not set or reserved.");
  }

  return this->_data;
}

const std::vector<uint8_t>& Sample::getData() const {
  if (this->_data.size() != this->_expectedDataLength) {
    throw std::runtime_error("Sample data was not set or reserved.");
  }

  return this->_data;
}

void Sample::setData(const std::vector<uint8_t>& data) {
  if (data.size() != this->_expectedDataLength) {
    const std::string message = fmt::format(
        "Cannot set sample data: unexpected new data length. "
        "Expected {} bytes, got {} bytes",
        this->_length, data.size());

    throw std::runtime_error(message);
  }

  this->_data = data;
}

Encoding Sample::getDataEncoding() const { return this->_dataEncoding; }

}  // namespace mod
