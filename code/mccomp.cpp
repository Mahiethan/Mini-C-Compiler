#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <string.h>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace llvm;
using namespace llvm::sys;

using namespace std;

FILE *pFile;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns one of these for known things.
enum TOKEN_TYPE {

  IDENT = -1,        // [a-zA-Z_][a-zA-Z_0-9]*
  ASSIGN = int('='), // '='

  // delimiters
  LBRA = int('{'),  // left brace
  RBRA = int('}'),  // right brace
  LPAR = int('('),  // left parenthesis
  RPAR = int(')'),  // right parenthesis
  SC = int(';'),    // semicolon
  COMMA = int(','), // comma

  // types
  INT_TOK = -2,   // "int"
  VOID_TOK = -3,  // "void"
  FLOAT_TOK = -4, // "float"
  BOOL_TOK = -5,  // "bool"

  // keywords
  EXTERN = -6,  // "extern"
  IF = -7,      // "if"
  ELSE = -8,    // "else"
  WHILE = -9,   // "while"
  RETURN = -10, // "return"
  // TRUE   = -12,     // "true"
  // FALSE   = -13,     // "false"

  // literals
  INT_LIT = -14,   // [0-9]+
  FLOAT_LIT = -15, // [0-9]+.[0-9]+
  BOOL_LIT = -16,  // "true" or "false" key words

  // logical operators
  AND = -17, // "&&"
  OR = -18,  // "||"

  // operators
  PLUS = int('+'),    // addition or unary plus
  MINUS = int('-'),   // substraction or unary negative
  ASTERIX = int('*'), // multiplication
  DIV = int('/'),     // division
  MOD = int('%'),     // modular
  NOT = int('!'),     // unary negation

  // comparison operators
  EQ = -19,      // equal
  NE = -20,      // not equal
  LE = -21,      // less than or equal to
  LT = int('<'), // less than
  GE = -23,      // greater than or equal to
  GT = int('>'), // greater than

  // special tokens
  EOF_TOK = 0, // signal end of file

  // invalid
  INVALID = -100 // signal invalid token
};

// TOKEN struct is used to keep track of information about a token
struct TOKEN {
  int type = -100;
  std::string lexeme;
  int lineNo;
  int columnNo;
};

static std::string IdentifierStr; // Filled in if IDENT
static int IntVal;                // Filled in if INT_LIT
static bool BoolVal;              // Filled in if BOOL_LIT
static float FloatVal;            // Filled in if FLOAT_LIT
static std::string StringVal;     // Filled in if String Literal
static int lineNo, columnNo;

static TOKEN returnTok(std::string lexVal, int tok_type) {
  TOKEN return_tok;
  return_tok.lexeme = lexVal;
  return_tok.type = tok_type;
  return_tok.lineNo = lineNo;
  return_tok.columnNo = columnNo - lexVal.length() - 1;
  return return_tok;
}

// Read file line by line -- or look for \n and if found add 1 to line number
// and reset column number to 0
/// gettok - Return the next token from standard input.
static TOKEN gettok() {

  static int LastChar = ' ';
  static int NextChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar)) {
    if (LastChar == '\n' || LastChar == '\r') {
      lineNo++;
      columnNo = 1;
    }
    LastChar = getc(pFile);
    columnNo++;
  }

  if (isalpha(LastChar) ||
      (LastChar == '_')) { // identifier: [a-zA-Z_][a-zA-Z_0-9]*
    IdentifierStr = LastChar;
    columnNo++;

    while (isalnum((LastChar = getc(pFile))) || (LastChar == '_')) {
      IdentifierStr += LastChar;
      columnNo++;
    }

    if (IdentifierStr == "int")
      return returnTok("int", INT_TOK);
    if (IdentifierStr == "bool")
      return returnTok("bool", BOOL_TOK);
    if (IdentifierStr == "float")
      return returnTok("float", FLOAT_TOK);
    if (IdentifierStr == "void")
      return returnTok("void", VOID_TOK);
    if (IdentifierStr == "bool")
      return returnTok("bool", BOOL_TOK);
    if (IdentifierStr == "extern")
      return returnTok("extern", EXTERN);
    if (IdentifierStr == "if")
      return returnTok("if", IF);
    if (IdentifierStr == "else")
      return returnTok("else", ELSE);
    if (IdentifierStr == "while")
      return returnTok("while", WHILE);
    if (IdentifierStr == "return")
      return returnTok("return", RETURN);
    if (IdentifierStr == "true") {
      BoolVal = true;
      return returnTok("true", BOOL_LIT);
    }
    if (IdentifierStr == "false") {
      BoolVal = false;
      return returnTok("false", BOOL_LIT);
    }

    return returnTok(IdentifierStr.c_str(), IDENT);
  }

  if (LastChar == '=') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // EQ: ==
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("==", EQ);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("=", ASSIGN);
    }
  }

  if (LastChar == '{') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("{", LBRA);
  }
  if (LastChar == '}') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("}", RBRA);
  }
  if (LastChar == '(') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("(", LPAR);
  }
  if (LastChar == ')') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok(")", RPAR);
  }
  if (LastChar == ';') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok(";", SC);
  }
  if (LastChar == ',') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok(",", COMMA);
  }

  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9]+.
    std::string NumStr;

    if (LastChar == '.') { // Floatingpoint Number: .[0-9]+
      do {
        NumStr += LastChar;
        LastChar = getc(pFile);
        columnNo++;
      } while (isdigit(LastChar));

      FloatVal = strtof(NumStr.c_str(), nullptr);
      return returnTok(NumStr, FLOAT_LIT);
    } else {
      do { // Start of Number: [0-9]+
        NumStr += LastChar;
        LastChar = getc(pFile);
        columnNo++;
      } while (isdigit(LastChar));

      if (LastChar == '.') { // Floatingpoint Number: [0-9]+.[0-9]+)
        do {
          NumStr += LastChar;
          LastChar = getc(pFile);
          columnNo++;
        } while (isdigit(LastChar));

        FloatVal = strtof(NumStr.c_str(), nullptr);
        return returnTok(NumStr, FLOAT_LIT);
      } else { // Integer : [0-9]+
        IntVal = strtod(NumStr.c_str(), nullptr);
        return returnTok(NumStr, INT_LIT);
      }
    }
  }

  if (LastChar == '&') {
    NextChar = getc(pFile);
    if (NextChar == '&') { // AND: &&
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("&&", AND);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("&", int('&'));
    }
  }

  if (LastChar == '|') {
    NextChar = getc(pFile);
    if (NextChar == '|') { // OR: ||
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("||", OR);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("|", int('|'));
    }
  }

  if (LastChar == '!') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // NE: !=
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("!=", NE);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("!", NOT);
      ;
    }
  }

  if (LastChar == '<') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // LE: <=
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("<=", LE);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("<", LT);
    }
  }

  if (LastChar == '>') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // GE: >=
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok(">=", GE);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok(">", GT);
    }
  }

  if (LastChar == '/') { // could be division or could be the start of a comment
    LastChar = getc(pFile);
    columnNo++;
    if (LastChar == '/') { // definitely a comment
      do {
        LastChar = getc(pFile);
        columnNo++;
      } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

      if (LastChar != EOF)
        return gettok();
    } else
      return returnTok("/", DIV);
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF) {
    columnNo++;
    LastChar = getc(pFile);
    return returnTok("0", EOF_TOK);
  }

  //This part of the original lexer code is changed, to not print out invalid tokens as its ascii values. This enables the lexer to detect invalid tokens.

  // // Otherwise, just return the character as its ascii value.
  // int ThisChar = LastChar;
  // std::string s(1, ThisChar);
  // LastChar = getc(pFile);
  // columnNo++;
  // return returnTok(s, int(ThisChar));

  //for ascii values - +, -, *, %
  if(LastChar == '-')
  {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("-", MINUS);
  }
  else if(LastChar == '+')
  {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("+", PLUS);
  }
  else if(LastChar == '*')
  {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("*", ASTERIX);
  }
  else if(LastChar == '%')
  {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("%", MOD);
  }

  //otherwise, pass current symbol as an invalid token
  int ThisChar = LastChar;
  std::string s(1, ThisChar);
  LastChar = getc(pFile);
  columnNo++;
  return returnTok(s, INVALID);
}

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static TOKEN CurTok;
static std::deque<TOKEN> tok_buffer;

static TOKEN getNextToken() {

  while(tok_buffer.size() != 2) //store two lookahead tokens
    tok_buffer.push_back(gettok());

  TOKEN temp = tok_buffer.front();
  tok_buffer.pop_front();

  return CurTok = temp;
}

static void putBackToken(TOKEN tok) { tok_buffer.push_front(tok); } //return token to buffer after looking-ahead two tokens

static void clearTokBuffer() {  //clear the buffer at the end
  while(tok_buffer.size() != 0) 
  {
    tok_buffer.pop_front();
  } 
}

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//

//Code to make indents for AST printing
static int indentLevel = 0;
static int indentAmount = 3;
static bool cont = true;
void increaseIndentLevel(){indentLevel = indentLevel + indentAmount;}
void decreaseIndentLevel(){
  indentLevel = (indentLevel > 0) ? indentLevel = indentLevel - indentAmount : indentLevel;
}
string addIndent()
{
  increaseIndentLevel();
  string final = "";
  if(cont == true)
    for(int i = 0; i < indentLevel; i++)
      if(i % 2 == 0)
        final.append("|");
      else
      {
        if(i == indentLevel - 1)
          final.append("-");
        else
          final.append(" ");
      }
  else
    final = std::string(indentLevel,' ');
  return final;
}

void resetIndent()
{
  indentLevel = 0;
}

/// ASTnode - Base class for all AST child nodes.
class ASTnode {
public:
  virtual ~ASTnode() {}
  virtual Value *codegen() = 0;
  virtual std::string to_string() const {return "";};
  virtual std::string getName() const {return "";};
  virtual TOKEN getTok() const {return {};};
};

/// IntASTnode - Class for integer literals like 1, 2, 10,
class IntASTnode : public ASTnode {
  int Val;
  TOKEN Tok;
  std::string Name;

public:
  IntASTnode(TOKEN tok, int val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override;
  virtual TOKEN getTok() const override{
    return Tok;
  }
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final = "IntegerLiteral: " + std::to_string(Val);
    decreaseIndentLevel();
    return final;
  };
};

/* add other AST nodes as nessasary */

/// FloatASTnode - Class for float literals like 1.0, 2.5, 10.23,
class FloatASTnode : public ASTnode {
  float Val;
  TOKEN Tok;
  std::string Name;

public:
  FloatASTnode(TOKEN tok, float val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override;
  virtual TOKEN getTok() const override{
    return Tok;
  }
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final = "FloatLiteral: " + std::to_string(Val);
    decreaseIndentLevel();
    return final;
  };
};

/// BoolASTnode - Class for boolean values: true, false
class BoolASTnode : public ASTnode {
  bool Val;
  TOKEN Tok;
  std::string Name;

public:
  BoolASTnode(TOKEN tok, bool val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override;
  virtual TOKEN getTok() const override{
    return Tok;
  }
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string boolVal = Val == true ? "true" : "false";
    string final =  "BoolLit: " + boolVal;
    decreaseIndentLevel();
    return final;
  };
};


/// VariableASTnode - Class for declaring variables, such as "a"
class VariableASTnode : public ASTnode{
  TOKEN Tok;
  string Val;
  string Type;

  public:
  VariableASTnode(TOKEN tok, string type, string val) : Type(type), Val(val), Tok(tok) {}
  string getVal()
  {
    return Val;
  }
  string getType()
  {
    return Type;
  }
  virtual Value *codegen() override;
  virtual TOKEN getTok() const override{
    return Tok;
  }
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final =  "VarDecl: " + Type + " " + Val; 
    decreaseIndentLevel();
    return final;
  };
};

/// VariableReferenceASTnode - Class for referenced variables
class VariableReferenceASTnode : public ASTnode{
  TOKEN Tok;
  string Name;

  public:
  VariableReferenceASTnode(TOKEN tok, string name) : Name(name), Tok(tok) {}
  virtual Value *codegen() override;
  virtual TOKEN getTok() const override{
    return Tok;
  }
  std::string getName() const override{ return Name; }
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final = "VarRef: " + Name;
    decreaseIndentLevel();
    return final;
  };
};

/// UnaryExprASTnode - Expression class for a unary operator, like ! or - (check production `rval_two`)
class UnaryExprASTnode : public ASTnode {
  string Opcode;
  std::unique_ptr<ASTnode> Operand;
  TOKEN Tok;

public:
  UnaryExprASTnode(string Opcode, std::unique_ptr<ASTnode> Operand, TOKEN tok)
      : Opcode(Opcode), Operand(std::move(Operand)), Tok(tok) {}

  virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final =  "UnaryExpr: " + Opcode  + "\n" + addIndent() + "--> " + Operand->to_string();
    decreaseIndentLevel();
    return final;
  };
};

