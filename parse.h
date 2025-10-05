#ifndef PARSE_H_
#define PARSE_H_

// Making it CommonMark Compliant
// https://spec.commonmark.org/0.31.2/#appendix-a-parsing-strategy

#include <deque>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// clang-format off
#define TOKENS      \
  X(None,       0)  \
  X(Root,       1)  \
  X(Paragraph,  2)  \
  X(Codeblock,  3)  \
  X(H1,         4)  \
  X(H2,         5)  \
  X(H3,         6)  \
  X(H4,         7)  \
  X(H5,         8)  \
  X(H6,         9)  \
  X(Newline,    10) \
  X(Whitespace, 11) \
  X(Text,       12) \
  X(Bold,       13) \
  X(Italic,     14) \
  X(BoldItalic, 15) \
  X(Code,       16)
// clang-format on

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

struct Block;
class Parser;
class Node;

typedef std::vector<std::string_view> Lexemes;
typedef std::pair<TokenType, std::string_view> Token;
typedef std::vector<Token> Tokens;
typedef std::vector<Block> Blocks;
typedef std::shared_ptr<Node> Tree;

struct Block {
  TokenType type = TokenType::None;
  bool isOpen = true;  // last child of a block is considered open
  std::string_view text;
  Blocks children;
};

class Parser {
 public:
  Parser();

  void AssignDocument(std::string_view);
  void AnalyzeBlocks(std::string_view);
  Block BuildBlocks();
  Block BuildParagraphBlock();
  Block GetBlock();
  std::string_view ScanNextLine();

  void Tokenize(std::string_view);  // returns a vector of lexemes
  void LexAnalysis();  // analyse lexemes and provide TokenType, returns Tokens
  Tree Parse();        // parse the Tokens
  Tree Parse(std::string_view);  // parse the string
  Tree GetRoot();

  Tokens GetTokens();
  void debug();

  static std::string DumpTree(const Tree&, int = 0);
  static std::string DumpTree(const Block&, int = 0);

 private:
  typedef struct StackItem {
    std::string_view marker;
    // metadata
    int index = 0;
    bool toErase = true;
  } StackItem;
  enum class ContainerType { Root, Paragraph, Heading };
  enum class BlockType { Root, Paragraph, Heading };

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
  Block block_ = {};
  BlockType blockType_ = BlockType::Root;

  std::string_view::iterator DocumentBegin();
  std::string_view::iterator DocumentEnd();

  bool IsDelimiter();
  void ClearStack();
  bool ToPop(const Token&);
  void FormatCorrectionInit();
  void FormatCorrection();
  void EmptyStack();
  bool FetchMarker(std::deque<StackItem>& backupStack);
  void StackCorrection(StackItem& HighItem, StackItem& LowItem);
  int LookAhead(std::string_view, const char&);
  int LookAhead(std::string_view,
                const char&,
                std::string_view::const_iterator);
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
