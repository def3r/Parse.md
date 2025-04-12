#include "parse.h"

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
    case TokenType::NEWLINE: { return std::move("Token::NEWLINE"); }
    case TokenType::WHITESPACE: { return std::move("Token::WHITESPACE"); }
  }
  // clang-format on
  return std::move("Token::NONE");
}

typedef std::pair<TokenType, std::string> Token;
typedef std::vector<Token> Tokens;

Parser::Parser() {
  Parser::populateMarkerMap();
}

void Parser::PushLexeme() {
  updateBegin = true;
  std::string text = (begin >= it) ? "" : std::string(begin, it);
  if (!text.empty()) {
    lexemes.push_back(text);
  }
}

void Parser::PushLexeme(size_t count) {
  updateBegin = true;
  PushLexeme();
  std::string text = (it + count == it) ? "" : std::string(it, it + count);
  lexemes.push_back(text);
}

void Parser::Tokenize(const std::string& line) {
  begin = line.begin();
  int count = 1;
  for (it = line.begin(); it != line.end(); ++it) {
    if (*it == '*') {
      count = lookAhead(line, '*');
      PushLexeme(count);
    }
    if (*it == '_') {
      count = lookAhead(line, '_');
      PushLexeme(count);
    }
    if (*it == '#') {
      count = lookAhead(line, '#');
      PushLexeme(count);
    }
    if (*it == '\n') {
      count = 1;
      PushLexeme(count);
    }

    if (updateBegin) {
      begin = it + count;
      it = it + --count;
      updateBegin = false;
    }
  }
  PushLexeme();
}

TokenType Parser::GetMarker(const std::string& marker) {
  if (markerMap.find(marker) != markerMap.end()) {
    return markerMap[marker];
  }
  // clang-format off
  if (
    marker[0] == '*' ||
    marker[0] == '_' ||
    marker[0] == '#'
  ) {  // clang-format on
    return TokenType::NONE;
  }
  return TokenType::TEXT;
}

void Parser::PushToken(const std::string& lexeme) {
  tokens.push_back({Parser::GetMarker(lexeme), lexeme});
}
void Parser::PushToken(TokenType type, const std::string& lexeme) {
  tokens.push_back({type, lexeme});
}

void Parser::Lexer() {
  for (const auto& lexeme : lexemes) {
    if (lexeme[0] == ' ') {
      it = lexeme.begin();
      if (lookAhead(lexeme, ' ') == lexeme.size()) {
        PushToken(TokenType::WHITESPACE, lexeme);
        continue;
      }
    }
    PushToken(lexeme);
  }
  return;
}

void Parser::Parse() {
  FormatCorrectionInit();
  return;
}
void Parser::Parse(const std::string& str) {
  Tokenize(str);
  Lexer();
  Parse();
  return;
}

Tokens Parser::getTokens() {
  return tokens;
}

void Parser::debug() {
  for (auto& token : tokens) {
    std::cout << TokenStr(token.first) << "\t\t" << token.second << "\n";
  }
}

void Parser::populateMarkerMap() {
  Parser::markerMap["*"] = TokenType::ITALIC;
  Parser::markerMap["**"] = TokenType::BOLD;
  Parser::markerMap["***"] = TokenType::BOLD_ITALIC;
  Parser::markerMap["_"] = TokenType::ITALIC;
  Parser::markerMap["__"] = TokenType::BOLD;
  Parser::markerMap["___"] = TokenType::BOLD_ITALIC;
  Parser::markerMap["#"] = TokenType::H1;
  Parser::markerMap["##"] = TokenType::H2;
  Parser::markerMap["###"] = TokenType::H3;
  Parser::markerMap["####"] = TokenType::H4;
  Parser::markerMap["#####"] = TokenType::H5;
  Parser::markerMap["######"] = TokenType::H6;
  Parser::markerMap["\n"] = TokenType::NEWLINE;
}

std::string Parser::StrCreat(const std::string& s, int n) {
  std::stringstream ss;
  while (n--) {
    ss << s;
  }
  return ss.str();
}

void Parser::ClearStack() {
  while (!syntaxStack->empty()) {
    syntaxStack->pop_back();
  }
}

bool Parser::ToPop(const Token& token) {
  // std::cout << syntaxStack.front().marker << " : " << token.second << "\n";
  return (!syntaxStack->empty() && syntaxStack->front().marker == token.second);
}

