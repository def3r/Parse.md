#include <gtest/gtest.h>
#include "lexer.h"

TEST(LexerTest, SimpleText) {
  Lexer t;
  t.tokenize("Mornin folks!");
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "Mornin folks!"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(LexerTest, HandlesStarBoldAndItalics) {
  Lexer t;
  t.tokenize("well, **this is *8ball* **");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "well, "},
      {TokenType::BOLD, "**"},
      {TokenType::TEXT, "this is "},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "8ball"},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, " "},
      {TokenType::BOLD, "**"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(LexerTest, HandlesStarsImbalance) {
  Lexer t;
  t.tokenize("this **ain't nothin*\n");
  // t.debug();
  // clang-format off
  std::vector<Token> expected = {
      {TokenType::TEXT, "this "},
      {TokenType::ITALIC, "*"}, // INTERESTING
      {TokenType::TEXT, "*"},
      {TokenType::TEXT, "ain't nothin"},
      {TokenType::ITALIC, "*"},
      {TokenType::TEXT, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(LexerTest, HandlesStarsRandom) {
  Lexer t;
  /*
   * *****
   * ***  -> Delete
   * **
   *
   * **   -> obviously correction++ (before prev)
   * **
   *
   * */
  t.tokenize("this *****ain't nothin*** this is cruel **\n");
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
      {TokenType::TEXT, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(LexerTest, HandlesStarsRandom2) {
  Lexer t;
  t.tokenize("this *is the *****ain't nothin** this is cruel **\n");
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
      {TokenType::TEXT, "\n"},
  };
  // clang-format on
  ASSERT_EQ(t.getTokens(), expected);
}

TEST(LexerTest, HandlesStarsRandom3) {
  Lexer t;
  t.tokenize("***So **it *works?* huh...**mornin*");
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

TEST(LexerTest, HandlesStarsRandom4) {
  Lexer t;
  /*
   * **   -> Delete
   * ***
   * *
   *
   * *    -> obviously ++correction (after prev)
   * *
   *
   * */
  t.tokenize("**how***bout*zis? where this text?");
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

TEST(LexerTest, HandlesStarsRandom5) {
  Lexer t;
  t.tokenize("**this should be**** entirely bold**");
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

TEST(LexerTest, HandlesUnderscoresRandom) {
  Lexer t;
  t.tokenize("__this should be____entirely bold__");
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

TEST(LexerTest, HandlesStarsAndUnderscores) {
  Lexer t;
  t.tokenize("__this *should__ be** entirely bold__");
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

TEST(LexerTest, HandlesStarsAndUnderscores2) {
  Lexer t;
  t.tokenize("__this *should be** entirely bold__");
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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
