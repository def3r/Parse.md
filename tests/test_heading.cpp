#include <gtest/gtest.h>
#include "../parse.h"

using namespace markdown;

TEST(HeadingTest, HandlesHash1Heading1) {
  Parser t;
  t.Parse("# This is H1 and something like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::H1, "#"},
      {TokenType::Text, "This is H1 and something like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(HeadingTest, HandlesHash1Heading2) {
  Parser t;
  t.Parse("  # This is H1 and something like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Whitespace, "  "}, // skibidi
      {TokenType::H1, "#"},
      {TokenType::Text, "This is H1 and something like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(HeadingTest, HandlesHash2Heading1) {
  Parser t;
  t.Parse(" ## This is H2 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Whitespace, " "}, // skibidi
      {TokenType::H2, "##"},
      {TokenType::Text, "This is H2 and some"},
      {TokenType::Text, "##"},
      {TokenType::Text, "thing like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(HeadingTest, HandlesHash3Heading1) {
  Parser t;
  t.Parse(" ### This is H3 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Whitespace, " "}, // skibidi
      {TokenType::H3, "###"},
      {TokenType::Text, "This is H3 and some"},
      {TokenType::Text, "##"},
      {TokenType::Text, "thing like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(HeadingTest, HandlesHash4Heading1) {
  Parser t;
  t.Parse(" #### This is H4 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Whitespace, " "}, // skibidi
      {TokenType::H4, "####"},
      {TokenType::Text, "This is H4 and some"},
      {TokenType::Text, "##"},
      {TokenType::Text, "thing like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(HeadingTest, HandlesHash5Heading1) {
  Parser t;
  t.Parse(" ##### This is H5 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Whitespace, " "}, // skibidi
      {TokenType::H5, "#####"},
      {TokenType::Text, "This is H5 and some"},
      {TokenType::Text, "##"},
      {TokenType::Text, "thing like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(HeadingTest, HandlesHash6Heading1) {
  Parser t;
  t.Parse(" ###### This is H6 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Whitespace, " "}, // skibidi
      {TokenType::H6, "######"},
      {TokenType::Text, "This is H6 and some"},
      {TokenType::Text, "##"},
      {TokenType::Text, "thing like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(HeadingTest, HandlesHashNewLine) {
  Parser s;
  s.Parse(
      " ###### This is H6 and some##thing like **this works**\n# Mornin new "
      "line");
  // s.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Whitespace, " "},
      {TokenType::H6, "######"},
      {TokenType::Text, "This is H6 and some"},
      {TokenType::Text, "##"},
      {TokenType::Text, "thing like "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this works"},
      {TokenType::Bold, "**"},
      {TokenType::Newline, "\n"},
      {TokenType::H1, "#"},
      {TokenType::Text, "Mornin new line"}
  };
  // clang-format on
  ASSERT_EQ(s.GetTokens(), expected);
}
