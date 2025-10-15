#ifndef PARSEMD_NODE_H_
#define PARSEMD_NODE_H_

#include "types.h"

namespace markdown {

struct NodeBase {
  NodeBase(TokenType type);
  // virtual ~NodeBase() = default;
  // TokenType Type() const;

  TokenType type;
};

struct ContainerNode : public NodeBase {
  ContainerNode(TokenType type);
  // void Add(Node n);
  // size_t ChildCount() const;
  // Node ChildAt(size_t i);

  Nodes children = {};
};

struct BlockNode : public ContainerNode {
  BlockNode();
  BlockNode(TokenType type);
  // void SetText(const std::string_view& text);

  std::string_view text;
};

struct InlineNode : public ContainerNode {
  InlineNode(TokenType type);
};

struct TextNode : public NodeBase {
  TextNode(TokenType type, std::string text);
  // std::string& Text();

  std::string text;
};

// Node Downcast helpers
std::shared_ptr<ContainerNode> ContainerNodePtr(Node node);
std::shared_ptr<BlockNode> BlockNodePtr(Node node);
std::shared_ptr<InlineNode> InlineNodePtr(Node node);
std::shared_ptr<TextNode> TextNodePtr(Node node);

inline bool IsHeading(Node node) {
  return node && node->type >= TokenType::H1 && node->type <= TokenType::H6;
}
inline bool IsBlock(Node node) {
  return node && node->type >= TokenType::Root && node->type <= TokenType::H6;
}
inline bool IsInline(Node node) {
  return node && node->type > TokenType::Text && node->type <= TokenType::Code;
}
inline bool IsText(Node node) {
  return node && node->type == TokenType::Text;
}

}  // namespace markdown

#endif  // !PARSEMD_NODE_H_
