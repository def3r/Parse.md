#ifndef PARSE_H_
#define PARSE_H_

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

enum class TokenType {
  NONE = 0,
  H1 = 1,
  H2 = 2,
  H3 = 3,
  H4 = 4,
  H5 = 5,
  H6 = 6,
  NEWLINE = 7,
  WHITESPACE = 8,
  TEXT = 9,
  BOLD,
  ITALIC,
  BOLD_ITALIC,
  CODE,
  CODEBLOCK,
};

const std::string TokenStr(const TokenType&);

typedef std::vector<std::string> Lexemes;
typedef std::pair<TokenType, std::string> Token;
typedef std::vector<Token> Tokens;

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
};

#endif  // PARSE_H_
