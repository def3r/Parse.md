#include "lexer.h"

#include <cmath>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

const std::string TokenStr(const TokenType& token) {
  // clang-format off
  switch (token) {
    case TokenType::NONE: { return std::move("Token::NONE"); }
    case TokenType::H1: { return std::move("Token::H1"); }
    case TokenType::H2: { return std::move("Token::H2"); }
    case TokenType::H3: { return std::move("Token::H3"); }
    case TokenType::H4: { return std::move("Token::H4"); }
    case TokenType::H5: { return std::move("Token::H5"); }
    case TokenType::H6: { return std::move("Token::H6"); }
    case TokenType::TEXT: { return std::move("Token::TEXT"); }
    case TokenType::BOLD: { return std::move("Token::BOLD"); }
    case TokenType::ITALIC: { return std::move("Token::ITALIC"); }
    case TokenType::BOLD_ITALIC: { return std::move("Token::BOLD_ITALIC"); }
    case TokenType::CODE: { return std::move("Token::CODE"); }
    case TokenType::CODEBLOCK: { return std::move("Token::CODEBLOCK"); }
  }
  // clang-format on
  return std::move("Token::NONE");
}

typedef std::pair<TokenType, std::string> Token;
typedef std::vector<Token> Tokens;

Lexer::Lexer() {
  Lexer::populateMarkerMap();
}

void Lexer::tokenize(const std::string& line) {
  begin = line.begin();
  int count = 1;
  for (it = line.begin(); it != line.end(); ++it) {
    if (*it == '*') {
      count = lookAhead(line, '*');
      if (count == 1) {
        TokenUtil(TokenType::ITALIC, "*");
      } else if (count == 2) {
        TokenUtil(TokenType::BOLD, "**");
      } else if (count == 3) {
        TokenUtil(TokenType::BOLD_ITALIC, "***");
      } else {
        TokenUtil(TokenType::NONE, StrCreat("*", count));
      }
    }
    if (*it == '_') {
      count = lookAhead(line, '_');
      if (count == 1) {
        TokenUtil(TokenType::ITALIC, "_");
      } else if (count == 2) {
        TokenUtil(TokenType::BOLD, "__");
      } else if (count == 3) {
        TokenUtil(TokenType::BOLD_ITALIC, "___");
      } else {
        TokenUtil(TokenType::NONE, StrCreat("_", count));
      }
    }

    if (updateBegin) {
      begin = it + count;
      it = it + --count;
      updateBegin = false;
    }
  }
  PushText();
  FormatCorrections();
}

Tokens Lexer::getTokens() {
  return tokens;
}

void Lexer::debug() {
  for (auto& token : tokens) {
    std::cout << TokenStr(token.first) << "\t\t" << token.second << "\n";
  }
}

void Lexer::populateMarkerMap() {
  Lexer::markerMap["*"] = TokenType::ITALIC;
  Lexer::markerMap["**"] = TokenType::BOLD;
  Lexer::markerMap["***"] = TokenType::BOLD_ITALIC;
  Lexer::markerMap["_"] = TokenType::ITALIC;
  Lexer::markerMap["__"] = TokenType::BOLD;
  Lexer::markerMap["___"] = TokenType::BOLD_ITALIC;
}

std::string Lexer::StrCreat(const std::string& s, int n) {
  std::stringstream ss;
  while (n--) {
    ss << s;
  }
  return ss.str();
}

void Lexer::ClearStack() {
  while (!syntaxStack.empty()) {
    syntaxStack.pop_back();
  }
}

bool Lexer::ToPop(const Token& token) {
  // std::cout << syntaxStack.front().marker << " : " << token.second << "\n";
  return (!syntaxStack.empty() && syntaxStack.front().marker == token.second);
}

void Lexer::FormatCorrections() {
  int i = -1;
  int correction = 0;
  for (const auto& token : tokens) {
    i++;
    if ((token.first < (TokenType)6 && token.first != (TokenType)0) ||
        token.first == TokenType::TEXT) {
      continue;
    }
    if (!ToPop(token)) {
      syntaxStack.push_front({token.second, i, true});
    } else {
      syntaxStack.pop_front();
    }
  }
  if (syntaxStack.empty()) {
    return;
  }

  FormatCorrections(syntaxStack, correction);
}

