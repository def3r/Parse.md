#include <gtest/gtest.h>
#include "../parse.h"

TEST(HeadingTest, HandlesHash1Heading1) {
  Parser t;
  t.Parse("# This is H1 and something like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::H1, "#"},
      {TokenType::TEXT, "This is H1 and something like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getLexTokens(), expected);
}

TEST(HeadingTest, HandlesHash1Heading2) {
  Parser t;
  t.Parse("  # This is H1 and something like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, "  "}, // skibidi
      {TokenType::H1, "#"},
      {TokenType::TEXT, "This is H1 and something like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getLexTokens(), expected);
}

TEST(HeadingTest, HandlesHash2Heading1) {
  Parser t;
  t.Parse(" ## This is H2 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H2, "##"},
      {TokenType::TEXT, "This is H2 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getLexTokens(), expected);
}

TEST(HeadingTest, HandlesHash3Heading1) {
  Parser t;
  t.Parse(" ### This is H3 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H3, "###"},
      {TokenType::TEXT, "This is H3 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getLexTokens(), expected);
}

TEST(HeadingTest, HandlesHash4Heading1) {
  Parser t;
  t.Parse(" #### This is H4 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H4, "####"},
      {TokenType::TEXT, "This is H4 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getLexTokens(), expected);
}

TEST(HeadingTest, HandlesHash5Heading1) {
  Parser t;
  t.Parse(" ##### This is H5 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H5, "#####"},
      {TokenType::TEXT, "This is H5 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getLexTokens(), expected);
}

TEST(HeadingTest, HandlesHash6Heading1) {
  Parser t;
  t.Parse(" ###### This is H6 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H6, "######"},
      {TokenType::TEXT, "This is H6 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getLexTokens(), expected);
}

TEST(HeadingTest, HandlesHashNewLine) {
  Parser s;
  s.Parse(
      " ###### This is H6 and some##thing like **this works**\n# Mornin new "
      "line");
  // s.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "},
      {TokenType::H6, "######"},
      {TokenType::TEXT, "This is H6 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
      {TokenType::NEWLINE, "\n"},
      {TokenType::H1, "#"},
      {TokenType::TEXT, "Mornin new line"}
  };
  // clang-format on
  ASSERT_EQ(s.getLexTokens(), expected);
}
