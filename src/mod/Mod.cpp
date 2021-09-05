#include "Mod.h"

#include <utility>

namespace mod {

Mod::Mod(std::string name, size_t songLength,
         std::vector<Sample> samples, std::vector<Pattern> patterns,
         std::vector<int> orders)
    : _name(std::move(name)),
      _songLength(songLength),
      _samples(std::move(samples)),
      _patterns(std::move(patterns)),
      _orders(std::move(orders)) {
  if (!this->_patterns.empty()) {
    this->_channels = this->_patterns[0].getChannels();
  }
}

size_t Mod::getChannels() const { return this->_channels; }

size_t Mod::getSongLength() const { return this->_songLength; }

size_t Mod::getSampleCount() const { return this->_samples.size(); }

size_t Mod::getPatternCount() const { return this->_patterns.size(); }

std::vector<Sample>& Mod::getSamples() { return this->_samples; }

std::vector<int>& Mod::getOrders() { return this->_orders; }

std::vector<Pattern>& Mod::getPatterns() { return this->_patterns; }

const std::vector<Sample>& Mod::getSamples() const {
  return this->_samples;
}

const std::vector<int>& Mod::getOrders() const { return this->_orders; }

const std::vector<Pattern>& Mod::getPatterns() const {
  return this->_patterns;
}

const std::string& Mod::getName() const { return this->_name; }

}  // namespace mod