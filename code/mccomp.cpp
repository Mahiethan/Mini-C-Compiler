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
  // virtual AllocaInst *codegen() override;
  virtual std::string to_string() const override {
  //return a sting representation of this AST node
    string final =  "VarDecl: " + Type + " " + Val; 
    decreaseIndentLevel();
    return final;
    // return "a";
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
  //return a sting representation of this AST node
    string final = "VarRef: " + Name;
    decreaseIndentLevel();
    return final;
  };
};

/// UnaryExprASTnode - Expression class for a unary operator, like ! or - (check production `rval_two`)
class UnaryExprASTnode : public ASTnode {
  string Opcode;
  std::unique_ptr<ASTnode> Operand;

public:
  UnaryExprASTnode(string Opcode, std::unique_ptr<ASTnode> Operand)
      : Opcode(Opcode), Operand(std::move(Operand)) {}

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

public:
  BinaryExprASTnode(string Opcode, std::unique_ptr<ASTnode> LHS,
                std::unique_ptr<ASTnode> RHS)
      : Opcode(Opcode), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

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

public:
  FuncCallASTnode(const std::string &Callee,
              std::vector<std::unique_ptr<ASTnode>> Args)
      : Callee(Callee), Args(std::move(Args)) {}

  virtual Value *codegen() override;
  virtual std::string to_string() const override {
  //return a string representation of this AST node
    string args = "";
    for(int i = 0; i < Args.size(); i++)
    {
      // if(i == Args.size() - 1)
      args.append("\n" + addIndent() + "--> Param" + Args[i]->to_string());
      // args.append(" -> Param" + Args[i]->to_string());
    // else
    //   args.append(addIndent() + " -> Param" + Args[i]->to_string() + "\n");
    }

    // if(args.length() != 0)
    //   args = "\n" + args;

    string final = "FunctionCall: " + Callee + args;
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
  // cout<<"Size of Else:"<<Else.size()<<endl;
  // cout<<"Size of Then:"<<Then.size()<<endl;
  final.append(ThenStr);
  if(Else.size() != 0)
   {
      ElseStr = "\n" + addIndent() + "--> ElseExpr:";
      // decreaseIndentLevel();
   }
  for(int j = 0; j < Else.size(); j++)
  {
    if(Else[j] != nullptr)
      ElseStr.append("\n" + addIndent() + "--> " + Else[j]->to_string()); 
  } 
  final.append(ElseStr);
  decreaseIndentLevel(); //one for if
  if(Else.size() != 0)
  {
    decreaseIndentLevel(); //one for else
  }
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
  // virtual AllocaInst *codegen() override;
  virtual std::string to_string() const override {
  //return a sting representation of this AST node
    string final =  "GlobalVarDecl: " + Ty + " " + Val; 
    decreaseIndentLevel();
    return final;
    // return "a";
  };
};


/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : public TopLevelASTnode {
  // string returnType;
  std::string Name;
  std::vector<unique_ptr<VariableASTnode>> Args;

public:
  PrototypeAST(std::string &Name, std::vector<unique_ptr<VariableASTnode>> Args)
      : Name(Name), Args(std::move(Args)) {}

  const std::string &getName() const { return Name; }

  string getArgName(int index)
  {
    return Args.at(index)->getVal();
    // string a = Args.at(index)->getVal();
    // cout<<"getargs: "<<a<<endl;
    // return a;
  }

  // virtual Value *codegen() override;
  virtual Function *codegen() override;

  virtual std::string to_string() const override {
  //return a sting representation of this AST node
  string name = getName();
  string args = "";
  //cout<<Args.size()<<endl;
  // if(Args.size() > 0)
  // {
  //   args.append("\n" + addIndent() + "--> Param" + Args[i]->to_string());
  // }
  for(int i = 0; i < Args.size(); i++)
  {
    //cout<<Args[i]->to_string()<<endl;
    // if(i == Args.size() - 1)
    //   args.append("\n" + addIndent() + "`-> Param" + Args[i]->to_string());
    // else
      args.append("\n" + addIndent() + "--> Param" + Args[i]->to_string());
  }

//  if(Args.size() > 0)
//   {
//     decreaseIndentLevel();
//   }
 
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

    

    // virtual Value *codegen() override;
    virtual Function *codegen() override;

    virtual std::string to_string() const override{
  //return a sting representation of this AST node
      string proto = Proto->to_string();
      string final =  proto;
      string body = "";
      if(Body.size() != 0)
      {
        body = "\n" + addIndent() + "--> Function Body:";
        // decreaseIndentLevel();
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

static vector<unique_ptr<TopLevelASTnode>> root; //root of the AST (TranslationUnitDecl)

///temporary vector/string stores

//null TOKEN
TOKEN nullToken = {};

//these are used to enclose unary expressions so proper precedence can be applied to them
TOKEN insertLPAR = {LPAR,"(",0,0};
TOKEN insertRPAR = {RPAR,")",0,0};

static string prototypeName = "";
static unique_ptr<VariableASTnode> argument = std::make_unique<VariableASTnode>(CurTok,"","");
static unique_ptr<GlobalVariableAST> globalVar = std::make_unique<GlobalVariableAST>(CurTok,"","");
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

void resetUnaryExpr()
{
  unary = "";
}


static bool errorReported = false;

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


/* Helper functions for parser and AST generation*/

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

void addFunctionAST()
{
  //define functionAST and add to root
  prototypeName.append(functiontype + " " + functionIdent.lexeme);
  resetFunctionIdent();
  resetFunctiontype();
  unique_ptr<PrototypeAST> Proto = std::make_unique<PrototypeAST>(prototypeName,std::move(argumentList));
  resetArgumentList();
  resetPrototypeName();
  resetFunctiontype();
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

static std::pair<std::string, std::unique_ptr<ASTnode>> curr;
unique_ptr<ASTnode> processStmtList()
{
  if(stmtList.size() > 0)
  {
    curr = std::move(stmtList.front());
    stmtList.pop_front();
  }
  else
  {
    return nullptr;
  }
  cout<<"Curr: "<<curr.first<<"size "<<stmtList.size()<<endl;
  // if((curr.first == "end_while") | (curr.first == "end_if") | (curr.first == "end_else"))//ignore "new"
  // {
  //   // cout<<"Discard: "<<curr.first<<endl;
  //   curr = std::move(stmtList.front());
  //   // cout<<"NewCurr: "<<curr.first<<endl;
  //   stmtList.pop_front();
  // }
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
    cout<<"start while"<<endl;
    unique_ptr<ASTnode> cond = processStmtList();
    vector<unique_ptr<ASTnode>> then = {};
    while(curr.first != "end_while")
    {
      unique_ptr<ASTnode> node = std::move(processStmtList());
      if(node != nullptr)
        then.push_back(std::move(node));
    }
    cout<<"exit while"<<endl;
    if(curr.first == "end_while")
    {
      // curr = std::move(stmtList.front());
      // stmtList.pop_front();
      curr.first = ""; //acknowledge end of while
      return std::move(make_unique<WhileExprASTnode>(std::move(cond),std::move(then)));
    }
    else
    {
      cout<<"ERRORRRRRRRRRRRRRRR!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
      return nullptr;
    }
    
  }
  else if(curr.first == "if")
  {
    cout<<"start if"<<endl;
    unique_ptr<ASTnode> cond = processStmtList();
    vector<unique_ptr<ASTnode>> Then = {};
    vector<unique_ptr<ASTnode>> Else = {};
    while((curr.first != "end_if"))
    {
      unique_ptr<ASTnode> node = std::move(processStmtList());
      if(node != nullptr)
        Then.push_back(std::move(node));
    }
    cout<<"exit if then"<<endl;

    if(curr.first == "end_if")
    {
      curr = std::move(stmtList.front());
      stmtList.pop_front();
    }

    if(curr.first == "no_else")
    {
      return std::move(make_unique<IfExprASTnode>(std::move(cond),std::move(Then),std::move(Else)));
    }
    else if(curr.first == "else")
    {
      cout<<"start else"<<endl;
      while(curr.first != "end_else")
      {
        // cout<<"WHAT IS IT::"<<curr.first<<endl;
      unique_ptr<ASTnode> node = std::move(processStmtList());
      if(node != nullptr)
        Else.push_back(std::move(node));
      }
      cout<<"exit else then"<<endl;
    } 
    cout<<"exit if"<<endl; 

    if(curr.first == "end_else")
    {
      curr.first = ""; //acknowledge end of else
      // curr = std::move(stmtList.front());
      // stmtList.pop_front();
      return std::move(make_unique<IfExprASTnode>(std::move(cond),std::move(Then),std::move(Else)));
    }
    else
    {
      cout<<"ERRORRRRRRRRRRRRRRR!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
      return nullptr;
    }

  }
  else if(curr.first == "return")
  {
    // return std::move(curr.second);
    unique_ptr<ASTnode> returnExpr = processStmtList();
    TOKEN returnTok = nullToken;
    if(returnExpr != nullptr)
      returnTok = returnExpr->getTok();
    unique_ptr<ReturnExprASTnode> returnNode = make_unique<ReturnExprASTnode>(std::move(returnExpr),functiontype, returnTok);
    return std::move(returnNode);
  }
  else return std::move(nullptr);
} 

void addToBody()
{
  while(stmtList.size() != 0)
  {
    cout<<"stmt size: "<<stmtList.size()<<endl;
    // printStmtList();
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

bool isMatchingLastParam(vector<TOKEN> expression)
{
  int buf = 0;
  for(int i = 1; i < expression.size(); i++)
  {
    if(expression.at(i).type == RPAR && buf == 0)
    {
      cout<<i<<"compared to"<<expression.size()-1<<endl;
        cout<<"ofund"<<endl;
      if(i == expression.size() - 1)
      {
        cout<<"Yes mate"<<endl;
        return true;
      }
      else
      {
        cout<<"Nah mate"<<endl;
        return false;
      }
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
  cout<<"nope"<<endl;
  return false;
}

void checkForUnary() //adds parentheses over unary expressions to assert correct precedence
{
  bool isOp = false;
  int valid = 0;
  bool unaryEnd = true;
  for(auto i = 0; i < expression.size(); i++)
  {
    TOKEN curr = expression.at(i);
    if(curr.lexeme == "(") 
    {
      valid++;
    }
    if(curr.lexeme == ")")
    {
      valid--;
    }
    if(getPrecedence(curr.lexeme) != 110) //operator found
    {
      if(isOp == true & unaryEnd == true & valid == 0)
      {
        auto itPos = expression.begin() + i;
        expression.insert(itPos,insertLPAR);
        cout<<expression.at(i).lexeme<<endl;
        if(i != expression.size()-1)
          i++;
        unaryEnd = false;
      }
      if(isOp == false & unaryEnd == false & valid == 0)
      {
        auto itPos = expression.begin() + i;
        expression.insert(itPos,insertRPAR);
        i++;
        unaryEnd = true;
      }
      isOp = true;
    }
    else
    {
      isOp = false;
    }
    if(i == expression.size()-1 & unaryEnd == false)
    {
      auto itPos = expression.begin() + i;
      expression.insert(itPos,insertRPAR);
      unaryEnd = true;
    }
  }
}

void printExpression(vector<TOKEN> exp)
{
  for(int i = 0; i < exp.size(); i++)
  {
    cout<<exp.at(i).lexeme<<endl;
  }
}

unique_ptr<ASTnode> createExprASTnode(vector<TOKEN> expression)
{
  if(expression.size() == 1) //literals
  {
    TOKEN t = expression.at(0);
    if(t.type == INT_LIT)
    {
      int val;
      try{
         val = stoi(t.lexeme);
      }
      catch(std::out_of_range)
      {
        errs()<<"Warning: Value "<<t.lexeme<<" out of range for int type. Setting it to 0\n";
        val = stoi("0");
      }
      
      return std::move(make_unique<IntASTnode>(t,val));
    }
    else if(t.type == FLOAT_LIT)
    {
      float val;
      try{
         val = stof(t.lexeme);
      }
      catch(std::out_of_range)
      {
        errs()<<"Warning: Value "<<t.lexeme<<" out of range for float type. Setting it to 0.0\n";
        val = stof("0.0");
      }
      return std::move(make_unique<FloatASTnode>(t,val));
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
  else if((expression.at(0).lexeme == "-" | expression.at(0).lexeme == "!") & (expression.at(1).lexeme == "-" | expression.at(1).lexeme == "!" | expression.at(1).lexeme == "(" | expression.size() == 2)) //unary
  {
    cout<<"unary"<<endl;
    string opcode = expression.at(0).lexeme;
    vector<TOKEN> operand = {};
    for(int i = 1; i < expression.size(); i++)
      operand.push_back(expression.at(i));

    return std::move(make_unique<UnaryExprASTnode>(opcode,std::move(createExprASTnode(operand))));
  }
  else if((expression.at(0).lexeme == "(") & (isMatchingLastParam(expression) == true)) //bracketed expr - start to end e.g (a + d + (a+f)) not (a+f)+(-e+d)
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
            cout<<expression.at(i).lexeme<<endl;
          }
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
    bool isOp = true;
    bool unaryEnd = true;
    int valid = 0; //operators inside bracketed expressions are invalid
    for(int i = 0; i < expression.size(); i++)
    {
      int currPrecedence = getPrecedence(expression.at(i).lexeme);
      if(expression.at(i).lexeme == "(") 
      {
        valid++;
      }
      if(expression.at(i).lexeme == ")")
      {
        valid--;
      }
      if(currPrecedence != 110) //operator found
      {
        // if(isOp == true & unaryEnd == true & valid == 0)
        // {
        //   auto itPos = expression.begin() + i;
        //   expression.insert(itPos,insertLPAR);
        //   cout<<expression.at(i).lexeme<<endl;
        //   if(i != expression.size()-1)
        //     i++;
        //   unaryEnd = false;
        // }

        // if(isOp == false & unaryEnd == false & valid == 0)
        // {
        //   auto itPos = expression.begin() + i;
        //   expression.insert(itPos,insertRPAR);
        //   if(i != expression.size()-1)
        //     i++;
        //   unaryEnd = true;
        // }

        if((currPrecedence <= minPrecedence) & (isOp == false) & (valid == 0)) //get lowest precedence operator, avoiding any unary operators
        { 
          op = expression.at(i).lexeme;
          // cout<<op<<endl;
          minPrecedence = currPrecedence;
          //cout<<minPrecedence<<endl;
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
      if(i == expression.size()-1 & unaryEnd == false)
      {
        auto itPos = expression.begin() + i;
        expression.insert(itPos,insertRPAR);
        unaryEnd = true;
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
    // cout<<"printing"<<endl;
    // printExpression(expression);
    return std::move(make_unique<BinaryExprASTnode>(op, std::move(createExprASTnode(lhs)), std::move(createExprASTnode(rhs)))); //recursive
  }
  return nullptr; //error
}

/* Function calls for each production */

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

bool p_arg_list_prime()
{
  if(CurTok.type == COMMA)
  {
    //return match(COMMA) & p_arg_list();
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
      cout<<"eat"<<endl;
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
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
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
      cout<<"eat"<<endl;
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

bool p_rval_one()
{
  cout<<"rval_one"<<endl;
  // cout<<CurTok.type<<endl;
  if(CurTok.type == LPAR)
  {
    //return match(LPAR) & p_expr() & match(RPAR);
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
    // return match(IDENT) & p_rval();
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
    // return match(INT_LIT);
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
    // return match(FLOAT_LIT);
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
    // return match(BOOL_LIT);
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


bool p_rval_two()
{
  cout<<"rval_two"<<endl;
  if(CurTok.type == MINUS)
  {
    //return match(MINUS) & p_rval_two();
    TOKEN temp = CurTok;
    if(!match(MINUS))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  -  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

    //unary.append("-");
    expression.push_back(temp);
    return p_rval_two();
  }
  else if(CurTok.type == NOT)
  {
    //return match(NOT) & p_rval_two();
    TOKEN temp = CurTok;
    if(!match(NOT))
    {
      if(!errorReported)
        errs()<<"Syntax error: Expected  !  at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }

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
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
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
    // return match(DIV) & p_rval_two() & p_rval_three_prime();
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
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
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
    //return match(MINUS) & p_rval_three() & p_rval_four_prime();
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
      cout<<"eat"<<endl;
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
    //return match(LT) & p_rval_four() & p_rval_five_prime();
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
    //return match(GE) & p_rval_four() & p_rval_five_prime();
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
    //return match(GT) & p_rval_four() & p_rval_five_prime();
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
      cout<<"eat"<<endl;
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
      cout<<"eat"<<endl;
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
      cout<<"eat"<<endl;
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
      cout<<"eat"<<endl;
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

bool p_return_stmt_prime()
{
  cout<<"return_stmt'"<<endl;
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
    // return p_expr() & match(SC);
    if(!p_expr())
    {
      if(!errorReported)
        errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
    
    // //checkForUnary();
    cout<<"Printing"<<endl;
    //printExpression();
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

bool p_return_stmt()
{
  cout<<"return_stmt"<<endl;
  //return match(RETURN) & p_return_stmt_prime();
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
  
  // pair <string,unique_ptr<ASTnode>> n = make_pair("end_return",std::move(nullptr));
  // stmtList.push_back(std::move(n));

  return true;
}

bool p_expr()
{
  cout<<"expr"<<endl;
  //return p_exprStart() & p_rval_eight();
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

  //print out expression
  // //printExpression();

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
      cout<<CurTok.type<<endl;

    if(contains(CurTok.type,FOLLOW_exprStart))
    {
      cout<<"eat"<<endl;
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

bool p_else_stmt()
{
  cout<<"else_stmt"<<endl;
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
      cout<<"eat"<<endl;
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


bool p_if_stmt()
{
  cout<<"if_stmt"<<endl;
  // return match(IF) & match(LPAR) & p_expr() & match(RPAR) & p_block() & p_else_stmt();

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
  //checkForUnary();
   cout<<"Printing"<<endl;
    //printExpression();
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
  //checkForUnary();
   cout<<"Printing"<<endl;
    //printExpression();
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

bool p_expr_stmt()
{
  cout<<"expr_stmt"<<endl;
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
    // //printExpression();

    //checkForUnary();
     cout<<"Printing"<<endl;
    //printExpression();
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
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
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
      cout<<"eat"<<endl;
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

bool p_local_decl()
{
  cout<<"local_decl"<<endl;
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
  //return p_var_type() & match(IDENT) & match(SC);
}

bool p_local_decls()
{
   cout<<"local_decls"<<endl;
   if(contains(CurTok.type, FIRST_local_decl))
   {
      if(!p_local_decl())
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
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
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
   }
}

bool p_param()
{
  cout<<"param"<<endl;
  //return p_var_type() & match(IDENT);
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

  //added to body - need to add more from stmt_list

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
  //return match(LBRA) & p_local_decls() & p_stmt_list() & match(RBRA);
}

bool p_var_type()
{
  cout<<"var_type"<<endl;
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
    cout<<vartype<<endl;
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

bool p_type_spec()
{
  cout<<"type_spec"<<endl;
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

bool p_param_list_prime()
{
  cout<<"param_list'"<<endl;
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
    {
      if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
      errorReported = true;
      return false;
    }
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
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false; //fail
      }
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
    cout<<"//////////////////////////////////////"<<endl;
    printStmtList();
        cout<<"//////////////////////////////////////"<<endl;

    addToBody();
    resetStmtList();
    addFunctionAST(); 
    return true;
    //return match(LPAR) & p_params() & match(RPAR) & p_block();
  }
  else
  {
    if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
    errorReported = true;
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
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false; //fail
      }
   }
}

bool p_extern()
{
  cout<<"extern"<<endl;
  //return match(EXTERN) & p_type_spec() & match(IDENT) & match(LPAR) & p_params() & match(RPAR) & match(SC);
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
  cout<<"size:"<<argumentList.size()<<endl;
  unique_ptr<PrototypeAST> Proto = std::make_unique<PrototypeAST>(prototypeName,std::move(argumentList));
  // root.push_back(std::make_unique<FunctionAST>(std::move(Proto),std::move(body)));
  root.push_back(std::move(Proto));
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
      {
        if(!errorReported)
          errs()<<"Syntax error: Invalid token "<<CurTok.lexeme<<" found at line "<<CurTok.lineNo<<" column "<<CurTok.columnNo<<".\n";
        errorReported = true;
        return false; //fail
      }
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
    // printStmtList();
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
    cout<<"Parsing successful."<<endl;
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

static vector<map<string,AllocaInst*>> NamedValuesList;
// static std::map<std::string, AllocaInst*> NamedValues;
static map<string,GlobalVariable*> GlobalVariables;

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
  cout<<"Int codegen\n";
  return ConstantInt::get(TheContext, APInt(32,Val,true));
}

Value *FloatASTnode::codegen() {
  cout<<"Float codegen\n";
  return ConstantFP::get(TheContext, APFloat(float(Val)));
}

Value *BoolASTnode::codegen() {
  cout<<"Bool codegen\n";
  return ConstantInt::get(TheContext, APInt(1,int(Val),false));
}

Value *VariableASTnode::codegen() {
  cout<<"VarDecl codegen\n";
// AllocaInst *VariableASTnode::codegen() {
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  AllocaInst* varAlloca = CreateEntryBlockAlloca(TheFunction, Val, Type);
  //store in NamedValues
  std::map<std::string, AllocaInst*> NamedValues = NamedValuesList.back();
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
    
    errs()<<"Semantic error: Redefinition of global variable "<<Val<<" with different type "<<Type<<". Variable "<<Val<<" of type "<<existTy<<" already exists.\n";
    return nullptr;
  }
  //Value* var = Builder.CreateLoad(Type::getInt32Ty(TheContext), varAlloca, Val);
  NamedValuesList.push_back(NamedValues);
  return varAlloca;
}

Value *VariableReferenceASTnode::codegen() {
  // Look this variable up in the function.
  cout<<"VarRef codegen\n";
  AllocaInst *V;

  //iterate thru all symbol tables, starting from end
  std::map<std::string, AllocaInst*> NamedValues;
  int i;
  for(i = NamedValuesList.size() - 1; i >= 0; i--)
  {
    NamedValues = NamedValuesList.at(i);
    V = NamedValues[Name];
    if(!V)
    {
      cout<<"Can't find "<<Name<<". Proceed with next symbol table"<<endl;
      continue;
    }
    else
    {
      cout<<"FOUND "<<Name<<endl;
      return V;
    }

  }
  cout<<"End of all symbol tables. Checking global variables of size "<<GlobalVariables.size()<<endl;
  
  //check if its a global variable instead
  GlobalVariable *GV = GlobalVariables[Name];
  if(!GV)
  {
    errs()<<"Semantic error: Unknown variable name: "<<Name<<"\n";
    return nullptr;
  }
  else 
  {
    cout<<"FOUND GLOBAL "<<Name<<endl;
    return GV;
  }
}

Value* UnaryExprASTnode::codegen()
{ 
  cout<<"Unary codegen\n";
  Value* operand = Operand->codegen();

  if(operand == nullptr)
    return nullptr;

  if(auto *AI = dyn_cast<AllocaInst>(operand))
  {
    operand = Builder.CreateLoad(AI->getAllocatedType(),operand,"load_temp");
  }

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

  if(Opcode == "!") //talk about how this is done with extraneous instructions but still gives correct answer 
  {
    if(type == "float")
    {
      // operand = Builder.CreateCast(Instruction::FPToSI,operand,Type::getInt32Ty(TheContext));
      // type = "int";
      operand = Builder.CreateFCmpONE(operand, ConstantFP::get(TheContext, APFloat((float) 0.0)),"bool_cast");
    }

    if(type == "int")
    {
      operand = Builder.CreateIntCast(operand, Type::getInt1Ty(TheContext), false);
    }

    return Builder.CreateNot(operand,"not_temp");
  }
  else if(Opcode == "-")
  {
    if(type == "bool")
    {
      operand = Builder.CreateIntCast(operand, Type::getInt32Ty(TheContext), false);
    }
    else if(type == "float") //do a subtraction with 0
    {
      float zero = 0.0;
      return Builder.CreateFNeg(operand,"fneg_temp");
    }
    return Builder.CreateNeg(operand,"neg_temp");
    // return Builder.CreateSub(ConstantInt::get(TheContext,APInt(32,0,true)),operand,"neg_temp");//fneg not used (only flips sign bit)

  }
  else
    return nullptr;
  // return nullptr;
}

Value* BinaryExprASTnode::codegen(){
  cout<<"Binary codegen\n";
  Value* lhs = LHS->codegen();
  //boolean short circuit code generation for logical operators || and &&
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


  int lhsType = 0;
  int rhsType = 0;
  string lhsTypeStr = "";
  string rhsTypeStr = "";

  if(Opcode != "=") //load LHS if it is not an assign operation
  {
    if(auto *AI = dyn_cast<AllocaInst>(lhs)) //for ordinary variable alloca
    {
      cout<<"lhs alloca\n";
      if(AI->getAllocatedType()->isFloatTy())
        lhs = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_temp");
      else if(AI->getAllocatedType()->isIntegerTy(32))
        lhs = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_temp");
      else if(AI->getAllocatedType()->isIntegerTy(1))
        lhs = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI, "load_temp");

    }  
    else if(auto *GV = dyn_cast<GlobalVariable>(lhs)) //for global variables
    {
      cout<<"lhs Global\n";
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
  else //keep LHS as an alloca and get its type
  {
    if(auto *AI = dyn_cast<AllocaInst>(lhs))
    {
      cout<<"lhs Alloca\n";
      if(AI->getAllocatedType()->isFloatTy())
        lhsType = 2;
      else if(AI->getAllocatedType()->isIntegerTy(32))
        lhsType = 1;
      else if(AI->getAllocatedType()->isIntegerTy(1))
        lhsType = 0;
    }
    else if(auto *GV = dyn_cast<GlobalVariable>(lhs))
    {
      cout<<"lhs Global\n";
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
    cout<<"rhs Alloca\n";
    if(AI->getAllocatedType()->isFloatTy())
      rhs = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_temp");
    else if(AI->getAllocatedType()->isIntegerTy(32))
      rhs = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_temp");
    else if(AI->getAllocatedType()->isIntegerTy(1))
      rhs = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI,"load_temp");
  }
  else if(auto *GV = dyn_cast<GlobalVariable>(rhs)) //if RHS is a global variable
  {
    cout<<"rhs Global\n";
    if(GV->getValueType()->isFloatTy())
      rhs = Builder.CreateLoad(Type::getFloatTy(TheContext), GV, "load_global_temp");
    else if(GV->getValueType()->isIntegerTy(32))
      rhs = Builder.CreateLoad(Type::getInt32Ty(TheContext), GV, "load_global_temp");
    else if(GV->getValueType()->isIntegerTy(1))
      rhs = Builder.CreateLoad(Type::getInt1Ty(TheContext), GV, "load_global_temp");
  }
  if(rhs->getType()->isIntegerTy(32))
      rhsType = 1;
    else if(rhs->getType()->isIntegerTy(1))
      rhsType = 0;
    else if(rhs->getType()->isFloatTy())
      rhsType = 2;

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

  cout<<"lhsType: "<<lhsTypeStr<<endl;
  cout<<"rhsType: "<<rhsTypeStr<<endl;
  
    //Modify for different types
    if(Opcode == "=") //ASSIGN
    {
      cout<<"lhsType for EQ: "<<lhsTypeStr<<endl;
      cout<<"rhsType for EQ: "<<rhsTypeStr<<endl;

      if(auto *st = dyn_cast<StoreInst>(rhs))
      {
        cout<<"yes\n";
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

      string name = LHS->getName();
      if(name != "")
      {
        // not needed
        // // cout<<"name:"<<name<<endl;
        // std::map<std::string, AllocaInst*> NamedValues = NamedValuesList.back();
        // AllocaInst *V = NamedValues[name];
        // if(!V)
        // {
        //   errs()<<"Semantic error: Unknown variable name: "<<name<<"\n";
        //   return nullptr;
        // }
        // // else

        if(lhsType < rhsType)
        {
          errs()<<"Semantic error: Widening conversion not possible from RHS type "<<rhsTypeStr<<" to LHS type "<<lhsTypeStr<<"\n";
          return nullptr;
        }
        else if(lhsType > rhsType)//perform widening conversions
        {
            // rhs = Builder.CreateSExt(rhs,lhs->getType(),"zext_temp");
            cout<<"casting eq\n";

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

    if(isLogical == true)
    {
      cout<<"Casting for logical operators\n";
              // cout<<lhsType<<endl;

      //set both operands to boolean type, for AND, OR operators.

            if(lhsType == 2)
            {
              // lhs = Builder.CreateIntCast(lhs, Type::getInt32Ty(TheContext), false);
              // lhs = Builder.CreateCast(Instruction::FPToSI,lhs,Type::getInt32Ty(TheContext));
              // lhsType = 1;
              lhs = Builder.CreateFCmpONE(lhs, ConstantFP::get(TheContext, APFloat((float) 0.0)),"float_bool_cast");

            }

            if(rhsType == 2)
            {
              // rhs = Builder.CreateIntCast(rhs, Type::getInt32Ty(TheContext), false);
              // rhs = Builder.CreateCast(Instruction::FPToSI,rhs,Type::getInt32Ty(TheContext));
              // rhsType = 1;
              rhs = Builder.CreateFCmpONE(rhs, ConstantFP::get(TheContext, APFloat((float) 0.0)),"float_bool_cast");
            }
            
            if(lhsType == 1)
              lhs = Builder.CreateICmpNE(lhs, ConstantInt::get(Type::getInt32Ty(TheContext), 0, false),"int_bool_cast");
            if(rhsType == 1)
              rhs = Builder.CreateICmpNE(rhs, ConstantInt::get(Type::getInt32Ty(TheContext), 0, false),"int_bool_cast");

            lhsType = 0;
            rhsType = 0;
    }
   
        if(Opcode == "||") //OR
          return Builder.CreateLogicalOr(lhs,rhs,"or_tmp"); 
        else if(Opcode == "&&") //AND
          return Builder.CreateLogicalAnd(lhs,rhs,"and_tmp"); 

      //Set both operands to equal types for +, -, *, /, %, ==, !=, <=, <, >= and > operators

      if(lhsType != rhsType)
      {
        cout<<"uneequal\n";
        int returnType = lhsType;
        if(rhsType > returnType)
          returnType = rhsType;

        cout<<"Casting all to "<<returnType<<endl;
        
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
      // else
      // {
      //   cout<<"No casting required\n";
      // }

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
          errs()<<"Semantic error: Division by zero not permitted.\n";
          return nullptr;
        }

        if(rhs->getType()->isFloatTy() & lhs->getType()->isFloatTy()) //float
          return Builder.CreateFDiv(lhs,rhs,"fdiv_tmp");
        else //for int or bool
          return Builder.CreateSDiv(lhs,rhs,"div_tmp");
      }
      else if(Opcode == "%") //MOD - check if both operands are not float!
      {
        if(rhs == ConstantInt::get(TheContext, APInt(32,int(0),false)) | rhs == ConstantInt::get(TheContext, APInt(1,int(false),false)) | rhs == ConstantFP::get(TheContext, APFloat((float)0.0)))
        {
          errs()<<"Semantic error: Taking remainder of division with zero not permitted.\n";
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
  
  //return nullptr;
}

Value* FuncCallASTnode::codegen(){
  // Look up the name in the global module table.
  cout<<"FuncCall codegen\n";
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
  {
    errs()<<"Semantic error: Unknown function "<<Callee<<" referenced.\n";
    return nullptr;
  }
  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
  {
    errs()<<"Semantic error: Incorrect no. of arguments passed for function "<<Callee<<".\n";
    return nullptr;
  }
  
  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) 
  {
    Value* args = Args[i]->codegen(); //get alloca
    //load arguments
    if(auto *AI = dyn_cast<AllocaInst>(args)) //for ordinary variable alloca
    {
      cout<<"alloca arg\n";
      if(AI->getAllocatedType()->isFloatTy())
        args = Builder.CreateLoad(Type::getFloatTy(TheContext), AI, "load_arg");
      else if(AI->getAllocatedType()->isIntegerTy(32))
        args = Builder.CreateLoad(Type::getInt32Ty(TheContext), AI, "load_arg");
      else if(AI->getAllocatedType()->isIntegerTy(1))
        args = Builder.CreateLoad(Type::getInt1Ty(TheContext), AI, "load_arg");

    }  
    else if(auto *GV = dyn_cast<GlobalVariable>(args)) //for global variables
    {
      cout<<"global arg\n";
      if(GV->getValueType()->isFloatTy())
        args = Builder.CreateLoad(Type::getFloatTy(TheContext), GV, "load_global_arg");
      else if(GV->getValueType()->isIntegerTy(32))
        args = Builder.CreateLoad(Type::getInt32Ty(TheContext), GV, "load_global_arg");
      else if(GV->getValueType()->isIntegerTy(1))
        args = Builder.CreateLoad(Type::getInt1Ty(TheContext), GV, "load_global_arg");
    }

    ArgsV.push_back(args);
    if (!ArgsV.back())
      return nullptr;
  }

  if(CalleeF->getReturnType()->isVoidTy())
    return Builder.CreateCall(CalleeF, ArgsV); //void functions cannot have a name
  else
    return Builder.CreateCall(CalleeF, ArgsV, "call_tmp"); 
    // return nullptr;
}

Value* IfExprASTnode::codegen(){
  cout<<"IfExpr codegen\n";
  cout<<"ELSE SIZE: "<<Else.size()<<endl;
  bool elseExist = false;
   if(Else.size() != 0)
    elseExist = true;
  
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
    cout<<"Alloca cond\n";
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
    cout<<"Global cond\n";
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
  cout<<"COND TYPE: "<<currType<<endl;

  if(currType != "bool") //cast to bool type
  {
    // errs()<<"Semantic error: Expected type `bool` for the condition statement at line no. "<<Cond->getTok().lineNo<<" column no. "<<Cond->getTok().columnNo<<". Cannot cast from type `"<<currType<<"` to `bool`.\n";
    // return nullptr;
    if(currType == "float")
      cond = Builder.CreateFCmpONE(cond, ConstantFP::get(TheContext, APFloat((float) 0.0)),"float_bool_cast");   
    else if(currType == "int")
      cond = Builder.CreateICmpNE(cond, ConstantInt::get(Type::getInt32Ty(TheContext), 0, false),"int_bool_cast");
  }

  Value* comp = Builder.CreateICmpNE(cond, ConstantInt::get(TheContext, APInt(1,0,false)), "if_cond");

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
    cout<<"size of then:"<<Then.size()<<endl;
    Value* thenVal = Then.at(i)->codegen();
    if(thenVal == nullptr)
      return nullptr;
    if(auto *R = dyn_cast<ReturnInst>(thenVal))
      {
        ret = thenVal;
        generateBranchForThen = false;
        break; //don't generate IR for instructions after return
      }
    // if(thenVal == nullptr)
    //   cout<<"ERROR!!!!"<<endl;
  }
  //remove symbol table of Then block
  NamedValuesList.pop_back();

  if(generateBranchForThen)
    Builder.CreateBr(end_);

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
      Builder.CreateBr(end_);
    //remove symbol table of Else block
    NamedValuesList.pop_back();
  }

  if(generateBranchForThen | generateBranchForElse) //only generate if_end if if_then and if_else don't have a return stmt
  {
    TheFunction->insert(TheFunction->end(), end_);
    Builder.SetInsertPoint(end_);
    return ConstantPointerNull::get(PointerType::getUnqual(Type::getVoidTy(TheContext))); //return null pointer of void type
  }
  else
    return ret;
}

Value* WhileExprASTnode::codegen(){
  cout<<"While codegen\n";
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
    cout<<"Alloca cond\n";
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
    cout<<"Global cond\n";
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
  cout<<"COND TYPE: "<<currType<<endl;

  if(currType != "bool") //cast to bool type
  {
    // errs()<<"Semantic error: Expected type `bool` for the condition statement at line no. "<<Cond->getTok().lineNo<<" column no. "<<Cond->getTok().columnNo<<". Cannot cast from type `"<<currType<<"` to `bool`.\n";
    // return nullptr;
    if(currType == "float")
      cond = Builder.CreateFCmpONE(cond, ConstantFP::get(TheContext, APFloat((float) 0.0)),"float_bool_cast");   
    else if(currType == "int")
      cond = Builder.CreateICmpNE(cond, ConstantInt::get(Type::getInt32Ty(TheContext), 0, false),"int_bool_cast");
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
    cout<<"size of then:"<<Then.size()<<endl;
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
    Builder.CreateBr(cond_);
  NamedValuesList.pop_back();

  
  TheFunction->insert(TheFunction->end(), false_);
  Builder.SetInsertPoint(false_);
  return ConstantPointerNull::get(PointerType::getUnqual(Type::getVoidTy(TheContext))); //return null pointer of void type
 
}

Value* ReturnExprASTnode::codegen(){
  // return nullptr;
  cout<<"Return codegen\n";
  if(ReturnExpr == nullptr)
  {
    return Builder.CreateRetVoid();
  }
  Value* returnExpr = ReturnExpr->codegen();

  // cout<<"FUNC RETURN: "<<FuncReturnType<<endl;

  if(returnExpr == nullptr)
    return nullptr;

  if(auto *AI = dyn_cast<AllocaInst>(returnExpr))
  {
    returnExpr = Builder.CreateLoad(AI->getAllocatedType(),returnExpr,"load_temp");
  }

  string correctType = FuncReturnType;
  string actualType = "";
  if(returnExpr->getType()->isIntegerTy(32))
    actualType = "int";
  else if(returnExpr->getType()->isIntegerTy(1))
    actualType = "bool";
  else if(returnExpr->getType()->isFloatTy())
    actualType = "float";
  
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
    cout<<"Prototype codegen\n";
  // Make the function type:

  //bool isExtern = false;
  string ReturnType = "";
  string origName = getName();
  size_t space_pos = origName.find(" ");    
  if (space_pos != std::string::npos) 
  {
    ReturnType = origName.substr(0, space_pos);
  }

  origName = origName.substr(space_pos + 1);
  // cout<<"F: "<<Name<<endl;

  if(ReturnType == "extern") //for extern functions
  {
    //isExtern = true;
    size_t space_pos_2 = origName.find(" ");    
    if (space_pos_2 != std::string::npos) 
    {
      ReturnType = origName.substr(0, space_pos_2);
      origName = origName.substr(space_pos_2 + 1);
    }
  }

  cout<<"E: "<<Name<<"|"<<endl;

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
      //ignore "void" - not allowed to be in argument list for LLVM IR
      // else if(ArgType == "void")
      // {
      //   cout<<"yes"<<endl;
      //   ArgTypes.push_back(Type::getVoidTy(TheContext));
      // }
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

//Probably better to change FunctionAST node to have a type variable
//Or get first word from name to identify type
//  if(Name == "int main")
//   Name = "main";

 Function *F = Function::Create(FT, Function::ExternalLinkage, origName, TheModule.get());
 //Set names for all arguments.
 unsigned Idx = 0;
 for (auto &Arg : F->args())
 {
   Arg.setName(getArgName(Idx));
   Idx++;
 }

 return F;
  // return nullptr;
}

Value* GlobalVariableAST::codegen(){
  cout<<"Global variable codgen\n";
  // return nullptr;
  bool isConstant = false; //idk if this matters for global variables - ask tutors if it causes problems - false sets to global, true sets it to constant
  string ty = getType();
  int alignSize = 4;
  Type* t = nullptr;

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

  cout<<"global proceed"<<endl;

  GlobalVariable* g = new GlobalVariable(*(TheModule.get()),t,isConstant,GlobalValue::CommonLinkage,Constant::getNullValue(t));
  g->setAlignment(MaybeAlign(alignSize));
  g->setName(Val);

  if(GlobalVariables.insert({Val,g}).second == false)
  {
    string existTy = "";
    if(GlobalVariables[Val]->getValueType()->isIntegerTy(32))
      existTy = "int";
    else if(GlobalVariables[Val]->getValueType()->isIntegerTy(1))
      existTy = "bool";
    else if(GlobalVariables[Val]->getValueType()->isFloatTy())
      existTy = "float";
    
    errs()<<"Semantic error: Redefinition of global variable "<<Val<<" with different type "<<ty<<". Variable "<<Val<<" of type "<<existTy<<" already exists.\n";
    return nullptr;
  }
  return g;
}

Function* FunctionAST::codegen(){
  cout<<"Function codegen\n";
  Function *TheFunction = TheModule->getFunction(Proto->getName());

if (!TheFunction)
  TheFunction = Proto->codegen();
if (!TheFunction)
  return nullptr;
 BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
 Builder.SetInsertPoint(BB);
 // Record the function arguments in the NamedValues map.
//  NamedValues.clear();
 std::map<std::string, AllocaInst*> NamedValues;

 for (auto &Arg : TheFunction->args()) //loading each argument in the symbol table
 {
  // if(!Arg.getType()->isVoidTy()) //ignore void
  // {
  //   AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName().str()); //creating an alloca for each argument
  //   Builder.CreateStore(&Arg, Alloca);
  //   NamedValues[std::string(Arg.getName())] = Alloca;
  // }
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

cout<<"ARgs "<<NamedValues.size()<<"stored now"<<endl;
  
 NamedValuesList.push_back(NamedValues);

 string returnType = "";
 bool returnSet = false;
  if(TheFunction->getReturnType()->isIntegerTy(32))
    returnType = "int"; //    returnType = "int (i32)";
  else if(TheFunction->getReturnType()->isIntegerTy(1))
    returnType = "bool"; // returnType = "bool (i1)";
  else if(TheFunction->getReturnType()->isFloatTy())
    returnType = "float"; //    returnType = "float (float)";

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

  Value *RetVal = Body.at(i)->codegen(); //go through all ASTnodes in this function body and run codegen()
  if(!RetVal)
  {
    return nullptr;
  }
  
  if((i == Body.size()-1) | isa<ReturnInst>(RetVal)) //check last ASTnode of function body or check for a return expression that has been made in the body early
  {
    // if((TheFunction->getReturnType()->isVoidTy()))
    // {
    //   Builder.CreateRetVoid(); //create void return for void functions
    // }
    // else
    // { 
      if(auto RT = dyn_cast<ReturnInst>(RetVal)) //if last statement is a return statement
      {
        // Value* returnVal = RT->getReturnValue(); 
        // //checking if the return statement is the correct type - matches the return type of the function
        // if(returnVal->getType()->isIntegerTy(32))
        // {
        //   if(returnType != "int")
        //   {
        //     cout<<"Semantic Error: Incorrect return value of type `int` used in function: "<<Proto->getName()<<".\nExpected return type `"<<returnType<<"`.\n";
        //     return nullptr;
        //   }
        // }
        // if(returnVal->getType()->isIntegerTy(1)) 
        // {
        //   if(returnType != "bool")
        //   {
        //     cout<<"Semantic Error: Incorrect return value of type `bool` used in function: "<<Proto->getName()<<".\nExpected return type `"<<returnType<<"`.\n";
        //     return nullptr;
        //   }
        // }
        // if(returnVal->getType()->isFloatTy())
        // {
        //   if(returnType != "float")
        //   {
        //     cout<<"Semantic Error: Incorrect return value of type `float` used in function: "<<Proto->getName()<<".\nExpected return type `"<<returnType<<"`.\n";
        //     return nullptr;
        //   }
        // }
        returnSet = true; //make sure last line is a return stmt
      }
      else //return statement not found
      {
        cout<<"Semantic Error: Return statement of type `"<<returnType<<"` expected in function: "<<Proto->getName()<<".\n";
        return nullptr;
      }
  
  }
  
}

// Validate the generated code, checking
//for consistency.
 verifyFunction(*TheFunction);

 NamedValuesList.pop_back(); //remove NamedValues of this function from the vector

 return TheFunction;

 // return nullptr;
}

//===----------------------------------------------------------------------===//
// AST Printer
//===----------------------------------------------------------------------===//

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const unique_ptr<TopLevelASTnode> &ast) { //changes made
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
      // llvm::outs() << "  |-> " << root[i] << "\n";
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

  //JUST FOR TESTING - REMOVE LATER
  // ConstantInt *zero = ConstantInt::get(IntegerType::getInt32Ty(TheContext), 0);
  // Builder.CreateRet(zero);

  // TheModule->print(errs(), nullptr); // print IR to terminal
  TheModule->print(dest, nullptr);
  //********************* End printing final IR ****************************

  fclose(pFile); // close the file that contains the code that was parsed
  return 0;
}