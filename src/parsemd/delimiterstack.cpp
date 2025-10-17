#include <iostream>
#include <iterator>
#include "parsemd/types.h"

#include "parsemd/delimiterstack.h"

namespace markdown {

DelimiterStack::DelimiterStack() {
  stack_ = {DelimiterStackItem{}};
  dummy_ = cur_ = stack_.begin();
  stackBottom_ = dummy_;
  openersBottom_[0] = openersBottom_[1] = dummy_;
}

void DelimiterStack::Push(DelimiterStackItem dsi) {
  stack_.push_back(dsi);
}

void DelimiterStack::Clear() {
  stack_.clear();
  stack_ = {DelimiterStackItem{}};
  dummy_ = cur_ = stack_.begin();
  stackBottom_ = dummy_;
  openersBottom_[0] = openersBottom_[1] = dummy_;
}

// https://spec.commonmark.org/0.31.2/#phase-2-inline-structure
// https://spec.commonmark.org/0.31.2/#can-open-emphasis
bool DelimiterStack::ProcessEmphasis(TokenList& candTokens) {
  // NOTE: No Links or Image support yet
  stackBottom_ = stack_.begin();
  openersBottom_[0] = openersBottom_[1] = {};
  cur_ = std::next(stackBottom_);

  while (cur_ != stack_.end()) {
    while (cur_ != stack_.end() && cur_->type == DelimiterType::Open) {
      if (openersBottom_[static_cast<int>(cur_->delim)] == dummy_) {
        openersBottom_[static_cast<int>(cur_->delim)] = cur_;
      }
      cur_ = std::next(cur_);
    }

    if (cur_ == stack_.end()) {
      break;
    }

    DelimStack::iterator opener = std::prev(cur_);
    while (opener != stackBottom_ &&
           opener != openersBottom_[static_cast<int>(cur_->delim)]) {
      if (opener->delim == cur_->delim) {
        if (opener->type != DelimiterType::Both &&
            cur_->type != DelimiterType::Both) {
          break;
        }
        if (opener->number % 3 == 0 && cur_->number % 3 == 0) {
          break;
        }
        if ((opener->number + cur_->number) % 3) {
          break;
        }
      }
      opener = std::prev(opener);
    }

    // found
    if (opener != stackBottom_ && opener->delim == cur_->delim) {
      DelimStack::iterator temp;
      while ((temp = std::next(opener)) != cur_) {
        temp->tokenIt->first = TokenType::Text;
        temp->tokenIt->second =
            std::string_view(temp->tokenIt->second.begin(), temp->number);
        stack_.erase(temp);
      }

      DelimiterStackItem &open = *opener, &close = *cur_;
      TokenType type = (open.number >= 2 && close.number >= 2)
                           ? TokenType::Strong
                           : TokenType::Emph;
      int len = 1 + (type == TokenType::Strong);
      open.number -= len;
      close.number -= len;
      std::string_view sv(open.tokenIt->second.begin(), len);

      if (open.number == 0) {
        open.tokenIt->first = type + 1;
        open.tokenIt->second = sv;
        stack_.erase(opener);
      } else {
        candTokens.insert(std::next(open.tokenIt), Token(type + 1, sv));
      }

      if (close.number == 0) {
        close.tokenIt->first = type + 2;
        close.tokenIt->second = sv;
        cur_ = std::next(cur_);
        stack_.erase(temp);
      } else {
        candTokens.insert(close.tokenIt, Token(type + 2, sv));
      }
    }

    // not found
    else {
      openersBottom_[static_cast<int>(cur_->delim)] = std::prev(cur_);
      if (cur_->type != DelimiterType::Both) {
        DelimStack::iterator temp = cur_;
        cur_->tokenIt->first = TokenType::Text;
        cur_ = std::next(cur_);
        stack_.erase(temp);
      } else {
        cur_ = std::next(cur_);
      }
    }
  }

  cur_ = std::next(dummy_);
  while (cur_ != stack_.end()) {
    cur_->tokenIt->first = TokenType::Text;
    cur_->tokenIt->second =
        std::string_view(cur_->tokenIt->second.begin(), cur_->number);
    cur_ = std::next(cur_);
    stack_.erase(std::prev(cur_));
  }
  cur_ = dummy_;

  return true;
}

void DelimiterStack::debug() {
  cur_ = stack_.begin();
  while (cur_ != stack_.end()) {
    auto token = *cur_;

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
    std::cout << token.tokenIt._M_node << std::endl;

    cur_ = std::next(cur_);
  }
  cur_ = stack_.begin();
  std::cout << "-------------------\n";
}

}  // namespace markdown
