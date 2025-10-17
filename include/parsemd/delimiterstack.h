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

 public:
  DelimiterStack();
  void Push(DelimiterStackItem dsi);
  void Clear();
  void debug();
  bool ProcessEmphasis(TokenList& candTokens);

 protected:
  using DelimStack = std::list<DelimiterStackItem>;

  DelimStack stack_;
  DelimStack::iterator dummy_, cur_, stackBottom_;
  DelimStack::iterator openersBottom_[2];
};

}  // namespace markdown

#endif  // !PARSEMD_DELIMITERSTACK_H_
