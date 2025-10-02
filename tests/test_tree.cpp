#include <gtest/gtest.h>
#include "../parse.h"

TEST(TreeTest, SimpleTest) {
  Parser t;
  t.Parse("Mornin folks!");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::ROOT
  Token::PARAGRAPH
    Token::TEXT Mornin folks!
)");
}

TEST(TreeTest, TestBoldItalic) {
  Parser t;
  t.Parse("This **handles** both ***bold** and italics*!");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::ROOT
  Token::PARAGRAPH
    Token::TEXT This 
    Token::BOLD
      Token::TEXT handles
    Token::TEXT  both 
    Token::ITALIC
      Token::BOLD
        Token::TEXT bold
      Token::TEXT  and italics
    Token::TEXT !
)");
}

TEST(TreeTest, TestParagraphs) {
  Parser t;
  t.Parse("This is Para 1\nParagraph 1 here\n\nParagraph 2");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::ROOT
  Token::PARAGRAPH
    Token::TEXT This is Para 1
    Token::TEXT  
    Token::TEXT Paragraph 1 here
  Token::PARAGRAPH
    Token::TEXT Paragraph 2
)");
}

TEST(TreeTest, TestNestedWithHeading) {
  Parser t;
  t.Parse("# This is **Heading 1**\nParagraph here\n##This is continuation");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::ROOT
  Token::H1
    Token::TEXT This is 
    Token::BOLD
      Token::TEXT Heading 1
  Token::PARAGRAPH
    Token::TEXT Paragraph here
    Token::TEXT  
    Token::TEXT ##This is continuation
)");
}
