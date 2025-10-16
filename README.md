# `Parse.md`
A Markdown parser library for C++.

<img width="1877" height="786" alt="Screenshot_20251016_215958" src="https://github.com/user-attachments/assets/5ab43efe-93cb-4650-87e8-33a63303a8af" />

> [!WARNING]
> *This library only supports a [subset](https://github.com/def3r/Parse.md/blob/main/include/parsemd/types.h#L16) of Markdown as of now.*

## Features
* `Parse.md` is small and lightweight.
- `Parse.md` abides by the [CommonMark Spec](https://spec.commonmark.org/0.31.2/).
* `Parse.md`'s AST allows to write small renderers or transpilers.

## Demo
> For an elaborate demo, see [demo.cpp](https://github.com/def3r/Parse.md/blob/main/demo.cpp)

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
