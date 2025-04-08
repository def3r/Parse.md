#include <gtest/gtest.h>
#include "../parse.h"

TEST(ParserTest, SimpleText) {
  Parser t;
  t.Parse("Mornin folks!");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "Mornin folks!"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(ParserTest, HandlesStarBoldAndItalics) {
  Parser t;
  t.Parse("well, **this is *8ball* **");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "well, "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this is "},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "8ball"},
      {TokenType::ITALIC, "*"},
      {TokenType::WHITESPACE, " "},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(ParserTest, HandlesStarsImbalance) {
  Parser t;
  t.Parse("this **ain't nothin*\n");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "this "},
      {TokenType::ITALIC, "*"}, // INTERESTING
      {TokenType::TEXT, "*"},
      {TokenType::TEXT, "ain't nothin"},
      {TokenType::ITALIC, "*"},
      {TokenType::NEWLINE, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom) {
  Parser t;
  /*
   * *****
   * ***  -> Delete
   * **
   *
   * **   -> obviously correction++ (before prev)
   * **
   *
   * */
  t.Parse("this *****ain't nothin*** this is cruel **\n");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "this "},
      {TokenType::BOLD, "**"},
      {TokenType::BOLD_ITALIC, "***"},
      {TokenType::TEXT, "ain't nothin"},
      {TokenType::BOLD_ITALIC, "***"},
      {TokenType::TEXT, " this is cruel "},
      {TokenType::BOLD, "**"},
      {TokenType::NEWLINE, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom2) {
  Parser t;
  t.Parse("this *is the *****ain't nothin** this is cruel **\n");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "this "},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "is the "},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "****"},
      {TokenType::TEXT, "ain't nothin"},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, " this is cruel "},
      {TokenType::BOLD, "**"},
      {TokenType::NEWLINE, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom3) {
  Parser t;
  t.Parse("***So **it *works?* huh...**mornin*");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::ITALIC, "*"}, // INTERESTING
      {TokenType::TEXT, "**"},
      {TokenType::TEXT, "So "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "it "},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "works?"},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, " huh..."},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "mornin"},
      {TokenType::ITALIC, "*"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom4) {
  Parser t;
  /*
   * **   -> Delete
   * ***
   * *
   *
   * *    -> obviously ++correction (after prev)
   * *
   *
   * */
  t.Parse("**how***bout*zis? where this text?");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "how"},
      {TokenType::BOLD, "**"},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "bout"},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "zis? where this text?"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom5) {
  Parser t;
  t.Parse("**this should be**** entirely bold**");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this should be"},
      {TokenType::BOLD, "**"},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, " entirely bold"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesUnderscoresRandom) {
  Parser t;
  t.Parse("__this should be____entirely bold__");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::BOLD, "__"},
      {TokenType::TEXT, "this should be"},
      {TokenType::BOLD, "__"},
      {TokenType::BOLD, "__"},
      {TokenType::TEXT, "entirely bold"},
      {TokenType::BOLD, "__"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceMultiple, HandlesStarsAndUnderscores) {
  Parser t;
  t.Parse("__this *should__ be** entirely bold__");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::BOLD, "__"},
      {TokenType::TEXT, "this "},
      {TokenType::TEXT, "*"},
      {TokenType::TEXT, "should"},
      {TokenType::BOLD, "__"},
      {TokenType::TEXT, " be"},
      {TokenType::TEXT, "**"},
      {TokenType::TEXT, " entirely bold"},
      {TokenType::TEXT, "__"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(NestedImbalanceMultiple, HandlesStarsAndUnderscores2) {
  Parser t;
  t.Parse("__this *should be** entirely bold__");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::BOLD, "__"},
      {TokenType::TEXT, "this "},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "should be"},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "*"},
      {TokenType::TEXT, " entirely bold"},
      {TokenType::BOLD, "__"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(HeadingTest, HandlesHash1Heading1) {
  Parser t;
  t.Parse("# This is H1 and something like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::H1, "#"},
      {TokenType::TEXT, " This is H1 and something like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(HeadingTest, HandlesHash1Heading2) {
  Parser t;
  t.Parse("  # This is H1 and something like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, "  "}, // skibidi
      {TokenType::H1, "#"},
      {TokenType::TEXT, " This is H1 and something like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(HeadingTest, HandlesHash2Heading1) {
  Parser t;
  t.Parse(" ## This is H2 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H2, "##"},
      {TokenType::TEXT, " This is H2 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(HeadingTest, HandlesHash3Heading1) {
  Parser t;
  t.Parse(" ### This is H3 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H3, "###"},
      {TokenType::TEXT, " This is H3 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(HeadingTest, HandlesHash4Heading1) {
  Parser t;
  t.Parse(" #### This is H4 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H4, "####"},
      {TokenType::TEXT, " This is H4 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(HeadingTest, HandlesHash5Heading1) {
  Parser t;
  t.Parse(" ##### This is H5 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H5, "#####"},
      {TokenType::TEXT, " This is H5 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(HeadingTest, HandlesHash6Heading1) {
  Parser t;
  t.Parse(" ###### This is H6 and some##thing like **this works**");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::WHITESPACE, " "}, // skibidi
      {TokenType::H6, "######"},
      {TokenType::TEXT, " This is H6 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
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
      {TokenType::TEXT, " This is H6 and some"},
      {TokenType::TEXT, "##"},
      {TokenType::TEXT, "thing like "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this works"},
      {TokenType::BOLD, "**"},
      {TokenType::NEWLINE, "\n"},
      {TokenType::H1, "#"},
      {TokenType::TEXT, " Mornin new line"}
  };
  // clang-format on
  ASSERT_EQ(s.getTokens(), expected);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
