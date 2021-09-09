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
  float _dataFrequency;
  std::vector<float> _data;

 public:
  /**
   * @throws std::illegal_argument
   * @param name
   * @param length
   * @param finetune
   * @param volume
   * @param repeatPoint
   * @param repeatLength
   * @param dataFrequency
   */
  Sample(std::string name, int length, int finetune, int volume,
         int repeatPoint, int repeatLength, float dataFrequency);

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
  [[nodiscard]] std::vector<float> &getData();

  /**
   * @throw std::runtime_error
   * @return
   */
  [[nodiscard]] const std::vector<float> &getData() const;
  /**
   * @param data
   * @throw std::runtime_error
   */
  void setData(const std::vector<float> &data);

  float getDataFrequency() const {
    return this->_dataFrequency;
  }
};

}  // namespace mod
