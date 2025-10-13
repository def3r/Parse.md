#include "parse.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
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

Scanner::Scanner() {
  Init("");
}
Scanner::Scanner(std::string_view data) {
  Init(data);
}

void Scanner::Init(std::string_view data) {
  data_ = data;
  begin_ = it_ = data_.begin();
}

char Scanner::At(CurPos curPos, int offset) {
  std::string_view::iterator it = GetIterator(curPos);
  auto size = std::distance(data_.begin(), it) + offset;
  if (size > data_.size() || size < 0) {
    return '\0';
  }
  if (it == data_.end() && offset > 0 || it == data_.begin() && offset < 0) {
    return '\0';
  }

  it += offset;
  // begin and end of a line are considered as whitespace
  if (it == data_.begin() || it == data_.end()) {
    return ' ';
  }

  return *it;
}

char Scanner::ScanNextByte() {
  return End() ? '\0' : *(it_++);
}

char Scanner::CurrentByte() {
  if (it_ == begin_) {
    return '\0';
  }
  return *(it_ - 1);
}

std::string_view Scanner::Scan(size_t size, CurPos curPos, int offset) {
  std::string_view::iterator it = GetIterator(curPos);
  if (!ValidArgs(offset, it)) {
    return std::string_view();
  }
  it += offset;
  if (std::distance(data_.begin(), it) + size > data_.size()) {
    return std::string_view();
  }

  return std::string_view(it, size);
}

std::string_view Scanner::ScanNextLine() {
  begin_ = it_;
  while (it_ != data_.end() && *it_ != '\n')
    ++it_;
  return std::string_view(
      begin_, std::distance(begin_, (it_ != data_.end()) ? it_++ : it_));
}

std::string_view Scanner::CurrentLine() {
  if (begin_ == it_)
    return "";
  std::string_view::iterator it = it_;
  while ((it - 1) != begin_ && *(it - 1) == '\n') {
    --it;
  }
  return std::string_view(begin_, std::distance(begin_, it));
}

std::string_view Scanner::FlushCurrentLine() {
  std::string_view line = CurrentLine();
  Flush();
  return line;
}

void Scanner::Flush() {
  if (it_ == data_.begin())
    return;
  begin_ = it_ - 1;
}

void Scanner::FlushBytes(size_t n) {
  if (std::distance(data_.begin(), begin_) + n > data_.size()) {
    std::cerr << "OVERFLOW while Flushing Bytes" << std::endl;
    begin_ = it_ = data_.end();
    return;
  }
  begin_ += n;
  it_ = begin_;
}

void Scanner::SkipNextBytes(size_t n) {
  if (std::distance(data_.begin(), it_) + n > data_.size()) {
    it_ = data_.end();
    return;
  }
  it_ = it_ + n;
}

bool Scanner::End() {
  return it_ == data_.end();
}

int Scanner::LookAhead(CurPos curPos, int offset) {
  std::string_view::iterator it = GetIterator(curPos);
  if (!ValidArgs(offset, it)) {
    return -1;
  }

  it = it + offset;
  char c = *it;
  int count = 1;
  while ((it + count) != data_.end() && *(it + count) == c) {
    count++;
  }

  followedByWhiteSpace_ = (*(it + count) == ' ');
  return count;
}

std::string_view::iterator Scanner::GetIterator(CurPos curPos) {
  switch (curPos) {
    case CurPos::Begin:
      return data_.begin();
    case CurPos::BeginIt:
      return begin_;
    case CurPos::Cur:
      return it_;
    case CurPos::EndIt:
      return it_;
    default:
      return data_.end();
  }
}

bool Scanner::ValidArgs(int offset, std::string_view::iterator it) {
  auto size = std::distance(data_.begin(), it) + offset;
  if (size > data_.size() || size < 0)
    return false;
  if (it == data_.end() && offset >= 0 || it == data_.begin() && offset < 0)
    return false;
  return true;
}

DelimiterStack::Node::Node() : dsi({}), next(nullptr), prev(nullptr) {}

DelimiterStack::Node::Node(DelimiterStackItem dsi,
                           std::shared_ptr<Node> next,
                           std::shared_ptr<Node> prev)
    : dsi(dsi), next(next), prev(prev) {}

void DelimiterStack::Node::Detach() {
  this->prev = nullptr;
  this->next = nullptr;
}

DelimiterStack::DelimiterStack()
    : stackBottom(nullptr), openersBottom{nullptr, nullptr} {
  head = std::make_shared<Node>();
  tail = std::make_shared<Node>();
  head->next = tail;
  tail->prev = head;
  cur = head;
}

