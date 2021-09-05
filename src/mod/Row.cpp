#include "Row.h"

#include <fmt/format.h>

#include <stdexcept>
#include <string>

namespace mod {

Row::Row(std::initializer_list<Note> notes)
    : _notes(notes), _channels(_notes.size()) {}

Row::Row(size_t channels) : _channels(channels) { _notes.reserve(_channels); }

void Row::addNote(const Note& note) {
  if (this->_notes.size() >= this->_channels) {
    const std::string message = "Tried to add excess note. Channels: " +
                                std::to_string(this->_channels);

    throw std::out_of_range(message);
  }

  this->_notes.push_back(note);
}

const std::vector<Note>& Row::getNotes() const { return this->_notes; }

const Note& Row::getNote(size_t index) const {
  if (index >= this->_channels) {
    const std::string message =
        fmt::format("Tried access not existing channel {}. Total channels: {}",
                    index, this->_channels);

    throw std::out_of_range(message);
  }

  return this->_notes[index];
}

size_t Row::getChannels() const { return this->_channels; }
}  // namespace mod