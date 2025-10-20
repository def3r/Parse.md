# Markdown Parser Benchmark (Master Test File)

This file is designed to test every aspect of a Markdown parser focusing on headings, paragraphs, strong, and emphasis.

It includes valid markup, nested markup, illegal markup, edge cases, and high-frequency stress tests.

---

## Section 1: Basic Valid Markup

### 1.1: Headings

# Heading 1
## Heading 2
### Heading 3
#### Heading 4
##### Heading 5
###### Heading 6

### 1.2: Paragraphs

This is a single-line paragraph.

This is a
multi-line paragraph. The renderer should join these lines
into a single paragraph.

This is the first paragraph.

This is the second paragraph. There are two newlines separating them.

### 1.3: Strong (Asterisks)

This paragraph contains **strong text** using asterisks.
This word is **bold**.
This **entire paragraph is strong**.

### 1.4: Strong (Underscores)

This paragraph contains __strong text__ using underscores.
This word is __bold__.
This __entire paragraph is strong__.

### 1.5: Emphasis (Asterisks)

This paragraph contains *emphasis text* using asterisks.
This word is *italic*.
This *entire paragraph is emphasized*.

### 1.6: Emphasis (Underscores)

This paragraph contains _emphasis text_ using underscores.
This word is _italic_.
This _entire paragraph is emphasized_.

---

## Section 2: High-Frequency & Nested Markup (Stress Test)

This section is for **Constraint 2**: using strong and emph as much as possible.

### 2.1: High-Frequency in Paragraphs

This *is* a *test* of *high-frequency* *emphasis* *using* *asterisks*. *Every* *other* *word* *should* *be* *italicized*, *which* *can* *be* *a* *challenge* *for* *some* *parsers* *that* *use* *regex* *greedily*. *This* *continues* *for* *several* *lines* *to* *ensure* *the* *parser* *can* *handle* *it* *without* *slowing* *down* *excessively*. *Lorem* *ipsum* *dolor* *sit* *amet*, *consectetur* *adipiscing* *elit*. *Sed* *do* *eiusmod* *tempor* *incididunt* *ut* *labore* *et* *dolore* *magna* *aliqua*.

This _is_ a _test_ of _high-frequency_ _emphasis_ _using_ _underscores_. _Every_ _other_ _word_ _should_ _be_ _italicized_, _which_ _can_ _be_ _a_ _challenge_ _for_ _some_ _parsers_ _that_ _use_ _regex_ _greedily_. _This_ _continues_ _for_ _several_ _lines_ _to_ _ensure_ _the_ _parser_ _can_ _handle_ _it_ _without_ _slowing_ _down_ _excessively_. _Lorem_ _ipsum_ _dolor_ _sit_ _amet_, _consectetur_ _adipiscing_ _elit_. _Sed_ _do_ _eiusmod_ _tempor_ _incididunt_ _ut_ _labore_ _et_ _dolore_ _magna_ _aliqua_.

This **is** a **test** of **high-frequency** **strong** **using** **asterisks**. **Every** **other** **word** **should** **be** **bolded**, **which** **can** **be** **a** **challenge** **for** **some** **parsers** **that** **use** **regex** **greedily**. **This** **continues** **for** **several** **lines** **to** **ensure** **the** **parser** **can** **handle** **it** **without** **slowing** **down** **excessively**. **Lorem** **ipsum** **dolor** **sit** **amet**, **consectetur** **adipiscing** **elit**. **Sed** **do** **eiusmod** **tempor** **incididunt** **ut** **labore** **et** **dolore** **magna** **aliqua**.

This __is__ a __test__ of __high-frequency__ __strong__ __using__ __underscores__. __Every__ __other__ __word__ __should__ __be__ __bolded__, __which__ __can__ __be__ __a__ __challenge__ __for__ __some__ __parsers__ __that__ __use__ __regex__ __greedily__. __This__ __continues__ __for__ __several__ __lines__ __to__ __ensure__ __the__ __parser__ __can__ __handle__ __it__ __without__slowing__ __down__ __excessively__. __Lorem__ __ipsum__ __dolor__ __sit__ __amet__, __consectetur__ __adipiscing__ __elit__. __Sed__ __do__ __eiusmod__ __tempor__ __incididunt__ __ut__ __labore__ __et__ __dolore__ __magna__ __aliqua__.

### 2.2: Nesting

This is **bold with *italic* inside**.
This is *italic with **bold** inside*.
This is __bold with _italic_ inside__.
This is _italic with __bold__ inside_.