void DelimiterStack::Push(DelimiterStackItem dsi) {
  std::shared_ptr<Node> newNode = std::make_shared<Node>(dsi, tail, cur);
  if (!newNode) {
    std::cerr << "Unable to allocate memory for New Delimiter Stack node"
              << std::endl;
    exit(1);
  }
  cur->next = newNode;
  tail->prev = newNode;
  cur = newNode;
}

void DelimiterStack::Clear() {
  cur = head->next;
  while (cur != tail) {
    cur = cur->next;
    cur->prev->Detach();
  }
  head->next = tail;
  tail->prev = head;
  cur = head;
  stackBottom = nullptr;
  openersBottom[0] = openersBottom[1] = nullptr;
}

bool DelimiterStack::ProcessEmphasis(Tokens& candTokens) {
  size_t size = candTokens.size();
  // NOTE: Assumption: No Links or Image support yet
  stackBottom = head;
  openersBottom[0] = openersBottom[1] = stackBottom;
  cur = stackBottom->next;

  while (cur != tail) {
    while (cur != tail && cur->dsi.type == DelimiterType::Open) {
      // std::cout << cur->dsi.tokenPtr->second << "|" << std::endl;
      if (openersBottom[static_cast<int>(cur->dsi.delim)] == stackBottom) {
        openersBottom[static_cast<int>(cur->dsi.delim)] = cur;
      }
      cur = cur->next;
    }
    // std::cout << "Found !open: " << cur->dsi.tokenPtr->second << "|"
    //           << std::endl;

    if (cur == tail) {
      break;
    }

    std::shared_ptr<Node> opener = cur->prev;
    while (opener != stackBottom &&
           opener != openersBottom[static_cast<int>(cur->dsi.delim)]) {
      if (opener->dsi.delim == cur->dsi.delim) {
        if (opener->dsi.type != DelimiterType::Both &&
            cur->dsi.type != DelimiterType::Both) {
          break;
        }
        if (opener->dsi.number % 3 == 0 && cur->dsi.number % 3 == 0) {
          break;
        }
        if ((opener->dsi.number + cur->dsi.number) % 3) {
          break;
        }
      }
      opener = opener->prev;
    }

    // std::cout << "Opener is stackBottom? " << (opener == stackBottom)
    //           << std::endl;

    // found
    if (opener != stackBottom && opener->dsi.delim == cur->dsi.delim) {
      std::shared_ptr<Node> temp;
      while ((temp = opener->next) != cur) {
        temp->dsi.tokenPtr->first = TokenType::Text;
        temp->dsi.tokenPtr->second = std::string_view(
            temp->dsi.tokenPtr->second.begin(), temp->dsi.number);
        opener->next = temp->next;
        temp->next->prev = opener;
        temp->Detach();
      }

      DelimiterStackItem &open = opener->dsi, &close = cur->dsi;
      TokenType type = (open.number >= 2 && close.number >= 2)
                           ? TokenType::Strong
                           : TokenType::Emph;
      int len = 1 + (type == TokenType::Strong);
      open.number -= len;
      close.number -= len;
      std::string_view sv(open.tokenPtr->second.begin(), len);

      if (open.number == 0) {
        open.tokenPtr->first = type + 1;
        open.tokenPtr->second = sv;
        opener->prev->next = opener->next;
        opener->next->prev = opener->prev;
        opener->Detach();
      } else {
        Tokens::iterator it =
            std::find(candTokens.begin(), candTokens.end(), open.tokenPtr);
        candTokens.insert(it + 1, std::make_shared<Token>(type + 1, sv));
      }

      if (close.number == 0) {
        close.tokenPtr->first = type + 2;
        close.tokenPtr->second = sv;
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        cur = cur->next;
        temp->Detach();
      } else {
        Tokens::iterator it =
            std::find(candTokens.begin(), candTokens.end(), close.tokenPtr);
        candTokens.insert(it, std::make_shared<Token>(type + 2, sv));
      }
    }

    // not found
    else {
      openersBottom[static_cast<int>(cur->dsi.delim)] = cur->prev;
      if (cur->dsi.type != DelimiterType::Both) {
        std::shared_ptr<Node> temp = cur;
        cur->dsi.tokenPtr->first = TokenType::Text;
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        cur = cur->next;
        temp->Detach();
      } else {
        cur = cur->next;
      }
    }
  }

  cur = head->next;
  while (cur != tail) {
    cur->dsi.tokenPtr->first = TokenType::Text;
    cur->dsi.tokenPtr->second =
        std::string_view(cur->dsi.tokenPtr->second.begin(), cur->dsi.number);
    cur = cur->next;
    cur->prev->Detach();
  }
  head->next = tail;
  tail->prev = head;
  cur = head;

  return true;
}

