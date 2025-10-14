#include "parsemd/node.h"

namespace markdown {

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

}  // namespace markdown
