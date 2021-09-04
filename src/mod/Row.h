#pragma once

#include <vector>

#include "Note.h"

namespace mod {

class Row {
 private:
  std::vector<Note> _notes;
  size_t _channels;

 public:
  Row(std::initializer_list<Note> notes);

  explicit Row(size_t channels);

  /**
   * @param note
   * @throw std::out_of_range
   */
  void addNote(const Note &note);

  [[nodiscard]] const std::vector<Note> &getNotes() const;

  /**
   * @param index
   * @throw std::out_of_range
   * @return
   */
  [[nodiscard]] const Note &getNote(size_t index) const;

  [[nodiscard]] size_t getChannels() const;
};

}  // namespace mod