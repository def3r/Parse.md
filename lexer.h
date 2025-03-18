#ifndef LEXER_H_
#define LEXER_H_

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
  TEXT,
  BOLD,
  ITALIC,
  BOLD_ITALIC,
  CODE,
  CODEBLOCK,
};

const std::string TokenStr(const TokenType&);

typedef std::pair<TokenType, std::string> Token;
typedef std::vector<Token> Tokens;

class Lexer {
 public:
  Lexer();
  void tokenize(const std::string&);
  Tokens getTokens();
  void debug();

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
  Tokens tokens;
  Stack TOS, TOSm1;
  int correction = 0;
  std::deque<Stack>* syntaxStack;
  inline static std::unordered_map<std::string, TokenType> markerMap;

  static void populateMarkerMap();

  std::string StrCreat(const std::string&, int);
  void ClearStack();
  bool ToPop(const Token&);
  void FormatCorrectionInit();
  void FormatCorrection();
  void EmptyStack();
  bool FetchMarker(std::deque<Stack>& backupStack);
  void StackCorrection(Stack& HighItem, Stack& LowItem);
  void PushText();
  void TokenUtil(const TokenType&, const std::string&);
  int lookAhead(const std::string&, char&&);
};

#endif  // LEXER_H_
