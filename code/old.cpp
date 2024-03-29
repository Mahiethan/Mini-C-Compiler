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

  //Changing this to check for invalid tokens, instead of passing it as ascii values:

  // // Otherwise, just return the character as its ascii value.
  // int ThisChar = LastChar;
  // std::string s(1, ThisChar);
  // LastChar = getc(pFile);
  // columnNo++;
  // return returnTok(s, int(ThisChar));

  //for ascii values - +, -, *, %
  if(LastChar == '-')
  {
    //cout<<"Minus "<<char(LastChar)<<endl;
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("-", MINUS);
  }
  else if(LastChar == '+')
  {
    //cout<<"Plus "<<char(LastChar)<<endl;
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("+", PLUS);
  }
  else if(LastChar == '*')
  {
    //cout<<"Asterix "<<char(LastChar)<<endl;
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("*", ASTERIX);
  }
  else if(LastChar == '%')
  {
    //cout<<"Mod "<<char(LastChar)<<endl;
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("%", MOD);
  }

  //otherwise, pass current symbol as an invalid token
  int ThisChar = LastChar;
  //cout<<"Invalid token "<<char(ThisChar)<<endl;
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

static void putBackToken(TOKEN tok) { tok_buffer.push_front(tok); }

static void clearTokBuffer() { 
  while(tok_buffer.size() != 0) 
  {
    tok_buffer.pop_front();
  } 
}

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//
static int indentLevel = 0;
void increaseIndentLevel(){indentLevel++;}
void decreaseIndentLevel(){
  indentLevel = (indentLevel > 0) ? --indentLevel : indentLevel;
}
string addIndent()
{
  increaseIndentLevel();
  return std::string(indentLevel,' ');
}

/// ASTnode - Base class for all AST nodes.
class ASTnode {
public:
  virtual ~ASTnode() {}
  //virtual Value *codegen() = 0;
  virtual std::string to_string() const {return "";};
};

/// IntASTnode - Class for integer literals like 1, 2, 10,
class IntASTnode : public ASTnode {
  int Val;
  TOKEN Tok;
  std::string Name;

public:
  IntASTnode(TOKEN tok, int val) : Val(val), Tok(tok) {}
  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final = "\n    " + addIndent() + "|-> IntegerLiteral: " + std::to_string(Val);
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
  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final = "\n    " + addIndent() + "|-> FloatLiteral: " + std::to_string(Val);
    decreaseIndentLevel();
    return final;
  };
};

/// FloatASTnode - Class for float literals like 1.0, 2.5, 10.23,
class BoolASTnode : public ASTnode {
  bool Val;
  TOKEN Tok;
  std::string Name;

public:
  BoolASTnode(TOKEN tok, bool val) : Val(val), Tok(tok) {}
  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string boolVal = Val == true ? "true" : "false";
    string final =  "\n    " + addIndent() + "|-> BoolLit: " + boolVal;
    decreaseIndentLevel();
    return final;
  };
};


/// VariableASTnode - Class for referencing variables, such as "a"
class VariableASTnode : public ASTnode{
  TOKEN Tok;
  string Val;
  string Type;

  public:
  VariableASTnode(TOKEN tok, string type, string val) : Type(type), Val(val), Tok(tok) {}
  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a sting representation of this AST node
    string final =  "\n    " + addIndent() + "|-> VarDecl: " + Type + " " + Val; 
    decreaseIndentLevel();
    return final;
    // return "a";
  };
};

/// VariableASTnode - Class for referenced variables
class VariableReferenceASTnode : public ASTnode{
  TOKEN Tok;
  string Name;

  public:
  VariableReferenceASTnode(TOKEN tok, string name) : Name(name), Tok(tok) {}
  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a sting representation of this AST node
    string final = "\n    " + addIndent() + "|-> VarRef: " + Name;
    decreaseIndentLevel();
    return final;
    // return "a";
  };
};

/// UnaryExprASTnode - Expression class for a unary operator, like ! or - (check production `rval_two`)
class UnaryExprASTnode : public ASTnode {
  string Opcode;
  std::unique_ptr<ASTnode> Operand;

public:
  UnaryExprASTnode(string Opcode, std::unique_ptr<ASTnode> Operand)
      : Opcode(Opcode), Operand(std::move(Operand)) {}

  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final =  "\n    " + addIndent() + "|-> UnaryExpr: " + Opcode  + Operand->to_string();
    decreaseIndentLevel();
    return final;
  };
};

/// BinaryExprASTnode - Expression class for a binary operator.
class BinaryExprASTnode : public ASTnode {
  string Opcode;
  std::unique_ptr<ASTnode> LHS, RHS;

public:
  BinaryExprASTnode(string Opcode, std::unique_ptr<ASTnode> LHS,
                std::unique_ptr<ASTnode> RHS)
      : Opcode(Opcode), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string final = "\n    " + addIndent() + "|-> BinaryExpr: " + Opcode + LHS->to_string() + RHS->to_string();
    decreaseIndentLevel();
    return final;
  };
};

/// FuncCallASTnode - Expression class for function calls, including any arguments.
class FuncCallASTnode : public ASTnode {
  std::string Callee;
  std::vector<std::unique_ptr<ASTnode>> Args;

public:
  FuncCallASTnode(const std::string &Callee,
              std::vector<std::unique_ptr<ASTnode>> Args)
      : Callee(Callee), Args(std::move(Args)) {}

  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string args = "";
    for(int i = 0; i < Args.size(); i++)
    {
      // if(i == Args.size() - 1)
      args.append(addIndent() + " |-> Param" + Args[i]->to_string());
      decreaseIndentLevel();
      // args.append(" |-> Param" + Args[i]->to_string());
    // else
    //   args.append(addIndent() + " |-> Param" + Args[i]->to_string() + "\n    ");
    }

