#include <gtest/gtest.h>
#include "parsemd/parser.h"

using namespace markdown;

TEST(BlockAnalysis, ParagraphTestSimple) {
  Parser t;
  t.AnalyzeBlocks("This is a paragraph");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is a paragraph"
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestSimpleNewline) {
  Parser t;
  t.AnalyzeBlocks("This is a paragraph\n");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is a paragraph"
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestSimple2Newline) {
  Parser t;
  t.AnalyzeBlocks("This is a paragraph\n\n");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is a paragraph"
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestSimple2Para) {
  Parser t;
  t.AnalyzeBlocks("This is paragraph 1\n\nThis is paragraph 2");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is paragraph 1"
  Token::Paragraph "This is paragraph 2"
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestMultilinePara) {
  Parser t;
  t.AnalyzeBlocks("This is paragraph 1\nThis is still paragraph 1");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is paragraph 1
This is still paragraph 1"
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestMultilineParaNewline) {
  Parser t;
  t.AnalyzeBlocks("This is paragraph 1\nThis is still paragraph 1\n");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is paragraph 1
This is still paragraph 1"
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestMultilineParaNewline2) {
  Parser t;
  t.AnalyzeBlocks("This is paragraph 1\nThis is still paragraph 1\n\n");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is paragraph 1
This is still paragraph 1"
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestEmpty) {
  Parser t;
  t.AnalyzeBlocks("");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTestEmptyNewline) {
  Parser t;
  t.AnalyzeBlocks("\n");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
)");
  // clang-format on
}

TEST(BlockAnalysis, ParagraphTest) {
  Parser t;
  t.AnalyzeBlocks(
      "\n     This is a paragraph too!\n\n\n\nAnd this is another one      ");
  Node root = t.GetRoot();
  // clang-format off
  ASSERT_EQ(
    Parser::DumpTree(root),
R"(Token::Root
  Token::Paragraph "This is a paragraph too!"
  Token::Paragraph "And this is another one"
)");
  // clang-format on
}
