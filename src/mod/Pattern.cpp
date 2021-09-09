#include "Pattern.h"
#include <string>

namespace mod {

Pattern::Pattern(size_t channels, size_t totalRows)
    : _channels(channels), _totalRows(totalRows) {
  _rows.reserve(this->_totalRows);
}

void Pattern::addRow(const Row& newRow) {
  if (newRow.getChannels() != this->_channels) {
    const std::string message =
        "newRow has " + std::to_string(newRow.getChannels()) +
        " channels, but expected " + std::to_string(this->_channels);

    throw std::invalid_argument(message);
  }

  if (this->_rows.size() == this->_totalRows) {
    const std::string message =
        "Tried to add excess row. Max rows specified: " +
        std::to_string(this->_totalRows);

    throw std::out_of_range(message);
  }

  this->_rows.push_back(newRow);
}

const Row& Pattern::getRow(size_t index) const {
  if (index >= this->_rows.size()) {
    const std::string message =
        "Out of range: " + std::to_string(index) + " out of " +
        std::to_string(this->_rows.size()) + " possible.";

    throw std::out_of_range(message);
  }

  return this->_rows[index];
}

Row& Pattern::getRow(size_t index) {
  if (index >= this->_rows.size()) {
    const std::string message =
        "Out of range: " + std::to_string(index) + " out of " +
        std::to_string(this->_rows.size()) + " possible.";

    throw std::out_of_range(message);
  }

  return this->_rows[index];
}

const std::vector<Row>& Pattern::getRows() const { return this->_rows; }

std::vector<Row>& Pattern::getRows() { return this->_rows; }

size_t Pattern::getChannels() const { return this->_channels; }

size_t Pattern::getTotalRows() const { return this->_totalRows; }

size_t Pattern::getOccupiedRows() const { return this->_rows.size(); }

}  // namespace mod