    if(args.length() != 0)
      args = "\n    " + args;

    string final = "\n    " + addIndent() + "|-> FunctionCall: " + Callee + args;
    decreaseIndentLevel();
    return final;
  };
};

/// IfExprASTnode - Expression class for if/then/else.
class IfExprASTnode : public ASTnode {
  std::unique_ptr<ASTnode> Cond;
  std::vector<std::unique_ptr<ASTnode>> Then, Else;

public:
  IfExprASTnode(std::unique_ptr<ASTnode> Cond, std::vector<std::unique_ptr<ASTnode>> Then,
            std::vector<std::unique_ptr<ASTnode>> Else)
      : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

   // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
   string ThenStr = "";
   string ElseStr = "";
  if(Else.size() != 0)
   {
      ElseStr = "\n    " + addIndent() + "|-> ElseExpr";
      decreaseIndentLevel();
   }
   string final = "\n    " + addIndent() + "|-> IfExpr:" + Cond->to_string();
  for(int i = 0; i < Then.size(); i++)
  {
    ThenStr.append(Then[i]->to_string()); //SORT THIS IDENT OUT
  } 
  // cout<<"Size of Else:"<<Else.size()<<endl;
  // cout<<"Size of Then:"<<Then.size()<<endl;
  final.append(ThenStr);
  for(int j = 0; j < Else.size(); j++)
  {
    ElseStr.append(Else[j]->to_string()); //CAUSING SEG FAULT!!!!!!!!!!!!!!!
  } 

  final.append(ElseStr);
  decreaseIndentLevel();
  return final;
  };
};

//Link to ADTs 
//https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl07.html

/// WhileExprASTnode - Expression class for while/do
class WhileExprASTnode : public ASTnode {
  std::unique_ptr<ASTnode> Cond;
  std::vector<std::unique_ptr<ASTnode>> Then;

public:
  WhileExprASTnode(std::unique_ptr<ASTnode> cond, std::vector<std::unique_ptr<ASTnode>> then)
      : Cond(std::move(cond)), Then(std::move(then)) {}

  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string ThenStr = "";
    string final = "\n    " + addIndent() + "|-> WhileExpr:" + Cond->to_string();
    for(int i = 0; i < Then.size(); i++)
    {
      ThenStr.append(Then[i]->to_string()); //SORT THIS IDENT OUT
    }
    final.append(ThenStr);
    decreaseIndentLevel();
    return final;
  };
};

/// ReturnExprASTnode - Expression class for return statements
class ReturnExprASTnode : public ASTnode {
  std::unique_ptr<ASTnode> ReturnExpr;

public:
  ReturnExprASTnode(std::unique_ptr<ASTnode> returnExpr)
      : ReturnExpr(std::move(returnExpr)) {}

  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    return "\n    " + addIndent() + "|-> ReturnStmt" + ReturnExpr->to_string();
  };
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : public ASTnode {
  // string returnType;
  std::string Name;
  std::vector<unique_ptr<VariableASTnode>> Args;

public:
  PrototypeAST(std::string &Name, std::vector<unique_ptr<VariableASTnode>> Args)
      : Name(Name), Args(std::move(Args)) {}

   const std::string &getName() const { return Name; }

  // virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a sting representation of this AST node
  string name = getName();
  string args = "";
  //cout<<Args.size()<<endl;
  if(Args.size() > 0)
  {
    args.append("\n    " + addIndent() + "|-> Parameters:");
  }
  for(int i = 0; i < Args.size(); i++)
  {
    //cout<<Args[i]->to_string()<<endl;
    args.append(Args[i]->to_string());
  }

 if(Args.size() > 0)
  {
    decreaseIndentLevel();
  }
 
  return name + args;
  };
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public ASTnode {
  std::unique_ptr<PrototypeAST> Proto;
  std::vector<std::unique_ptr<ASTnode>> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto, //can have no prototypes (just block of expressions e.g. global variables)
              std::vector<std::unique_ptr<ASTnode>> Body) //Body can contain multiple expressions
      : Proto(std::move(Proto)), Body(std::move(Body)) {}

    

    // virtual Value *codegen() override;
    virtual std::string to_string() const override{
  //return a sting representation of this AST node
      string proto = Proto->to_string();
      string final =  "|-> FunctionDecl: " + proto;
      string body = "";
      if(Body.size() != 0)
      {
        body = "\n    " + addIndent() + "|-> Function Body:";
        // decreaseIndentLevel();
      }
      for(int i = 0; i < Body.size(); i++)
      {
        body.append(Body[i]->to_string());
      }

      final.append(body);
      indentLevel = 0;
      return final;
  };
};

static vector<unique_ptr<ASTnode>> root; //root of the AST (TranslationUnitDecl)

//temporary vector/string stores

//null TOKEN
TOKEN nullToken = {};

static string prototypeName = "";
static unique_ptr<VariableASTnode> argument = std::make_unique<VariableASTnode>(CurTok,"","");
static string vartype = "";
static string functiontype = "";
static vector<unique_ptr<VariableASTnode>> argumentList = {};
static vector<unique_ptr<ASTnode>> body = {};
static deque<pair<string,unique_ptr<ASTnode>>> stmtList;
static TOKEN functionIdent = nullToken;
static TOKEN variableIdent = nullToken;
//static vector<string> varNames = {}; //can have more than one variable names IDENT = IDENT = ...
//static unique_ptr<BinaryExprASTnode> assignment = std::make_unique<BinaryExprASTnode>('=', nullptr, nullptr);

