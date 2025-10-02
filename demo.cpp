#include <iostream>
#include <string>
#include <unordered_map>
#include "parse.h"

using namespace std;

const string CSI = "\033[";
typedef unordered_map<TokenType, pair<string, string>> RenderRules;

void Render(const shared_ptr<Node>&, RenderRules&);
void populateHTMLMap(RenderRules&);
void populateANISMap(RenderRules&);

int main() {
  Parser p;
  string s = R"(# Heading 1
Some Text under **Heading 1**
## Heading 2
Some Text under **Heading 2**

*Paragraph 2* under **Heading 2**

### Heading 3
Some Text under ***Heading 3***
)";
  p.Parse(s);
  auto doc = p.GetRoot();

  RenderRules renderHTML, renderANSI;
  populateHTMLMap(renderHTML);
  populateANISMap(renderANSI);

  cout << Parser::DumpTree(doc);
  // Render(doc, renderANSI);
  // Render(doc, renderHTML);
}

void Render(const shared_ptr<Node>& token, RenderRules& rr) {
  cout << rr[token->type].first << token->value;
  for (auto child : token->children)
    Render(child, rr);
  cout << rr[token->type].second;
}

void populateHTMLMap(RenderRules& m) {
  m[TokenType::TEXT] = {"", ""};
  m[TokenType::BOLD] = {"<b>", "</b>"};
  m[TokenType::ITALIC] = {"<i>", "</i>"};
  m[TokenType::BOLD_ITALIC] = {"<b><i>", "</i></b>"};
  m[TokenType::PARAGRAPH] = {"<p>", "</p>\n"};

  for (int i = 0; i <= 5; i++) {
    m[TokenType::H1 + i] = {"<H" + to_string(i + 1) + ">",
                            "</H" + to_string(i + 1) + ">\n"};
  }
}

void populateANISMap(RenderRules& m) {
  m[TokenType::TEXT] = {"", ""};
  m[TokenType::BOLD] = {CSI + "1m", CSI + "22m"};
  m[TokenType::ITALIC] = {CSI + "3m", CSI + "23m"};
  m[TokenType::BOLD_ITALIC] = {CSI + "1m" + CSI + "3m",
                               CSI + "23m" + CSI + "22m"};
  m[TokenType::PARAGRAPH] = {"", "\n\n"};

  for (int i = 0; i <= 5; i++) {
    m[TokenType::H1 + i] = {CSI + to_string(i + 31) + "m", CSI + "0m\n"};
  }
}