void DelimiterStack::debug() {
  cur = head->next;
  while (cur != tail) {
    auto token = cur->dsi;

    switch (token.delim) {
      case Delimiter::Asteriks:
        std::cout << "Asterisks\t";
        break;
      case Delimiter::Underscore:
        std::cout << "Underscore\t";
        break;
    }
    std::cout << token.number << "\t";
    switch (token.type) {
      case DelimiterType::Open:
        std::cout << "Open\t";
        break;
      case DelimiterType::Close:
        std::cout << "Close\t";
        break;
      case DelimiterType::Both:
        std::cout << "Both\t";
        break;
    }
    std::cout << token.tokenPtr << std::endl;

    cur = cur->next;
  }
  cur = tail->prev;
  std::cout << "-------------------\n";
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
    // case '#': return true;
  }
  // clang-format on
  return false;
}

std::string_view::iterator Parser::DocumentBegin() {
  return static_cast<std::string_view>(document_).begin();
}
std::string_view::iterator Parser::DocumentEnd() {
  return static_cast<std::string_view>(document_).end();
}

Block Parser::GetBlock() {
  return block_;
}

void Parser::AssignDocument(std::string_view doc) {
  scanner.Init(doc);
  document_ = doc;
  begin_ = it_ = DocumentBegin();
}

