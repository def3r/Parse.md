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
  X(ROOT, 15)        \
  X(PARAGRAPH, 16)

enum class TokenType {
#define X(TOKEN_NAME, TOKEN_VAL) TOKEN_NAME = TOKEN_VAL,
  TOKENS
#undef X
};
#define isHeading(token) (token >= TokenType::H1 && token <= TokenType::H6)

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

  void Tokenize(const std::string&);  // returns a vector of lexemes
  void LexAnalysis();  // analyse lexemes and provide TokenType, returns Tokens
  std::shared_ptr<Node> Parse();                    // parse the Tokens
  std::shared_ptr<Node> Parse(const std::string&);  // parse the string
  std::shared_ptr<Node> GetRoot();

  Tokens getLexTokens();
  void debug();

  static std::string DumpTree(const std::shared_ptr<Node>&, int = 0);

 private:
  typedef struct Stack {
    std::string marker;
    // metadata
    int index = 0;
    bool toErase = true;
  } Stack;
  enum class ContainerType { ROOT, PARAGRAPH, HEADING };

  std::string::const_iterator begin, it;
  bool updateBegin = false;
  bool followsWhiteSpace = false;
  bool renderBlank = false;
  Lexemes lexemes;
  Tokens lexTokens;
  Stack TOS, TOSm1;
  int correction = 0;
  std::deque<Stack>* syntaxStack;
  ContainerType containerType = ContainerType::ROOT;
  Tokens::iterator itToken;
  std::shared_ptr<Node> root = 0;

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

  std::shared_ptr<Node> FinalPass();
  std::shared_ptr<Node> BuildTree();
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
  // TokenType type();
  // const std::string& value();
  // const std::vector<std::shared_ptr<Node>>& children();
  Node(TokenType);
  Node(std::string);
  Node();

  TokenType type;
  std::string value;
  std::vector<std::shared_ptr<Node>> children;
};

// utils
void ltrim(std::string&);

#endif  // PARSE_H_
