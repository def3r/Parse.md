#include <gtest/gtest.h>
#include "../parse.h"

using namespace markdown;

TEST(ParserTest, SimpleText) {
  Parser t;
  t.Parse("Mornin folks!");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Text, "Mornin folks!"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(ParserTest, HandlesStarBoldAndItalics) {
  Parser t;
  t.Parse("well, **this is *8ball* **");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Text, "well, "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "this is "},
      {TokenType::Italic, "*"},
      {TokenType::Text, "8ball"},
      {TokenType::Italic, "*"},
      {TokenType::Whitespace, " "},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(ParserTest, HandlesStarsImbalance) {
  Parser t;
  t.Parse("this **ain't nothin*\n");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Text, "this "},
      {TokenType::Italic, "*"}, // INTERESTING
      {TokenType::Text, "*"},
      {TokenType::Text, "ain't nothin"},
      {TokenType::Italic, "*"},
      {TokenType::Newline, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
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
      {TokenType::Text, "this "},
      {TokenType::Bold, "**"},
      {TokenType::BoldItalic, "***"},
      {TokenType::Text, "ain't nothin"},
      {TokenType::BoldItalic, "***"},
      {TokenType::Text, " this is cruel "},
      {TokenType::Bold, "**"},
      {TokenType::Newline, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom2) {
  Parser t;
  t.Parse("this *is the *****ain't nothin** this is cruel **\n");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Text, "this "},
      {TokenType::Italic, "*"},
      {TokenType::Text, "is the "},
      {TokenType::Italic, "*"},
      {TokenType::Text, "****"},
      {TokenType::Text, "ain't nothin"},
      {TokenType::Bold, "**"},
      {TokenType::Text, " this is cruel "},
      {TokenType::Bold, "**"},
      {TokenType::Newline, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom3) {
  Parser t;
  t.Parse("***So **it *works?* huh...**mornin*");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Italic, "*"}, // INTERESTING
      {TokenType::Text, "**"},
      {TokenType::Text, "So "},
      {TokenType::Bold, "**"},
      {TokenType::Text, "it "},
      {TokenType::Italic, "*"},
      {TokenType::Text, "works?"},
      {TokenType::Italic, "*"},
      {TokenType::Text, " huh..."},
      {TokenType::Bold, "**"},
      {TokenType::Text, "mornin"},
      {TokenType::Italic, "*"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
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
      {TokenType::Bold, "**"},
      {TokenType::Text, "how"},
      {TokenType::Bold, "**"},
      {TokenType::Italic, "*"},
      {TokenType::Text, "bout"},
      {TokenType::Italic, "*"},
      {TokenType::Text, "zis? where this text?"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesStarsRandom5) {
  Parser t;
  t.Parse("**this should be**** entirely bold**");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Bold, "**"},
      {TokenType::Text, "this should be"},
      {TokenType::Bold, "**"},
      {TokenType::Bold, "**"},
      {TokenType::Text, " entirely bold"},
      {TokenType::Bold, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(NestedImbalanceTest, HandlesUnderscoresRandom) {
  Parser t;
  t.Parse("__this should be____entirely bold__");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Bold, "__"},
      {TokenType::Text, "this should be"},
      {TokenType::Bold, "__"},
      {TokenType::Bold, "__"},
      {TokenType::Text, "entirely bold"},
      {TokenType::Bold, "__"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(NestedImbalanceMultiple, HandlesStarsAndUnderscores) {
  Parser t;
  t.Parse("__this *should__ be** entirely bold__");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Bold, "__"},
      {TokenType::Text, "this "},
      {TokenType::Text, "*"},
      {TokenType::Text, "should"},
      {TokenType::Bold, "__"},
      {TokenType::Text, " be"},
      {TokenType::Text, "**"},
      {TokenType::Text, " entirely bold"},
      {TokenType::Text, "__"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

TEST(NestedImbalanceMultiple, HandlesStarsAndUnderscores2) {
  Parser t;
  t.Parse("__this *should be** entirely bold__");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::Bold, "__"},
      {TokenType::Text, "this "},
      {TokenType::Italic, "*"},
      {TokenType::Text, "should be"},
      {TokenType::Italic, "*"},
      {TokenType::Text, "*"},
      {TokenType::Text, " entirely bold"},
      {TokenType::Bold, "__"},
  };
  // clang-format on
  ASSERT_EQ(t.GetTokens(), expected);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
