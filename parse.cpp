#include "parse.h"

#include <deque>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace markdown {

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
  return os << TokenStr(tt);
}

Parser::Parser() {}

void Parser::PushLexeme() {
  updateBegin_ = true;
  // std::string text = (begin_ >= it_) ? "" : std::string(begin_, it_);
  if (begin_ < it_) {
    lexemes_.emplace_back(
        begin_,
        std::distance(begin_, it_));  // C++ 20 makes life easier but, whatever
  }
}

void Parser::PushLexeme(size_t count) {
  updateBegin_ = true;
  PushLexeme();
  if (it_ + count != it_) {
    lexemes_.emplace_back(it_, count);
  }
}

bool Parser::IsDelimiter() {
  // clang-format off
  switch (*it_) {
    case '*': return true;
    case '_': return true;
    case '#': return true;
  }
  // clang-format on
  return false;
}

void Parser::Tokenize(std::string_view doc) {
  document_ = doc;
  begin_ = doc.begin();
  int count = 1;
  for (it_ = doc.begin(); it_ != doc.end(); ++it_) {
    if (*it_ == '\n') {
      count = 1;
      PushLexeme(count);
    } else if (IsDelimiter()) {
      count = LookAhead(doc, *it_);
      PushLexeme(count);
    }

    if (updateBegin_) {
      begin_ = it_ + count;
      it_ = it_ + --count;
      updateBegin_ = false;
    }
  }
  PushLexeme();
}

void Parser::PushToken(std::string_view lexeme) {
  // std::cout << Parser::GetMarker(lexeme) << "|" << lexeme << "|" <<
  // std::endl;
  candTokens_.push_back({detail::GetMarker(lexeme), lexeme});
}
void Parser::PushToken(TokenType type, std::string_view lexeme) {
  candTokens_.push_back({type, lexeme});
}

void Parser::LexAnalysis() {
  for (const auto& lexeme : lexemes_) {
    if (lexeme[0] == ' ') {
      it_ = lexeme.begin();
      if (LookAhead(lexeme, ' ') == lexeme.size()) {
        PushToken(TokenType::Whitespace, lexeme);
        continue;
      }
    }
    PushToken(lexeme);
  }
  return;
}

Tree Parser::Parse() {
  FormatCorrectionInit();
  FinalPass();
  return this->root_;
}
Tree Parser::Parse(std::string_view doc) {
  Tokenize(doc);
  LexAnalysis();
  return Parse();
}

Tokens Parser::GetTokens() {
  return candTokens_;
}

void Parser::debug() {
  for (auto& token : candTokens_) {
    std::cout << TokenStr(token.first) << "\t\t" << token.second << "\n";
  }
}

void Parser::ClearStack() {
  while (!syntaxStack_->empty()) {
    syntaxStack_->pop_back();
  }
}

bool Parser::ToPop(const Token& token) {
  return (!syntaxStack_->empty() &&
          syntaxStack_->front().marker == token.second);
}

