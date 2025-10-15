#ifndef PARSEMD_PARSE_H_
#define PARSEMD_PARSE_H_

// Making it CommonMark Compliant
// https://spec.commonmark.org/0.31.2/#appendix-a-parsing-strategy

#include "delimiterstack.h"
#include "scanner.h"
#include "types.h"

namespace markdown {

class Parser {
 public:
  Parser();

  void AssignDocument(std::string_view);
  void AnalyzeBlocks(std::string_view);
  void AnalyzeBlocks();
  Node BuildBlocks();
  Node BuildParagraphBlock();
  Node GetBlock();

  // void Tokenize(std::string_view);
  // void LexAnalysis();
  Node Parse();
  Node Parse(std::string_view);
  Node GetRoot();

  Tokens GetTokens();

  DelimiterStack delimStack = {};
  void AnalyzeInline();
  void PushCandToken();
  void PushCandToken(size_t);
  Node BuildInline(Tokens::iterator it);

  // static std::string DumpTree(const Tree&, int = 0);
  static std::string DumpTree(const Node&, int = 0);

 private:
  enum class BlockType { Root, Paragraph, Heading };

  std::string document_;
  Scanner scanner = {};
  Tokens candTokens_;
  std::shared_ptr<BlockNode> block_ = {};
  BlockType blockType_ = BlockType::Root;
};

}  // namespace markdown

#endif  // PARSEMD_PARSE_H_
