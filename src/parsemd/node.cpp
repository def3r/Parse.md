#include "parsemd/node.h"
#include <memory>
#include "parsemd/types.h"

namespace markdown {

NodeBase::NodeBase(TokenType type) : type(type) {}

ContainerNode::ContainerNode(TokenType type) : NodeBase(type) {}

BlockNode::BlockNode() : ContainerNode(TokenType::None) {}
BlockNode::BlockNode(TokenType type) : ContainerNode(type) {}

InlineNode::InlineNode(TokenType type) : ContainerNode(type) {}

TextNode::TextNode(TokenType type, std::string text)
    : NodeBase(type), text(text) {}

std::shared_ptr<ContainerNode> ContainerNodePtr(Node node) {
  if (!node.get())
    return nullptr;
  if (IsText(node->type))
    return nullptr;
  return std::static_pointer_cast<ContainerNode>(node);
}

std::shared_ptr<BlockNode> BlockNodePtr(Node node) {
  if (!node.get())
    return nullptr;
  if (!IsBlock(node->type))
    return nullptr;
  return std::static_pointer_cast<BlockNode>(node);
}

std::shared_ptr<InlineNode> InlineNodePtr(Node node) {
  if (!node.get())
    return nullptr;
  if (!IsInline(node->type) || IsText(node->type))
    return nullptr;
  return std::static_pointer_cast<InlineNode>(node);
}

std::shared_ptr<TextNode> TextNodePtr(Node node) {
  if (!node.get())
    return nullptr;
  if (!IsText(node->type))
    return nullptr;
  return std::static_pointer_cast<TextNode>(node);
}

}  // namespace markdown
