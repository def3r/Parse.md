# `Parse.md`
A Markdown parser library for C++.

<img width="1891" height="873" alt="Screenshot_20251002_123152" src="https://github.com/user-attachments/assets/685b7fe2-648b-44a0-977a-e3e411088c47" />

> [!WARNING]
> *This library is incomplete and only supports a [subset](https://github.com/def3r/Parse.md/blob/main/parse.h#L11) of Markdown as of now.*

## Features
* `Parse.md` is small and lightweight.
- `Parse.md` abides by the [CommonMark Spec](https://spec.commonmark.org/0.31.2/).
* `Parse.md`'s AST allows to write small renderers or transpilers.
* It gives complete control over the lexical analysis and its candidate tokens.

## Demo
> For a detailed demo, see [demo.cpp](https://github.com/def3r/Parse.md/blob/main/demo.cpp)

```c++
#include <iostream>
#include "parsemd/parser.h"

using namespace markdown;

int main() {
  Parser p;
  p.Parse("### This is **strong** and *italic*");
  Node root = p.GetRoot();
  std::cout << Parser::DumpTree(root);
}
```
