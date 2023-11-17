# CS325-Coursework
Coursework for CS325 Compiler Design

On DCS workstations, run this command to set up the correct version of GCC and LLVM for the coursework:

```bash
source setup.sh
```
<ins>New additions:</ins>

- Lexical analyser runs before parser
    - The lexical analyser checks that there are no invalid tokens before the parsing can start
        - This is done so lexical errors can be outputted
- Lexer now checks for -,+,% and * and gives it correct token types, instead of passing it through as ascii values
- Lexer now checks for invalid symbols - passes it on as invalid tokens
    - These invalid tokens are detected by the lexical analyser 
- Root node is a list of TopLevelASTnodes as children (not an ASTnode)
    - This causes a change in the AST printer (line 3674)
- Int or float values out of range are set to 0 or 0.0 respectively 
    - See lines 1341 and 1355
- Zero division not permitted for all types - gives semantic error
    - Because it gives undefined behaviour (refer to LLVM reference manual for sdiv function)
- Zero modulo operation also not permitted for all types
    - Because it gives undefined behaviour (refer to LLVM reference manual for srem function)
- Added boolean short circuit code generation (lazy evaluation) for logical operators || and &&
    - If LHS of an || expression is true, whole expression evaluates to true, no need to produce codegen() of RHS
    - If LHS of an && expression is false, whole expression evaluates to false,
    no need to produce codegen() of RHS
- Widening conversions are also applied to return statements
    - The return types can be widened to the correct type with a warning
    - If the type cannot be widened to the correct type, a semantic error occurs