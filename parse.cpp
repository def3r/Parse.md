#include "parse.h"

#include <deque>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

const std::string TokenStr(const TokenType& token) {
  switch (token) {
#define X(TOKEN_NAME, TOKEN_VAL) \
  case TokenType::TOKEN_NAME:    \
    return "Token::" #TOKEN_NAME;
    TOKENS
#undef X
  }
  return "Token::NONE";
}

std::ostream& operator<<(std::ostream& os, const TokenType& tt) {
  os << TokenStr(tt);
  return os;
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
    marker[0] == '_'
    // || marker[0] == '#'
  ) {  // clang-format on
    return TokenType::NONE;
  }
  return TokenType::TEXT;
}

void Parser::PushToken(const std::string& lexeme) {
  // std::cout << Parser::GetMarker(lexeme) << "|" << lexeme << "|" <<
  // std::endl;
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
}
void Parser::Parse(const std::string& str) {
  Tokenize(str);
  Lexer();
  Parse();
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
  return (!syntaxStack->empty() && syntaxStack->front().marker == token.second);
}

void Parser::FormatCorrectionInit() {
  int index = -1;
  correction = 0;
  std::deque<Stack> syntaxStack = {};
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
      // std::cout << syntaxStack.front().marker << " <:> " << token.second <<
      // "\n";
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

std::shared_ptr<Node> Parser::MakeTree(Tokens::iterator& it) {
  if (it == this->tokens.end())
    return nullptr;
  if (it->first == TokenType::NONE) {
    ++it;
    return nullptr;
  }

  if (it->first >= TokenType::H1 && it->first <= TokenType::H6) {
    Tokens::iterator itNext = it + 1;
    if (itNext == this->tokens.end()) {
      it->first = TokenType::TEXT;
    } else if (itNext->first == TokenType::WHITESPACE) {
      itNext->first = TokenType::NONE;
    } else if (itNext->first == TokenType::TEXT && itNext->second[0] == ' ') {
      ltrim(itNext->second);
    } else {
      it->first = TokenType::TEXT;
    }
  }

  std::shared_ptr<Node> node = std::make_shared<Node>(
      it->first == TokenType::WHITESPACE ? TokenType::TEXT : it->first);

  if (!(it->first == TokenType::TEXT || it->first == TokenType::WHITESPACE)) {
    TokenType endTokenType =
        (it->first >= TokenType::H1 && it->first <= TokenType::H6)
            ? TokenType::NEWLINE
            : it->first;
    ++it;
    while (it != this->tokens.end() && it->first != endTokenType) {
      std::shared_ptr<Node> child = MakeTree(it);
      if (child == nullptr)
        continue;
      node->children.push_back(child);
    }
    it = (it == this->tokens.end()) ? it : it + 1;
    return node;
  }

  while (it != this->tokens.end() &&
         (it->first == TokenType::TEXT || it->first == TokenType::WHITESPACE)) {
    node->value += it->second;
    ++it;
  }

  return node;
}

std::shared_ptr<Node> Parser::GetDoc() {
  std::shared_ptr<Node> root = std::make_shared<Node>(TokenType::ROOT);
  Tokens::iterator it = this->tokens.begin();

  while (it != this->tokens.end()) {
    std::shared_ptr<Node> child = MakeTree(it);
    if (child == nullptr)
      continue;
    root->children.push_back(child);
  }

  return root;
}

Node::Node() {
  this->type = TokenType::NONE;
  this->value = "";
  this->children = {};
}

Node::Node(TokenType type) : type(type) {
  this->value = "";
  this->children = {};
}

void ltrim(std::string& s) {
  s.erase(0, s.find_first_not_of(' '));
}
