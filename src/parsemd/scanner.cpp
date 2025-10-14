#include <iostream>
#include <iterator>

#include "parsemd/scanner.h"

namespace markdown {

Scanner::Scanner() {
  Init("");
}
Scanner::Scanner(std::string_view data) {
  Init(data);
}

void Scanner::Init(std::string_view data) {
  data_ = data;
  begin_ = it_ = data_.begin();
}

char Scanner::At(CurPos curPos, int offset) {
  std::string_view::iterator it = GetIterator(curPos);
  auto size = std::distance(data_.begin(), it) + offset;
  if (size < 0)
    return '\0';
  if (static_cast<size_t>(size) > data_.size())
    return '\0';
  if ((it == data_.end() && offset > 0) || (it == data_.begin() && offset < 0))
    return '\0';

  it += offset;
  // begin and end of a line are considered as whitespace
  if (it == data_.begin() || it == data_.end())
    return ' ';

  return *it;
}

char Scanner::ScanNextByte() {
  return End() ? '\0' : *(it_++);
}

char Scanner::CurrentByte() {
  if (it_ == begin_) {
    return '\0';
  }
  return *(it_ - 1);
}

std::string_view Scanner::Scan(size_t size, CurPos curPos, int offset) {
  std::string_view::iterator it = GetIterator(curPos);
  if (!ValidArgs(offset, it)) {
    return std::string_view();
  }
  it += offset;
  if (std::distance(data_.begin(), it) + size > data_.size()) {
    return std::string_view();
  }

  return std::string_view(it, size);
}

std::string_view Scanner::ScanNextLine() {
  begin_ = it_;
  while (it_ != data_.end() && *it_ != '\n')
    ++it_;
  return std::string_view(
      begin_, std::distance(begin_, (it_ != data_.end()) ? it_++ : it_));
}

std::string_view Scanner::CurrentLine() {
  if (begin_ == it_)
    return "";
  std::string_view::iterator it = it_;
  while ((it - 1) != begin_ && *(it - 1) == '\n') {
    --it;
  }
  return std::string_view(begin_, std::distance(begin_, it));
}

std::string_view Scanner::FlushCurrentLine() {
  std::string_view line = CurrentLine();
  Flush();
  return line;
}

void Scanner::Flush() {
  if (it_ == data_.begin())
    return;
  begin_ = it_ - 1;
}

void Scanner::FlushBytes(size_t n) {
  if (std::distance(data_.begin(), begin_) + n > data_.size()) {
    std::cerr << "OVERFLOW while Flushing Bytes" << std::endl;
    begin_ = it_ = data_.end();
    return;
  }
  begin_ += n;
  it_ = begin_;
}

void Scanner::SkipNextBytes(size_t n) {
  if (std::distance(data_.begin(), it_) + n > data_.size()) {
    it_ = data_.end();
    return;
  }
  it_ = it_ + n;
}

bool Scanner::End() {
  return it_ == data_.end();
}

int Scanner::LookAhead(CurPos curPos, int offset) {
  std::string_view::iterator it = GetIterator(curPos);
  if (!ValidArgs(offset, it)) {
    return -1;
  }

  it = it + offset;
  char c = *it;
  int count = 1;
  while ((it + count) != data_.end() && *(it + count) == c) {
    count++;
  }

  followedByWhiteSpace_ = (*(it + count) == ' ');
  return count;
}

std::string_view::iterator Scanner::GetIterator(CurPos curPos) {
  switch (curPos) {
    case CurPos::Begin:
      return data_.begin();
    case CurPos::BeginIt:
      return begin_;
    case CurPos::Cur:
      return it_;
    case CurPos::EndIt:
      return it_;
    default:
      return data_.end();
  }
}

bool Scanner::ValidArgs(int offset, std::string_view::iterator it) {
  auto size = std::distance(data_.begin(), it) + offset;
  if (size < 0)
    return false;
  if (static_cast<size_t>(size) > data_.size())
    return false;
  if ((it == data_.end() && offset >= 0) || (it == data_.begin() && offset < 0))
    return false;
  return true;
}

}  // namespace markdown