static vector<TOKEN> expression = {};
string unary;

void resetPrototypeName()
{
  prototypeName = "";
}

// void resetAssignment()
// {
//   assignment = std::make_unique<BinaryExprASTnode>('=', nullptr, nullptr);
// }

// void resetVarName()
// {
//   varNames = {};
// }

void resetArgument()
{
  argument = std::make_unique<VariableASTnode>(nullToken,"","");
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

void resetUnaryExpr()
{
  unary = "";
}

// void printExpression()
// {
//   for(int i = 0; i < expression.size(); i++)
//     cout<<expression[i];
//   cout<<"\n";
// }



//===----------------------------------------------------------------------===//
// Recursive Descent Parser - Function call for each production
//===----------------------------------------------------------------------===//

/* FIRST() sets*/
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

/* FOLLOW() sets*/
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

static bool contains(int type, vector<TOKEN_TYPE> list)
{
  for(int i = 0; i < list.size(); i++)
  {
    if(list[i] == type)
      return true;
  }
  return false;
}

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

/* Add function calls for each production */

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

void addFunctionAST()
{
  //define functionAST and add to root
  prototypeName.append(functiontype + " " + functionIdent.lexeme);
  resetFunctionIdent();
  resetFunctiontype();
  unique_ptr<PrototypeAST> Proto = std::make_unique<PrototypeAST>(prototypeName,std::move(argumentList));
  resetArgumentList();
  resetPrototypeName();
  unique_ptr<FunctionAST> Func = std::make_unique<FunctionAST>(std::move(Proto),std::move(body));
  resetBody();
  root.push_back(std::move(Func));
}

void printStmtList()
{
  for(int i = 0; i < stmtList.size(); i++)
  {
    cout<<stmtList.at(i).first<<endl;
  }
}

unique_ptr<ASTnode> processStmtList()
{
  auto curr = std::move(stmtList.front());
  stmtList.pop_front();
  cout<<"Curr: "<<curr.first<<endl;
  while((curr.first == "new" | curr.first == "else") & (stmtList.size() > 0)) //ignore "new" and "else"
  {
    cout<<"Discard: "<<curr.first<<endl;
    curr = std::move(stmtList.front());
    cout<<"NewCurr: "<<curr.first<<endl;
    stmtList.pop_front();
  }
  cout<<"FinalCurr: "<<curr.first<<endl;
  if(curr.first == "vardecl")
  {
    return std::move(curr.second);
  }
  else if(curr.first == "expr")
  {
    return std::move(curr.second);
  }
  else if(curr.first == "while")
  {
    unique_ptr<ASTnode> cond = processStmtList();
    vector<unique_ptr<ASTnode>> then = {};
    while(stmtList.front().first != "new")
    {
      then.push_back(std::move(processStmtList()));
    }
    cout<<"exit while"<<endl;
    return make_unique<WhileExprASTnode>(std::move(cond),std::move(then));
  }
  else if(curr.first == "if")
  {
    unique_ptr<ASTnode> cond = processStmtList();
    vector<unique_ptr<ASTnode>> Then = {};
    vector<unique_ptr<ASTnode>> Else = {};
    while((stmtList.front().first != "new") & (stmtList.front().first != "else"))
    {
      Then.push_back(std::move(processStmtList()));
    }
    cout<<"exit then"<<endl;
    if(stmtList.front().first == "else")
    {
      while((stmtList.front().first != "new"))
      {
        cout<<stmtList.front().first<<endl;
        Else.push_back(std::move(processStmtList()));
        //cout<<Else.size()<<endl;
      }
      cout<<"exit else"<<endl;
    }    

    return make_unique<IfExprASTnode>(std::move(cond),std::move(Then),std::move(Else));
  }
  else if(curr.first == "return")
  {
    // return std::move(curr.second);
    unique_ptr<ASTnode> returnExpr = processStmtList();
    return make_unique<ReturnExprASTnode>(std::move(returnExpr));
  }
  else return std::move(nullptr);
} 

void addToBody()
{
  while(stmtList.size() != 0)
  {
        printStmtList();

    unique_ptr<ASTnode> ptr = std::move(processStmtList());
    if(ptr != nullptr)
      body.push_back(std::move(ptr));
    cout<<stmtList.size()<<endl;
  }
}

//static int expr_index = 0;


// string peekNextExprToken(vector<string> )
// {
//   string peek = expression.at(expr_index);
//   // if(expr_index < expression.size())
//   //   expr_index++;
//   return peek;
// }

int getPrecedence(string op)
{
  if(op == "*" | op == "/" | op == "%")
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
  else if(op == "=")
    return 10;
  else
    return 110; //invalid
}

unique_ptr<ASTnode> createExprASTnode(vector<TOKEN> expression)
{
  if(expression.size() == 1) //literals
  {
    TOKEN t = expression.at(0);
    if(t.type == INT_LIT)
    {
      return std::move(make_unique<IntASTnode>(t,stoi(t.lexeme)));
    }
    else if(t.type == FLOAT_LIT)
    {
      return std::move(make_unique<FloatASTnode>(t,stof(t.lexeme)));
    }
    else if(t.type == BOOL_LIT)
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
  else if((expression.at(0).lexeme == "-" | expression.at(0).lexeme == "!")) //unary
  {
    cout<<"unary"<<endl;
    string opcode = expression.at(0).lexeme;
    vector<TOKEN> operand = {};
    for(int i = 1; i < expression.size(); i++)
      operand.push_back(expression.at(i));

    return std::move(make_unique<UnaryExprASTnode>(opcode,std::move(createExprASTnode(operand))));
  }
  else if(expression.at(0).lexeme == "(" & expression.at(expression.size()-1).lexeme == ")") //bracketed expr
  {
    cout<<"bracketed expr"<<endl;
    vector<TOKEN> newExpr = {};
    for(int i = 1; i < expression.size() - 1; i++)
    {
      newExpr.push_back(expression.at(i));
    } 
    return std::move(createExprASTnode(newExpr));
  }
  else if((expression.at(0).type == IDENT) & (expression.at(1).type == LPAR)) //function call with or without arguments
  {
    string callee = expression.at(0).lexeme;
    // if(expression.size() > 1)
    // {
      cout<<callee<<endl;
      // cout<<expression.at(0).lexeme<<endl;
      // cout<<expression.at(1).lexeme<<endl;
      // cout<<expression.at(2).lexeme<<endl;
      vector<unique_ptr<ASTnode>> args = {};
      vector<TOKEN> expr = {};
      bool start = false;
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
            cout<<expression.at(i).lexeme<<endl;
          }
        }
      }
      return std::move(make_unique<FuncCallASTnode>(callee,std::move(args)));
    // }
    // else //no args
    // {
    //   return std::move(make_unique<FuncCallASTnode>(callee,std::move(args)));
    // }
  }
  else //TESTT
  {
    int minPrecedence = 100;
    string op = "";
    int index = 0;
    bool isOp = false;
    bool valid = true; //operators inside bracketed expressions are invalid
    for(int i = 0; i < expression.size(); i++)
    {
      int currPrecedence = getPrecedence(expression.at(i).lexeme);
      if(expression.at(i).lexeme == "(") 
      {
        valid = false;
      }
      if(expression.at(i).lexeme == ")")
      {
        valid = true;
      }
      if(currPrecedence != 110) //operator found
      {
        if((currPrecedence <= minPrecedence) & (isOp == false) & (valid == true)) //get lowest precedence operator, avoiding any unary operators
        { 
          op = expression.at(i).lexeme;
          // cout<<op<<endl;
          minPrecedence = currPrecedence;
          //cout<<minPrecedence<<endl;
          index = i;
        }
        isOp = true;
      }
      else
      {
        isOp = false;
      }
      
    }

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

    cout<<op<<endl;
    return std::move(make_unique<BinaryExprASTnode>(op, std::move(createExprASTnode(lhs)), std::move(createExprASTnode(rhs)))); //recursive
  }
  return nullptr; //error
}