This is **bold with _italic_ (mixed) inside**.
This is *italic with __bold__ (mixed) inside*.
This is __bold with *italic* (mixed) inside__.
This is _italic with **bold** (mixed) inside_.

### 2.3: Triple Delimiters (Bold + Emph)

This should be ***bold and italic*** (asterisks).
This should be ___bold and italic___ (underscores).
This should be **_bold and italic_** (mixed 1).
This should be *__bold and italic__* (mixed 2).
This should be __*bold and italic*__ (mixed 3).
This should be _**bold and italic**_ (mixed 4).

### 2.4: High-Frequency in Headings

# *This* _Heading_ **Is** __Full__ *Of* _Tags_
## **This** *Is* __A__ _Second_ **Level** *Heading* __With__ _Many_ **Tags**
### ***This*** ___Heading___ **_Is_** *__Even__* __*More*__ _**Complex**_
#### *a* _b_ **c** __d__ *e* _f_ **g** __h__ *i* _j_ **k** __l__ *m* _n_ **o** __p__

---

## Section 3: Illegal Markup & Edge Cases (Stress Test)

This section is for **Constraint 3**: illegal formats and delimiter runs.

### 3.1: Illegal Headings

#This is not a heading (no space)
##This is not a heading
###This is not a heading

This should be a paragraph:
#This is not a heading

####### This is not a heading (too many hashes), just a paragraph.

Not a heading:
#

### 3.2: Unclosed Delimiters

This paragraph has an *unclosed emphasis tag. It should render as a literal asterisk.
This paragraph has a **unclosed strong tag. It should render as literal asterisks.
This paragraph has an _unclosed emphasis tag.
This paragraph has a __unclosed strong tag.

*unclosed at start of line
**unclosed at start of line
_unclosed at start of line
__unclosed at start of line

The file might end on an unclosed tag. **This
The file might end on an unclosed tag. *This

### 3.3: Mismatched Delimiters

This is **mismatched and *should* not** work as expected.
This is __mismatched and _should_ not__ work as expected.
This is **mismatched and _should_ not** work as expected.
This is *mismatched and __should__ not* work as expected.

### 3.4: Illegal Delimiter Runs & Interleaving (The Hard Stuff)

This is the *__killer test__* which many parsers fail.
This is __*the other killer*__ test.
Interleaved: *This is _italic* and this is_ bold. (Should not parse)
Interleaved: **This is _bold** and this is_ italic. (Should not parse)

Empty delimiters: ****, ____, **, __, *, _

Excessive delimiters:
****This is just bold****
***This is bold and italic***
*****This is bold and italic with one leftover asterisk*****
******This is double bold, or just bold?******
_______This is bold and italic with two leftover underscores_______

### 3.5: Whitespace and Punctuation Edge Cases

Emphasis * surrounded by space * should not render.
Strong ** surrounded by space ** should not render.
Emphasis_with_no_spaces (underscores) should not render.
This_is_a_snake_case_variable and should not be _italic_.
This is a_b_c but this is _a_b_c_ which might be emphasis.
This is `a_b_c_d` vs `a_b_c_d_e`.

What about 5*5 = 25? This should not be emphasis.
What about 10__20 = 30? This should not be strong.

This is **bold *with nesting* and** punctuation!
This is **bold**! And *italic*?
This is (**bold**). And (*italic*).

Escaped characters:
\*This is not italic\*
\*\*This is not bold\*\*
\_This is not italic\_
\_\_This is not bold\_\_
This is \***not bold or italic**\*.

### 3.6: The Ultimate Stress Paragraph

This **paragraph** *is* a __complete__ _mess_ of **nested**, *interleaved*, __unclosed_, and *mismatched* tags. **It *tries* to _combine_ __everything__ at *once*.** Will it __*crash*__ the _parser_? Or will **it** *render* __correctly__ (which _means_ **rendering** *most* __of__ _this_ **as** *literal* __text__)? Let's *see*. This is **bold*. This is *italic**. This is __unclosed. This is _unclosed. This is ****a lot**** of ___stars___. What about * a * b * c * d * e * f * g * h * i * j * k * l * m * n * o * p * q * r * s * t * u * v * w * x * y * z*? What about __ a __ b __ c __ d __ e __ f __ g __ h __ i __ j __ k __ l __ m __ n __ o __ p __ q __r __ s __ t __ u __ v __ w __ x __ y __ z__? This is *__interleaved_*. This is **_interleaved__**. This is `foo_bar_baz`. This is `5*10*15`. This is a**b**c. This is a*b*c.