/// BinaryExprASTnode - Expression class for a binary operator.
class BinaryExprASTnode : public ASTnode {
  string Opcode;
  std::unique_ptr<ASTnode> LHS, RHS;
  TOKEN Tok;

public:
  BinaryExprASTnode(string Opcode, std::unique_ptr<ASTnode> LHS,
                std::unique_ptr<ASTnode> RHS, TOKEN tok)
      : Opcode(Opcode), LHS(std::move(LHS)), RHS(std::move(RHS)), Tok(tok) {}

  virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final = "BinaryExpr: " + Opcode + "\n" + addIndent() + "--> " + LHS->to_string() + "\n" + addIndent() + "--> " + RHS->to_string();
    decreaseIndentLevel();
    return final;
  };
};

/// FuncCallASTnode - Expression class for function calls, including any arguments.
class FuncCallASTnode : public ASTnode {
  std::string Callee;
  std::vector<std::unique_ptr<ASTnode>> Args;
  TOKEN Tok;

public:
  FuncCallASTnode(const std::string &Callee,
              std::vector<std::unique_ptr<ASTnode>> Args, TOKEN tok)
      : Callee(Callee), Args(std::move(Args)), Tok(tok) {}

  virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string args = "";
    for(int i = 0; i < Args.size(); i++)
    {
      args.append("\n" + addIndent() + "--> Param" + Args[i]->to_string());
    }
    string final = "FunctionCall: " + Callee + args;
    decreaseIndentLevel();
    return final;
  };
};

/// IfExprASTnode - Expression class for if statement
class IfExprASTnode : public ASTnode {
  std::unique_ptr<ASTnode> Cond;
  std::vector<std::unique_ptr<ASTnode>> Then, Else;

public:
  IfExprASTnode(std::unique_ptr<ASTnode> Cond, std::vector<std::unique_ptr<ASTnode>> Then,
            std::vector<std::unique_ptr<ASTnode>> Else)
      : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
   string ThenStr = "";
   string ElseStr = "";
  string final = "IfExpr:\n" + addIndent() + "--> " + Cond->to_string();
  
  for(int i = 0; i < Then.size(); i++)
  {
    if(Then[i] != nullptr)
      ThenStr.append("\n" + addIndent() + "--> " + Then[i]->to_string()); //SORT THIS IDENT OUT
  } 

  final.append(ThenStr);
  if(Else.size() != 0)
   {
      ElseStr = "\n" + addIndent() + "--> ElseExpr:";
   }
  for(int j = 0; j < Else.size(); j++)
  {
    if(Else[j] != nullptr)
      ElseStr.append("\n" + addIndent() + "--> " + Else[j]->to_string()); 
  } 
  final.append(ElseStr);
  decreaseIndentLevel(); 
  if(Else.size() != 0)
  {
    decreaseIndentLevel(); 
  }
  return final;
  };
};

/// WhileExprASTnode - Expression class for while/do
class WhileExprASTnode : public ASTnode {
  std::unique_ptr<ASTnode> Cond;
  std::vector<std::unique_ptr<ASTnode>> Then;

public:
  WhileExprASTnode(std::unique_ptr<ASTnode> cond, std::vector<std::unique_ptr<ASTnode>> then)
      : Cond(std::move(cond)), Then(std::move(then)) {}

  virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string ThenStr = "";
    string final = "WhileExpr:\n" + addIndent() + "--> " + Cond->to_string();
    for(int i = 0; i < Then.size(); i++)
    {
      if(Then[i] != nullptr)
        ThenStr.append("\n" + addIndent() + "--> " + Then[i]->to_string());
    }
    final.append(ThenStr);
    decreaseIndentLevel();
    return final;
  };
};

/// ReturnExprASTnode - Expression class for return statements
class ReturnExprASTnode : public ASTnode {
  std::unique_ptr<ASTnode> ReturnExpr;
  string FuncReturnType;
  TOKEN Tok;

public:
  ReturnExprASTnode(std::unique_ptr<ASTnode> returnExpr, string funcReturnType, TOKEN tok)
      : ReturnExpr(std::move(returnExpr)), FuncReturnType(funcReturnType), Tok(tok) {}

  virtual Value *codegen() override;
  virtual TOKEN getTok() const override{
    return Tok;
  }
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string returnExpr = "";
    string final = "";
    if(ReturnExpr != nullptr)
      final = "ReturnStmt\n" + addIndent() + "--> " + ReturnExpr->to_string();
    else
       final = "ReturnStmt: " + FuncReturnType;
    decreaseIndentLevel();
    return final;
  };
};

///Base class for all top level nodes, such as function declarations, prototypes and global variables
class TopLevelASTnode {
public:
  virtual ~TopLevelASTnode() {}
  virtual Value *codegen() = 0;
  virtual std::string to_string() const {return "";};
};

//GlobalVariableAST - This class represents global variable declarations
class GlobalVariableAST : public TopLevelASTnode {
  TOKEN Tok;
  string Val;
  string Ty;

  public:
  GlobalVariableAST(TOKEN tok, string type, string val) : Ty(type), Val(val), Tok(tok) {}
  string getVal()
  {
    return Val;
  }
  string getType()
  {
    return Ty;
  }
  virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final =  "GlobalVarDecl: " + Ty + " " + Val; 
    decreaseIndentLevel();
    return final;
  };
};

/// PrototypeAST - This class represents the "prototype" for a function, capturing its name, and its argument names
class PrototypeAST : public TopLevelASTnode {
  std::string Name;
  std::vector<unique_ptr<VariableASTnode>> Args;

public:
  PrototypeAST(std::string &Name, std::vector<unique_ptr<VariableASTnode>> Args)
      : Name(Name), Args(std::move(Args)) {}

  const std::string &getName() const { return Name; }

  string getArgName(int index)
  {
    return Args.at(index)->getVal();
  }

  virtual Function *codegen() override;

  virtual std::string to_string() const override {
  //return a string representation of this AST node
  string name = getName();
  string args = "";

  for(int i = 0; i < Args.size(); i++)
  {
      args.append("\n" + addIndent() + "--> Param" + Args[i]->to_string());
  } 
  return "FunctionDecl: " + name + args;
  };
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public TopLevelASTnode {
  std::unique_ptr<PrototypeAST> Proto;
  std::vector<std::unique_ptr<ASTnode>> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto, //can have no prototypes (just block of expressions e.g. global variables)
              std::vector<std::unique_ptr<ASTnode>> Body) //Body can contain multiple expressions
      : Proto(std::move(Proto)), Body(std::move(Body)) {}

        virtual Function *codegen() override;

    virtual std::string to_string() const override{
  //return a string representation of this AST node
      string proto = Proto->to_string();
      string final =  proto;
      string body = "";
      if(Body.size() != 0)
      {
        body = "\n" + addIndent() + "--> Function Body:";
      }
      for(int i = 0; i < Body.size(); i++)
      {
        body.append("\n" + addIndent() + "--> " + Body[i]->to_string());
      }

      final.append(body);
      resetIndent();
      return final;
  };
};

//===----------------------------------------------------------------------===//
// Helpful data stores and variables to use during parsing and AST node generation
//===----------------------------------------------------------------------===//

///root of the AST - consists of a vector of TopLevelASTnodes
static vector<unique_ptr<TopLevelASTnode>> root; 

//null value for TOKEN variables
TOKEN nullToken = {};

static string prototypeName = ""; //stores prototype name during parsing
static unique_ptr<VariableASTnode> argument = std::make_unique<VariableASTnode>(CurTok,"",""); //stores a function argument/parameter
static unique_ptr<GlobalVariableAST> globalVar = std::make_unique<GlobalVariableAST>(CurTok,"",""); //stores a global variable
static string vartype = ""; //string that specifies type of variable, this is added to the VariableASTnode
static string functiontype = ""; //string that specifies type of function, this is added to the FunctionASTnode
static vector<unique_ptr<VariableASTnode>> argumentList = {}; //stores list of arguments of a function
static vector<unique_ptr<ASTnode>> body = {}; //stores contents of a function body as a vector of ASTnodes
static TOKEN functionIdent = nullToken; //stores identifier of a function
static TOKEN variableIdent = nullToken; //storesd identifier of a variable

static deque<pair<string,unique_ptr<ASTnode>>> stmtList; //temporary queue of named AST nodes used to process control flow statements in processStmt()

static vector<TOKEN> expression = {}; //these vector of token are used to parse the expression and create the correct ASTnodes

static bool errorReported = false; //used to make sure duplicate syntax error message are not being printed 

///Following functions are used to correctly flush these data stores after use for one AST node.

void resetPrototypeName()
{
  prototypeName = "";
}

void resetArgument()
{
  argument = std::make_unique<VariableASTnode>(nullToken,"","");
}

void resetGlobalVar()
{
  globalVar = std::make_unique<GlobalVariableAST>(nullToken,"","");
}

void resetVartype()
{
  vartype = "";
}

void resetFunctiontype()
{
  functiontype = "";
}

void resetArgumentList()
{
  argumentList.clear();
}

void resetBody()
{
  body.clear();
}

void resetStmtList()
{
  stmtList.clear();
}

void resetFunctionIdent()
{
  functionIdent = nullToken;
}

void resetVariableToken()
{
  variableIdent = nullToken;
}

void resetExpression()
{
  expression.clear();
}

//===----------------------------------------------------------------------===//
// Helper functions to use during parsing and AST node generation
//===----------------------------------------------------------------------===//

//used to check if CurTok is present inside a FIRST or FOLLOW set
static bool contains(int type, vector<TOKEN_TYPE> list)
{
  for(int i = 0; i < list.size(); i++)
  {
    if(list[i] == type)
      return true;
  }
  return false;
}

//Function that is used to check if CurTok matches the input type, then consumes the token.
bool match(TOKEN_TYPE token)
{
  if(CurTok.type == token)
  {
    getNextToken(); //consume token
    return true;
  }
  else
    return false;
}

//Creates a FunctionASTnode using all the necessary values gathered during parsing
void addFunctionAST()
{
  prototypeName.append(functiontype + " " + functionIdent.lexeme); //create prototype name
  resetFunctionIdent();
  resetFunctiontype();
  unique_ptr<PrototypeAST> Proto = std::make_unique<PrototypeAST>(prototypeName,std::move(argumentList)); //create PrototypeAST node
  resetArgumentList();
  resetPrototypeName();
  resetFunctiontype();
  unique_ptr<FunctionAST> Func = std::make_unique<FunctionAST>(std::move(Proto),std::move(body)); //create FunctionAST node, containing PrototypeAST node, created earlier, and vector of AST nodes, body.
  resetBody();
  root.push_back(std::move(Func)); //add FunctionAST to root
}

static std::pair<std::string, std::unique_ptr<ASTnode>> curr; //used in function below to store current statement token to process

//Function used to process all control flow statements and contained expressions, and create their correct AST nodes
unique_ptr<ASTnode> processStmtList()
{
  if(stmtList.size() > 0) //only pop if size is greater than one to avoid seg faults.
  {
    curr = std::move(stmtList.front());
    stmtList.pop_front();
  }
  else
  {
    return nullptr;
  }

  if(curr.first == "vardecl") //if a variable declaration is detected, return its AST node
  {
    return std::move(curr.second);
  }
  else if(curr.first == "expr") //return expression AST node
  {
    return std::move(curr.second);
  }
  else if(curr.first == "while") //process while statement and block
  {
    unique_ptr<ASTnode> cond = processStmtList(); //process the condition statement and store it here
    vector<unique_ptr<ASTnode>> then = {};
    while(curr.first != "end_while") //if 'end_while' flag is not detected, keep on adding following ASTnodes in queue to the Then block of while
    {
      unique_ptr<ASTnode> node = std::move(processStmtList());
      if(node != nullptr)
        then.push_back(std::move(node));
    }
    if(curr.first == "end_while") //don't add this flag to any blocks - signifies end of a block
    {
      curr.first = ""; //acknowledge end of while
      return std::move(make_unique<WhileExprASTnode>(std::move(cond),std::move(then))); //return created while block
    }
    else
    {
      return nullptr;
    }
    
  }
  else if(curr.first == "if") //process if statement and block
  {
    unique_ptr<ASTnode> cond = processStmtList(); //process the condition statement and store it here
    vector<unique_ptr<ASTnode>> Then = {};
    vector<unique_ptr<ASTnode>> Else = {};

    while((curr.first != "end_if")) //keep on adding following ASTnodes to the Then block until the "end_if" flag is detected
    {
      unique_ptr<ASTnode> node = std::move(processStmtList());
      if(node != nullptr)
        Then.push_back(std::move(node));
    }

    if(curr.first == "end_if") //skip this flag
    {
      curr = std::move(stmtList.front());
      stmtList.pop_front();
    }

    if(curr.first == "no_else") //this flag means that no Else block exists, so return the If ASTnode with a filled Then block, and empty Else block
    {
      return std::move(make_unique<IfExprASTnode>(std::move(cond),std::move(Then),std::move(Else)));
    }
    else if(curr.first == "else") //this flag means that an Else block exists
    {
      while(curr.first != "end_else") //keep on adding following ASTnodes to the Else block until the "end_else" flag is detected
      {
        unique_ptr<ASTnode> node = std::move(processStmtList());
        if(node != nullptr)
          Else.push_back(std::move(node));
      }
    } 

    if(curr.first == "end_else") //flag signified end of Else block
    {
      curr.first = ""; //acknowledge end of else
      return std::move(make_unique<IfExprASTnode>(std::move(cond),std::move(Then),std::move(Else))); //return If ASTnode with filled Then and Else blocks
    }
    else
    {
      return nullptr;
    }

  }
  else if(curr.first == "return") //create a returnExpr node for return statements
  {
    unique_ptr<ASTnode> returnExpr = processStmtList();
    TOKEN returnTok = nullToken;
    if(returnExpr != nullptr)
      returnTok = returnExpr->getTok();
    unique_ptr<ReturnExprASTnode> returnNode = make_unique<ReturnExprASTnode>(std::move(returnExpr),functiontype, returnTok);
    return std::move(returnNode);
  }
  else return std::move(nullptr); //if value in stmt_list is unrecognisable, return nullptr
} 

