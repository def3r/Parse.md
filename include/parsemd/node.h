#ifndef PARSEMD_NODE_H_
#define PARSEMD_NODE_H_

#include "types.h"

namespace markdown {

struct Node {
 public:
  Node(TokenType);
  Node(std::string);
  Node();

  TokenType type;
  std::string value;
  std::vector<std::shared_ptr<Node>> children;
};

// NOTE: I still think this is a better idea, but lets see
//
// class Node {
//  protected:
//   TokenType type_;
//
//  public:
//   virtual ~Node() = default;
//   virtual TokenType type() const = 0;
// };
//
// class ContainerNode : public Node {
//  public:
//   std::vector<std::shared_ptr<Node>> children;
// };
//
// class LeafNode : public Node {
//  public:
//   std::string value;
//   LeafNode(const std::string);
// };

}  // namespace markdown

#endif  // !PARSEMD_NODE_H_
