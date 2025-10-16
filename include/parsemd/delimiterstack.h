#ifndef PARSEMD_DELIMITERSTACK_H_
#define PARSEMD_DELIMITERSTACK_H_

#include "types.h"

namespace markdown {

class DelimiterStack {
 public:
  enum class Delimiter { Asteriks = 0, Underscore = 1 };
  enum class DelimiterType { Open, Close, Both };
  struct DelimiterStackItem {
    Delimiter delim;
    size_t number;
    bool isActive;
    DelimiterType type;
    TokenList::iterator tokenIt;
  };

  struct Node {
    DelimiterStackItem dsi;
    std::shared_ptr<Node> next, prev;

    Node();
    Node(DelimiterStackItem dsi,
         std::shared_ptr<Node> next,
         std::shared_ptr<Node> prev);
    void Detach();
  };

 public:
  DelimiterStack();
  void Push(DelimiterStackItem dsi);
  void Clear();
  void debug();
  bool ProcessEmphasis(TokenList& candTokens);

 protected:
  std::shared_ptr<Node> head, tail, cur;
  std::shared_ptr<Node> stackBottom;
  std::shared_ptr<Node> openersBottom[2];
};

}  // namespace markdown

#endif  // !PARSEMD_DELIMITERSTACK_H_
