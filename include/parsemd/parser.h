#ifndef PARSEMD_PARSE_H_
#define PARSEMD_PARSE_H_

// Making it CommonMark Compliant
// https://spec.commonmark.org/0.31.2/#appendix-a-parsing-strategy

#include "block.h"
#include "delimiterstack.h"
#include "scanner.h"
#include "types.h"

namespace markdown {
class Parser {
 public:
  Parser();

  void AssignDocument(std::string_view);
  void AnalyzeBlocks(std::string_view);
  Block BuildBlocks();
  Block BuildParagraphBlock();
  Block GetBlock();

  void Tokenize(std::string_view);  // returns a vector of lexemes
  void LexAnalysis();  // analyse lexemes and provide TokenType, returns Tokens
  Tree Parse();        // parse the Tokens
  Tree Parse(std::string_view);  // parse the string
  Tree GetRoot();

  Tokens GetTokens();
  void debug();

  DelimiterStack delimStack = {};
  void AnalyzeInline();
  void PushCandToken();
  void PushCandToken(size_t);
  Block BuildInline();

  static std::string DumpTree(const Tree&, int = 0);
  static std::string DumpTree(const Block&, int = 0);

 private:
  enum class ContainerType { Root, Paragraph, Heading };
  enum class BlockType { Root, Paragraph, Heading };

  std::string document_;
  Scanner scanner = {};
  Tokens candTokens_;

  ContainerType containerType_ = ContainerType::Root;
  Tokens::iterator itToken_;
  Tree root_ = 0;
  Block block_ = {};
  BlockType blockType_ = BlockType::Root;

  Tree FinalPass();
  Tree BuildTree();
  void BuildParagraph(Node&);
  void BuildText(Node&);
  void BuildChildren(Node&);
  bool isParagraphEnd() const;
  bool validHeading();
  Tokens::iterator itTokenInc();
  Tokens::iterator itTokenInc(int);
};

}  // namespace markdown

#endif  // PARSEMD_PARSE_H_
