# CS325-Coursework
Coursework for CS325 Compiler Design

On DCS workstations, run this command to set up the correct version of GCC and LLVM for the coursework:

```bash
source setup.sh
```
New additions:

- Lexical analyser runs before parser
    - The lexical analyser checks that there are no invalid tokens before the parsing can start
        - This is done so lexical errors can be outputted
- Lexer now checks for -,+,% and * and gives it correct token types, instead of passing it through as ascii values
- Lexer now checks for invalid symbols - passes it on as invalid tokens
    - These invalid tokens are detected by the lexical analyser 
- Root node is a list of FunctionAST nodes as children (not an ASTnode)
    - This causes a change in the AST printer (line 1619)