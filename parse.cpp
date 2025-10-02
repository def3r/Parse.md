#include "parse.h"

#include <deque>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

TokenType operator+(TokenType t, int i) {
  return static_cast<TokenType>(static_cast<int>(t) + i);
}

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
  lexTokens.push_back({Parser::GetMarker(lexeme), lexeme});
}
void Parser::PushToken(TokenType type, const std::string& lexeme) {
  lexTokens.push_back({type, lexeme});
}

void Parser::LexAnalysis() {
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

std::shared_ptr<Node> Parser::Parse() {
  FormatCorrectionInit();
  FinalPass();
  return this->root;
}
std::shared_ptr<Node> Parser::Parse(const std::string& str) {
  Tokenize(str);
  LexAnalysis();
  return Parse();
}

Tokens Parser::getLexTokens() {
  return lexTokens;
}

void Parser::debug() {
  for (auto& token : lexTokens) {
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
  for (const auto& token : lexTokens) {
    index++;
    // Skip for NEWLINE, WHITESPACE, TEXT
    if (token.first >= TokenType::NEWLINE && token.first <= TokenType::TEXT) {
      continue;
    }

    if (token.first >= TokenType::H1 && token.first <= TokenType::H6) {
      if (index == 0) {
        continue;
      }
      TokenType prevToken = lexTokens[index - 1].first;
      if (prevToken != TokenType::NEWLINE &&
          prevToken != TokenType::WHITESPACE) {
        lexTokens.insert(lexTokens.begin() + index,
                         {TokenType::TEXT, token.second});
        lexTokens.erase(lexTokens.begin() + index + 1);
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
        lexTokens.insert(lexTokens.begin() + TOS.index + customCorr,
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
    lexTokens.insert(
        lexTokens.begin() + syntaxStack->back().index + ++correction,
        {TokenType::TEXT, syntaxStack->back().marker});
    if (syntaxStack->back().toErase) {
      lexTokens.erase(lexTokens.begin() + syntaxStack->back().index +
                      --correction);
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
    lexTokens.insert(lexTokens.begin() + LowItem.index + correction++,
                     {Parser::markerMap[LowItem.marker], LowItem.marker});
  }
  if (syntaxStack->back().toErase) {
    lexTokens.erase(lexTokens.begin() + syntaxStack->back().index +
                    correction--);
    syntaxStack->back().toErase = false;
  }
  lexTokens.insert(lexTokens.begin() + syntaxStack->back().index + ++correction,
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

std::shared_ptr<Node> Parser::BuildTree() {
  if (itToken == this->lexTokens.end())
    return nullptr;
  if (itToken->first == TokenType::NONE) {
    itTokenInc();
    return nullptr;
  }
  if (isHeading(itToken->first) && !validHeading()) {
    return nullptr;
  }
  if (isParagraphEnd()) {
    return nullptr;
  }

  // clang-format off
  TokenType tokenType = itToken->first == TokenType::WHITESPACE
                        ? TokenType::TEXT : itToken->first;
  if (tokenType == TokenType::NEWLINE) {
    itTokenInc();
    return this->containerType == ContainerType::PARAGRAPH
           ? std::make_shared<Node>(" ") : nullptr;
  }
  if (this->containerType == ContainerType::ROOT && !isHeading(tokenType)) {
    tokenType = TokenType::PARAGRAPH;
    this->containerType = ContainerType::PARAGRAPH;
  }
  // clang-format on

  std::shared_ptr<Node> node = std::make_shared<Node>(tokenType);
  if (tokenType == TokenType::PARAGRAPH) {
    BuildParagraph(*node);
  } else if (tokenType == TokenType::TEXT) {
    BuildText(*node);
  } else {
    BuildChildren(*node);
    itTokenInc();
  }

  return node;
}

Tokens::iterator Parser::itTokenInc() {
  return itTokenInc(1);
}
Tokens::iterator Parser::itTokenInc(int inc) {
  Tokens::iterator cur = itToken;
  itToken = (itToken == this->lexTokens.end()) ? itToken : itToken + inc;
  return cur;
}

void Parser::BuildParagraph(Node& node) {
  while (itToken != this->lexTokens.end()) {
    this->containerType = ContainerType::PARAGRAPH;
    std::shared_ptr<Node> child = BuildTree();
    if (child != nullptr) {
      node.children.push_back(child);
    } else if (isParagraphEnd()) {
      itToken = itToken + 2;
      return;
    } else if (this->containerType == ContainerType::HEADING) {
      return;
    }
  }
}

void Parser::BuildText(Node& node) {
  while (itToken != this->lexTokens.end() &&
         (itToken->first == TokenType::TEXT ||
          itToken->first == TokenType::WHITESPACE)) {
    node.value += itToken->second;
    itTokenInc();
  }
}

void Parser::BuildChildren(Node& node) {
  TokenType endTokenType =
      (isHeading(node.type)) ? TokenType::NEWLINE : itToken->first;
  itTokenInc();
  while (itToken != this->lexTokens.end() && itToken->first != endTokenType) {
    if (isHeading(node.type)) {
      this->containerType = ContainerType::HEADING;
    }
    std::shared_ptr<Node> child = BuildTree();
    if (child != nullptr) {
      node.children.push_back(child);
    }
  }
}

bool Parser::validHeading() {
  Tokens::iterator itNext = itToken + 1;
  if (itNext == this->lexTokens.end()) {
    itToken->first = TokenType::TEXT;
  } else if (itNext->first == TokenType::WHITESPACE) {
    itNext->first = TokenType::NONE;
  } else if (itNext->first == TokenType::TEXT && itNext->second[0] == ' ') {
    if (this->containerType == ContainerType::PARAGRAPH) {
      this->containerType = ContainerType::HEADING;
      return false;
    }
    ltrim(itNext->second);
  } else {
    itToken->first = TokenType::TEXT;
  }
  return true;
}

bool Parser::isParagraphEnd() const {
  return (itToken + 1) != this->lexTokens.end() &&
         itToken->first == TokenType::NEWLINE &&
         (itToken + 1)->first == TokenType::NEWLINE &&
         this->containerType == ContainerType::PARAGRAPH;
}

std::shared_ptr<Node> Parser::GetRoot() {
  return this->root;
}

std::shared_ptr<Node> Parser::FinalPass() {
  root = std::make_shared<Node>(TokenType::ROOT);
  itToken = this->lexTokens.begin();

  while (itToken != this->lexTokens.end()) {
    this->containerType = ContainerType::ROOT;
    std::shared_ptr<Node> child = BuildTree();
    if (child != nullptr)
      root->children.push_back(child);
  }

  return root;
}

std::string Parser::DumpTree(const std::shared_ptr<Node>& node, int depth) {
  if (node == nullptr)
    return "";

  std::stringstream ss;
  ss << std::string(depth * 2, ' ') << node->type;
  if (!node->value.empty()) {
    ss << ' ' << node->value;
  }
  ss << "\n";
  for (auto child : node->children) {
    ss << DumpTree(child, depth + 1);
  }

  return ss.str();
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

Node::Node(std::string value) : value(value) {
  this->type = TokenType::TEXT;
  this->children = {};
}

void ltrim(std::string& s) {
  s.erase(0, s.find_first_not_of(' '));
}