Block Parser::BuildBlocks() {
  std::string_view line;
  while ((line = scanner.ScanNextLine()) != "" || !scanner.End()) {
    if (line.empty()) {
      continue;
    }

    size_t pos = line.find_first_not_of(' ');
    size_t whitespaceCnt = pos;
    // lets not care about whitespace count rn (codeblock burn)
    if (line[pos] == '#') {
      Scanner scnr(line);
      int count = scnr.LookAhead(Scanner::CurPos::Begin, pos);
      std::string_view marker(line.begin() + pos, count);

      if (scnr.followedByWhiteSpace_ &&
          detail::GetMarker(marker) != TokenType::Text) {
        markdown::trim(line, pos + count + 1);
        return {
            .type = detail::GetMarker(marker),
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
      htrim(text);
      paragraph.text = text;
      return paragraph;
    }
    count += line.size() + 1;
    line = scanner.ScanNextLine();
  }

  std::string_view text(begin, count);
  htrim(text);
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

Block Parser::BuildInline() {
  Block b;

  return b;
}

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
        // count = 1;
        // PushCandToken(count);
      } else if (detail::IsDelimiter(c)) {
        count = scanner.LookAhead(Scanner::CurPos::Cur, -1);
        if (detail::IsValidDelimiter(
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
    delimStack.ProcessEmphasis(candTokens_);
    delimStack.debug();
    for (auto token : candTokens_) {
      std::cout << token->first << "\t" << std::quoted(token->second)
                << std::endl;
    }
    // delimStack.debug();
    delimStack.Clear();
    candTokens_ = {};
  }
}

void Parser::PushCandToken() {
  std::string_view lexeme = scanner.CurrentLine();
  if (!lexeme.empty()) {
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
      std::make_shared<Token>(detail::GetMarker(lexeme), lexeme));
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
      .tokenPtr = tokenPtr,
  };

  if (c == '*') {
    int state = 0;
    if (detail::IsLeftFlanking(prev, next)) {
      state++;
    }
    if (detail::IsRightFlanking(prev, next)) {
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
    if (detail::IsLeftFlanking(prev, next) &&
        (!detail::IsRightFlanking(prev, next) || detail::IsPunctuation(prev))) {
      state++;
    }
    if (detail::IsRightFlanking(prev, next) &&
        (!detail::IsLeftFlanking(prev, next) || detail::IsPunctuation(next))) {
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
  int count = 1;
  for (it_ = DocumentBegin(); it_ != DocumentEnd(); ++it_) {
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
  // std::cout << Parser::GetMarker(lexeme) << "|" << lexeme <<
  // "|" << std::endl;
  candTokens_.push_back(
      std::make_shared<Token>(detail::GetMarker(lexeme), lexeme));
}
void Parser::PushToken(TokenType type, std::string_view lexeme) {
  candTokens_.push_back(std::make_shared<Token>(type, lexeme));
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
    std::cout << TokenStr(token->first) << "\t\t" << token->second << "\n";
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
    if (token->first >= TokenType::Newline && token->first <= TokenType::Text) {
      continue;
    }

    if (token->first >= TokenType::H1 && token->first <= TokenType::H6) {
      if (index == 0) {
        continue;
      }
      TokenType prevToken = candTokens_[index - 1]->first;
      if (prevToken != TokenType::Newline &&
          prevToken != TokenType::Whitespace) {
        candTokens_.insert(
            candTokens_.begin() + index,
            std::make_shared<Token>(TokenType::Text, token->second));
        candTokens_.erase(candTokens_.begin() + index + 1);
      }
      continue;
    }

    if (!ToPop(*token)) {
      syntaxStack.push_front({token->second, index, true});
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
                           std::make_shared<Token>(
                               detail::GetMarker(TOS_.marker), TOS_.marker));
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
        std::make_shared<Token>(TokenType::Text, syntaxStack_->back().marker));
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
                       std::make_shared<Token>(
                           detail::GetMarker(LowItem.marker), LowItem.marker));
  }
  if (syntaxStack_->back().toErase) {
    candTokens_.erase(candTokens_.begin() + syntaxStack_->back().index +
                      correction_--);
    syntaxStack_->back().toErase = false;
  }
  candTokens_.insert(
      candTokens_.begin() + syntaxStack_->back().index + ++correction_,
      std::make_shared<Token>(detail::GetMarker(LowItem.marker),
                              LowItem.marker));
}

int Parser::LookAhead(std::string_view line, const char& c) {
  int count = 1;
  while ((it_ + count) != line.end() && *(it_ + count) == c) {
    count++;
  }
  followsWhiteSpace_ = detail::IsWhitespace(*(it_ + count));
  return count;
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
         ((*itToken_)->first == TokenType::Text ||
          (*itToken_)->first == TokenType::Whitespace)) {
    node.value += (*itToken_)->second;
    itTokenInc();
  }
}

void Parser::BuildChildren(Node& node) {
  TokenType endTokenType =
      (isHeading(node.type)) ? TokenType::Newline : (*itToken_)->first;
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
         (*itToken_)->first == TokenType::Newline &&
         (*(itToken_ + 1))->first == TokenType::Newline &&
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

void trim(std::string_view& sv, size_t lPos, size_t rPos) {
  sv.remove_prefix(lPos);
  sv.remove_prefix(sv.find_first_not_of(' '));
  sv.remove_suffix(sv.size() - sv.find_last_not_of(' ') - 1);
  sv.remove_suffix(rPos);
}

void htrim(std::string_view& sv) {
  std::string_view::iterator it = sv.begin();
  int count = 0;
  while ((it + count) != sv.end() && detail::IsWhitespace(*(it + count))) {
    count++;
  }
  sv.remove_prefix(count);

  count = sv.size() - 1;
  it = sv.begin();
  while ((it + count) != sv.end() && detail::IsWhitespace(*(it + count))) {
    count--;
  }
  sv.remove_suffix(sv.size() - count - 1);
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

bool detail::IsDelimiter(char c) {
  // clang-format off
  switch (c) {
    case '*': return true;
    case '_': return true;
    // case '#': return true;
  }
  // clang-format on
  return false;
}

bool detail::IsValidDelimiter(char prev, char delim, char next) {
  if (!(delim == '*' || delim == '_')) {
    std::cerr << "Unhandled Delimiter: " << std::quoted(std::to_string(delim))
              << std::endl;
    std::exit(1);
  }

  return detail::IsLeftFlanking(prev, next) ||
         detail::IsRightFlanking(prev, next);
}

bool detail::IsWhitespace(char c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r') ? true : false;
}

bool detail::IsPunctuation(char c) {
  // ASCII only
  if (c >= 33 && c <= 47)
    return true;
  else if (c >= 58 && c <= 64)
    return true;
  else if (c >= 91 && c <= 96)
    return true;
  else if (c >= 123 && c <= 126)
    return true;

  return false;
}

bool detail::IsLeftFlanking(char prev, char next) {
  // NOTE: No Unicode support
  if (detail::IsWhitespace(next)) {
    return false;
  }

  if (detail::IsPunctuation(next) &&
      !(detail::IsWhitespace(prev) || detail::IsPunctuation(prev))) {
    return false;
  }

  return true;
}

bool detail::IsRightFlanking(char prev, char next) {
  // NOTE: No Unicode support
  if (detail::IsWhitespace(prev)) {
    return false;
  }

  if (detail::IsPunctuation(prev) &&
      !(detail::IsWhitespace(next) || detail::IsPunctuation(next))) {
    return false;
  }

  return true;
}

}  // namespace markdown
