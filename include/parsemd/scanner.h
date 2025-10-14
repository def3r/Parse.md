#ifndef PARSEMD_SCANNER_H_
#define PARSEMD_SCANNER_H_

#include "types.h"

namespace markdown {

class Scanner {
 public:
  enum class CurPos { Begin, BeginIt, Cur, EndIt, End };
  friend class Parser;
  Scanner();
  Scanner(std::string_view data);
  void Init(std::string_view data);
  char ScanNextByte();
  char CurrentByte();
  std::string_view ScanNextLine();
  std::string_view CurrentLine();
  std::string_view FlushCurrentLine();
  void Flush();
  void FlushBytes(size_t n);
  void SkipNextBytes(size_t n);
  bool End();

  // Random Access, internal state unaffected
  char At(CurPos curPos = CurPos::Begin, int offset = 0);
  std::string_view Scan(size_t size,
                        CurPos curPos = CurPos::Begin,
                        int offset = 0);
  int LookAhead(CurPos curPos = CurPos::Begin, int offset = 0);

 protected:
  std::string_view data_;
  std::string_view::iterator begin_, it_;
  bool updateBegin_ = false;
  bool followedByWhiteSpace_ = false;

  std::string_view::iterator GetIterator(CurPos curPos);
  bool ValidArgs(int offset, std::string_view::iterator);
};

}  // namespace markdown

#endif  // !PARSEMD_SCANNER_H_
