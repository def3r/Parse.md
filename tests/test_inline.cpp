#include <gtest/gtest.h>
#include "parsemd/parser.h"

using namespace markdown;

TEST(InlineAnalysis, InlineTestSimple) {
  Parser t;
  t.AnalyzeBlocks("This is **strong** and *emph*");
  t.AnalyzeInline();
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph
    Token::Text "This is "
    Token::Strong
      Token::Text "strong"
    Token::Text " and "
    Token::Emph
      Token::Text "emph"
)");
  // clang-format on
}

TEST(InlineAnalysis, InlineTestSimpleU) {
  Parser t;
  t.AnalyzeBlocks("This is __strong__ and _emph_");
  t.AnalyzeInline();
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph
    Token::Text "This is "
    Token::Strong
      Token::Text "strong"
    Token::Text " and "
    Token::Emph
      Token::Text "emph"
)");
  // clang-format on
}