void Lexer::FormatCorrections(std::deque<Stack>& syntaxStack, int& correction) {
  enum LastConsumed { TOS, TOSm1, NONE };
  std::deque<Stack> backupStack;
  LastConsumed lastConsumed = LastConsumed::NONE;
  while (!syntaxStack.empty() && syntaxStack.size() >= 2) {
    backupStack.clear();
    Stack TOS, TOSm1;
    if (!FetchMarker(syntaxStack, TOS, TOSm1, backupStack)) {
      break;
    }

    if (TOS.marker.length() == TOSm1.marker.length()) {
      if (!TOS.toErase) {
        int customCorr =
            (lastConsumed == LastConsumed::TOS) ? ++correction : correction++;
        tokens.insert(tokens.begin() + TOS.index + customCorr,
                      {Lexer::markerMap[TOS.marker], TOS.marker});
      }
      lastConsumed = LastConsumed::NONE;
    } else if (TOS.marker.length() > TOSm1.marker.length()) {
      StackCorrection(syntaxStack, TOS, TOSm1, correction);
      lastConsumed = LastConsumed::TOSm1;
    } else {
      StackCorrection(syntaxStack, TOSm1, TOS, correction);
      lastConsumed = LastConsumed::TOS;
    }
    if (backupStack.size() > 0) {
      FormatCorrections(backupStack, correction);
    }
  }
  while (!syntaxStack.empty()) {
    tokens.insert(tokens.begin() + syntaxStack.back().index + ++correction,
                  {TokenType::TEXT, syntaxStack.back().marker});
    if (syntaxStack.back().toErase) {
      tokens.erase(tokens.begin() + syntaxStack.back().index + --correction);
    }
    syntaxStack.pop_back();
  }
}

bool Lexer::FetchMarker(std::deque<Stack>& syntaxStack,
                        Stack& TOS,
                        Stack& TOSm1,
                        std::deque<Stack>& backupStack) {
  TOS = syntaxStack.back();
  syntaxStack.pop_back();

  TOSm1 = syntaxStack.back();
  syntaxStack.pop_back();
  if (TOS.marker[0] != TOSm1.marker[0]) {
    while (syntaxStack.size() >= 1 && TOS.marker[0] != TOSm1.marker[0]) {
      backupStack.push_back(TOSm1);
      TOSm1 = syntaxStack.back();
      syntaxStack.pop_back();
    }
    if (syntaxStack.size() == 0 && TOS.marker[0] != TOSm1.marker[0]) {
      syntaxStack.push_back(TOSm1);
      syntaxStack.push_back(TOS);
      return false;
    }
  }
  return true;
}

void Lexer::StackCorrection(std::deque<Stack>& syntaxStack,
                            Stack& HighItem,
                            Stack& LowItem,
                            int& correction) {
  HighItem.marker = HighItem.marker.substr(
      0, HighItem.marker.length() - LowItem.marker.length());
  syntaxStack.push_back(std::move(HighItem));

  if (!LowItem.toErase) {
    tokens.insert(tokens.begin() + LowItem.index + correction++,
                  {Lexer::markerMap[LowItem.marker], LowItem.marker});
  }
  if (syntaxStack.back().toErase) {
    tokens.erase(tokens.begin() + syntaxStack.back().index + correction--);
    syntaxStack.back().toErase = false;
  }
  tokens.insert(tokens.begin() + syntaxStack.back().index + ++correction,
                {Lexer::markerMap[LowItem.marker], LowItem.marker});
}

void Lexer::PushText() {
  std::string text = (begin >= it) ? "" : std::string(begin, it);
  if (!text.empty()) {
    tokens.push_back({TokenType::TEXT, text});
  }
}

void Lexer::TokenUtil(const TokenType& token, const std::string& marker) {
  updateBegin = true;
  PushText();
  tokens.push_back({token, marker});
}

int Lexer::lookAhead(const std::string& line, char&& c) {
  int count = 1;
  while ((it + count) != line.end() && *(it + count) == c) {
    count++;
  }
  return count;
}
