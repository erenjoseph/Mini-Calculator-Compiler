# Mini Calculator Compiler

A small compiler **front-end** for a tiny calculator language, written in C, with a matching browser demo. It runs three phases in order and shows the output of each:

```
Source -> [Lexer] -> tokens -> [Parser] -> AST -> [Semantic analyzer] -> result
```

A phase runs only if the previous one succeeded. It does **not** generate code or execute the program.

## Project structure

| File | Purpose |
|------|---------|
| `main.c` | Driver: reads input, runs the phases, prints each result |
| `lexer.c` / `lexer.h` | Lexical analysis: turns source text into tokens |
| `parser.c` / `parser.h` | Hand-written recursive-descent parser that builds the AST |
| `ast.c` / `ast.h` | AST node definitions and printing |
| `semantic.c` / `semantic.h` | Semantic checks (undefined variables, division by zero) |
| `sample_input.txt` | Example program |
| `index.html` | Standalone browser version (no server needed) |

## Build and run

Requires a C compiler such as `gcc`.

```bash
gcc -o calc *.c
./calc sample_input.txt     # run on a file
./calc                      # read from stdin (finish with Ctrl+D; Ctrl+Z then Enter on Windows)
```

On Windows (MinGW): `gcc -o calc.exe *.c` then `calc.exe sample_input.txt`.

Usage: `./calc [source_file]`

## Web version

Open `index.html` in any browser. Type or paste a program, then click **Compile**. Use **Load Example** to fill in a sample and **Clear** to reset. Results appear in the **Tokens**, **Syntax**, **AST**, **Semantic** and **Errors** tabs.

## Language

Programs are a list of statements, each ending with `;`.

```
a = 10;
b = 20;
c = a + b * 2;
print(c);
print((a + b) / 2);
```

### Grammar

```
Program        -> { Statement }
Statement      -> Assignment | PrintStatement
Assignment     -> IDENTIFIER '=' Expression ';'
PrintStatement -> 'print' '(' Expression ')' ';'
Expression     -> Term   { ('+' | '-') Term }
Term           -> Unary  { ('*' | '/') Unary }
Unary          -> ('+' | '-') Unary | Factor
Factor         -> NUMBER | IDENTIFIER | '(' Expression ')'
```

Operators: `+ - * /` and parentheses. `*` and `/` bind tighter than `+` and `-`.

## Phases

1. **Lexical analysis** splits the source into tokens (identifiers, numbers, operators, `=`, `(`, `)`, `;`) and records line numbers.
2. **Syntax analysis** checks the tokens against the grammar and builds an Abstract Syntax Tree.
3. **Semantic analysis** walks the AST and reports:
   - use of a variable before it is assigned (so `a = a + 1;` is an error if `a` is new)
   - division by zero

## Example output

Running `./calc sample_input.txt` prints, in order: the numbered source, the token table (line, token type, lexeme), the parse result, the AST, the semantic analysis, and a final result.
