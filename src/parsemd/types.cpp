#include "parsemd/types.h"

namespace markdown {

TokenType operator+(TokenType t, int i) {
  return static_cast<TokenType>(static_cast<int>(t) + i);
}

const std::string TokenStr(const TokenType& token) {
  switch (token) {
#define X(TOKEN_NAME, TOKEN_VAL) \
  case TokenType::TOKEN_NAME:    \
    return "Token::" #TOKEN_NAME;
    TOKENS
#undef X
  }
  return "Token::None";
}

std::ostream& operator<<(std::ostream& os, const TokenType& tt) {
  return os << TokenStr(tt);
}

}  // namespace markdown