bool p_arg_list_prime()
{
  if(CurTok.type == COMMA)
  {
    //return match(COMMA) & p_arg_list();
    TOKEN temp = CurTok;
    if(!match(COMMA))
      return false;
    
    expression.push_back(temp);

    return p_arg_list();
  }
  else
  {
    if(contains(CurTok.type, FOLLOW_arg_list_prime))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool p_arg_list()
{
  return p_expr() & p_arg_list_prime();
}

bool p_args()
{
  if(contains(CurTok.type,FIRST_arg_list))
    return p_arg_list();
  else
  {
    if(contains(CurTok.type,FOLLOW_args))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool p_rval()
{
  cout<<"rval"<<endl;
  if(CurTok.type == LPAR)
  {
    //return match(LPAR) & p_args() & match(RPAR);
    TOKEN temp = CurTok;
    if(!match(LPAR))
      return false;
    
    expression.push_back(temp);

    if(!p_args())
      return false;

    temp = CurTok;
    if(!match(RPAR))
      return false;

    expression.push_back(temp);

    return true;
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool p_rval_one()
{
  cout<<"rval_one"<<endl;
  // cout<<CurTok.type<<endl;
  if(CurTok.type == LPAR)
  {
    //return match(LPAR) & p_expr() & match(RPAR);
    TOKEN temp = CurTok;
    if(!match(LPAR))
      return false;
    
    expression.push_back(temp);

    if(!p_expr())
      return false;
    
    temp = CurTok;
    if(!match(RPAR))
      return false;
    
    expression.push_back(temp);
    return true;
  }
  else if(CurTok.type == IDENT)
  {
    // return match(IDENT) & p_rval();
    variableIdent = CurTok;
    if(!match(IDENT))
      return false;
    
    expression.push_back(variableIdent);
    resetVariableToken();

    return p_rval();
  }
  else if(CurTok.type == INT_LIT)
  {
    // return match(INT_LIT);
    variableIdent = CurTok;
    if(!match(INT_LIT))
      return false;

    expression.push_back(variableIdent);
    resetVariableToken();

    return true;
  }
  else if(CurTok.type == FLOAT_LIT)
  {
    // return match(FLOAT_LIT);
    variableIdent = CurTok;
    if(!match(FLOAT_LIT))
      return false;

    expression.push_back(variableIdent);
    resetVariableToken();

    return true;
  }
  else if(CurTok.type == BOOL_LIT)
  {
    // return match(BOOL_LIT);
    variableIdent = CurTok;
    if(!match(BOOL_LIT))
      return false;

    expression.push_back(variableIdent);
    resetVariableToken();

    return true;
  }
  else
  {
    return false;
  }
}


bool p_rval_two()
{
  cout<<"rval_two"<<endl;
  if(CurTok.type == MINUS)
  {
    //return match(MINUS) & p_rval_two();
    TOKEN temp = CurTok;
    if(!match(MINUS))
      return false;

    //unary.append("-");
    expression.push_back(temp);
    return p_rval_two();
  }
  else if(CurTok.type == NOT)
  {
    //return match(NOT) & p_rval_two();
    TOKEN temp = CurTok;
    if(!match(NOT))
      return false;

    //unary.append("!");
    expression.push_back(temp);
    return p_rval_two();
  }
  else if(contains(CurTok.type,FIRST_rval_one))
  {
    return p_rval_one();
  }
  else
  {
    return false;
  }
}

bool p_rval_three()
{
  cout<<"rval_three"<<endl;
  return p_rval_two() & p_rval_three_prime();
}

bool p_rval_three_prime()
{
  cout<<"rval_three'"<<endl;
  if(CurTok.type == ASTERIX)
  {
    // return match(ASTERIX) & p_rval_two() & p_rval_three_prime();
    TOKEN temp = CurTok;
    if(!match(ASTERIX))
      return false;
    
    expression.push_back(temp);
    return p_rval_two() & p_rval_three_prime();
  }
  else if(CurTok.type == DIV)
  {
    TOKEN temp = CurTok;
    if(!match(DIV))
      return false;
    
    expression.push_back(temp);
    return p_rval_two() & p_rval_three_prime();
    // return match(DIV) & p_rval_two() & p_rval_three_prime();
  }
  else if(CurTok.type == MOD)
  {
    TOKEN temp = CurTok;
    if(!match(MOD))
      return false;
    
    expression.push_back(temp);
    return p_rval_two() & p_rval_three_prime();
    // return match(MOD) & p_rval_two() & p_rval_three_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_three_prime))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
  }
}

bool p_rval_four()
{
  cout<<"rval_four"<<endl;
  return p_rval_three() & p_rval_four_prime();
}

bool p_rval_four_prime()
{
  cout<<"rval_four'"<<endl;
  // cout<<CurTok.type<<endl;
  if(CurTok.type == PLUS)
  {
    //return match(PLUS) & p_rval_three() & p_rval_four_prime();
    TOKEN temp = CurTok;
    if(!match(PLUS))
      return false;
    
    expression.push_back(temp);

    return p_rval_three() & p_rval_four_prime();
  }
  else if(CurTok.type == MINUS)
  {
    //return match(MINUS) & p_rval_three() & p_rval_four_prime();
    TOKEN temp = CurTok;
    if(!match(MINUS))
      return false;
    
    expression.push_back(temp);

    return p_rval_three() & p_rval_four_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_four_prime))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
  }
}

bool p_rval_five()
{
  cout<<"rval_five"<<endl;
  return p_rval_four() & p_rval_five_prime();
}

bool p_rval_five_prime()
{
  cout<<"rval_five'"<<endl;
  if(CurTok.type == LE)
  {
    // return match(LE) & p_rval_four() & p_rval_five_prime();
    TOKEN temp = CurTok;
    if(!match(LE))
      return false;

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == LT)
  {
    //return match(LT) & p_rval_four() & p_rval_five_prime();
    TOKEN temp = CurTok;
    if(!match(LT))
      return false;

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == GE)
  {
    //return match(GE) & p_rval_four() & p_rval_five_prime();
    TOKEN temp = CurTok;
    if(!match(GE))
      return false;

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == GT)
  {
    //return match(GT) & p_rval_four() & p_rval_five_prime();
    TOKEN temp = CurTok;
    if(!match(GT))
      return false;

    expression.push_back(temp);
    return p_rval_four() & p_rval_five_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_five_prime))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
  }
}

bool p_rval_six()
{
  cout<<"rval_six"<<endl;
  return p_rval_five() & p_rval_six_prime();
}

bool p_rval_six_prime()
{
  cout<<"rval_six'"<<endl;
  if(CurTok.type == EQ)
  {
    //return match(EQ) & p_rval_five() & p_rval_six_prime();
    TOKEN temp = CurTok;
    if(!match(EQ))
      return false;

    expression.push_back(temp);

    return p_rval_five() & p_rval_six_prime();
  }
  else if(CurTok.type == NE)
  {
    //return match(NE) & p_rval_five() & p_rval_six_prime();
    TOKEN temp = CurTok;
    if(!match(NE))
      return false;

    expression.push_back(temp);

    return p_rval_five() & p_rval_six_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_six_prime))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
  }
}

