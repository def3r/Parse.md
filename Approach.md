# Parser.md
For this parser, we use stack based approach to tokenise and then parse.
The parsing of a std::string happens in 3 steps:
- Tokenizing
- Lexing
- Parsing

## Tokenizing:
Tokenizing is the process to break down an entire string into various smaller sub strings that might in future be of our interest.
For example: `int a = b + 9;` can be broken into: `["int", "a", "=", "b", "+", "9", ";"]`
For markdown, it would be something like this:
`# This might *be **important***` -> `["#", " This might ", "*", "be ", "**", "important", "***"]`
This array of maybe important substrings is called **lexemes**

To implement a tokenizer, we traverse the string and find lexemes
We maintain two std::string::iterator(s), `begin` and `it`, both iterating the string we need to tokenize.

```cpp
// init:
begin = line.begin();
it = begin;
```
We iterate over the line by incrementing `it` until it reaches `line.end()`
For each iteration over line, we check whether the current `char` is of our interest. Since we are implementing markdown parser, this could be a newline, `#`, `*`, `~`, `_` etc.
Not only we are concerned about which character is under the iterator right now, but also how many of those we have consecutively. A single `#` could `<H1>`, but consecutive three `#` ie `###` means `<H3>`. The meaning changes with the number of characters.
Thus, we need to have a look ahead method that would give us the count of consecutive characters ahead in the line.

```cpp
int Parser::lookAhead(const std::string& line, char&& c);
```
> [!NOTE]
> the iterator `it` is a private data member and is available to `lookAhead`


### Interface
```cpp
int main() {
    std::string str = "# Parse me `this` *riddler* __lol__";
    Parser p;
    p.Parse(str);
    NodeBase* root = p.SyntaxTree();
}
```