//Function which uses the processStmtList() to add control flow AST nodes to the body of parent function
void addToBody()
{
  while(stmtList.size() != 0)
  {
    unique_ptr<ASTnode> ptr = std::move(processStmtList());
    if(ptr != nullptr)
      body.push_back(std::move(ptr));
  }
}

//Used to determine precedence of input operator
int getPrecedence(string op)
{
  if(op == "*" | op == "/" | op == "%") //highest precedence
    return 70;
  else if(op == "+" | op == "-")
    return 60;
  else if(op == "<=" | op == "<" | op == ">=" | op == ">")
    return 50;
  else if(op == "==" | op == "!=")
    return 40;
  else if(op == "&&")
    return 30;
  else if(op == "||")
    return 20;
  else if(op == "=") //lowest precedence
    return 10;
  else
    return 110; //invalid (not an operator)
}

//This helper function is used to check if first left paranthesis is closed by the right paranthesis at the end of the expression
bool isMatchingLastParam(vector<TOKEN> expression)
{
  int buf = 0;
  for(int i = 1; i < expression.size(); i++)
  {
    if(expression.at(i).type == RPAR && buf == 0)
    {
      if(i == expression.size() - 1)
        return true;
      else
        return false;
    }
    else if(expression.at(i).type == RPAR && buf > 0)
    {
      buf--;
    }
    else if(expression.at(i).type == LPAR)
    {
      buf++;
    } 
  }
  return false;
}

//Essential helper function used to parse expressions and create correct AST nodes
unique_ptr<ASTnode> createExprASTnode(vector<TOKEN> expression)
{
  if(expression.size() == 1) //literals
  {
    TOKEN t = expression.at(0);
    if(t.type == INT_LIT) //for int literals
    {
      int val;
      try{ //make sure that the string conversion results in a value that is within the valid integer range
         val = stoi(t.lexeme);
      }
      catch(std::out_of_range) //otherwise, set the value to 0 to avoid undefined values
      {
        errs()<<"Warning: Value "<<t.lexeme<<" out of range for int type. Setting it to 0\n";
        val = stoi("0");
      }
      
      return std::move(make_unique<IntASTnode>(t,val)); //return IntAST node
    }
    else if(t.type == FLOAT_LIT) //for int literals
    {
      float val;
      try{ //make sure that the string conversion results in a value that is within the valid float range
         val = stof(t.lexeme);
      }
      catch(std::out_of_range) //otherwise, set the value to 0.0 to avoid undefined values
      {
        errs()<<"Warning: Value "<<t.lexeme<<" out of range for float type. Setting it to 0.0\n";
        val = stof("0.0");
      }
      return std::move(make_unique<FloatASTnode>(t,val));
    }
    else if(t.type == BOOL_LIT) //for boolean literals, true or false
    {
      if(t.lexeme == "true")
        return make_unique<BoolASTnode>(t,true);
      else if(t.lexeme == "false")
        return make_unique<BoolASTnode>(t,false);
    }
    else if(t.type == IDENT)
    {
      return std::move(make_unique<VariableReferenceASTnode>(t,t.lexeme));
    }
    else
    {
      return std::move(nullptr);
    }
  }
  //for unary expressions 
  else if((expression.at(0).lexeme == "-" | expression.at(0).lexeme == "!") & (expression.at(1).lexeme == "-" | expression.at(1).lexeme == "!" | expression.at(1).lexeme == "(" | expression.size() == 2)) 
  {
    string opcode = expression.at(0).lexeme;
    vector<TOKEN> operand = {}; //this can either be a single value or a long expression that needs to be parsed recursively
    for(int i = 1; i < expression.size(); i++)
      operand.push_back(expression.at(i));

    return std::move(make_unique<UnaryExprASTnode>(opcode,std::move(createExprASTnode(operand)),expression.at(0))); //return UnaryExprASTnode with opcode and operand(s)
  }
  else if((expression.at(0).lexeme == "(") & (isMatchingLastParam(expression) == true)) //for bracketed expr from start to end e.g (a + d + (a+f)), not (a+f)+(-e+d)
  {
    vector<TOKEN> newExpr = {}; //remove the first and last parantheses
    for(int i = 1; i < expression.size() - 1; i++)
    {
      newExpr.push_back(expression.at(i)); 
    } 
    return std::move(createExprASTnode(newExpr)); //parse resulting expression and return it
  }
  else if((expression.at(0).type == IDENT) & (expression.at(1).type == LPAR)) //function call with or without arguments
  {
    TOKEN funcTok = expression.at(0); //save identifier token to store in ASTnode, for use in printing out errors

    string callee = expression.at(0).lexeme; //get function callee name

    vector<unique_ptr<ASTnode>> args = {};
    vector<TOKEN> expr = {};
    bool start = false;
    if(expression.at(2).type != RPAR) //if function call has arguments
    {
      for(int i = 2; i < expression.size(); i++) //ignoring first LPAR and last RPAR
      {
        if(i==2)
          start = true;

        if(i == expression.size()-1)
        {
          start = false;
          args.push_back(std::move(createExprASTnode(expr))); //parse expr and add to args
          expr.clear();
        }

        if(expression.at(i).type == COMMA)
        {
          args.push_back(std::move(createExprASTnode(expr))); //parse expr and add to args
          expr.clear();
        }
        else
        {
          if(start == true)
          {
            expr.push_back(expression.at(i));
          }
        }
      }
    }
      return std::move(make_unique<FuncCallASTnode>(callee,std::move(args),funcTok)); //return FuncCallASTnode
  }
  else 
  {
    int minPrecedence = 100;
    string op = "";
    TOKEN opTok;
    int index = 0;
    bool isOp = true;
    bool unaryEnd = true;
    int valid = 0; //operators inside bracketed expressions are invalid so value set to 1, otherwise set to 0
    for(int i = 0; i < expression.size(); i++)
    {
      int currPrecedence = getPrecedence(expression.at(i).lexeme);
      if(expression.at(i).lexeme == "(") 
      {
        valid++; //don't parse bracket expressions here
      }
      if(expression.at(i).lexeme == ")")
      {
        valid--;
      }
      if(currPrecedence != 110) //operator found
      {
        if((currPrecedence <= minPrecedence) & (isOp == false) & (valid == 0)) //get lowest precedence operator, avoiding any unary operators
        { 
          op = expression.at(i).lexeme;
          opTok = expression.at(i);
          minPrecedence = currPrecedence;
          index = i;
          if(op == "=") //stop at the earliest `=` token
          {
            break;
          }
        }
        isOp = true;
      }
      else
      {
        isOp = false;
      }
    }

    //split expression into two parts, either side of the operator
    vector<TOKEN> lhs = {};
    vector<TOKEN> rhs = {};
    for(int i = 0; i < index; i++)
    {
      lhs.push_back(expression.at(i)); 
    }
    for(int i = index+1; i < expression.size(); i++)
    {
      rhs.push_back(expression.at(i));
    }

    return std::move(make_unique<BinaryExprASTnode>(op, std::move(createExprASTnode(lhs)), std::move(createExprASTnode(rhs)),opTok)); //recursive call to parse both the lhs and rhs of the expression
  }
  return nullptr;
}

//===----------------------------------------------------------------------===//
// FIRST sets for each production rule
//===----------------------------------------------------------------------===//

vector<TOKEN_TYPE> FIRST_extern_list{EXTERN};

vector<TOKEN_TYPE> FIRST_extern_list_prime{EXTERN};

vector<TOKEN_TYPE> FIRST_extern{EXTERN};

