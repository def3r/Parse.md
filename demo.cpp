// Example showing how to render Markdown:
//  1. to HTML and
//  2. to terminal (ANSI)

#include <iostream>
#include <string>
#include <unordered_map>

#include "parsemd/node.h"
#include "parsemd/parser.h"
#include "parsemd/types.h"

using namespace std;
namespace md = markdown;

const string CSI = "\033[";
using RenderRules = unordered_map<md::TokenType, pair<string, string>>;

// clang-format off
struct Renderer {
  Renderer(const RenderRules& rules) : rules(rules) {}
  void Render(const md::Node& node);
  void RenderContainer(const md::Container& node);
  void RenderText(const md::Text& node);

 protected:
  const RenderRules& rules;
};
struct HTMLRenderer : public Renderer {
  HTMLRenderer();
  static const RenderRules& HTMLRenderRules();
};
struct ANSIRenderer : public Renderer {
  ANSIRenderer();
  static const RenderRules& ANSIRenderRules();
};
// clang-format on

struct Mode {
  bool tree = false;
  bool html = false;
  bool ansi = false;
};
Mode getMode(int argc, char* argv[]);

// demo.cpp
// opts:
//    tree
//    ansi
//    html
int main(int argc, char* argv[]) {
  Mode mode = getMode(argc, argv);

  HTMLRenderer html;
  ANSIRenderer ansi;

  string s = R"(# Heading 1
Some Text under **Heading 1**

## Heading 2
Some Text under **Heading 2**

*Paragraph 2* under **Heading 2**

### Heading 3
Some Text under ***Heading 3***
)";

  md::Parser p;
  p.Parse(s);
  md::Node doc = p.GetRoot();

  if (mode.tree)
    cout << md::Parser::DumpTree(doc);
  if (mode.html)
    html.Render(doc);
  if (mode.ansi)
    ansi.Render(doc);
}

Mode getMode(int argc, char* argv[]) {
  Mode m{};
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "html") {
      m.html = true;
    } else if (std::string(argv[i]) == "ansi") {
      m.ansi = true;
    } else if (std::string(argv[i]) == "tree") {
      m.tree = true;
    }
  }
  return m;
}

void Renderer::Render(const md::Node& node) {
  if (!node) {
    return;
  }
  if (md::IsText(node)) {
    RenderText(md::TextNodePtr(node));
  } else {
    RenderContainer(md::ContainerNodePtr(node));
  }
}

void Renderer::RenderContainer(const md::Container& node) {
  cout << rules.at(node->Type()).first;
  for (md::Node child : node->children)
    Render(child);
  cout << rules.at(node->Type()).second;
}

void Renderer::RenderText(const md::Text& node) {
  std::cout << node->text;
}

HTMLRenderer::HTMLRenderer() : Renderer(HTMLRenderRules()) {}
const RenderRules& HTMLRenderer::HTMLRenderRules() {
  static RenderRules rules;
  if (rules.size() == 0) {
    rules[md::TokenType::Root] = rules[md::TokenType::Text] =
        rules[md::TokenType::Softbreak] = {"", ""};
    rules[md::TokenType::Strong] = {"<b>", "</b>"};
    rules[md::TokenType::Emph] = {"<i>", "</i>"};
    rules[md::TokenType::Paragraph] = {"<p>", "</p>\n"};

    // clang-format off
    for (int i = 0; i <= 5; i++) {
      rules[md::TokenType::H1 + i] = {
        "<H" + to_string(i + 1) + ">",
        "</H" + to_string(i + 1) + ">\n"
      };
    }
    // clang-format on
  }
  return rules;
}

// clang-format off
ANSIRenderer::ANSIRenderer() : Renderer(ANSIRenderRules()) {}
const RenderRules& ANSIRenderer::ANSIRenderRules() {
  static RenderRules rules;
  if (rules.size() == 0) {
    rules[md::TokenType::Root] = rules[md::TokenType::Text] =
        rules[md::TokenType::Softbreak] = {"", ""};
    rules[md::TokenType::Strong] = {CSI + "1m", CSI + "22m"};
    rules[md::TokenType::Emph] = {CSI + "3m", CSI + "23m"};
    rules[md::TokenType::StrongEmph] = {
      CSI + "1m" + CSI + "3m",
      CSI + "23m" + CSI + "22m"
    };
    rules[md::TokenType::Paragraph] = {"", "\n\n"};

    for (int i = 0; i <= 5; i++) {
      rules[md::TokenType::H1 + i] = {
        CSI + to_string(i + 31) + "m",
        CSI + "0m\n"
      };
    }
  }
  return rules;
}
// clang-format on
