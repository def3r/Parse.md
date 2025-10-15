#include <iostream>

#include "parsemd/delimiterstack.h"

namespace markdown {

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

// https://spec.commonmark.org/0.31.2/#phase-2-inline-structure
// https://spec.commonmark.org/0.31.2/#can-open-emphasis
bool DelimiterStack::ProcessEmphasis(Tokens& candTokens) {
  // NOTE: No Links or Image support yet
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
        temp->dsi.tokenIt->first = TokenType::Text;
        temp->dsi.tokenIt->second = std::string_view(
            temp->dsi.tokenIt->second.begin(), temp->dsi.number);
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
      std::string_view sv(open.tokenIt->second.begin(), len);

      if (open.number == 0) {
        open.tokenIt->first = type + 1;
        open.tokenIt->second = sv;
        opener->prev->next = opener->next;
        opener->next->prev = opener->prev;
        opener->Detach();
      } else {
        candTokens.insert(std::next(open.tokenIt), Token(type + 1, sv));
      }

      if (close.number == 0) {
        close.tokenIt->first = type + 2;
        close.tokenIt->second = sv;
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        cur = cur->next;
        temp->Detach();
      } else {
        candTokens.insert(close.tokenIt, Token(type + 2, sv));
      }
    }

    // not found
    else {
      openersBottom[static_cast<int>(cur->dsi.delim)] = cur->prev;
      if (cur->dsi.type != DelimiterType::Both) {
        std::shared_ptr<Node> temp = cur;
        cur->dsi.tokenIt->first = TokenType::Text;
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
    cur->dsi.tokenIt->first = TokenType::Text;
    cur->dsi.tokenIt->second =
        std::string_view(cur->dsi.tokenIt->second.begin(), cur->dsi.number);
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
    std::cout << token.tokenIt._M_node << std::endl;

    cur = cur->next;
  }
  cur = tail->prev;
  std::cout << "-------------------\n";
}

}  // namespace markdown
