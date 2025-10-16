#include <iostream>
#include <string>
#include <unordered_map>

#include "parsemd/node.h"
#include "parsemd/parser.h"
#include "parsemd/types.h"

using namespace std;
namespace md = markdown;

const string CSI = "\033[";
typedef unordered_map<md::TokenType, pair<string, string>> RenderRules;

void populateHTMLMap(RenderRules&);
void populateANISMap(RenderRules&);
struct Renderer {
  Renderer(RenderRules& rr) : rr(rr) {}
  void Render(const md::Node& node);
  void RenderContainer(const md::Container& node);
  void RenderText(const md::Text& node);

 private:
  RenderRules& rr;
};

struct Mode {
  bool tree = false;
  bool html = false;
  bool ansi = false;
};
Mode getMode(int argc, char* argv[]);

int main(int argc, char* argv[]) {
  Mode mode = getMode(argc, argv);

  // initialize renderer
  RenderRules renderHTML, renderANSI;
  populateHTMLMap(renderHTML);
  populateANISMap(renderANSI);
  Renderer html(renderHTML), ansi(renderANSI);

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

void populateHTMLMap(RenderRules& m) {
  m[md::TokenType::Text] = {"", ""};
  m[md::TokenType::Strong] = {"<b>", "</b>"};
  m[md::TokenType::Emph] = {"<i>", "</i>"};
  m[md::TokenType::StrongEmph] = {"<b><i>", "</i></b>"};
  m[md::TokenType::Paragraph] = {"<p>", "</p>\n"};

  for (int i = 0; i <= 5; i++) {
    m[md::TokenType::H1 + i] = {"<H" + to_string(i + 1) + ">",
                                "</H" + to_string(i + 1) + ">\n"};
  }
}

void populateANISMap(RenderRules& m) {
  m[md::TokenType::Text] = {"", ""};
  m[md::TokenType::Strong] = {CSI + "1m", CSI + "22m"};
  m[md::TokenType::Emph] = {CSI + "3m", CSI + "23m"};
  m[md::TokenType::StrongEmph] = {CSI + "1m" + CSI + "3m",
                                  CSI + "23m" + CSI + "22m"};
  m[md::TokenType::Paragraph] = {"", "\n\n"};

  for (int i = 0; i <= 5; i++) {
    m[md::TokenType::H1 + i] = {CSI + to_string(i + 31) + "m", CSI + "0m\n"};
  }
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
  cout << rr[node->Type()].first;
  for (md::Node child : node->children)
    Render(child);
  cout << rr[node->Type()].second;
}

void Renderer::RenderText(const md::Text& node) {
  std::cout << node->text;
}
