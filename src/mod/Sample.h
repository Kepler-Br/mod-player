#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Encoding.h"

namespace mod {

class Sample {
 private:
  std::string _name;
  int _length;
  int _finetune;
  int _volume;
  int _repeatPoint;
  int _repeatLength;
  std::vector<uint8_t> _data;
  Encoding _dataEncoding;
  size_t _expectedDataLength;

 public:
  Sample(std::string name, int length, int finetune, int volume,
         int repeatPoint, int repeatLength, Encoding dataEncoding);

  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] int getLength() const;
  [[nodiscard]] int getFinetune() const;
  [[nodiscard]] int getVolume() const;
  [[nodiscard]] int getRepeatPoint() const;
  [[nodiscard]] int getRepeatLength() const;
  void reserveData();
  /**
   * @throw std::runtime_error
   * @return
   */
  [[nodiscard]] std::vector<uint8_t> &getData();

  /**
   * @throw std::runtime_error
   * @return
   */
  [[nodiscard]] const std::vector<uint8_t> &getData() const;
  /**
   * @param data
   * @throw std::runtime_error
   */
  void setData(const std::vector<uint8_t> &data);

  Encoding getDataEncoding() const;
};

}  // namespace mod
