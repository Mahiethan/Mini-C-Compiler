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
    return returnTok("0", EOF_TOK);
  }

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  std::string s(1, ThisChar);
  LastChar = getc(pFile);
  columnNo++;
  return returnTok(s, int(ThisChar));
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

  if (tok_buffer.size() == 0)
    tok_buffer.push_back(gettok());

  TOKEN temp = tok_buffer.front();
  tok_buffer.pop_front();

  return CurTok = temp;
}

static void putBackToken(TOKEN tok) { tok_buffer.push_front(tok); }

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//

/// ASTnode - Base class for all AST nodes.
class ASTnode {
public:
  virtual ~ASTnode() {}
  virtual Value *codegen() = 0;
  virtual std::string to_string() const {return "";};
};

/// IntASTnode - Class for integer literals like 1, 2, 10,
class IntASTnode : public ASTnode {
  int Val;
  TOKEN Tok;
  std::string Name;

public:
  IntASTnode(TOKEN tok, int val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override;
  // virtual std::string to_string() const override {
  // return a sting representation of this AST node
  //};
};

/* add other AST nodes as nessasary */

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
    return match(COMMA) & p_arg_list();
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
    return match(LPAR) & p_args() & match(RPAR);
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
  cout<<CurTok.type<<endl;
  if(CurTok.type == LPAR)
  {
    return match(LPAR) & p_expr() & match(RPAR);
  }
  else if(CurTok.type == INT_LIT)
  {
    return match(INT_LIT);
  }
  else if(CurTok.type == FLOAT_LIT)
  {
    return match(FLOAT_LIT);
  }
  else if(CurTok.type == BOOL_LIT)
  {
    return match(BOOL_LIT);
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
    return match(MINUS) & p_rval_one();
  }
  else if(CurTok.type == NOT)
  {
    return match(NOT) & p_rval_one();
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
    return match(ASTERIX) & p_rval_two() & p_rval_three_prime();
  }
  else if(CurTok.type == DIV)
  {
    return match(DIV) & p_rval_two() & p_rval_three_prime();
  }
  else if(CurTok.type == MOD)
  {
    return match(MOD) & p_rval_two() & p_rval_three_prime();
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
  cout<<CurTok.type<<endl;
  if(CurTok.type == PLUS)
  {
    return match(PLUS) & p_rval_three() & p_rval_four_prime();
  }
  else if(CurTok.type == MINUS)
  {
    return match(MINUS) & p_rval_three() & p_rval_four_prime();
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
    return match(LE) & p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == LT)
  {
    return match(LT) & p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == GE)
  {
    return match(GE) & p_rval_four() & p_rval_five_prime();
  }
  else if(CurTok.type == GT)
  {
    return match(GT) & p_rval_four() & p_rval_five_prime();
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
    return match(EQ) & p_rval_five() & p_rval_six_prime();
  }
  else if(CurTok.type == NE)
  {
    return match(NE) & p_rval_five() & p_rval_six_prime();
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
    return match(AND) & p_rval_six() & p_rval_seven_prime();
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
    return match(OR) & p_rval_seven() & p_rval_eight_prime();
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
    return p_expr() & match(SC);
  }
  else
  {
    return false;
  }
}

bool p_return_stmt()
{
  cout<<"return_stmt"<<endl;
  return match(RETURN) & p_return_stmt_prime();
}

bool p_expr()
{
  cout<<"expr"<<endl;
  if(CurTok.type == IDENT)
  {
    return match(IDENT) & match(ASSIGN) & p_expr();
  }
  else if(contains(CurTok.type,FIRST_rval_eight))
  {
    return p_rval_eight();
  }
  else
  {
    return false;
  }
}

bool p_else_stmt()
{
  cout<<"else_stmt"<<endl;
  if(CurTok.type == ELSE)
  {
    return match(ELSE) & p_block();
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
  return match(IF) & match(LPAR) & p_expr() & match(RPAR) & p_block() & p_else_stmt();
}

bool p_while_stmt()
{
  return match(WHILE) & match(LPAR) & p_expr() & match(RPAR) & p_stmt();
}

bool p_expr_stmt()
{
  cout<<"expr_stmt"<<endl;
  if(contains(CurTok.type, FIRST_expr))
  {
    return p_expr();
  }
  else if(CurTok.type == SC)
  {
    return match(SC);
  }
  else
  {
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
  if(contains(CurTok.type, FIRST_stmt))
  {
    return p_stmt() & p_stmt_list();
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
  return p_var_type() & match(IDENT) & match(SC);
}

bool p_local_decls()
{
   cout<<"local_decls"<<endl;
   if(contains(CurTok.type, FIRST_local_decl))
   {
      return p_local_decl() & p_local_decls();
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
  return p_var_type() & match(IDENT);
}

bool p_block()
{
  cout<<"block"<<endl;
  return match(LBRA) & p_local_decls() & p_stmt_list() & match(RBRA);
}

bool p_var_type()
{
  cout<<"var_type"<<endl;
  if(CurTok.type == INT_TOK)
  {
    return match(INT_TOK);
  }
  else if(CurTok.type == FLOAT_TOK)
  {
    return match(FLOAT_TOK);
  }
  else if(CurTok.type == BOOL_TOK)
  {
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
    return match(COMMA) & p_param_list();
  else
  {
    if(contains(CurTok.type,FOLLOW_param_list_prime))
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
    return match(SC);
  }
  else if(CurTok.type == LPAR)
  {
    return match(LPAR) & p_params() & match(RPAR) & p_block();
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
  return match(EXTERN) & p_type_spec() & match(IDENT) & match(LPAR) & p_params() & match(RPAR) & match(SC);
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
    return p_var_type() & match(IDENT) & p_decl_prime();
  }
  else if(CurTok.type == VOID_TOK)
  {
    return match(VOID_TOK) & match(IDENT) & match(LPAR) & p_params() & match(RPAR) & p_block();
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
static void parser() {
  // add body
  if(p_program())
  {
    if(CurTok.type == EOF_TOK)
    {
      cout<<"Parsing successful"<<endl;
    }
  }
  else
  {
    cout<<"Parsing failed"<<endl;
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
                                     const ASTnode &ast) {
  os << ast.to_string();
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
  // while (CurTok.type != EOF_TOK) {
  //   fprintf(stderr, "Token: %s with type %d\n", CurTok.lexeme.c_str(),
  //           CurTok.type);
  //   getNextToken();
  // }
  // fprintf(stderr, "Lexer Finished\n");

  // Make the module, which holds all the code.
  TheModule = std::make_unique<Module>("mini-c", TheContext);

  // Run the parser now.
  parser();
  //fprintf(stderr, "Parsing Finished\n");

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
