# `Parse.md`
A Markdown parser library for C++.

<img width="1877" height="786" alt="Screenshot_20251016_215958" src="https://github.com/user-attachments/assets/5ab43efe-93cb-4650-87e8-33a63303a8af" />

> [!WARNING]
> *This library only supports a [subset](https://github.com/def3r/Parse.md/blob/main/include/parsemd/types.h#L16) of Markdown as of now.*

## Features
* `Parse.md` is small, lightweight and fast.
- `Parse.md` abides by the [CommonMark Spec](https://spec.commonmark.org/0.31.2/).
* `Parse.md`'s AST allows to write small renderers or format converters.

## Benchmark
Benchmark, as run on 12th Gen Intel Core i5-12500H (4.5 GHz max) runnin Arch Linux (kernel 6.16.4), compiled with g++ (15.2.1) after compiler optimizations (Release build)

Throughput: 22 MB/s

| Benchmark | Time | CPU | Iterations |
|:----------|-----:|----:|-----------:|
| ParseMd Empty String |0.036 us | 0.036 us | 19429443 |
| ParseMd Simple String | 3.53 us | 3.53 us | 198560 |
| ParseMd 100KB | 4222 us | 4214 us | 166 |
| ParseMd 1MB | 44227 us | 44136 us | 16 |


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
