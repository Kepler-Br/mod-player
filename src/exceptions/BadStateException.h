#pragma once

class BadStateException : public std::logic_error {
  using std::logic_error::logic_error;
};