bool p_rval_seven()
{
  cout<<"rval_seven"<<endl;
  return p_rval_six() & p_rval_seven_prime();
}

bool p_rval_seven_prime()
{
  cout<<"rval_seven'"<<endl;
  if(CurTok.type == AND)
  {
    //return match(AND) & p_rval_six() & p_rval_seven_prime();
    TOKEN temp = CurTok;
    if(!match(AND))
      return false;

    expression.push_back(temp);

    return p_rval_six() & p_rval_seven_prime();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_seven_prime))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
  }
}

bool p_rval_eight()
{
  cout<<"rval_eight"<<endl;
  return p_rval_seven() & p_rval_eight_prime();
}

bool p_rval_eight_prime()
{
  cout<<"rval_eight'"<<endl;
  if(CurTok.type == OR)
  {
    //return match(OR) & p_rval_seven() & p_rval_eight_prime();
    TOKEN temp = CurTok;
    if(!match(OR))
      return false;

    expression.push_back(temp);

    if(!p_rval_seven())
      return false;

    if(!p_rval_eight_prime())
      return false;

    return true;
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_rval_eight_prime))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
  }
}

bool p_return_stmt_prime()
{
  cout<<"return_stmt'"<<endl;
  if(CurTok.type == SC)
  {
    return match(SC);
  }
  else if(contains(CurTok.type,FIRST_expr))
  {
    // return p_expr() & match(SC);
    if(!p_expr())
      return false;
    
    unique_ptr<ASTnode> expr = createExprASTnode(expression);
    pair <string,unique_ptr<ASTnode>> p = make_pair("expr",std::move(expr));
    stmtList.push_back(std::move(p));
    resetExpression();

    return match(SC);
  }
  else
  {
    return false;
  }
}

