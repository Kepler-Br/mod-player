#pragma once

#include <stdexcept>
#include <vector>

#include "Row.h"

namespace mod {

class Pattern {
 private:
  std::vector<Row> _rows;
  size_t _channels;
  size_t _totalRows;

 public:
  Pattern(size_t channels, size_t totalRows);

  /**
   * @param newRow
   * @throw std::invalid_argument
   * @throw std::out_of_range
   */
  void addRow(const Row &newRow);

  /**
   *
   * @param index
   * @throw std::out_of_range
   * @return
   */
  [[nodiscard]] const Row &getRow(size_t index) const;

  /**
   *
   * @param index
   * @throw std::out_of_range
   * @return
   */
  Row &getRow(size_t index);

  [[nodiscard]] const std::vector<Row> &getRows() const;

  std::vector<Row> &getRows();

  [[nodiscard]] size_t getChannels() const;

  [[nodiscard]] size_t getTotalRows() const;

  [[nodiscard]] size_t getOccupiedRows() const;
};

}  // namespace mod
