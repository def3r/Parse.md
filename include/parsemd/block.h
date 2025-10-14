#ifndef PARSEMD_BLOCK_H_
#define PARSEMD_BLOCK_H_

#include "types.h"

namespace markdown {

struct Block {
  TokenType type = TokenType::None;
  bool isOpen = true;  // last child of a block is considered open
  std::string_view text;
  Blocks children;
};

}  // namespace markdown

#endif  // !PARSEMD_BLOCK_H_