bool p_return_stmt()
{
  cout<<"return_stmt"<<endl;
  //return match(RETURN) & p_return_stmt_prime();
  if(!match(RETURN))
    return false;
  
  pair <string,unique_ptr<ASTnode>> p = make_pair("return",std::move(nullptr));
  stmtList.push_back(std::move(p));

  return p_return_stmt_prime();
}

bool p_expr()
{
  cout<<"expr"<<endl;
  //return p_exprStart() & p_rval_eight();
  if(!p_exprStart())
    return false;

  if(!p_rval_eight())
    return false;

  //print out expression
  // printExpression();

  //addExpressionToAST()

  //body.push_back();
  return true;
}

bool p_exprStart()
{
  cout<<"exprStart"<<endl;
  cout<<CurTok.type<<endl;
  TOKEN firstLookAhead = CurTok;
  cout<<firstLookAhead.type<<endl;
  getNextToken();
  cout<<CurTok.type<<endl;
  if(firstLookAhead.type == IDENT & CurTok.type == ASSIGN)
  {
      putBackToken(CurTok);
      CurTok = firstLookAhead;
      //cout<<CurTok.type<<endl;

      variableIdent = CurTok;
      if(!match(IDENT))
        return false;
      
      TOKEN temp = CurTok;
      if(!match(ASSIGN))
        return false;

      expression.push_back(variableIdent);
      expression.push_back(temp);
      resetVariableToken();

      return p_exprStart();

      //https://en.wikipedia.org/wiki/Operator-precedence_parser
      //create vector that contains expression
      //pass it onto new function which parses it and apply precedence when creating AST
      //checkwith clang -cc1 -ast-dump sampleTests/testOne.c

      //return match(IDENT) & match(ASSIGN) & p_exprStart();
    
  }
  else
  {
    putBackToken(CurTok);
      CurTok = firstLookAhead;
      cout<<CurTok.type<<endl;

    if(contains(CurTok.type,FOLLOW_exprStart))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool p_else_stmt()
{
  cout<<"else_stmt"<<endl;
  if(CurTok.type == ELSE)
  {
    // return match(ELSE) & p_block();
    if(!match(ELSE))
      return false;

    pair <string,unique_ptr<ASTnode>> p = make_pair("else",std::move(nullptr));
    stmtList.push_back(std::move(p));
    
    if(!p_block())
      return false;

    return true;

  }
  else
  {
    if(contains(CurTok.type,FOLLOW_else_stmt))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
    {
      return false;
    }
  }
}


bool p_if_stmt()
{
  cout<<"if_stmt"<<endl;
  // return match(IF) & match(LPAR) & p_expr() & match(RPAR) & p_block() & p_else_stmt();
  if(!(match(IF) & match(LPAR) & p_expr() & match(RPAR)))
    return false;
  
  pair <string,unique_ptr<ASTnode>> p = make_pair("if",std::move(nullptr));
  stmtList.push_back(std::move(p));
  unique_ptr<ASTnode> expr = createExprASTnode(expression);
  pair <string,unique_ptr<ASTnode>> e = make_pair("expr",std::move(expr));
  stmtList.push_back(std::move(e));
  resetExpression();

  if(!p_block())
    return false;
  
  if(!p_else_stmt())
    return false;
  
  pair <string,unique_ptr<ASTnode>> n = make_pair("new",std::move(nullptr));
  stmtList.push_back(std::move(n));

  return true;
}

bool p_while_stmt()
{
  // return match(WHILE) & match(LPAR) & p_expr() & match(RPAR) & p_stmt();
  if((match(WHILE) & match(LPAR) & p_expr() & match(RPAR)) == false)
    return false;
  
  pair <string,unique_ptr<ASTnode>> p = make_pair("while",std::move(nullptr));
  stmtList.push_back(std::move(p));
  unique_ptr<ASTnode> expr = createExprASTnode(expression);
  pair <string,unique_ptr<ASTnode>> e = make_pair("expr",std::move(expr));
  stmtList.push_back(std::move(e));
  resetExpression();


  if(!p_stmt())
    return false;
  
  pair <string,unique_ptr<ASTnode>> n = make_pair("new",std::move(nullptr));
  stmtList.push_back(std::move(n));
  return true;

}

bool p_expr_stmt()
{
  cout<<"expr_stmt"<<endl;
  if(contains(CurTok.type, FIRST_expr))
  {
    if(!p_expr())
      return false;
    if(!match(SC))
      return false;
    // printExpression();

    unique_ptr<ASTnode> expr = createExprASTnode(expression);
    pair <string,unique_ptr<ASTnode>> p = make_pair("expr",std::move(expr));
    stmtList.push_back(std::move(p));
    resetExpression();

    return true;
  }
  else if(CurTok.type == SC)
  {
    return match(SC);
  }
  else
  {
    errs()<<"Syntax error: Token "<<CurTok.lexeme<<" invalid. \n";
    return false;
  }
}

bool p_stmt()
{
  cout<<"stmt"<<endl;
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
    return false;
  }
}

bool p_stmt_list()
{
  cout<<"stmt_list"<<endl;
   cout<<CurTok.type<<endl;
  if(contains(CurTok.type, FIRST_stmt_list))
  {
    if(!p_stmt())
      return false;
    
    return p_stmt_list();
  }
  else
  {
    if(contains(CurTok.type, FOLLOW_stmt_list))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
  }
}

bool p_local_decl()
{
  cout<<"local_decl"<<endl;
  if(!p_var_type())
  {
    return false;
  }

  variableIdent = CurTok;
  if(!match(IDENT))
  {
    return false;
  }

  if(!match(SC))
  {
    return false;
  }

  unique_ptr<VariableASTnode> var = std::make_unique<VariableASTnode>(variableIdent, vartype, variableIdent.lexeme);
  pair <string,unique_ptr<ASTnode>> p = make_pair("vardecl",std::move(var));
  stmtList.push_back(std::move(p));
  resetVariableToken();
  resetVartype();

  return true;
  //return p_var_type() & match(IDENT) & match(SC);
}

bool p_local_decls()
{
   cout<<"local_decls"<<endl;
   if(contains(CurTok.type, FIRST_local_decl))
   {
      if(!p_local_decl())
      {
        return false;
      }

      //added to body

      // if(!p_local_decls()) //recursive
      // {
      //   return false;
      // }
      

      return p_local_decls();

      //return p_local_decl() & p_local_decls();
   }
   else
   {
    if(contains(CurTok.type, FOLLOW_local_decls))
    {
      cout<<"eat"<<endl;
      return true;
    }
    else
      return false;
   }
}

bool p_param()
{
  cout<<"param"<<endl;
  //return p_var_type() & match(IDENT);
  if(!p_var_type())
    return false;

  TOKEN identifier = CurTok;
  
  if(!match(IDENT))
    return false;

  //argument.append(identifier);

  argument = std::make_unique<VariableASTnode>(identifier, vartype, identifier.lexeme);
  argumentList.push_back(std::move(argument));
  resetVartype();
  resetArgument();

  return true;
}

bool p_block()
{
  cout<<"block"<<endl;

  if(!match(LBRA))
  {
    return false;
  }

  if(!p_local_decls())
  {
    return false;
  }

  //added to body - need to add more from stmt_list

   if(!p_stmt_list())
  {
    return false;
  }

  if(!match(RBRA))
  {
    return false;
  }

  resetExpression();


  return true;
  //return match(LBRA) & p_local_decls() & p_stmt_list() & match(RBRA);
}

bool p_var_type()
{
  cout<<"var_type"<<endl;
  if(CurTok.type == INT_TOK)
  {
    vartype.append("int");
    cout<<vartype<<endl;
    return match(INT_TOK);
  }
  else if(CurTok.type == FLOAT_TOK)
  {
    vartype.append("float");
    return match(FLOAT_TOK);
  }
  else if(CurTok.type == BOOL_TOK)
  {
    vartype.append("bool");
    return match(BOOL_TOK);
  }
  else
  {
    return false;
  }
}

bool p_type_spec()
{
  cout<<"type_spec"<<endl;
  if(CurTok.type == VOID_TOK)
  {
    prototypeName.append("void");
    return match(VOID_TOK);
  }
  else if(contains(CurTok.type,FIRST_var_type))
  {
    return p_var_type();
  }
  else
  {
    return false;
  }
}

bool p_param_list_prime()
{
  cout<<"param_list'"<<endl;
  if(CurTok.type == COMMA)
  {
    
    if(!match(COMMA))
      return false;

    if(!p_param_list())
      return false;
    
    return true;
    //return match(COMMA) & p_param_list();
  }
  else
  {
    if(contains(CurTok.type,FOLLOW_param_list_prime)) //end of all parameters
    {
      cout<<"eat"<<endl;

      return true;
    }
    else
      return false;
  }
}

bool p_param_list()
{
  cout<<"param_list"<<endl;
  return p_param() & p_param_list_prime();
}

bool p_params()
{
  cout<<"params"<<endl;
  if(contains(CurTok.type,FIRST_param_list))
  {
    return p_param_list();
  }
  else if(CurTok.type == VOID_TOK)
  {
    argument = std::make_unique<VariableASTnode>(CurTok, "void", "");
    argumentList.push_back(std::move(argument));
    //cout<<"size:"<<argumentList.size()<<endl;
    return match(VOID_TOK);
  }
  else //epsilon
  {
    if(contains(CurTok.type,FOLLOW_params))
      {
        cout<<"eat"<<endl;
        return true; //consume epsilon
      }
      else
        return false; //fail
  }
}

bool p_decl_prime()
{
  cout<<"decl'"<<endl;
  if(CurTok.type == SC) 
  {
    variableIdent = functionIdent; //not a function
    vartype = functiontype;
    resetFunctionIdent();
    resetFunctiontype();


    if(!match(SC))
      return false;

    argument = std::make_unique<VariableASTnode>(variableIdent,vartype,variableIdent.lexeme);
    root.push_back(std::move(argument));
    resetVartype();
    resetVariableToken();
    resetArgument();

    return true;

  }
  else if(CurTok.type == LPAR)
  {
    if(!match(LPAR))
    {
      return false;
    }

    if(!p_params())
    {
      return false;
    }

    //argument list defined

    if(!match(RPAR))
    {
      return false;
    }

    //variables for function prototype defined

    if(!p_block())
    {
      return false;
    }
    // printStmtList();
    addToBody();
    resetStmtList();
    addFunctionAST(); 
    return true;
    //return match(LPAR) & p_params() & match(RPAR) & p_block();
  }
  else
  {
    return false; //fail
  }
}

bool p_extern_list_prime()
{
  cout<<"extern_list'"<<endl;
  if(contains(CurTok.type,FIRST_extern_list))
   {
      return p_extern_list();
   }
   else //epsilon
   {
      if(contains(CurTok.type,FOLLOW_extern_list_prime))
      {
        cout<<"eat"<<endl;
        return true; //consume epsilon
      }
      else
        return false; //fail
   }
}

bool p_extern()
{
  cout<<"extern"<<endl;
  //return match(EXTERN) & p_type_spec() & match(IDENT) & match(LPAR) & p_params() & match(RPAR) & match(SC);
  if(!match(EXTERN))
  {
    return false;
  }

  prototypeName.append("extern ");

  if(!p_type_spec())
  {
    return false;
  }

  prototypeName.append(vartype + " ");
  resetVartype();
  
  string ident = CurTok.lexeme;

  if(!match(IDENT))
  {
    return false;
  }

  prototypeName.append(ident + " ");

  //got function name

  if(!match(LPAR))
  {
    return false;
  }

  if(!p_params())
  {
    return false;
  }

  if(!match(RPAR))
  {
    return false;
  }

   if(!match(SC))
  {
    return false;
  }
  cout<<"size:"<<argumentList.size()<<endl;
  unique_ptr<PrototypeAST> Proto = std::make_unique<PrototypeAST>(prototypeName,std::move(argumentList));
  root.push_back(std::make_unique<FunctionAST>(std::move(Proto),std::move(body)));
  resetArgumentList();
  resetPrototypeName();

  return true;

}

bool p_decl_list_prime()
{
  cout<<"decl_list'"<<endl;
  cout<<CurTok.type<<endl;
   if(contains(CurTok.type,FIRST_decl_list))
   {
      return p_decl_list();
   }
   else //epsilon
   {
      if(contains(CurTok.type,FOLLOW_decl_list_prime))
      {
        cout<<"eat"<<endl;
        return true; //consume epsilon
      }
      else
        return false; //fail
   }
}

bool p_decl()
{
  cout<<"decl"<<endl;
  if(contains(CurTok.type,FIRST_var_type))
  {
    //return p_var_type() & match(IDENT) & p_decl_prime();
    if(!p_var_type())
    {
      return false;
    }
    //vartype defined
    functiontype.append(vartype); //in case of a function decl
    resetVartype();

    functionIdent = CurTok; //in case of a function decl
    if(!match(IDENT))
    {
      return false;
    }

    //prototypeName.append(functionIdent.lexeme);

    //functionIdent defined

    if(!p_decl_prime())
    {
      return false;
    }

    return true;
  }
  else if(CurTok.type == VOID_TOK)
  {
    //return match(VOID_TOK) & match(IDENT) & match(LPAR) & p_params() & match(RPAR) & p_block();
    if(!match(VOID_TOK))
    {
      return false;
    }

    functiontype.append("void");

    functionIdent = CurTok;
    if(!match(IDENT))
    {
      return false;
    }

     if(!match(LPAR))
    {
      return false;
    }

    if(!p_params())
    {
      return false;
    }

    //argument list defined

    if(!match(RPAR))
    {
      return false;
    }

    //variables for function prototype defined

    if(!p_block())
    {
      return false;
    }
    // printStmtList();
    addToBody();
    resetStmtList();
    addFunctionAST(); 
    return true;
  }
  else
  {
    return false; //fail
  }
}

bool p_decl_list()
{
  cout<<"decl_list"<<endl;
  return p_decl() & p_decl_list_prime();
}

bool p_extern_list()
{
  cout<<"extern_list"<<endl;
  return p_extern() & p_extern_list_prime();
}

bool p_program()
{
  /*
  if curTok is in FIRST(extern_list)
    run extern_list()
  else if curTok is in FIRST(decl_list)
    run decl_list()
  else
    error
  */
  cout<<"program"<<endl;
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

// program ::= extern_list decl_list
static bool parser() {
  // add body
  getNextToken();
  if(p_program() & (CurTok.type == EOF_TOK))
  {
    cout<<"Parsing successful"<<endl;
    return true;
  }
  else
  {
    return false;
  }

}

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;

//===----------------------------------------------------------------------===//
// AST Printer
//===----------------------------------------------------------------------===//

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const unique_ptr<ASTnode> &ast) {
  os << ast->to_string(); //changes made
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
      cout<<"Lexical error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<endl;
      return 1;
    }
    //print each token
    //fprintf(stderr, "Token: %s with type %d\n", CurTok.lexeme.c_str(),CurTok.type);
    getNextToken();
  }
  fprintf(stderr, "Lexer Finished\n");
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
    cout<<"Parsing failed"<<endl;
    return 1;
  }
  //fprintf(stderr, "Parsing Finished\n");


  //Printing out AST
  llvm::outs() << "\nPrinting out AST:"<< "\n";
  llvm::outs() << "root"<< "\n";
  for(int i = 0; i < root.size(); i++)
  {
    // if(i == root.size() - 1)
    //   llvm::outs() << " |-> " << root[i] << "\n";
    // else
      llvm::outs() <<root[i]<< "\n\n";
      // llvm::outs() << "  |-> " << root[i] << "\n";
  }

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