#include <iomanip>
#include <iostream>
#include <string>

#include "parsemd/internal.h"

namespace markdown {
namespace internal {

TokenType GetMarker(std::string_view marker) {
  for (Marker m : markers) {
    if (m.marker == marker)
      return m.type;
  }
  // clang-format off
  if (
    marker[0] == '*' ||
    marker[0] == '_'
    // || marker[0] == '#'
  ) {  // clang-format on
    return TokenType::None;
  }
  return TokenType::Text;
}

bool IsDelimiter(char c) {
  // clang-format off
  switch (c) {
    case '*': return true;
    case '_': return true;
    // case '#': return true;
  }
  // clang-format on
  return false;
}

bool IsValidDelimiter(char prev, char delim, char next) {
  if (!(delim == '*' || delim == '_')) {
    std::cerr << "Unhandled Delimiter: " << std::quoted(std::to_string(delim))
              << std::endl;
    std::exit(1);
  }

  return IsLeftFlanking(prev, next) || IsRightFlanking(prev, next);
}

bool IsWhitespace(char c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r') ? true : false;
}

bool IsPunctuation(char c) {
  // ASCII only
  if (c >= 33 && c <= 47)
    return true;
  else if (c >= 58 && c <= 64)
    return true;
  else if (c >= 91 && c <= 96)
    return true;
  else if (c >= 123 && c <= 126)
    return true;

  return false;
}

bool IsLeftFlanking(char prev, char next) {
  // NOTE: No Unicode support
  if (IsWhitespace(next)) {
    return false;
  }

  if (IsPunctuation(next) && !(IsWhitespace(prev) || IsPunctuation(prev))) {
    return false;
  }

  return true;
}

bool IsRightFlanking(char prev, char next) {
  // NOTE: No Unicode support
  if (IsWhitespace(prev)) {
    return false;
  }

  if (IsPunctuation(prev) && !(IsWhitespace(next) || IsPunctuation(next))) {
    return false;
  }

  return true;
}

void htrim(std::string_view& sv) {
  std::string_view::iterator it = sv.begin();
  int count = 0;
  while ((it + count) != sv.end() && IsWhitespace(*(it + count))) {
    count++;
  }
  sv.remove_prefix(count);

  count = sv.size() - 1;
  it = sv.begin();
  while ((it + count) != sv.end() && IsWhitespace(*(it + count))) {
    count--;
  }
  sv.remove_suffix(sv.size() - count - 1);
}

Tokens::iterator listIteratorAdvance(Tokens::iterator& it, int inc) {
  while (inc--) {
    ++it;
  }
  return it;
}

}  // namespace internal
}  // namespace markdown
