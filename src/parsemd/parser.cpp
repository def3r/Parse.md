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

namespace markdown {

Parser::Parser() {}

Node Parser::GetBlock() {
  return std::static_pointer_cast<NodeBase>(block_);
}

void Parser::AssignDocument(std::string_view doc) {
  scanner.Init(doc);
  document_ = doc;
}

Node Parser::BuildBlocks() {
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
        Block node = std::make_shared<BlockNode>(internal::GetMarker(marker));
        node->text = line;
        return std::static_pointer_cast<NodeBase>(node);
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

Node Parser::BuildParagraphBlock() {
  Block paragraph = std::make_shared<BlockNode>(TokenType::Paragraph);
  std::string_view line = scanner.CurrentLine();
  std::string_view::iterator begin = line.begin();
  size_t count = 0;

  while (!line.empty() || !scanner.End()) {
    // paragraph ends
    size_t pos = line.find_first_not_of(' ');
    if (line.empty() || pos == std::string_view::npos) {
      std::string_view text(begin, count);
      internal::htrim(text);
      paragraph->text = text;
      return std::static_pointer_cast<NodeBase>(paragraph);
    }

    count += line.size();
    line = scanner.ScanNextLine();
  }

  std::string_view text(begin, count);
  internal::htrim(text);
  paragraph->text = text;
  return std::static_pointer_cast<NodeBase>(paragraph);
}

void Parser::AnalyzeBlocks(std::string_view doc) {
  AssignDocument(doc);
  AnalyzeBlocks();
}
void Parser::AnalyzeBlocks() {
  scanner.Init(document_);
  block_ = std::make_shared<BlockNode>(TokenType::Root);

  while (!scanner.End()) {
    blockType_ = BlockType::Root;
    Node child = BuildBlocks();
    if (child) {
      block_->children.push_back(child);
    }
  }
}

Node Parser::BuildInline(TokenList::iterator it) {
  if (candTokens_.empty() || it == candTokens_.end())
    return {};

  TokenType type = it->first;
  if (type == TokenType::Text) {
    TokenList::iterator begin = it;
    size_t count = 0;
    while (it != candTokens_.end() && it->first == TokenType::Text) {
      count += it->second.size();
      ++it;
    }
    Text b = std::make_shared<TextNode>(
        TokenType::Text, std::string(begin->second.begin(), count));
    candTokens_.erase(begin, it);
    return b;
  }

  Inline b = std::make_shared<InlineNode>(TokenType::None);
  if (type == TokenType::Softbreak) {
    b->type_ = TokenType::Softbreak;
    candTokens_.erase(it);
  }

  else if (type == TokenType::EmphOpen) {
    b->type_ = TokenType::Emph;
    while (std::next(it)->first != TokenType::EmphClose) {
      b->children.push_back(BuildInline(std::next(it)));
    }
    candTokens_.erase(it, std::next(std::next(it)));
  }

  else if (type == TokenType::StrongOpen) {
    b->type_ = TokenType::Strong;
    while (std::next(it)->first != TokenType::StrongClose) {
      b->children.push_back(BuildInline(std::next(it)));
    }
    candTokens_.erase(it, std::next(std::next(it)));
  }

  return b;
}

void Parser::AnalyzeInline() {
  if (block_->type_ == TokenType::None) {
    return;
  }

  int count = 1;
  for (Node& node : block_->children) {
    if (node->type_ < TokenType::Root || node->type_ > TokenType::H6) {
      continue;
    }
    Block block = BlockNodePtr(node);

    scanner.Init(block->text);
    while (!scanner.End()) {
      char c = scanner.ScanNextByte();
      if (c == '\n') {
        PushCandToken();
        scanner.Flush();
        std::string_view lexeme = scanner.Scan(1, Scanner::CurPos::BeginIt);
        candTokens_.emplace_back(TokenType::Softbreak, lexeme);
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
      block->children.push_back(BuildInline(candTokens_.begin()));
    }
    block->text = {};
    candTokens_ = {};
  }
}

void Parser::PushCandToken() {
  std::string_view lexeme = scanner.CurrentLine();
  if (!lexeme.empty() && lexeme != "\n") {
    candTokens_.emplace_back(TokenType::Text, lexeme);
  }
}

void Parser::PushCandToken(size_t count) {
  std::string_view lexeme = scanner.CurrentLine();
  if (!lexeme.empty()) {
    lexeme.remove_suffix(1);
    candTokens_.emplace_back(TokenType::Text, lexeme);
  }
  scanner.Flush();

  lexeme = scanner.Scan(count, Scanner::CurPos::BeginIt);
  if (lexeme.empty()) {
    std::cerr << "WARNING: Trying to push empty token as a candidate token!"
              << std::endl;
    return;
  }

  // NOTE: push after validation of char c
  candTokens_.emplace_back(internal::GetMarker(lexeme), lexeme);
  TokenList::iterator tokenPtr = std::prev(candTokens_.end());
  char c = scanner.CurrentByte();
  char prev = scanner.At(Scanner::CurPos::Cur, -2);
  char next = scanner.At(Scanner::CurPos::Cur, count - 1);

  DelimiterStack::DelimiterStackItem dsi = {
      .delim = (c == '*') ? DelimiterStack::Delimiter::Asteriks
                          : DelimiterStack::Delimiter::Underscore,
      .number = count,
      .isActive = true,
      .type = DelimiterStack::DelimiterType::Open,
      .tokenIt = tokenPtr,
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

  scanner.FlushBytes(count);
}

// void Parser::Tokenize(std::string_view doc) {
//   AssignDocument(doc);
// }

Node Parser::Parse() {
  AnalyzeBlocks();
  AnalyzeInline();
  return GetBlock();
}
Node Parser::Parse(std::string_view doc) {
  AnalyzeBlocks(doc);
  AnalyzeInline();
  return Parse();
}

Node Parser::GetRoot() {
  return GetBlock();
}

TokenList Parser::GetTokens() {
  return candTokens_;
}

std::string Parser::DumpTree(const Node& node, int depth) {
  std::stringstream ss;
  ss << std::string(depth * 2, ' ') << node->type_;
  if (IsText(node->type_)) {
    ss << ' ' << std::quoted(TextNodePtr(node)->text);
    ss << "\n";
    return ss.str();
  }

  auto cnode = ContainerNodePtr(node);
  if (IsBlock(node->type_) && node->type_ != TokenType::Root) {
    auto bnode = BlockNodePtr(node);
    if (!bnode->text.empty()) {
      ss << ' ' << std::quoted(bnode->text);
    }
  }

  ss << "\n";
  for (auto child : cnode->children) {
    ss << DumpTree(child, depth + 1);
  }

  return ss.str();
}

}  // namespace markdown
