#ifndef PARSEMD_PARSE_TYPES_H_
#define PARSEMD_PARSE_TYPES_H_

#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace markdown {

// clang-format off
#define TOKENS            \
  X(None,             0)  \
  /*    Block Tokens    */\
  X(Root,             1)  \
  X(Paragraph,        2)  \
  X(Codeblock,        3)  \
  X(H1,               4)  \
  X(H2,               5)  \
  X(H3,               6)  \
  X(H4,               7)  \
  X(H5,               8)  \
  X(H6,               9)  \
                          \
  X(Softbreak,        10) \
  X(Whitespace,       11) \
                          \
  /*    Inline Tokens   */\
  X(Text,             12) \
  X(Strong,           13) \
  X(StrongOpen,       14) \
  X(StrongClose,      15) \
  X(Emph,             16) \
  X(EmphOpen,         17) \
  X(EmphClose,        18) \
  X(StrongEmph,       19) \
  X(Code,             20)
// clang-format on

enum class TokenType {
#define X(TOKEN_NAME, TOKEN_VAL) TOKEN_NAME = TOKEN_VAL,
  TOKENS
#undef X
};
TokenType operator+(TokenType, int);
const std::string TokenStr(const TokenType&);
std::ostream& operator<<(std::ostream&, const TokenType&);

inline bool IsHeading(TokenType token) {
  return token >= TokenType::H1 && token <= TokenType::H6;
}
inline bool IsBlock(TokenType token) {
  return token >= TokenType::Root && token <= TokenType::H6;
}
inline bool IsInline(TokenType token) {
  return token > TokenType::Text && token <= TokenType::Code;
}
inline bool IsText(TokenType token) {
  return token == TokenType::Text;
}

// clang-format off
struct NodeBase;
struct ContainerNode;
struct BlockNode;
struct InlineNode;
struct TextNode;

class Scanner;
class Parser;

using Token     = std::pair<TokenType, std::string_view>;
using Tokens    = std::list<Token>;
using Node      = std::shared_ptr<NodeBase>;
using Nodes     = std::vector<Node>;
using Container = std::shared_ptr<ContainerNode>;
using Block     = std::shared_ptr<BlockNode>;
using Inline    = std::shared_ptr<InlineNode>;
using Text      = std::shared_ptr<TextNode>;
// clang-format on

}  // namespace markdown

#endif  // !PARSEMD_PARSE_TYPES_H_
