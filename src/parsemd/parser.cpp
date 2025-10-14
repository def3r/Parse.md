#include "parsemd/parser.h"
#include "parsemd/delimiterstack.h"
#include "parsemd/internal.h"
#include "parsemd/node.h"
#include "parsemd/types.h"
#include "parsemd/utils.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace markdown {

Parser::Parser() {}

Block Parser::GetBlock() {
  return block_;
}

void Parser::AssignDocument(std::string_view doc) {
  scanner.Init(doc);
  document_ = doc;
}

Block Parser::BuildBlocks() {
  std::string_view line;
  while ((line = scanner.ScanNextLine()) != "" || !scanner.End()) {
    if (line.empty()) {
      continue;
    }

    size_t pos = line.find_first_not_of(' ');
    // lets not care about whitespace count rn (codeblock burn)
    if (line[pos] == '#') {
      Scanner scnr(line);
      int count = scnr.LookAhead(Scanner::CurPos::Begin, pos);
      std::string_view marker(line.begin() + pos, count);

      if (scnr.followedByWhiteSpace_ &&
          internal::GetMarker(marker) != TokenType::Text) {
        markdown::trim(line, pos + count + 1);
        return {
            .type = internal::GetMarker(marker),
            .isOpen = false,
            .text = line,
            .children = {},
        };
      }
    }

    // always true for now
    if (blockType_ == BlockType::Root) {
      return BuildParagraphBlock();
    }

    std::cout << std::quoted(line) << "\n";
  }
  return {};
}

Block Parser::BuildParagraphBlock() {
  Block paragraph = Block{
      .type = TokenType::Paragraph,
      .isOpen = false,
      .text = {},
      .children = {},
  };
  std::string_view line = scanner.CurrentLine();
  std::string_view::iterator begin = line.begin();
  size_t count = 0;

  while (!line.empty() || !scanner.End()) {
    // paragraph ends
    size_t pos = line.find_first_not_of(' ');
    if (line.empty() || pos == std::string_view::npos) {
      std::string_view text(begin, count - 1);
      internal::htrim(text);
      paragraph.text = text;
      return paragraph;
    }
    count += line.size() + 1;
    line = scanner.ScanNextLine();
  }

  std::string_view text(begin, count);
  internal::htrim(text);
  paragraph.text = text;
  return paragraph;
}

void Parser::AnalyzeBlocks(std::string_view doc) {
  AssignDocument(doc);
  scanner.Init(doc);
  block_.type = TokenType::Root;
  block_.text = "";

  while (!scanner.End()) {
    blockType_ = BlockType::Root;
    Block child = BuildBlocks();
    if (child.type != TokenType::None) {
      block_.children.push_back(child);
    }
  }
}

Block Parser::BuildInline(Tokens::iterator it) {
  if (candTokens_.empty() || it == candTokens_.end())
    return {};

  Block b = {
      .type = TokenType::None,
      .isOpen = true,
      .text = "",
      .children = {},
  };

  TokenType type = (*it)->first;
  if (type == TokenType::Text) {
    b.type = TokenType::Text;
    Tokens::iterator begin = it;
    size_t count = 0;
    while (it != candTokens_.end() && (*it)->first == TokenType::Text) {
      count += (*it)->second.size();
      ++it;
    }
    b.text = std::string_view((*begin)->second.begin(), count);
    // PERF: Probably consider using a list instead of vector?
    candTokens_.erase(begin, it);
  }

  else if (type == TokenType::Softbreak) {
    b.type = TokenType::Softbreak;
    candTokens_.erase(it);
  }

  else if (type == TokenType::EmphOpen) {
    b.type = TokenType::Emph;
    while ((*(it + 1))->first != TokenType::EmphClose) {
      b.children.push_back(BuildInline(it + 1));
    }
    candTokens_.erase(it, it + 2);
  }

  else if (type == TokenType::StrongOpen) {
    b.type = TokenType::Strong;
    while ((*(it + 1))->first != TokenType::StrongClose) {
      b.children.push_back(BuildInline(it + 1));
    }
    candTokens_.erase(it, it + 2);
  }

  return b;
}  // namespace markdown

void Parser::AnalyzeInline() {
  if (block_.type == TokenType::None) {
    return;
  }

  int count = 1;
  for (Block& block : block_.children) {
    scanner.Init(block.text);
    while (!scanner.End()) {
      char c = scanner.ScanNextByte();
      if (c == '\n') {
        PushCandToken();
        scanner.Flush();
        std::string_view lexeme = scanner.Scan(1, Scanner::CurPos::BeginIt);
        candTokens_.push_back(
            std::make_shared<Token>(TokenType::Softbreak, lexeme));
        scanner.FlushBytes(1);
      } else if (internal::IsDelimiter(c)) {
        count = scanner.LookAhead(Scanner::CurPos::Cur, -1);
        if (internal::IsValidDelimiter(
                scanner.At(Scanner::CurPos::Cur, -2), c,
                scanner.At(Scanner::CurPos::Cur, count - 1))) {
          PushCandToken(count);
        } else {
          scanner.SkipNextBytes(count - 1);
        }
      }
    }
    PushCandToken();

    // for (auto token : candTokens_) {
    //   std::cout << token->first << "\t" << std::quoted(token->second)
    //             << std::endl;
    // }
    // delimStack.debug();
    delimStack.ProcessEmphasis(candTokens_);
    // for (auto token : candTokens_) {
    //   std::cout << token->first << "\t" << std::quoted(token->second)
    //             << std::endl;
    // }
    // delimStack.debug();
    delimStack.Clear();

    while (!candTokens_.empty()) {
      block.children.push_back(BuildInline(candTokens_.begin()));
    }
    block.text = {};
    candTokens_ = {};
  }
}

