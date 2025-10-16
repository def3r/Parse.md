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

struct Renderer {
  Renderer(const RenderRules& rr) : rr(rr) {}

  void Render(const md::Node& node) {
    if (!node) {
      return;
    }
    if (md::IsText(node)) {
      RenderText(md::TextNodePtr(node));
    } else {
      RenderContainer(md::ContainerNodePtr(node));
    }
  }

  void RenderContainer(const md::Container& node) {
    cout << rr[node->Type()].first;
    for (md::Node child : node->children)
      Render(child);
    cout << rr[node->Type()].second;
  }

  void RenderText(const md::Text& node) { std::cout << node->text; }

 private:
  RenderRules rr;
};

void populateHTMLMap(RenderRules&);
void populateANISMap(RenderRules&);

int main() {
  md::Parser p;
  string s = R"(# Heading 1
Some Text under **Heading 1**

## Heading 2
Some Text under **Heading 2**

*Paragraph 2* under **Heading 2**

### Heading 3
Some Text under ***Heading 3***
)";
  p.Parse(s);
  md::Node doc = p.GetRoot();

  RenderRules renderHTML, renderANSI;
  populateHTMLMap(renderHTML);
  populateANISMap(renderANSI);
  Renderer html(renderHTML), ansi(renderANSI);

  cout << md::Parser::DumpTree(doc);
  html.Render(doc);
  ansi.Render(doc);
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
