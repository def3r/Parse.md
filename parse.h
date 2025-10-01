#ifndef PARSE_H_
#define PARSE_H_

#include <deque>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#define TOKENS       \
  X(NONE, 0)         \
  X(H1, 1)           \
  X(H2, 2)           \
  X(H3, 3)           \
  X(H4, 4)           \
  X(H5, 5)           \
  X(H6, 6)           \
  X(NEWLINE, 7)      \
  X(WHITESPACE, 8)   \
  X(TEXT, 9)         \
  X(BOLD, 10)        \
  X(ITALIC, 11)      \
  X(BOLD_ITALIC, 12) \
  X(CODE, 13)        \
  X(CODEBLOCK, 14)   \
  X(ROOT, 15)

enum class TokenType {
#define X(TOKEN_NAME, TOKEN_VAL) TOKEN_NAME = TOKEN_VAL,
  TOKENS
#undef X
};

const std::string TokenStr(const TokenType&);
std::ostream& operator<<(std::ostream&, const TokenType&);

typedef std::vector<std::string> Lexemes;
typedef std::pair<TokenType, std::string> Token;
typedef std::vector<Token> Tokens;

class Parser;
class Node;

class Parser {
 public:
  Parser();
  void tokenize(const std::string&);
  Tokens getTokens();
  void debug();
  void Tokenize(const std::string& str);  // returns a vector of lexemes
  void Lexer();  // analyse lexemes and provide TokenType, returns Tokens
  void Parse();  // parse the Tokens
  void Parse(const std::string& str);  // parse the Tokens
  void MakeDoc();
  std::shared_ptr<Node> GetDoc();

 private:
  std::string::const_iterator begin, it;
  bool updateBegin = false;
  bool followsWhiteSpace = false;
  bool renderBlank = false;
  typedef struct Stack {
    std::string marker;
    // metadata
    int index = 0;
    bool toErase = true;
  } Stack;
  Lexemes lexemes;
  Tokens tokens;
  Stack TOS, TOSm1;
  int correction = 0;
  std::deque<Stack>* syntaxStack;

  inline static std::unordered_map<std::string, TokenType> markerMap;
  static TokenType GetMarker(const std::string& str);
  static void populateMarkerMap();

  std::string StrCreat(const std::string&, int);
  void ClearStack();
  bool ToPop(const Token&);
  void FormatCorrectionInit();
  void FormatCorrection();
  void EmptyStack();
  bool FetchMarker(std::deque<Stack>& backupStack);
  void StackCorrection(Stack& HighItem, Stack& LowItem);
  int lookAhead(const std::string&, char&&);
  void PushLexeme(size_t count);
  void PushLexeme();
  void PushToken(const std::string& lexeme);
  void PushToken(TokenType type, const std::string& lexeme);
  std::shared_ptr<Node> MakeTree(Tokens::iterator& it);
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
  // TokenType type();
  // const std::string& value();
  // const std::vector<std::shared_ptr<Node>>& children();
  Node(TokenType);
  Node();

  TokenType type;
  std::string value;
  std::vector<std::shared_ptr<Node>> children;
};

// utils
void ltrim(std::string&);

#endif  // PARSE_H_
