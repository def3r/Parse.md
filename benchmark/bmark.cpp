#include <benchmark/benchmark.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "parsemd/parser.h"

using namespace std;

static std::string md_content, md_100KB, md_1MB;
static markdown::Parser p;

static void LoadFileOnce() {
  std::ifstream fs("data.md");
  if (!fs) {
    throw std::runtime_error(
        "data.md not found. Please execute `randomData.py` to generate "
        "data.md");
  }
  std::ostringstream ss;
  ss << fs.rdbuf();
  md_content = ss.str();
  md_100KB = md_content.substr(0, 100 * 1024);
  md_1MB = md_content.substr(0, 1024 * 1024);
}

static void BM_Parse1MB(benchmark::State& state) {
  for (auto _ : state) {
    p.Parse(md_1MB);
  }
}

static void BM_Parse100KB(benchmark::State& state) {
  for (auto _ : state) {
    p.Parse(md_100KB);
  }
}

static void BM_ParseEmpty(benchmark::State& state) {
  for (auto _ : state) {
    p.Parse("");
  }
}

static void BM_ParseSimple(benchmark::State& state) {
  for (auto _ : state) {
    p.Parse(R"(# Heading 1
Some Text under **Heading 1**

## Heading 2
Some Text under **Heading 2**

*Paragraph 2* under **Heading 2**

### Heading 3
Some Text under ***Heading 3***
)");
  }
}

int main(int argc, char* argv[]) {
  LoadFileOnce();
  benchmark::Initialize(&argc, argv);
  benchmark::SetDefaultTimeUnit(benchmark::kMicrosecond);
  benchmark::RegisterBenchmark("ParseMd Empty String", BM_ParseEmpty);
  benchmark::RegisterBenchmark("ParseMd Simple String", BM_ParseSimple);
  benchmark::RegisterBenchmark("ParseMd 100KB", BM_Parse100KB);
  benchmark::RegisterBenchmark("ParseMd 1MB", BM_Parse1MB);
  benchmark::RunSpecifiedBenchmarks();
}