vector<TOKEN_TYPE> FIRST_decl_list{VOID_TOK,INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FIRST_decl_list_prime{VOID_TOK,INT_TOK,FLOAT_TOK,BOOL_TOK}; 

vector<TOKEN_TYPE> FIRST_decl{VOID_TOK,INT_TOK,FLOAT_TOK,BOOL_TOK}; 

vector<TOKEN_TYPE> FIRST_decl_prime{SC, LPAR}; 

vector<TOKEN_TYPE> FIRST_type_spec{VOID_TOK, INT_TOK, FLOAT_TOK, BOOL_TOK};

vector<TOKEN_TYPE> FIRST_var_type{INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FIRST_params{VOID_TOK,INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FIRST_param_list{INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FIRST_param_list_prime{COMMA};

vector<TOKEN_TYPE> FIRST_param{INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FIRST_block{LBRA};

vector<TOKEN_TYPE> FIRST_local_decls{INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FIRST_local_decl{INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FIRST_stmt_list{MINUS, NOT, LPAR, IDENT, INT_TOK,FLOAT_TOK,BOOL_TOK, SC, LBRA, IF, WHILE, RETURN};

vector<TOKEN_TYPE> FIRST_stmt{MINUS, NOT, LPAR, IDENT, INT_TOK,FLOAT_TOK,BOOL_TOK, SC, LBRA, IF, WHILE, RETURN};

vector<TOKEN_TYPE> FIRST_expr_stmt{MINUS, NOT, LPAR, IDENT, INT_TOK,FLOAT_TOK,BOOL_TOK, SC};

vector<TOKEN_TYPE> FIRST_while_stmt{WHILE};

vector<TOKEN_TYPE> FIRST_if_stmt{IF};

vector<TOKEN_TYPE> FIRST_else_stmt{ELSE};

vector<TOKEN_TYPE> FIRST_return_stmt{RETURN};

vector<TOKEN_TYPE> FIRST_return_stmt_prime{SC, MINUS, NOT, LPAR, IDENT, INT_TOK, FLOAT_TOK, BOOL_TOK};

vector<TOKEN_TYPE> FIRST_expr{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_exprStart{IDENT};

vector<TOKEN_TYPE> FIRST_rval_eight{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval_eight_prime{OR};

vector<TOKEN_TYPE> FIRST_rval_seven{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval_seven_prime{AND};

vector<TOKEN_TYPE> FIRST_rval_six{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval_six_prime{EQ, NE};

vector<TOKEN_TYPE> FIRST_rval_five{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval_five_prime{LE, LT, GE, GT};

vector<TOKEN_TYPE> FIRST_rval_four{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval_four_prime{PLUS, MINUS};

vector<TOKEN_TYPE> FIRST_rval_three{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval_three_prime{ASTERIX, DIV, MOD};

vector<TOKEN_TYPE> FIRST_rval_two{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval_one{LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_rval{LPAR};

vector<TOKEN_TYPE> FIRST_args{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_arg_list{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FIRST_arg_list_prime{COMMA};

//===----------------------------------------------------------------------===//
// FOLLOW sets for production rules with an epsilon production
//===----------------------------------------------------------------------===//

vector<TOKEN_TYPE> FOLLOW_extern_list_prime{VOID_TOK,INT_TOK,FLOAT_TOK,BOOL_TOK};

vector<TOKEN_TYPE> FOLLOW_decl_list_prime{EOF_TOK};

vector<TOKEN_TYPE> FOLLOW_params{RPAR};

vector<TOKEN_TYPE> FOLLOW_param_list_prime{RPAR};

vector<TOKEN_TYPE> FOLLOW_local_decls{MINUS, NOT, LPAR, IDENT, INT_TOK, FLOAT_TOK, BOOL_TOK, SC, LBRA, IF, WHILE, RETURN, RBRA};

vector<TOKEN_TYPE> FOLLOW_stmt_list{RBRA};

vector<TOKEN_TYPE> FOLLOW_else_stmt{MINUS, NOT, LPAR, IDENT, INT_TOK, FLOAT_TOK, BOOL_TOK, SC, LBRA, IF, WHILE, RETURN, RBRA};

vector<TOKEN_TYPE> FOLLOW_exprStart{MINUS, NOT, LPAR, IDENT, INT_LIT, FLOAT_LIT, BOOL_LIT};

vector<TOKEN_TYPE> FOLLOW_rval_eight_prime{SC, RPAR, COMMA};

vector<TOKEN_TYPE> FOLLOW_rval_seven_prime{OR, SC, RPAR, COMMA};

vector<TOKEN_TYPE> FOLLOW_rval_six_prime{AND, OR, SC, RPAR, COMMA};

vector<TOKEN_TYPE> FOLLOW_rval_five_prime{EQ, NE, AND, OR, SC, RPAR, COMMA};

vector<TOKEN_TYPE> FOLLOW_rval_four_prime{LE, LT, GE, GT, EQ, NE, AND, OR, SC, RPAR, COMMA};

vector<TOKEN_TYPE> FOLLOW_rval_three_prime{PLUS, MINUS, LE, LT, GE, GT, EQ, NE, AND, OR, SC, RPAR, COMMA};

vector<TOKEN_TYPE> FOLLOW_rval{ASTERIX, DIV, MOD, PLUS, MINUS, LE, LT, GE, GT, EQ, NE, AND, OR, SC, RPAR, COMMA};

vector<TOKEN_TYPE> FOLLOW_args{RPAR};

vector<TOKEN_TYPE> FOLLOW_arg_list_prime{RPAR};

//===----------------------------------------------------------------------===//
// Recursive Descent Parser - Using function calls for each production
//===----------------------------------------------------------------------===//

bool p_extern_list(); bool p_extern_list_prime();
bool p_extern();
bool p_type_spec();
bool p_decl_list(); bool p_decl_list_prime();
bool p_decl();
bool p_decl_prime();
bool p_var_type();
bool p_params();
bool p_param_list(); bool p_param_list_prime();
bool p_param();
bool p_block();
bool p_local_decls();
bool p_local_decl();
bool p_stmt_list();
bool p_stmt();
bool p_expr_stmt();
bool p_while_stmt();
bool p_if_stmt();
bool p_else_stmt();
bool p_return_stmt(); bool p_return_stmt_prime();
bool p_expr();
bool p_exprStart();
bool p_rval_eight(); bool p_rval_eight_prime();
bool p_rval_seven(); bool p_rval_seven_prime();
bool p_rval_six(); bool p_rval_six_prime();
bool p_rval_five(); bool p_rval_five_prime();
bool p_rval_four(); bool p_rval_four_prime();
bool p_rval_three(); bool p_rval_three_prime();
bool p_rval_two(); bool p_rval_one(); bool p_rval();
bool p_args(); bool p_arg_list(); bool p_arg_list_prime();

/* Defining functions for each production */

/// arg_list' ::= "," arg_list | epsilon
bool p_arg_list_prime()
{
  if(CurTok.type == COMMA)
  {
    TOKEN temp = CurTok;
    if(!match(COMMA))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ,  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);

    return p_arg_list();
  }
  else
  {
    if(contains(CurTok.type, FOLLOW_arg_list_prime))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}

///arg_list ::= expr arg_list'
bool p_arg_list()
{
  return p_expr() & p_arg_list_prime();
}

///args ::= arg_list | epsilon
bool p_args()
{
  if(contains(CurTok.type,FIRST_arg_list))
    return p_arg_list();
  else
  {
    if(contains(CurTok.type,FOLLOW_args))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}

///rval ::= "(" args ")" | epsilon
bool p_rval()
{
  if(CurTok.type == LPAR)
  {
    TOKEN temp = CurTok;
    if(!match(LPAR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  (  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);

    if(!p_args())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    temp = CurTok;
    if(!match(RPAR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  )  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);

    return true;
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}

///rval_one ::= "(" expr ")" | IDENT rval | INT_LIT | FLOAT_LIT | BOOL_LIT 
bool p_rval_one()
{
  if(CurTok.type == LPAR)
  {
    TOKEN temp = CurTok;
    if(!match(LPAR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ()  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);

    if(!p_expr())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    temp = CurTok;
    if(!match(RPAR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  )  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);
    return true;
  }
  else if(CurTok.type == IDENT)
  {
    variableIdent = CurTok;
    if(!match(IDENT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected an identifier at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(variableIdent);
    resetVariableToken();

    return p_rval();
  }
  else if(CurTok.type == INT_LIT)
  {
    variableIdent = CurTok;
    if(!match(INT_LIT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected an int literal at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(variableIdent);
    resetVariableToken();

    return true;
  }
  else if(CurTok.type == FLOAT_LIT)
  {
    variableIdent = CurTok;
    if(!match(FLOAT_LIT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected a float literal at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(variableIdent);
    resetVariableToken();

    return true;
  }
  else if(CurTok.type == BOOL_LIT)
  {
    variableIdent = CurTok;
    if(!match(BOOL_LIT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected a bool literal at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(variableIdent);
    resetVariableToken();

    return true;
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
}

/// rval_two ::= "-" rval_two | "!" rval_two | rval_one
bool p_rval_two()
{
  if(CurTok.type == MINUS)
  {
    TOKEN temp = CurTok;
    if(!match(MINUS))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  -  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);
    return p_rval_two();
  }
  else if(CurTok.type == NOT)
  {
    TOKEN temp = CurTok;
    if(!match(NOT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  !  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);
    return p_rval_two();
  }
  else if(contains(CurTok.type,FIRST_rval_one))
  {
    return p_rval_one();
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
}

/// rval_three ::= rval_two rval_three'
bool p_rval_three()
{
  return p_rval_two() & p_rval_three_prime();
}

/// rval_three' ::= "*" rval_two rval_three' | "/" rval_two rval_three' | "%" rval_two rval_three' | epsilon
bool p_rval_three_prime()
{
  if(CurTok.type == ASTERIX)
  {
    TOKEN temp = CurTok;
    if(!match(ASTERIX))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  *  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);
    return p_rval_two() & p_rval_three_prime();
  }
  else if(CurTok.type == DIV)
  {
    TOKEN temp = CurTok;
    if(!match(DIV))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  /  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);
    return p_rval_two() & p_rval_three_prime();
  }
  else if(CurTok.type == MOD)
  {
    TOKEN temp = CurTok;
    if(!match(MOD))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected"<<"  %  "<<"at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);
    return p_rval_two() & p_rval_three_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_three_prime))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


/// rval_four ::= rval_three rval_four'
bool p_rval_four()
{
  return p_rval_three() & p_rval_four_prime();
}


/// rval_four' ::= "+" rval_three rval_four' | "-" rval_three rval_four' | epsilon
bool p_rval_four_prime()
{
  if(CurTok.type == PLUS)
  {
    TOKEN temp = CurTok;
    if(!match(PLUS))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected"<<"  +  "<<"at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);

    return p_rval_three() & p_rval_four_prime();
  }
  else if(CurTok.type == MINUS)
  {
    TOKEN temp = CurTok;
    if(!match(MINUS))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  -  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    expression.push_back(temp);

    return p_rval_three() & p_rval_four_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_four_prime))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


/// rval_five ::= rval_four rval_five'
bool p_rval_five()
{
  return p_rval_four() & p_rval_five_prime();
}


/// rval_five' ::= "<=" rval_four rval_five' | "<" rval_four rval_five' | ">=" rval_four rval_five' | ">" rval_four rval_five' | epsilon
bool p_rval_five_prime()
{
  if(CurTok.type == LE)
  {
    TOKEN temp = CurTok;
    if(!match(LE))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  <=  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == LT)
  {
    TOKEN temp = CurTok;
    if(!match(LT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  <  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == GE)
  {
    TOKEN temp = CurTok;
    if(!match(GE))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  >=  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == GT)
  {
    TOKEN temp = CurTok;
    if(!match(GT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  >  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_five_prime))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


/// rval_six ::= rval_five rval_six'
bool p_rval_six()
{
  return p_rval_five() & p_rval_six_prime();
}



/// rval_six' ::= "==" rval_five rval_six' | "!=" rval_five rval_six' | epsilon
bool p_rval_six_prime()
{
  if(CurTok.type == EQ)
  {
    TOKEN temp = CurTok;
    if(!match(EQ))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ==  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);

    return p_rval_five() & p_rval_six_prime();
  }
  else if(CurTok.type == NE)
  {
    //return match(NE) & p_rval_five() & p_rval_six_prime();
    TOKEN temp = CurTok;
    if(!match(NE))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  !=  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);

    return p_rval_five() & p_rval_six_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_six_prime))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}



/// rval_seven ::= rval_six rval_seven'
bool p_rval_seven()
{
  return p_rval_six() & p_rval_seven_prime();
}



/// rval_seven' ::= "&&" rval_six rval_seven' | epsilon
bool p_rval_seven_prime()
{
  if(CurTok.type == AND)
  {
    TOKEN temp = CurTok;
    if(!match(AND))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  &&  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);

    return p_rval_six() & p_rval_seven_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_seven_prime))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


/// rval_eight ::= rval_seven rval_eight'
bool p_rval_eight()
{
  return p_rval_seven() & p_rval_eight_prime();
}


/// rval_eight' ::= "||" rval_seven rval_eight' | epsilon
bool p_rval_eight_prime()
{
  if(CurTok.type == OR)
  {
    TOKEN temp = CurTok;
    if(!match(OR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ||  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    expression.push_back(temp);

    if(!p_rval_seven())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    if(!p_rval_eight_prime())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    return true;
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_eight_prime))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


/// return_stmt' ::= ";" | expr ";" 
bool p_return_stmt_prime()
{
  if(CurTok.type == SC)
  {
    if(!match(SC))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ;  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    return true;
  }
  else if(contains(CurTok.type,FIRST_expr))
  {
    if(!p_expr())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    unique_ptr<ASTnode> expr = createExprASTnode(expression);
    pair <string,unique_ptr<ASTnode>> p = make_pair("expr",std::move(expr));
    stmtList.push_back(std::move(p));
    resetExpression();

    if(!match(SC))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ;  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    return true;
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
}


/// return_stmt ::= "return" return_stmt' 
bool p_return_stmt()
{
  if(!match(RETURN))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  `return`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
  
  pair <string,unique_ptr<ASTnode>> p = make_pair("return",std::move(nullptr));
  stmtList.push_back(std::move(p));

  if(!p_return_stmt_prime())
  {
    if(!errorReported)
      errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  return true;
}


/// expr ::= exprStart rval_eight
bool p_expr()
{
  if(!p_exprStart())
  {
    if(!errorReported)
      errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  if(!p_rval_eight())
  {
    if(!errorReported)
      errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
  return true;
}


///exprStart ::= IDENT "=" exprStart | epsilon
bool p_exprStart() //has a look-ahead of two tokens
{

  TOKEN firstLookAhead = CurTok;
  getNextToken();
  if(firstLookAhead.type == IDENT & CurTok.type == ASSIGN)
  {
      putBackToken(CurTok);
      CurTok = firstLookAhead;

      variableIdent = CurTok;
      if(!match(IDENT))
      {
        if(!errorReported)
          errs()<<"Syntax error: Expected an identifier at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false;
      }
      
      TOKEN temp = CurTok;
      if(!match(ASSIGN))
      {
        if(!errorReported)
          errs()<<"Syntax error: Expected  =  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false;
      }

      expression.push_back(variableIdent);
      expression.push_back(temp);
      resetVariableToken();

      return p_exprStart();

  }
  else
  {
    putBackToken(CurTok);
      CurTok = firstLookAhead;
    if(contains(CurTok.type,FOLLOW_exprStart))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


/// else_stmt  ::= "else" block | epsilon   
bool p_else_stmt()
{
  if(CurTok.type == ELSE)
  {
    // return match(ELSE) & p_block();
    if(!match(ELSE))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  `else`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    pair <string,unique_ptr<ASTnode>> p = make_pair("else",std::move(nullptr));
    stmtList.push_back(std::move(p));
    
    if(!p_block())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    pair <string,unique_ptr<ASTnode>> end_else = make_pair("end_else",std::move(nullptr));
    stmtList.push_back(std::move(end_else));

    return true;

  }
  else
  {
    if(contains(CurTok.type,FOLLOW_else_stmt))
    {
      //consume token
      pair <string,unique_ptr<ASTnode>> no_else = make_pair("no_else",std::move(nullptr));
      stmtList.push_back(std::move(no_else));
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


 
/// if_stmt ::= "if" "(" expr ")" block else_stmt 
bool p_if_stmt()
{
  if(!match(IF))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  `if`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  if(!match(LPAR))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  (  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  if(!p_expr())
  {
    if(!errorReported)
      errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  if(!match(RPAR))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  )  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
  
  pair <string,unique_ptr<ASTnode>> p = make_pair("if",std::move(nullptr));
  stmtList.push_back(std::move(p));
  unique_ptr<ASTnode> expr = createExprASTnode(expression);
  pair <string,unique_ptr<ASTnode>> e = make_pair("expr",std::move(expr));
  stmtList.push_back(std::move(e));
  resetExpression();

  if(!p_block())
  {
    if(!errorReported)
      errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  pair <string,unique_ptr<ASTnode>> end_if = make_pair("end_if",std::move(nullptr));
  stmtList.push_back(std::move(end_if));

  if(!p_else_stmt())
  {
    if(!errorReported)
      errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  return true;
}


/// while_stmt ::= "while" "(" expr ")" stmt 
bool p_while_stmt()
{
  // return match(WHILE) & match(LPAR) & p_expr() & match(RPAR) & p_stmt();
  if(!match(WHILE))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  `while`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

  if(!match(LPAR))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  (  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

  if(!p_expr())
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

  if(!match(RPAR))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  )  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }
  
  pair <string,unique_ptr<ASTnode>> p = make_pair("while",std::move(nullptr));
  stmtList.push_back(std::move(p));
  unique_ptr<ASTnode> expr = createExprASTnode(expression);
  pair <string,unique_ptr<ASTnode>> e = make_pair("expr",std::move(expr));
  stmtList.push_back(std::move(e));
  resetExpression();


  if(!p_stmt())
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }
  
  pair <string,unique_ptr<ASTnode>> n = make_pair("end_while",std::move(nullptr));
  stmtList.push_back(std::move(n));
  return true;

}


/// expr_stmt ::= expr ";" |  ";"
bool p_expr_stmt()
{
  if(contains(CurTok.type, FIRST_expr))
  {
    if(!p_expr())
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    if(!match(SC))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ;  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    unique_ptr<ASTnode> expr = createExprASTnode(expression);
    pair <string,unique_ptr<ASTnode>> p = make_pair("expr",std::move(expr));
    stmtList.push_back(std::move(p));
    resetExpression();

    return true;
  }
  else if(CurTok.type == SC)
  {
     if(!match(SC))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ;  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    return true;
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
}


/// stmt ::= expr_stmt | block | if_stmt | while_stmt | return_stmt
bool p_stmt()
{
  if(contains(CurTok.type,FIRST_expr_stmt))
  {
    return p_expr_stmt();
  }
  else if(contains(CurTok.type,FIRST_block))
  {
    return p_block();
  }
  else if(contains(CurTok.type,FIRST_if_stmt))
  {
    return p_if_stmt();
  }
  else if(contains(CurTok.type,FIRST_while_stmt))
  {
    return p_while_stmt();
  }
  else if(contains(CurTok.type,FIRST_return_stmt))
  {
    return p_return_stmt();
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
}


/// stmt_list ::= stmt stmt_list | epsilon 
bool p_stmt_list()
{
  if(contains(CurTok.type, FIRST_stmt_list))
  {
    if(!p_stmt())
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    return p_stmt_list();
  }
  else
  {
    if(contains(CurTok.type, FOLLOW_stmt_list))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


///local_decl ::= var_type IDENT ";" 
bool p_local_decl()
{
  if(!p_var_type())
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

  variableIdent = CurTok;
  if(!match(IDENT))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected an identifier at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  if(!match(SC))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  ;  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  unique_ptr<VariableASTnode> var = std::make_unique<VariableASTnode>(variableIdent, vartype, variableIdent.lexeme);
  pair <string,unique_ptr<ASTnode>> p = make_pair("vardecl",std::move(var));
  stmtList.push_back(std::move(p));
  resetVariableToken();
  resetVartype();

  return true;
}


/// local_decls ::= local_decl local_decls | epsilon
bool p_local_decls()
{
   if(contains(CurTok.type, FIRST_local_decl))
   {
      if(!p_local_decl())
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
        return false;
      }
      return p_local_decls();
   }
   else
   {
    if(contains(CurTok.type, FOLLOW_local_decls))
    {
      //consume token
      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
   }
}


/// param ::= var_type IDENT 
bool p_param()
{
  if(!p_var_type())
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

  TOKEN identifier = CurTok;
  
  if(!match(IDENT))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected an identifier at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  argument = std::make_unique<VariableASTnode>(identifier, vartype, identifier.lexeme);
  argumentList.push_back(std::move(argument));
  resetVartype();
  resetArgument();

  return true;
}


/// block ::= "{" local_decls stmt_list "}" 
bool p_block()
{

  if(!match(LBRA))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  {  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

  if(!p_local_decls())
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

   if(!p_stmt_list())
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
    return false;
  }

  if(!match(RBRA))
  {
    if(!errorReported)
        errs()<<"Syntax error: Expected  }  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }

  resetExpression();

  return true;
}


/// var_type  ::= "int" |  "float" |  "bool"
bool p_var_type()
{
  if(CurTok.type == INT_TOK)
  {
    if(!match(INT_TOK))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  `int`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    vartype.append("int");
    return true;
  }
  else if(CurTok.type == FLOAT_TOK)
  {
    if(!match(FLOAT_TOK))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  `float`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    vartype.append("float");
    return true;
  }
  else if(CurTok.type == BOOL_TOK)
  {
    if(!match(BOOL_TOK))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  `bool`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    vartype.append("bool");
    return true;
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
}


/// type_spec ::= "void" |  var_type
bool p_type_spec()
{
  if(CurTok.type == VOID_TOK)
  {
    if(!match(VOID_TOK))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  `void`   at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    prototypeName.append("void");
    return true;
  }
  else if(contains(CurTok.type,FIRST_var_type))
  {
    return p_var_type();
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false;
  }
}



/// param_list' ::= "," param_list | epsilon
bool p_param_list_prime()
{
  if(CurTok.type == COMMA)
  {
    
    if(!match(COMMA))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ,  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    if(!p_param_list())
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    return true;
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_param_list_prime)) //end of all parameters
    {
      //consume token

      return true;
    }
    else
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
  }
}


/// param_list ::= param param_list'
bool p_param_list()
{
  return p_param() & p_param_list_prime();
}



/// params ::= param_list | "void" | epsilon
bool p_params()
{
  if(contains(CurTok.type,FIRST_param_list))
  {
    return p_param_list();
  }
  else if(CurTok.type == VOID_TOK)
  {
    argument = std::make_unique<VariableASTnode>(CurTok, "void", "");
    argumentList.push_back(std::move(argument));
    return match(VOID_TOK);
  }
  else //epsilon
  {
    if(contains(CurTok.type,FOLLOW_params))
      {
        return true; //consume epsilon
      }
      else
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false; 
      }
  }
}

/// decl' ::= ";" | "(" params ")" block   
bool p_decl_prime()
{
  if(CurTok.type == SC) 
  {
    variableIdent = functionIdent; //it is not a function
    vartype = functiontype;
    resetFunctionIdent();
    resetFunctiontype();

    if(!match(SC))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ;  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    //global variable
    globalVar = std::make_unique<GlobalVariableAST>(variableIdent,vartype,variableIdent.lexeme);
    root.push_back(std::move(globalVar));
    resetVartype();
    resetVariableToken();
    resetGlobalVar();

    return true;

  }
  else if(CurTok.type == LPAR)
  {
    if(!match(LPAR))
    {
      if(!errorReported)
          errs()<<"Syntax error: Expected  (  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    if(!p_params())
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    //argument list defined

    if(!match(RPAR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  )  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    //variables for function prototype defined

    if(!p_block())
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    addToBody();
    resetStmtList();
    addFunctionAST(); 
    return true;
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false; 
  }
}


/// extern_list' ::= extern_list | epsilon
bool p_extern_list_prime()
{
  if(contains(CurTok.type,FIRST_extern_list))
   {
      return p_extern_list();
   }
   else //epsilon
   {
      if(contains(CurTok.type,FOLLOW_extern_list_prime))
      {
        return true; //consume epsilon
      }
      else
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false; 
      }
   }
}


/// extern ::= "extern" type_spec IDENT "(" params ")" ";"   
bool p_extern()
{
  if(!match(EXTERN))
  {
      if(!errorReported)
        errs()<<"Syntax error: Expected  `extern`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
  }

  prototypeName.append("extern ");

  if(!p_type_spec())
  {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
  }

  prototypeName.append(vartype + " ");
  resetVartype();
  
  string ident = CurTok.lexeme;

  if(!match(IDENT))
  {
      if(!errorReported)
        errs()<<"Syntax error: Expected an identifier at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
  }

  prototypeName.append(ident);

  //got function name

  if(!match(LPAR))
  {
      if(!errorReported)
        errs()<<"Syntax error: Expected  (  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
  }

  if(!p_params())
  {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
  }

  if(!match(RPAR))
  {
      if(!errorReported)
        errs()<<"Syntax error: Expected  )  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
  }

   if(!match(SC))
  {
      if(!errorReported)
        errs()<<"Syntax error: Expected  ;  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
  }
  unique_ptr<PrototypeAST> Proto = std::make_unique<PrototypeAST>(prototypeName,std::move(argumentList));
  root.push_back(std::move(Proto));
  resetArgumentList();
  resetPrototypeName();

  return true;

}



/// decl_list' ::= decl_list | epsilon
bool p_decl_list_prime()
{
   if(contains(CurTok.type,FIRST_decl_list))
   {
      return p_decl_list();
   }
   else //epsilon
   {
      if(contains(CurTok.type,FOLLOW_decl_list_prime))
      {
        return true; //consume epsilon
      }
      else
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false; //fail
      }
   }
}



/// decl ::= var_type IDENT decl' | "void" IDENT "(" params ")" block    
bool p_decl()
{
  if(contains(CurTok.type,FIRST_var_type))
  {
    if(!p_var_type())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    //vartype defined
    functiontype.append(vartype); //in case of a function decl
    resetVartype();

    functionIdent = CurTok; //in case of a function decl
    if(!match(IDENT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected an identifier at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    if(!p_decl_prime())
    {
      return false;
    }

    return true;
  }
  else if(CurTok.type == VOID_TOK)
  {
    if(!match(VOID_TOK))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  `void`  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    functiontype.append("void");

    functionIdent = CurTok;
    if(!match(IDENT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected an identifier at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

     if(!match(LPAR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  (  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    if(!p_params())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    //argument list defined

    if(!match(RPAR))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  )  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    //variables for function prototype defined

    if(!p_block())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    addToBody();
    resetStmtList();
    addFunctionAST(); 
    return true;
  }
  else
  {
    if(!errorReported)
      errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
    return false; 
  }
}


/// decl_list ::= decl decl_list'
bool p_decl_list()
{
  return p_decl() & p_decl_list_prime();
}


/// extern_list ::= extern extern_list' 
bool p_extern_list()
{
  return p_extern() & p_extern_list_prime();
}


// program ::= extern_list decl_list
bool p_program()
{
  if(contains(CurTok.type, FIRST_extern_list) == true)
  {
     return p_extern_list() & p_decl_list();
  }
  else if(contains(CurTok.type, FIRST_decl_list) == true)
  {
      return p_decl_list();
  }
  else
  {
    return false; //error
  }
}

//function to initialise parsing and get outcome, before moving on to code generation
static bool parser() {
  getNextToken();
  if(p_program() & (CurTok.type == EOF_TOK))
  {
    cout<<"Parsing successful."<<endl;
    return true; //continue to print AST and generate IR
  }
  else
  {
    return false;
  }

}

//===----------------------------------------------------------------------===//
// Code Generation - Defining codegen() functions for each AST node
//===----------------------------------------------------------------------===//

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;

static vector<map<string,AllocaInst*>> NamedValuesList; //vector of symbol tables
static map<string,GlobalVariable*> GlobalVariables; //symbol table for global variables

static AllocaInst* CreateEntryBlockAlloca(Function *TheFunction, const std::string &VarName, string type) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
  TheFunction->getEntryBlock().begin());
  if(type == "int")
    return TmpB.CreateAlloca(Type::getInt32Ty(TheContext), 0, VarName.c_str()); //the type (first argument) can be changed (for now its Int32)
  else if(type == "float")
    return TmpB.CreateAlloca(Type::getFloatTy(TheContext), 0, VarName.c_str());
  else if(type == "bool")
    return TmpB.CreateAlloca(Type::getInt1Ty(TheContext), 0, VarName.c_str());
  else
    return nullptr;
}

Value *IntASTnode::codegen() {
  return ConstantInt::get(TheContext, APInt(32,Val,true)); //int32 type
}

Value *FloatASTnode::codegen() {
  return ConstantFP::get(TheContext, APFloat(float(Val))); //float type
}

Value *BoolASTnode::codegen() {
  return ConstantInt::get(TheContext, APInt(1,int(Val),false)); //int1 type
}

Value *VariableASTnode::codegen() {
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  AllocaInst* varAlloca = CreateEntryBlockAlloca(TheFunction, Val, Type);
  //store in NamedValues
  std::map<std::string, AllocaInst*> NamedValues = NamedValuesList.back(); //obtain most recent symbol table (of current scope)
  NamedValuesList.pop_back();
  
  if(NamedValues.insert({Val,varAlloca}).second == false) //check if symbol table already contains same variable name
  {
    string existTy = "";
    if(NamedValues[Val]->getAllocatedType()->isIntegerTy(32))
      existTy = "int";
    else if(NamedValues[Val]->getAllocatedType()->isIntegerTy(1))
      existTy = "bool";
    else if(NamedValues[Val]->getAllocatedType()->isFloatTy())
      existTy = "float";
    
    errs()<<"Semantic error: Redefinition of variable "<<Val<<" with different type "<<Type<<" at column no. "<<Tok.columnNo<<", line no. "<<Tok.lineNo<<". Variable "<<Val<<" of type "<<existTy<<" already exists within current scope.\n";
    return nullptr;
  }
  NamedValuesList.push_back(NamedValues); //push symbol table back to the vector again after adding new variable
  return varAlloca;
}

Value *VariableReferenceASTnode::codegen() {
  // Look this variable up in the function.
  AllocaInst *V;

  //check through all symbol tables, starting from recent table
  std::map<std::string, AllocaInst*> NamedValues;
  int i;
  for(i = NamedValuesList.size() - 1; i >= 0; i--)
  {
    NamedValues = NamedValuesList.at(i);
    V = NamedValues[Name];
    if(!V)
    {
      continue; //it is not a local variable - no alloca found
    }
    else
    {
      return V; //return alloca so it can be loaded
    }

  }
  
  //check if its a global variable instead
  GlobalVariable *GV = GlobalVariables[Name];
  if(!GV)
  {
    errs()<<"Semantic error: Unknown variable name: "<<Name<<" at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
    return nullptr;
  }
  else 
  {
    return GV; //return global variable
  }
}

Value* UnaryExprASTnode::codegen()
{ 
  Value* operand = Operand->codegen();

  if(operand == nullptr)
    return nullptr;
  
  //load the operand depending on whether it is a local or global variable
  if(auto *AI = dyn_cast<AllocaInst>(operand))
  {
    operand = Builder.CreateLoad(AI->getAllocatedType(),operand,"load_temp");
  }
  else if(auto *GV = dyn_cast<GlobalVariable>(operand))
  {
    operand = Builder.CreateLoad(GV->getValueType(),operand,"load_global_temp");
  }

  //get the type of the operand
  string type = "";

  if(operand->getType()->isIntegerTy(32))
  {
    type = "int";
  }
  else if(operand->getType()->isIntegerTy(1))
  {
    type = "bool";
  }
  else if(operand->getType()->isFloatTy())
  {
    type = "float";
  }

  //unary operations only possible on operands of `bool` type
  if(Opcode == "!") 
  {
    if(type == "bool")
      return Builder.CreateNot(operand,"not_temp");
    else
    {
      errs()<<"Semantic error:  Cannot cast from `"<<type<<"` to `bool` at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
      return nullptr;    }
  }
  else if(Opcode == "-")
  {
    if(type == "bool")
    {
      operand = Builder.CreateIntCast(operand, Type::getInt32Ty(TheContext), false);
    }
    else if(type == "float") 
    {
      return Builder.CreateFNeg(operand,"fneg_temp");
    }
    return Builder.CreateNeg(operand,"neg_temp");
  }
  else
    return nullptr;
}

Value* BinaryExprASTnode::codegen(){
  Value* lhs = LHS->codegen();
  //boolean short circuit code generation for logical operators || and && - only works for constants (see `limitations` section in report)
  if(Opcode == "&&")
  {
    if(lhs == ConstantInt::get(TheContext, APInt(1,int(false),false))) //return false if lhs is false
      return ConstantInt::get(TheContext, APInt(1,int(false),false));
  }
  else if(Opcode == "||")
  {
    if(lhs == ConstantInt::get(TheContext, APInt(1,int(true),false))) //return true if lhs is true
      return ConstantInt::get(TheContext, APInt(1,int(true),false));
  }
  Value* rhs = RHS->codegen();

  bool isLHSAlloca = true;

  if(lhs == nullptr | rhs == nullptr)
    return nullptr;

  //storing types of lhs and rhs
  int lhsType = 0;
  int rhsType = 0;
  string lhsTypeStr = "";
  string rhsTypeStr = "";

  if(Opcode != "=") //load LHS if it is not an assign operation
  {
    if(auto *AI = dyn_cast<AllocaInst>(lhs)) //for ordinary variable alloca
    {
      if(AI->getAllocatedType()->isFloatTy())
        lhs = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_temp");
      else if(AI->getAllocatedType()->isIntegerTy(32))
        lhs = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_temp");
      else if(AI->getAllocatedType()->isIntegerTy(1))
        lhs = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI, "load_temp");

    }  
    else if(auto *GV = dyn_cast<GlobalVariable>(lhs)) //for global variables
    {
      if(GV->getValueType()->isFloatTy())
        lhs = Builder.CreateLoad(Type::getFloatTy(TheContext), GV, "load_global_temp");
      else if(GV->getValueType()->isIntegerTy(32))
        lhs = Builder.CreateLoad(Type::getInt32Ty(TheContext), GV, "load_global_temp");
      else if(GV->getValueType()->isIntegerTy(1))
        lhs = Builder.CreateLoad(Type::getInt1Ty(TheContext), GV, "load_global_temp");
    }
     if(lhs->getType()->isFloatTy())
        lhsType = 2;
      else if(lhs->getType()->isIntegerTy(32))
        lhsType = 1;
      else if(lhs->getType()->isIntegerTy(1))
        lhsType = 0;
  }
  else //keep LHS as an alloca/global and get its type
  {
    if(auto *AI = dyn_cast<AllocaInst>(lhs))
    {
      if(AI->getAllocatedType()->isFloatTy())
        lhsType = 2;
      else if(AI->getAllocatedType()->isIntegerTy(32))
        lhsType = 1;
      else if(AI->getAllocatedType()->isIntegerTy(1))
        lhsType = 0;
    }
    else if(auto *GV = dyn_cast<GlobalVariable>(lhs))
    {
      if(GV->getValueType()->isFloatTy())
        lhsType = 2;
      else if(GV->getValueType()->isIntegerTy(32))
        lhsType = 1;
      else if(GV->getValueType()->isIntegerTy(1))
        lhsType = 0;
    }
  }

  if(auto *AI = dyn_cast<AllocaInst>(rhs)) //load RHS
  {
    if(AI->getAllocatedType()->isFloatTy())
      rhs = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_temp");
    else if(AI->getAllocatedType()->isIntegerTy(32))
      rhs = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_temp");
    else if(AI->getAllocatedType()->isIntegerTy(1))
      rhs = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI,"load_temp");
  }
  else if(auto *GV = dyn_cast<GlobalVariable>(rhs)) //if RHS is a global variable
  {
    if(GV->getValueType()->isFloatTy())
      rhs = Builder.CreateLoad(Type::getFloatTy(TheContext), GV, "load_global_temp");
    else if(GV->getValueType()->isIntegerTy(32))
      rhs = Builder.CreateLoad(Type::getInt32Ty(TheContext), GV, "load_global_temp");
    else if(GV->getValueType()->isIntegerTy(1))
      rhs = Builder.CreateLoad(Type::getInt1Ty(TheContext), GV, "load_global_temp");
  }

  //get type of loaded RHS
  if(rhs->getType()->isIntegerTy(32))
      rhsType = 1;
    else if(rhs->getType()->isIntegerTy(1))
      rhsType = 0;
    else if(rhs->getType()->isFloatTy())
      rhsType = 2;

  ///get types as a "string" - for printing errors
  switch(lhsType)
  {
    case 0: lhsTypeStr = "bool"; break;
    case 1: lhsTypeStr = "int"; break;
    case 2: lhsTypeStr = "float"; break;
    default: break;
  }

  switch(rhsType)
  {
    case 0: rhsTypeStr = "bool"; break;
    case 1: rhsTypeStr = "int"; break;
    case 2: rhsTypeStr = "float"; break;
    default: break;
  }
  
    if(Opcode == "=") //ASSIGN
    {
      if(auto *st = dyn_cast<StoreInst>(rhs)) //get type again if a operand is a StoreInst e.g. a = b = 10;
      {
        rhs = st->getValueOperand();

        if(rhs->getType()->isIntegerTy(32))
          rhsType = 1;
        else if(rhs->getType()->isIntegerTy(1))
          rhsType = 0;
        else if(rhs->getType()->isFloatTy())
          rhsType = 2;

        switch(rhsType)
        {
          case 0: rhsTypeStr = "bool"; break;
          case 1: rhsTypeStr = "int"; break;
          case 2: rhsTypeStr = "float"; break;
          default: break;
        }
      }

      string name = LHS->getName(); //we know lhs is an alloca/global
      if(name != "")
      {
        //perform widening conversion before storing to lhs
        if(lhsType < rhsType)
        {
          errs()<<"Semantic error: Widening conversion not possible from RHS type "<<rhsTypeStr<<" to LHS type "<<lhsTypeStr<<" at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
          return nullptr;
        }
        else if(lhsType > rhsType)//perform widening conversions
        {
            if(lhsType == 2) //to float
            {
              if(rhsType == 0) //bool to float
              {
                rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
                rhs = Builder.CreateCast(Instruction::SIToFP,rhs,Type::getFloatTy(TheContext),"btof_cast");
              }
              else //int to float
                rhs = Builder.CreateCast(Instruction::SIToFP,rhs,Type::getFloatTy(TheContext),"itof_cast");
            }
            else if(lhsType == 1) //bool to int
              rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false, "btoi_cast");
 
        }
          return Builder.CreateStore(rhs,lhs);
      }
      else
        return nullptr;
    }

    bool isLogical = false;

    if(Opcode == "||" | Opcode == "&&")
    {
      isLogical = true;
    }

   if(isLogical == true) //makes sure that the operands of logical operands are both `bool` types
    {
      if(lhsType == 2 | rhsType == 2)
      {
        errs()<<"Semantic error: Cannot cast from `float` to `bool` at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
        return nullptr;
      }

      
      if(lhsType == 1 | rhsType == 1)
      {
        errs()<<"Semantic error: Cannot cast from `int` to `bool` at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
        return nullptr;
      }
    }
   
        if(Opcode == "||") //OR
          return Builder.CreateLogicalOr(lhs,rhs,"or_tmp"); 
        else if(Opcode == "&&") //AND
          return Builder.CreateLogicalAnd(lhs,rhs,"and_tmp"); 

      //Set both operands to equal types for +, -, *, /, %, ==, !=, <=, <, >= and > operators

      //for arithmetic operations, make sure to perform usual arithmetic conversions (get both operands to the same time) via widening only
      if(lhsType != rhsType)
      {
        int returnType = lhsType;
        if(rhsType > returnType)
          returnType = rhsType;
        
        if(returnType == rhsType)
        {
            if(returnType == 2) //to float
            {
              if(lhsType == 0) //bool to float
              {
                lhs = Builder.CreateIntCast(lhs, Type::getInt32Ty(TheContext), false); //bool to int
                lhs = Builder.CreateCast(Instruction::SIToFP,lhs,rhs->getType()); //int to float
              }
              else //int to float
                lhs = Builder.CreateCast(Instruction::SIToFP,lhs,rhs->getType());
              lhsType = 2;
            }
            else if(returnType == 1) //bool to int
            {
              lhs = Builder.CreateIntCast(lhs, Type::getInt32Ty(TheContext), false);
              lhsType = 1;
            }
           
        }
        else
        {
          if(returnType == 2) //to float
          {
            if(rhsType == 0) //bool to float
            {
                rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
                rhs = Builder.CreateCast(Instruction::SIToFP,rhs,lhs->getType());
            }
            else //int to float
                rhs = Builder.CreateCast(Instruction::SIToFP,rhs,lhs->getType());
            rhsType = 2;
          }
            else if(returnType == 1) //bool to int
            {
                rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
                rhsType = 1;
            }
          }
      }

      if(Opcode == "+") //PLUS
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFAdd(lhs,rhs,"fadd_tmp");
        else //for int or bool
          return Builder.CreateAdd(lhs,rhs,"add_tmp");
      }
      else if(Opcode == "-") //MINUS
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFSub(lhs,rhs,"fsub_tmp");
        else //for int or bool
          return Builder.CreateSub(lhs,rhs,"sub_tmp");
      }
      else if(Opcode == "*") //MULT
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFMul(lhs,rhs,"fmul_tmp");
        else //for int or bool
          return Builder.CreateMul(lhs,rhs,"mul_tmp");
      }
      else if(Opcode == "/") //DIV - print error for zero division
      {
        if(rhs == ConstantInt::get(TheContext, APInt(32,int(0),false)) | rhs == ConstantInt::get(TheContext, APInt(1,int(false),false)) | rhs == ConstantFP::get(TheContext, APFloat(float(0.0))))
        {
          errs()<<"Semantic error: Division by zero not permitted at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
          return nullptr;
        }

        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFDiv(lhs,rhs,"fdiv_tmp");
        else //for int or bool
          return Builder.CreateSDiv(lhs,rhs,"div_tmp");
      }
      else if(Opcode == "%") //MOD - make sure second operand is not equal to 0, regardless of type
      {
        if(rhs == ConstantInt::get(TheContext, APInt(32,int(0),false)) | rhs == ConstantInt::get(TheContext, APInt(1,int(false),false)) | rhs == ConstantFP::get(TheContext, APFloat((float)0.0)))
        {
          errs()<<"Semantic error: Taking remainder of division with zero not permitted at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
          return nullptr;
        }
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFRem(lhs,rhs,"fmod_tmp");
        else //for int or bool
          return Builder.CreateSRem(lhs,rhs,"mod_tmp");
      }
      else if(Opcode == "==") //EQ
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFCmpOEQ(lhs,rhs,"feq_tmp");
        else //for int or bool
          return Builder.CreateICmpEQ(lhs,rhs,"eq_tmp");
      }
       else if(Opcode == "!=") //NEQ
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFCmpONE(lhs,rhs,"fne_tmp");
        else //for int or bool
          return Builder.CreateICmpNE(lhs,rhs,"ne_tmp");
      }
      else if(Opcode == "<=") //LE
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFCmpOLE(lhs,rhs,"fle_tmp");
        else if(rhs->getType()->isIntegerTy(1) & lhs->getType()->isIntegerTy(1)) //bool
        {
          lhs = Builder.CreateIntCast(lhs, Type::getInt32Ty(TheContext), false);
          rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
        }
       
        return Builder.CreateICmpSLE(lhs,rhs,"le_tmp");
      }
      else if(Opcode == "<") //LT
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFCmpOLT(lhs,rhs,"flt_tmp");
        else if(rhs->getType()->isIntegerTy(1) & lhs->getType()->isIntegerTy(1)) //bool
        {
          lhs = Builder.CreateIntCast(lhs, Type::getInt32Ty(TheContext), false);
          rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
        }
        return Builder.CreateICmpSLT(lhs,rhs,"lt_tmp");
      }
      else if(Opcode == ">=") //GE
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFCmpOGE(lhs,rhs,"fge_tmp");
        else if(rhs->getType()->isIntegerTy(1) & lhs->getType()->isIntegerTy(1)) //bool
        {
          lhs = Builder.CreateIntCast(lhs, Type::getInt32Ty(TheContext), false);
          rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
        }
        return Builder.CreateICmpSGE(lhs,rhs,"ge_tmp");
      }
      else if(Opcode == ">") //GT
      {
        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFCmpOGT(lhs,rhs,"fgt_tmp");
        else if(rhs->getType()->isIntegerTy(1) & lhs->getType()->isIntegerTy(1)) //bool
        {
          lhs = Builder.CreateIntCast(lhs, Type::getInt32Ty(TheContext), false);
          rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
        }
        return Builder.CreateICmpSGT(lhs,rhs,"gt_tmp");
      }
      else
        return nullptr;
}

Value* FuncCallASTnode::codegen(){
  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF) //Function not found
  {
    errs()<<"Semantic error: Unknown function "<<Callee<<" referenced at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
    return nullptr;
  }
  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
  {
    errs()<<"Semantic error: Incorrect no. of arguments passed for function "<<Callee<<" at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
    return nullptr;
  }
  
  std::vector<Value *> ArgsV; //vector of arguments for function call
  for (unsigned i = 0, e = Args.size(); i != e; ++i) 
  {
    Value* args = Args[i]->codegen(); //get alloca/global
    string currType = "";
    //load arguments
    if(auto *AI = dyn_cast<AllocaInst>(args)) //for ordinary variable alloca
    {
      if(AI->getAllocatedType()->isFloatTy())
      {
        args = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_arg");
        currType = "float";
      }
      else if(AI->getAllocatedType()->isIntegerTy(32))
      {
        args = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_arg");
        currType = "int";
      }
      else if(AI->getAllocatedType()->isIntegerTy(1))
      {
        args = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI, "load_arg");
        currType = "bool";
      }

    }  
    else if(auto *GV = dyn_cast<GlobalVariable>(args)) //for global variables
    {
      if(GV->getValueType()->isFloatTy())
      {
        args = Builder.CreateLoad(Type::getFloatTy(TheContext), GV, "load_global_arg");
        currType = "float";
      }
      else if(GV->getValueType()->isIntegerTy(32))
      {
        args = Builder.CreateLoad(Type::getInt32Ty(TheContext), GV, "load_global_arg");
        currType = "int";
      }
      else if(GV->getValueType()->isIntegerTy(1))
      {
        args = Builder.CreateLoad(Type::getInt1Ty(TheContext), GV, "load_global_arg");
        currType = "bool";
      }
    }
    else
    {
      if(args->getType()->isIntegerTy(32))
      {
        currType = "int";
      }
      else if(args->getType()->isIntegerTy(1))
      {
        currType = "bool";
      }
      else if(args->getType()->isFloatTy())
      {
        currType = "float";
      }
    }

    //making sure types of each argument is correct
    //if not, see if it can be widened
    Type* actualType = CalleeF->getArg(0)->getType();
    string actualTypeStr = "";

    if(actualType->isIntegerTy(32))
      actualTypeStr = "int";
    else if(actualType->isIntegerTy(1))
      actualTypeStr = "bool";
    else if(actualType->isFloatTy())
      actualTypeStr = "float";

    if(currType != actualTypeStr)
    {
      if(actualTypeStr == "bool")
      {
        errs()<<"Semantic error: Cannot cast from `"<<currType<<"` to `"<<actualTypeStr<<"` at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
        return nullptr;
      }
      else if(actualTypeStr == "int")
      {
        if(currType == "float")
        {
          errs()<<"Semantic error: Cannot cast from `"<<currType<<"` to `"<<actualTypeStr<<"` at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<".\n";
          return nullptr;
        }
        else //bool to int
        {
          args = Builder.CreateIntCast(args, Type::getInt32Ty(TheContext), false, "btoi_cast");
        }
      }
      else if(actualTypeStr == "float")
      {
        if(currType == "bool")
        {
          args = Builder.CreateIntCast(args, Type::getInt32Ty(TheContext), false);
          args = Builder.CreateCast(Instruction::SIToFP,args,Type::getFloatTy(TheContext),"btof_cast");
        }
        else //int to float
        {
          args = Builder.CreateCast(Instruction::SIToFP,args,Type::getFloatTy(TheContext),"itof_cast");
        }
      }
      }

    ArgsV.push_back(args);
    if (!ArgsV.back())
      return nullptr;
  }

  if(CalleeF->getReturnType()->isVoidTy())
    return Builder.CreateCall(CalleeF, ArgsV); //void functions cannot have a name
  else
    return Builder.CreateCall(CalleeF, ArgsV, "call_tmp"); 
}

Value* IfExprASTnode::codegen(){

  bool elseExist = false;
   if(Else.size() != 0) //no need to create else branch if there is no else block
    elseExist = true;
  
  //create necessary basic block to add expressions to
  Function* TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock* true_ = BasicBlock::Create(TheContext, "if_then", TheFunction);
  BasicBlock* false_;

  if(elseExist)
    false_ = BasicBlock::Create(TheContext, "if_else", TheFunction);

  BasicBlock* end_ = BasicBlock::Create(TheContext, "if_end");
  Value* cond = Cond->codegen(); //generate condition expression
  if(cond == nullptr)
      return nullptr;

  //check type of cond - making sure it is a bool
  string currType = "";
  if(auto *AI = dyn_cast<AllocaInst>(cond)) //load RHS
  {
    if(AI->getAllocatedType()->isFloatTy())
    {
      cond = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_temp");
      currType = "float";
    }
    else if(AI->getAllocatedType()->isIntegerTy(32))
    {
      cond = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_temp");
      currType = "int";
    }
    else if(AI->getAllocatedType()->isIntegerTy(1))
    {
      cond = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI, "load_temp");
      currType = "bool";
    }
  }
  else if(auto *GV = dyn_cast<GlobalVariable>(cond)) //if RHS is a global variable
  {
    if(GV->getValueType()->isFloatTy())
    {
      cond = Builder.CreateLoad(Type::getFloatTy(TheContext), GV, "load_temp");
      currType = "float";
    }
    else if(GV->getValueType()->isIntegerTy(32))
    {
      cond = Builder.CreateLoad(Type::getInt32Ty(TheContext), GV, "load_temp");
      currType = "int";
    }
    else if(GV->getValueType()->isIntegerTy(1))
    {
      cond = Builder.CreateLoad(Type::getInt1Ty(TheContext), GV, "load_temp");
      currType = "bool";
    }
  }
  else
  {
    if(cond->getType()->isFloatTy())
      currType = "float";
    else if(cond->getType()->isIntegerTy(32))
      currType = "int";
    else if(cond->getType()->isIntegerTy(1))
      currType = "bool";
  }

  //make sure the condition statement is of `bool` type
  if(currType != "bool")
  {
    errs()<<"Semantic error: Expected type `bool` for the condition statement at line no. "<<Cond->getTok().lineNo<<" column no. "<<Cond->getTok().columnNo<<". Cannot cast from type `"<<currType<<"` to `bool`.\n";
    return nullptr;
  }

  Value* comp = Builder.CreateICmpNE(cond, ConstantInt::get(TheContext, APInt(1,0,false)), "if_cond");

  //create conditional branch instruction
  if(elseExist)
    Builder.CreateCondBr(comp, true_, false_);
  else
    Builder.CreateCondBr(comp, true_, end_);

  Builder.SetInsertPoint(true_);
  ///Then block
  std::map<std::string, AllocaInst*> NamedValues_Then; //Creating symbol table for Then block
  NamedValuesList.push_back(NamedValues_Then);

  bool generateBranchForThen = true;
  bool generateBranchForElse = true;
  Value* ret;

  for(int i = 0; i < Then.size(); i++)
  {
    //new block - create a new symbol table
    Value* thenVal = Then.at(i)->codegen();
    if(thenVal == nullptr)
      return nullptr;
    if(auto *R = dyn_cast<ReturnInst>(thenVal))
      {
        ret = thenVal;
        generateBranchForThen = false;
        break; //don't generate IR for instructions after return
      }
  }
  //remove symbol table of Then block
  NamedValuesList.pop_back();

  if(generateBranchForThen)
    Builder.CreateBr(end_);  //create unconditional branch instruction to end, given that `if_then` block had no return statements

  if(elseExist)
  {
    Builder.SetInsertPoint(false_);
    ///Else block
    //new block - create a new symbol table
    std::map<std::string, AllocaInst*> NamedValues_Else; //Creating symbol table for Else block
    NamedValuesList.push_back(NamedValues_Else);
    for(int i = 0; i < Else.size(); i++)
    {
      Value* elseVal = Else.at(i)->codegen();
      if(elseVal == nullptr)
        return nullptr;
      if(auto *R = dyn_cast<ReturnInst>(elseVal))
      {
        ret = elseVal;
        generateBranchForElse = false;
        break; //don't generate IR for instructions after return
      }
    }

    if(generateBranchForElse)
      Builder.CreateBr(end_); //create unconditional branch to end if else block has no return statements

    //remove symbol table of Else block
    NamedValuesList.pop_back();
  }

  if(generateBranchForThen | generateBranchForElse) //only generate 'if_end' block if 'if_then' and 'if_else' don't have a return stmt
  {
    TheFunction->insert(TheFunction->end(), end_);
    Builder.SetInsertPoint(end_);
    return ConstantPointerNull::get(PointerType::getUnqual(Type::getVoidTy(TheContext))); //return null pointer of void type
  }
  else
    return ret;
}

Value* WhileExprASTnode::codegen(){
  Function* TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock* cond_ = BasicBlock::Create(TheContext, "while_cond", TheFunction);
  BasicBlock* true_ = BasicBlock::Create(TheContext, "while_body", TheFunction);
  BasicBlock* false_ = BasicBlock::Create(TheContext, "while_end");
  Builder.CreateBr(cond_);
  Builder.SetInsertPoint(cond_);

  Value* cond = Cond->codegen(); //generate condition expression
  if(cond == nullptr)
      return nullptr;
  //check type of cond - making sure it is a bool
  string currType = "";
  if(auto *AI = dyn_cast<AllocaInst>(cond)) //load RHS
  {
    if(AI->getAllocatedType()->isFloatTy())
    {
      cond = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_temp");
      currType = "float";
    }
    else if(AI->getAllocatedType()->isIntegerTy(32))
    {
      cond = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_temp");
      currType = "int";
    }
    else if(AI->getAllocatedType()->isIntegerTy(1))
    {
      cond = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI, "load_temp");
      currType = "bool";
    }
  }
  else if(auto *GV = dyn_cast<GlobalVariable>(cond)) //if RHS is a global variable
  {
    if(GV->getValueType()->isFloatTy())
    {
      cond = Builder.CreateLoad(Type::getFloatTy(TheContext), GV, "load_temp");
      currType = "float";
    }
    else if(GV->getValueType()->isIntegerTy(32))
    {
      cond = Builder.CreateLoad(Type::getInt32Ty(TheContext), GV, "load_temp");
      currType = "int";
    }
    else if(GV->getValueType()->isIntegerTy(1))
    {
      cond = Builder.CreateLoad(Type::getInt1Ty(TheContext), GV, "load_temp");
      currType = "bool";
    }
  }
  else
  {
    if(cond->getType()->isFloatTy())
      currType = "float";
    else if(cond->getType()->isIntegerTy(32))
      currType = "int";
    else if(cond->getType()->isIntegerTy(1))
      currType = "bool";
  }

  if(currType != "bool") //cast to bool type
  {
    errs()<<"Semantic error: Expected type `bool` for the condition statement at line no. "<<Cond->getTok().lineNo<<" column no. "<<Cond->getTok().columnNo<<". Cannot cast from type `"<<currType<<"` to `bool`.\n";
    return nullptr;
  }

  Value* comp = Builder.CreateICmpNE(cond, ConstantInt::get(TheContext, APInt(1,0, false)), "if_cond");
  Builder.CreateCondBr(comp, true_, false_);
  Builder.SetInsertPoint(true_);
  ///Then block
  std::map<std::string, AllocaInst*> NamedValues_Then; //Creating symbol table for Then block
  NamedValuesList.push_back(NamedValues_Then);

  bool generateBranchForBody = true;

  for(int i = 0; i < Then.size(); i++)
  {
    //new block - create a new symbol table
    Value* thenVal = Then.at(i)->codegen();
    if(thenVal == nullptr)
      return nullptr;
    if(auto *R = dyn_cast<ReturnInst>(thenVal))
      {
        generateBranchForBody = false;
        break; //don't generate IR for instructions after return
      }
  }
  //remove symbol table of Then block
  if(generateBranchForBody)
    Builder.CreateBr(cond_); //if body doesn't contain return statement, make unconditional jump to cond branch
  NamedValuesList.pop_back();

  TheFunction->insert(TheFunction->end(), false_);
  Builder.SetInsertPoint(false_);
  return ConstantPointerNull::get(PointerType::getUnqual(Type::getVoidTy(TheContext))); //return null pointer of void type
 
}

Value* ReturnExprASTnode::codegen(){
  if(ReturnExpr == nullptr)
  {
    return Builder.CreateRetVoid();
  }
  Value* returnExpr = ReturnExpr->codegen();

  if(returnExpr == nullptr)
    return nullptr;

  //create load
  if(auto *AI = dyn_cast<AllocaInst>(returnExpr))
  {
    returnExpr = Builder.CreateLoad(AI->getAllocatedType(),returnExpr,"load_temp");
  }
  else if(auto *GV = dyn_cast<GlobalVariable>(returnExpr))
  {
    returnExpr = Builder.CreateLoad(GV->getValueType(),returnExpr,"load_global_temp");
  }

  string correctType = FuncReturnType;
  string actualType = "";
  if(returnExpr->getType()->isIntegerTy(32))
    actualType = "int";
  else if(returnExpr->getType()->isIntegerTy(1))
    actualType = "bool";
  else if(returnExpr->getType()->isFloatTy())
    actualType = "float";
  
  //if type of return statement not correct, try to do widening conversion, otherwise error
  if(correctType != actualType)
  {
    if(actualType == "float")
    {
      errs()<<"Semantic Error: Incorrect return type `"<<actualType<<"` used in line no: "<<Tok.lineNo<<" column no: "<<Tok.columnNo<<". Cannot cast to expected return type `"<<correctType<<"`.\n";
      return nullptr;
    }
    else if(actualType == "int")
    {
      if(correctType == "float")
      {
        errs()<<"Warning: Incorrect return type `"<<actualType<<"` used in line no: "<<Tok.lineNo<<" column no: "<<Tok.columnNo<<". Casting to expected return type `"<<correctType<<"`.\n";
        returnExpr = Builder.CreateCast(Instruction::SIToFP,returnExpr,Type::getFloatTy(TheContext),"itof_cast");
      }
      else //bool
      {
        errs()<<"Semantic Error: Incorrect return type `"<<actualType<<"` used in line no: "<<Tok.lineNo<<" column no: "<<Tok.columnNo<<". Cannot cast to expected return type `"<<correctType<<"`.\n";
        return nullptr;
      }
    }
    else if(actualType == "bool")
    {
      if(correctType == "float")
      {
        errs()<<"Warning: Incorrect return type `"<<actualType<<"` used in line no: "<<Tok.lineNo<<" column no: "<<Tok.columnNo<<". Casting to expected return type `"<<correctType<<"`.\n";
        returnExpr = Builder.CreateIntCast(returnExpr, Type::getInt32Ty(TheContext), false);
        returnExpr = Builder.CreateCast(Instruction::SIToFP,returnExpr,Type::getFloatTy(TheContext),"btof_cast");
      }
      else //int
      {
        errs()<<"Warning: Incorrect return type `"<<actualType<<"` used in line no: "<<Tok.lineNo<<" column no: "<<Tok.columnNo<<". Casting to expected return type `"<<correctType<<"`.\n";
        returnExpr = Builder.CreateIntCast(returnExpr, Type::getInt32Ty(TheContext), false, "btoi_cast");
      }
    }
  }

  return Builder.CreateRet(returnExpr);
}

Function* PrototypeAST::codegen(){
  // Make the function type:
  string ReturnType = "";
  string origName = getName();
  size_t space_pos = origName.find(" ");    

  //string manipulation to get 'type' and 'name'
  if (space_pos != std::string::npos) 
  {
    ReturnType = origName.substr(0, space_pos);
  }

  origName = origName.substr(space_pos + 1);

  if(ReturnType == "extern") //for extern functions, get 'extern' out from the name
  {
    size_t space_pos_2 = origName.find(" ");    
    if (space_pos_2 != std::string::npos) 
    {
      ReturnType = origName.substr(0, space_pos_2);
      origName = origName.substr(space_pos_2 + 1);
    }
  }

  string ArgType = "";
  vector<Type *> ArgTypes;

  for(int i = 0; i < Args.size(); i++)
  {
      ArgType = Args.at(i)->getType();
      if(ArgType == "int")
        ArgTypes.push_back(Type::getInt32Ty(TheContext));
      else if(ArgType == "float")
        ArgTypes.push_back(Type::getFloatTy(TheContext));
      else if(ArgType == "bool")
        ArgTypes.push_back(Type::getInt1Ty(TheContext));  //bool is Int1 type
      //ignoring void arguments - not allowed to be part of list
  }

  FunctionType *FT;

  if(ReturnType == "int")
  {
    FT = FunctionType::get(Type::getInt32Ty(TheContext), ArgTypes, false);
  }
  else if(ReturnType == "float")
  {
    FT = FunctionType::get(Type::getFloatTy(TheContext), ArgTypes, false);
  }
  else if(ReturnType == "bool")
  {
    FT = FunctionType::get(Type::getInt1Ty(TheContext), ArgTypes, false);
  }
  else if(ReturnType == "void")
  {
    FT = FunctionType::get(Type::getVoidTy(TheContext), ArgTypes, false);
  }

 Function *F = Function::Create(FT, Function::ExternalLinkage, origName, TheModule.get());
 //Set names for all arguments.
 unsigned Idx = 0;
 for (auto &Arg : F->args())
 {
   Arg.setName(getArgName(Idx));
   Idx++;
 }

 return F;
}

Value* GlobalVariableAST::codegen(){
  bool isConstant = false; 
  string ty = getType();
  int alignSize = 4; //set correct alignment
  Type* t = nullptr;

  //get type of global variable
  if(ty == "int")
    t = Type::getInt32Ty(TheContext);
  else if(ty == "float")
    t = Type::getFloatTy(TheContext);
  else if(ty == "bool")
  {
    t = Type::getInt1Ty(TheContext);
    alignSize = 1;
  }
  else
    return nullptr;

  //create new global variable and set name
  GlobalVariable* g = new GlobalVariable(*(TheModule.get()),t,isConstant,GlobalValue::CommonLinkage,Constant::getNullValue(t));
  g->setAlignment(MaybeAlign(alignSize));
  g->setName(Val);

  //add global variable to global symbol table, if it is new
  if(GlobalVariables.insert({Val,g}).second == false)
  {
    string existTy = "";
    if(GlobalVariables[Val]->getValueType()->isIntegerTy(32))
      existTy = "int";
    else if(GlobalVariables[Val]->getValueType()->isIntegerTy(1))
      existTy = "bool";
    else if(GlobalVariables[Val]->getValueType()->isFloatTy())
      existTy = "float";
    
    errs()<<"Semantic error: Redefinition of global variable "<<Val<<" with different type "<<ty<<" at line no. "<<Tok.lineNo<<" column no. "<<Tok.columnNo<<". Variable "<<Val<<" of type "<<existTy<<" already exists.\n";
    return nullptr;
  }
  return g;
}

Function* FunctionAST::codegen(){
  Function *TheFunction = TheModule->getFunction(Proto->getName());

if (!TheFunction)
  TheFunction = Proto->codegen();
if (!TheFunction)
  return nullptr;
 BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
 Builder.SetInsertPoint(BB);
 // Record the function arguments in the NamedValues map.
 std::map<std::string, AllocaInst*> NamedValues;

 for (auto &Arg : TheFunction->args()) //loading each argument in the symbol table
 {
    string type = "";
    if(Arg.getType()->isIntegerTy(32))
      type = "int";
    if(Arg.getType()->isFloatTy())
      type = "float";
    else if(Arg.getType()->isIntegerTy(1))
      type = "bool";
      
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName().str(), type); //creating an alloca for each argument
    Builder.CreateStore(&Arg, Alloca);
    NamedValues[std::string(Arg.getName())] = Alloca;
 }
  
 NamedValuesList.push_back(NamedValues);

 string returnType = "";
 bool returnSet = false;
  if(TheFunction->getReturnType()->isIntegerTy(32))
    returnType = "int"; //    returnType = "int (i32)";
  else if(TheFunction->getReturnType()->isIntegerTy(1))
    returnType = "bool"; // returnType = "bool (i1)";
  else if(TheFunction->getReturnType()->isFloatTy())
    returnType = "float"; //    returnType = "float (float)";
  else if(TheFunction->getReturnType()->isVoidTy())
    returnType = "void";

if(Body.size() == 0)//empty function body
{
  if(!(TheFunction->getReturnType()->isVoidTy()))
    {
      errs()<<"Semantic Error: Return statement of type  "<<returnType<<"  expected in function: "<<Proto->getName()<<".\n";
      return nullptr;
    }
  else
  {
    Builder.CreateRetVoid();
    returnSet = true;
  }
}

for(int i = 0; i < Body.size(); i++)
{
  if(returnSet) //do not generate further instructions after the return statement
    break;

  Value *RetVal = Body.at(i)->codegen(); //go through all ASTnodes in this function body and run codegen() for each ASTnode
  if(!RetVal)
  {
    return nullptr;
  }
  
  if((i == Body.size()-1) | isa<ReturnInst>(RetVal)) //check last ASTnode of function body or check for a return expression that has been made in the body early
  {
      if(auto RT = dyn_cast<ReturnInst>(RetVal)) //if last statement is a return statement
      {
        returnSet = true; //make sure last line is a return stmt
      }
      else if((i == Body.size()-1) & (returnType == "void"))
      {
        Builder.CreateRetVoid();
        returnSet = true;
      }
      else //return statement not found
      {
        errs()<<"Semantic Error: Return statement of type `"<<returnType<<"` expected in function: "<<Proto->getName()<<".\n";
        return nullptr;
      }
  }
}

// Validate the generated code, checking
//for consistency.
 verifyFunction(*TheFunction);

 NamedValuesList.pop_back(); //remove NamedValues of this function from the vector

 return TheFunction;
}

//===----------------------------------------------------------------------===//
// AST Printer
//===----------------------------------------------------------------------===//

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const unique_ptr<TopLevelASTnode> &ast) { 
  os << ast->to_string(); 
  return os;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char **argv) {
  if (argc == 2) {
    pFile = fopen(argv[1], "r");
    if (pFile == NULL)
      perror("Error opening file");
  } else {
    std::cout << "Usage: ./code InputFile\n";
    return 1;
  }

  // initialize line number and column numbers to zero
  lineNo = 1;
  columnNo = 1;

  //get the first token
  getNextToken();
  //start lexical analysis - identify any invalid tokens before starting the parser
  while (CurTok.type != EOF_TOK) {
    if(CurTok.type == INVALID)
    {
      errs()<<"Lexical error: Invalid token "<<CurTok.lexeme<<" found at line no. "<<CurTok.lineNo<<" column no. "<<CurTok.columnNo<<".\n"; //print error and exit if invalid token is found
      return 1;
    }
    //print each token
    //fprintf(stderr, "Token: %s with type %d\n", CurTok.lexeme.c_str(),CurTok.type);
    getNextToken();
  }
  fprintf(stderr, "Lexer Finished.\n");
  clearTokBuffer(); //clear token buffer before re-reading file and starting parsing

  // Make the module, which holds all the code.
  TheModule = std::make_unique<Module>("mini-c", TheContext);

  //read file from beginning
  fseek(pFile,0,SEEK_SET);

  lineNo = 1;
  columnNo = 1;

  //skip EOF
  getNextToken();
  // Run the parser now.
  if(!parser())
  {
    cout<<"Parsing failed."<<endl;
    return 1;
  }
  //fprintf(stderr, "Parsing Finished\n");

  //Printing out AST
  llvm::outs() << "\nPrinting out AST:"<< "\n\n";
  llvm::outs() << "root"<< "\n|\n";
  for(int i = 0; i < root.size(); i++)
  {
    ///IR Code Generator - this operates while traversing AST nodes
    if(root[i]->codegen() == nullptr)
    {
      errs()<<"IR code generation failed.\n";
      return 1; 
    }

    if(i == root.size() - 1)
    {
      llvm::outs() << "|-> " << root[i] << "\n";
    }
    else
    {
      llvm::outs() << "|-> " << root[i]<< "\n|\n";
    }
  }

  llvm::outs() << "\nAST successfully printed."<< "\n\n";
  llvm::outs() << "IR code generation successful."<< "\n";

  //********************* Start printing final IR **************************
  // Print out all of the generated code into a file called output.ll
  auto Filename = "output.ll";
  std::error_code EC;
  raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

  if (EC) {
    errs() << "Could not open file: " << EC.message();
    return 1;
  }

  // TheModule->print(errs(), nullptr); // print IR to terminal
  TheModule->print(dest, nullptr);
  //********************* End printing final IR ****************************

  fclose(pFile); // close the file that contains the code that was parsed
  return 0;
}