#ifndef PARSE_H_
#define PARSE_H_

#include <deque>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#define TOKENS      \
  X(None, 0)        \
  X(H1, 1)          \
  X(H2, 2)          \
  X(H3, 3)          \
  X(H4, 4)          \
  X(H5, 5)          \
  X(H6, 6)          \
  X(Newline, 7)     \
  X(Whitespace, 8)  \
  X(Text, 9)        \
  X(Bold, 10)       \
  X(Italic, 11)     \
  X(BoldItalic, 12) \
  X(Code, 13)       \
  X(Codeblock, 14)  \
  X(Root, 15)       \
  X(Paragraph, 16)

namespace markdown {

enum class TokenType {
#define X(TOKEN_NAME, TOKEN_VAL) TOKEN_NAME = TOKEN_VAL,
  TOKENS
#undef X
};
#define isHeading(token) (token >= TokenType::H1 && token <= TokenType::H6)
TokenType operator+(TokenType, int);

const std::string TokenStr(const TokenType&);
std::ostream& operator<<(std::ostream&, const TokenType&);

class Parser;
class Node;

typedef std::vector<std::string_view> Lexemes;
typedef std::pair<TokenType, std::string_view> Token;
typedef std::vector<Token> Tokens;
typedef std::shared_ptr<Node> Tree;

class Parser {
 public:
  Parser();

  void Tokenize(std::string_view);  // returns a vector of lexemes
  void LexAnalysis();  // analyse lexemes and provide TokenType, returns Tokens
  Tree Parse();        // parse the Tokens
  Tree Parse(std::string_view);  // parse the string
  Tree GetRoot();

  Tokens GetTokens();
  void debug();

  static std::string DumpTree(const Tree&, int = 0);

 private:
  typedef struct StackItem {
    std::string_view marker;
    // metadata
    int index = 0;
    bool toErase = true;
  } StackItem;
  enum class ContainerType { Root, Paragraph, Heading };

  std::string document_;
  std::string_view::const_iterator begin_, it_;
  bool updateBegin_ = false;
  bool followsWhiteSpace_ = false;
  bool renderBlank_ = false;
  Lexemes lexemes_;
  Tokens candTokens_;
  StackItem TOS_, TOSm1_;
  int correction_ = 0;
  std::deque<StackItem>* syntaxStack_;
  ContainerType containerType_ = ContainerType::Root;
  Tokens::iterator itToken_;
  Tree root_ = 0;

  bool IsDelimiter();
  void ClearStack();
  bool ToPop(const Token&);
  void FormatCorrectionInit();
  void FormatCorrection();
  void EmptyStack();
  bool FetchMarker(std::deque<StackItem>& backupStack);
  void StackCorrection(StackItem& HighItem, StackItem& LowItem);
  int LookAhead(std::string_view, const char&);
  void PushLexeme(size_t count);
  void PushLexeme();
  void PushToken(std::string_view lexeme);
  void PushToken(TokenType type, std::string_view lexeme);

  Tree FinalPass();
  Tree BuildTree();
  void BuildParagraph(Node&);
  void BuildText(Node&);
  void BuildChildren(Node&);
  bool isParagraphEnd() const;
  bool validHeading();
  Tokens::iterator itTokenInc();
  Tokens::iterator itTokenInc(int);
};

// NOTE: I still think this is a better idea, but lets see
//
// class Node {
//  protected:
//   TokenType type_;
//
//  public:
//   virtual ~Node() = default;
//   virtual TokenType type() const = 0;
// };
//
// class ContainerNode : public Node {
//  public:
//   std::vector<std::shared_ptr<Node>> children;
// };
//
// class LeafNode : public Node {
//  public:
//   std::string value;
//   LeafNode(const std::string);
// };

class Node {
 public:
  Node(TokenType);
  Node(std::string);
  Node();

  TokenType type;
  std::string value;
  std::vector<std::shared_ptr<Node>> children;
};

// utils
void ltrim(std::string_view&);

namespace detail {
struct Marker {
  const char* marker;
  TokenType type;
};

inline constexpr Marker markers[] = {
    {"***", TokenType::BoldItalic}, {"**", TokenType::Bold},
    {"*", TokenType::Italic},       {"___", TokenType::BoldItalic},
    {"__", TokenType::Bold},        {"_", TokenType::Italic},
    {"#", TokenType::H1},           {"##", TokenType::H2},
    {"###", TokenType::H3},         {"####", TokenType::H4},
    {"#####", TokenType::H5},       {"######", TokenType::H6},
    {"\n", TokenType::Newline},
};

TokenType GetMarker(std::string_view str);

}  // namespace detail

}  // namespace markdown

#endif  // PARSE_H_