void Parser::PushCandToken() {
  std::string_view lexeme = scanner.CurrentLine();
  if (!lexeme.empty() && lexeme != "\n") {
    candTokens_.push_back(std::make_shared<Token>(TokenType::Text, lexeme));
  }
}
void Parser::PushCandToken(size_t count) {
  std::string_view lexeme = scanner.CurrentLine();
  if (!lexeme.empty()) {
    lexeme.remove_suffix(1);
    candTokens_.push_back(std::make_shared<Token>(TokenType::Text, lexeme));
  }
  scanner.Flush();

  lexeme = scanner.Scan(count, Scanner::CurPos::BeginIt);
  if (lexeme.empty()) {
    std::cerr << "WARNING: Trying to push empty token as a candidate token!"
              << std::endl;
    return;
  }

  // NOTE: push after validation of char c
  candTokens_.push_back(
      std::make_shared<Token>(internal::GetMarker(lexeme), lexeme));
  std::shared_ptr<Token> tokenPtr = candTokens_.back();
  char c = scanner.CurrentByte();
  char prev = scanner.At(Scanner::CurPos::Cur, -2);
  char next = scanner.At(Scanner::CurPos::Cur, count - 1);
  // std::cout << prev << "|" << c << "|" << next << "|" << count << std::endl;

  DelimiterStack::DelimiterStackItem dsi = {
      .delim = (c == '*') ? DelimiterStack::Delimiter::Asteriks
                          : DelimiterStack::Delimiter::Underscore,
      .number = count,
      .isActive = true,
      .type = DelimiterStack::DelimiterType::Open,
      .tokenPtr = tokenPtr,
  };

  if (c == '*') {
    int state = 0;
    if (internal::IsLeftFlanking(prev, next)) {
      state++;
    }
    if (internal::IsRightFlanking(prev, next)) {
      state += 2;
    }

    switch (state) {
      case 1:
        dsi.type = DelimiterStack::DelimiterType::Open;
        break;
      case 2:
        dsi.type = DelimiterStack::DelimiterType::Close;
        break;
      case 3:
        dsi.type = DelimiterStack::DelimiterType::Both;
        break;
    }

  } else if (c == '_') {
    int state = 0;
    if (internal::IsLeftFlanking(prev, next) &&
        (!internal::IsRightFlanking(prev, next) ||
         internal::IsPunctuation(prev))) {
      state++;
    }
    if (internal::IsRightFlanking(prev, next) &&
        (!internal::IsLeftFlanking(prev, next) ||
         internal::IsPunctuation(next))) {
      state += 2;
    }

    switch (state) {
      case 1:
        dsi.type = DelimiterStack::DelimiterType::Open;
        break;
      case 2:
        dsi.type = DelimiterStack::DelimiterType::Close;
        break;
      case 3:
        dsi.type = DelimiterStack::DelimiterType::Both;
        break;
    }

  } else {
    std::cerr << "Unhandled Delimiter " << std::quoted(std::to_string(c))
              << std::endl;
    std::exit(1);
  }

  delimStack.Push(dsi);
  // delimiterStack.emplace_back(dsi);

  scanner.FlushBytes(count);
}

void Parser::Tokenize(std::string_view doc) {
  AssignDocument(doc);
}

void Parser::LexAnalysis() {}

Tree Parser::Parse() {
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
    std::cout << TokenStr(token->first) << "\t\t" << token->second << "\n";
  }
}

Tree Parser::BuildTree() {
  if (itToken_ == this->candTokens_.end())
    return nullptr;
  if ((*itToken_)->first == TokenType::None) {
    itTokenInc();
    return nullptr;
  }
  if (isHeading((*itToken_)->first) && !validHeading()) {
    return nullptr;
  }
  if (isParagraphEnd()) {
    return nullptr;
  }

  // clang-format off
  TokenType tokenType = (*itToken_)->first == TokenType::Whitespace
                        ? TokenType::Text : (*itToken_)->first;
  if (tokenType == TokenType::Softbreak) {
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
         ((*itToken_)->first == TokenType::Text ||
          (*itToken_)->first == TokenType::Whitespace)) {
    node.value += (*itToken_)->second;
    itTokenInc();
  }
}

void Parser::BuildChildren(Node& node) {
  TokenType endTokenType =
      (isHeading(node.type)) ? TokenType::Softbreak : (*itToken_)->first;
  itTokenInc();
  while (itToken_ != this->candTokens_.end() &&
         (*itToken_)->first != endTokenType) {
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
    (*itToken_)->first = TokenType::Text;
  } else if ((*itNext)->first == TokenType::Whitespace) {
    (*itNext)->first = TokenType::None;
  } else if ((*itNext)->first == TokenType::Text &&
             (*itNext)->second[0] == ' ') {
    if (this->containerType_ == ContainerType::Paragraph) {
      this->containerType_ = ContainerType::Heading;
      return false;
    }
    ltrim((*itNext)->second);
  } else {
    (*itToken_)->first = TokenType::Text;
  }
  return true;
}

bool Parser::isParagraphEnd() const {
  return (itToken_ + 1) != this->candTokens_.end() &&
         (*itToken_)->first == TokenType::Softbreak &&
         (*(itToken_ + 1))->first == TokenType::Softbreak &&
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

std::string Parser::DumpTree(const Block& node, int depth) {
  std::stringstream ss;
  ss << std::string(depth * 2, ' ') << node.type;
  if (!node.text.empty()) {
    ss << ' ' << std::quoted(node.text);
  }
  ss << "\n";
  for (auto child : node.children) {
    ss << DumpTree(child, depth + 1);
  }

  return ss.str();
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

}  // namespace markdown
