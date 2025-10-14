#ifndef PARSEMD_INTERNAL_H_
#define PARSEMD_INTERNAL_H_

#include "types.h"

namespace markdown {

namespace internal {

struct Marker {
  const char* marker;
  TokenType type;
};

inline constexpr Marker markers[] = {
    {"***", TokenType::StrongEmph}, {"**", TokenType::Strong},
    {"*", TokenType::Emph},         {"___", TokenType::StrongEmph},
    {"__", TokenType::Strong},      {"_", TokenType::Emph},
    {"#", TokenType::H1},           {"##", TokenType::H2},
    {"###", TokenType::H3},         {"####", TokenType::H4},
    {"#####", TokenType::H5},       {"######", TokenType::H6},
    {"\n", TokenType::Newline},
};

TokenType GetMarker(std::string_view str);
bool IsDelimiter(char c);
bool IsValidDelimiter(char prev, char delim, char next);
bool IsWhitespace(char);
bool IsPunctuation(char);
bool IsLeftFlanking(char prev, char next);
bool IsRightFlanking(char prev, char next);

void htrim(std::string_view& sv);

}  // namespace internal

}  // namespace markdown

#endif  // !PARSEMD_INTERNAL_H_
