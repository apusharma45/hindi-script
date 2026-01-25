# Minimum Working Language (MWL v1) — Sentence-style HindiScript

Statement rules:
- No semicolons.
- Each statement ends at NEWLINE.
- Blocks are explicit: shuru ... khatam

Core statements (v1):
- rakho <id> me <expr>          (assignment)
- likho <expr>                 (print)
- agar <expr> to <block>        (if)
  warna <block>                 (else, optional)
- jabtak <expr> <block>         (while)
- banao <name> <params...> <block> (function)
- wapas <expr>                 (return)

Expressions (v1):
- infix arithmetic: + - * /
- comparisons: > < >= <= == !=
- function call: name(expr, expr, ...)

Not included in v1:
- pipeline operator |>
- advanced logical keywords (aur/ya/nahi) unless added later