void Parser::FormatCorrectionInit() {
  int index = -1;
  correction = 0;
  std::deque<Stack> syntaxStack;
  this->syntaxStack = &syntaxStack;
  for (const auto& token : tokens) {
    index++;
    // Skip for NEWLINE, WHITESPACE, TEXT
    if (token.first >= TokenType::NEWLINE && token.first <= TokenType::TEXT) {
      continue;
    }

    if (token.first >= TokenType::H1 && token.first <= TokenType::H6) {
      if (index == 0) {
        continue;
      }
      TokenType prevToken = tokens[index - 1].first;
      if (prevToken != TokenType::NEWLINE &&
          prevToken != TokenType::WHITESPACE) {
        tokens.insert(tokens.begin() + index, {TokenType::TEXT, token.second});
        tokens.erase(tokens.begin() + index + 1);
      }
      continue;
    }

    if (!ToPop(token)) {
      syntaxStack.push_front({token.second, index, true});
    } else {
      syntaxStack.pop_front();
    }
  }
  if (syntaxStack.empty()) {
    return;
  }

  FormatCorrection();
}

void Parser::FormatCorrection() {
  enum LastConsumed { Top, Topm1, NONE };
  std::deque<Stack> backupStack;
  LastConsumed lastConsumed = LastConsumed::NONE;
  while (!syntaxStack->empty() && syntaxStack->size() >= 2) {
    backupStack.clear();
    if (!FetchMarker(backupStack)) {
      break;
    }

    if (TOS.marker.length() == TOSm1.marker.length()) {
      if (!TOS.toErase) {
        int customCorr =
            (lastConsumed == LastConsumed::Top) ? ++correction : correction++;
        tokens.insert(tokens.begin() + TOS.index + customCorr,
                      {Parser::markerMap[TOS.marker], TOS.marker});
      }
      lastConsumed = LastConsumed::NONE;
    } else if (TOS.marker.length() > TOSm1.marker.length()) {
      StackCorrection(TOS, TOSm1);
      lastConsumed = LastConsumed::Topm1;
    } else {
      StackCorrection(TOSm1, TOS);
      lastConsumed = LastConsumed::Top;
    }
    if (backupStack.size() > 0) {
      auto* saveContext = syntaxStack;
      syntaxStack = &backupStack;
      FormatCorrection();
      syntaxStack = saveContext;
    }
  }
  EmptyStack();
}

void Parser::EmptyStack() {
  while (!syntaxStack->empty()) {
    tokens.insert(tokens.begin() + syntaxStack->back().index + ++correction,
                  {TokenType::TEXT, syntaxStack->back().marker});
    if (syntaxStack->back().toErase) {
      tokens.erase(tokens.begin() + syntaxStack->back().index + --correction);
    }
    syntaxStack->pop_back();
  }
}

bool Parser::FetchMarker(std::deque<Stack>& backupStack) {
  TOS = syntaxStack->back();
  syntaxStack->pop_back();

  TOSm1 = syntaxStack->back();
  syntaxStack->pop_back();
  if (TOS.marker[0] != TOSm1.marker[0]) {
    while (syntaxStack->size() >= 1 && TOS.marker[0] != TOSm1.marker[0]) {
      backupStack.push_back(TOSm1);
      TOSm1 = syntaxStack->back();
      syntaxStack->pop_back();
    }
    if (syntaxStack->size() == 0 && TOS.marker[0] != TOSm1.marker[0]) {
      syntaxStack->push_back(TOSm1);
      syntaxStack->push_back(TOS);
      return false;
    }
  }
  return true;
}

void Parser::StackCorrection(Stack& HighItem, Stack& LowItem) {
  HighItem.marker = HighItem.marker.substr(
      0, HighItem.marker.length() - LowItem.marker.length());
  syntaxStack->push_back(std::move(HighItem));

  if (!LowItem.toErase) {
    tokens.insert(tokens.begin() + LowItem.index + correction++,
                  {Parser::markerMap[LowItem.marker], LowItem.marker});
  }
  if (syntaxStack->back().toErase) {
    tokens.erase(tokens.begin() + syntaxStack->back().index + correction--);
    syntaxStack->back().toErase = false;
  }
  tokens.insert(tokens.begin() + syntaxStack->back().index + ++correction,
                {Parser::markerMap[LowItem.marker], LowItem.marker});
}

int Parser::lookAhead(const std::string& line, char&& c) {
  int count = 1;
  while ((it + count) != line.end() && *(it + count) == c) {
    count++;
  }
  followsWhiteSpace = (*(it + count) == ' ');
  return count;
}