void Parser::FormatCorrectionInit() {
  int index = -1;
  correction_ = 0;
  std::deque<StackItem> syntaxStack = {};
  this->syntaxStack_ = &syntaxStack;
  for (const auto& token : candTokens_) {
    index++;
    // Skip for NEWLINE, WHITESPACE, TEXT
    if (token.first >= TokenType::Newline && token.first <= TokenType::Text) {
      continue;
    }

    if (token.first >= TokenType::H1 && token.first <= TokenType::H6) {
      if (index == 0) {
        continue;
      }
      TokenType prevToken = candTokens_[index - 1].first;
      if (prevToken != TokenType::Newline &&
          prevToken != TokenType::Whitespace) {
        candTokens_.insert(candTokens_.begin() + index,
                           {TokenType::Text, token.second});
        candTokens_.erase(candTokens_.begin() + index + 1);
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
  std::deque<StackItem> backupStack;
  LastConsumed lastConsumed = LastConsumed::NONE;
  while (!syntaxStack_->empty() && syntaxStack_->size() >= 2) {
    backupStack.clear();
    if (!FetchMarker(backupStack)) {
      break;
    }

    if (TOS_.marker.length() == TOSm1_.marker.length()) {
      if (!TOS_.toErase) {
        int customCorr =
            (lastConsumed == LastConsumed::Top) ? ++correction_ : correction_++;
        candTokens_.insert(candTokens_.begin() + TOS_.index + customCorr,
                           {detail::GetMarker(TOS_.marker), TOS_.marker});
      }
      lastConsumed = LastConsumed::NONE;
    } else if (TOS_.marker.length() > TOSm1_.marker.length()) {
      StackCorrection(TOS_, TOSm1_);
      lastConsumed = LastConsumed::Topm1;
    } else {
      StackCorrection(TOSm1_, TOS_);
      lastConsumed = LastConsumed::Top;
    }
    if (backupStack.size() > 0) {
      auto* saveContext = syntaxStack_;
      syntaxStack_ = &backupStack;
      FormatCorrection();
      syntaxStack_ = saveContext;
    }
  }
  EmptyStack();
}

void Parser::EmptyStack() {
  while (!syntaxStack_->empty()) {
    candTokens_.insert(
        candTokens_.begin() + syntaxStack_->back().index + ++correction_,
        {TokenType::Text, syntaxStack_->back().marker});
    if (syntaxStack_->back().toErase) {
      candTokens_.erase(candTokens_.begin() + syntaxStack_->back().index +
                        --correction_);
    }
    syntaxStack_->pop_back();
  }
}

bool Parser::FetchMarker(std::deque<StackItem>& backupStack) {
  TOS_ = syntaxStack_->back();
  syntaxStack_->pop_back();

  TOSm1_ = syntaxStack_->back();
  syntaxStack_->pop_back();
  if (TOS_.marker[0] != TOSm1_.marker[0]) {
    while (syntaxStack_->size() >= 1 && TOS_.marker[0] != TOSm1_.marker[0]) {
      backupStack.push_back(TOSm1_);
      TOSm1_ = syntaxStack_->back();
      syntaxStack_->pop_back();
    }
    if (syntaxStack_->size() == 0 && TOS_.marker[0] != TOSm1_.marker[0]) {
      syntaxStack_->push_back(TOSm1_);
      syntaxStack_->push_back(TOS_);
      return false;
    }
  }
  return true;
}

void Parser::StackCorrection(StackItem& HighItem, StackItem& LowItem) {
  HighItem.marker = HighItem.marker.substr(
      0, HighItem.marker.length() - LowItem.marker.length());
  syntaxStack_->push_back(std::move(HighItem));

  if (!LowItem.toErase) {
    candTokens_.insert(candTokens_.begin() + LowItem.index + correction_++,
                       {detail::GetMarker(LowItem.marker), LowItem.marker});
  }
  if (syntaxStack_->back().toErase) {
    candTokens_.erase(candTokens_.begin() + syntaxStack_->back().index +
                      correction_--);
    syntaxStack_->back().toErase = false;
  }
  candTokens_.insert(
      candTokens_.begin() + syntaxStack_->back().index + ++correction_,
      {detail::GetMarker(LowItem.marker), LowItem.marker});
}

int Parser::LookAhead(std::string_view line, const char& c) {
  int count = 1;
  while ((it_ + count) != line.end() && *(it_ + count) == c) {
    count++;
  }
  followsWhiteSpace_ = (*(it_ + count) == ' ');
  return count;
}

Tree Parser::BuildTree() {
  if (itToken_ == this->candTokens_.end())
    return nullptr;
  if (itToken_->first == TokenType::None) {
    itTokenInc();
    return nullptr;
  }
  if (isHeading(itToken_->first) && !validHeading()) {
    return nullptr;
  }
  if (isParagraphEnd()) {
    return nullptr;
  }

  // clang-format off
  TokenType tokenType = itToken_->first == TokenType::Whitespace
                        ? TokenType::Text : itToken_->first;
  if (tokenType == TokenType::Newline) {
    itTokenInc();
    return this->containerType_ == ContainerType::Paragraph
           ? std::make_shared<Node>(" ") : nullptr;
  }
  if (this->containerType_ == ContainerType::Root && !isHeading(tokenType)) {
    tokenType = TokenType::Paragraph;
    this->containerType_ = ContainerType::Paragraph;
  }
  // clang-format on

  std::shared_ptr<Node> node = std::make_shared<Node>(tokenType);
  if (tokenType == TokenType::Paragraph) {
    BuildParagraph(*node);
  } else if (tokenType == TokenType::Text) {
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
  Tokens::iterator cur = itToken_;
  itToken_ = (itToken_ == this->candTokens_.end()) ? itToken_ : itToken_ + inc;
  return cur;
}

void Parser::BuildParagraph(Node& node) {
  while (itToken_ != this->candTokens_.end()) {
    this->containerType_ = ContainerType::Paragraph;
    std::shared_ptr<Node> child = BuildTree();
    if (child != nullptr) {
      node.children.push_back(child);
    } else if (isParagraphEnd()) {
      itToken_ = itToken_ + 2;
      return;
    } else if (this->containerType_ == ContainerType::Heading) {
      return;
    }
  }
}

void Parser::BuildText(Node& node) {
  while (itToken_ != this->candTokens_.end() &&
         (itToken_->first == TokenType::Text ||
          itToken_->first == TokenType::Whitespace)) {
    node.value += itToken_->second;
    itTokenInc();
  }
}

void Parser::BuildChildren(Node& node) {
  TokenType endTokenType =
      (isHeading(node.type)) ? TokenType::Newline : itToken_->first;
  itTokenInc();
  while (itToken_ != this->candTokens_.end() &&
         itToken_->first != endTokenType) {
    if (isHeading(node.type)) {
      this->containerType_ = ContainerType::Heading;
    }
    std::shared_ptr<Node> child = BuildTree();
    if (child != nullptr) {
      node.children.push_back(child);
    }
  }
}

bool Parser::validHeading() {
  Tokens::iterator itNext = itToken_ + 1;
  if (itNext == this->candTokens_.end()) {
    itToken_->first = TokenType::Text;
  } else if (itNext->first == TokenType::Whitespace) {
    itNext->first = TokenType::None;
  } else if (itNext->first == TokenType::Text && itNext->second[0] == ' ') {
    if (this->containerType_ == ContainerType::Paragraph) {
      this->containerType_ = ContainerType::Heading;
      return false;
    }
    ltrim(itNext->second);
  } else {
    itToken_->first = TokenType::Text;
  }
  return true;
}

bool Parser::isParagraphEnd() const {
  return (itToken_ + 1) != this->candTokens_.end() &&
         itToken_->first == TokenType::Newline &&
         (itToken_ + 1)->first == TokenType::Newline &&
         this->containerType_ == ContainerType::Paragraph;
}

Tree Parser::GetRoot() {
  return this->root_;
}

Tree Parser::FinalPass() {
  root_ = std::make_shared<Node>(TokenType::Root);
  itToken_ = this->candTokens_.begin();

  while (itToken_ != this->candTokens_.end()) {
    this->containerType_ = ContainerType::Root;
    std::shared_ptr<Node> child = BuildTree();
    if (child != nullptr)
      root_->children.push_back(child);
  }

  return root_;
}

std::string Parser::DumpTree(const Tree& node, int depth) {
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
  this->type = TokenType::None;
  this->value = "";
  this->children = {};
}

Node::Node(TokenType type) : type(type) {
  this->value = "";
  this->children = {};
}

Node::Node(std::string value) : value(value) {
  this->type = TokenType::Text;
  this->children = {};
}

void ltrim(std::string_view& s) {
  s.remove_prefix(s.find_first_not_of(' '));
}

TokenType detail::GetMarker(std::string_view marker) {
  for (Marker m : markers) {
    if (m.marker == marker)
      return m.type;
  }
  // clang-format off
  if (
    marker[0] == '*' ||
    marker[0] == '_'
    // || marker[0] == '#'
  ) {  // clang-format on
    return TokenType::None;
  }
  return TokenType::Text;
}

}  // namespace markdown
