#ifndef PARSEMD_NODE_H_
#define PARSEMD_NODE_H_

#include "types.h"

namespace markdown {

struct NodeBase {
  NodeBase(TokenType type);
  TokenType Type() const;

  friend class Parser;

 private:
  TokenType type_;
};

struct ContainerNode : public NodeBase {
  ContainerNode(TokenType type);
  Nodes children = {};
};

struct BlockNode : public ContainerNode {
  BlockNode();
  BlockNode(TokenType type);

  friend class Parser;

 private:
  std::string_view text_;
};

struct InlineNode : public ContainerNode {
  InlineNode(TokenType type);
};

struct TextNode : public NodeBase {
  TextNode(TokenType type, std::string text);
  std::string text;
};

// Node Downcast helpers
Container ContainerNodePtr(Node node);
Block BlockNodePtr(Node node);
Inline InlineNodePtr(Node node);
Text TextNodePtr(Node node);

inline bool IsHeading(Node node) {
  return node && node->Type() >= TokenType::H1 && node->Type() <= TokenType::H6;
}
inline bool IsBlock(Node node) {
  return node && node->Type() >= TokenType::Root &&
         node->Type() <= TokenType::H6;
}
inline bool IsInline(Node node) {
  return node && node->Type() > TokenType::Text &&
         node->Type() <= TokenType::Code;
}
inline bool IsText(Node node) {
  return node && node->Type() == TokenType::Text;
}

}  // namespace markdown

#endif  // !PARSEMD_NODE_H_
