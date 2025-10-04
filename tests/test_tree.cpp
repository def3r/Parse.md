#include <gtest/gtest.h>
#include "../parse.h"

using namespace markdown;

TEST(TreeTest, SimpleTest) {
  Parser t;
  t.Parse("Mornin folks!");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::Root
  Token::Paragraph
    Token::Text Mornin folks!
)");
}

TEST(TreeTest, TestBoldItalic) {
  Parser t;
  t.Parse("This **handles** both ***bold** and italics*!");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::Root
  Token::Paragraph
    Token::Text This 
    Token::Bold
      Token::Text handles
    Token::Text  both 
    Token::Italic
      Token::Bold
        Token::Text bold
      Token::Text  and italics
    Token::Text !
)");
}

TEST(TreeTest, TestParagraphs) {
  Parser t;
  t.Parse("This is Para 1\nParagraph 1 here\n\nParagraph 2");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::Root
  Token::Paragraph
    Token::Text This is Para 1
    Token::Text  
    Token::Text Paragraph 1 here
  Token::Paragraph
    Token::Text Paragraph 2
)");
}

TEST(TreeTest, TestNestedWithHeading) {
  Parser t;
  t.Parse("# This is **Heading 1**\nParagraph here\n##This is continuation");
  auto tree = t.GetRoot();

  ASSERT_EQ(Parser::DumpTree(tree), R"(Token::Root
  Token::H1
    Token::Text This is 
    Token::Bold
      Token::Text Heading 1
  Token::Paragraph
    Token::Text Paragraph here
    Token::Text  
    Token::Text ##This is continuation
)");
}
