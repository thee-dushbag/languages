#ifndef _CLOX_COMPILER_H
#define _CLOX_COMPILER_H

#include "common.h"
#include "scanner.h"
#include "chunk.h"
#include "object.h"

CLOX_BEG_DECLS

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // > < >= <=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // - !
  PREC_CALL,       // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParserRule;

typedef struct {
  Token current;
  Token previous;
  bool had_error; // Signifies a compilation error
  bool panic_mode; // Helps in resynchronizing after an error
} Parser;

void expr_binary();
void expr_unary();
void expression();
void expr_grouping();
void expr_number();
void literal();
void expr_string();

#define TKPREC_RULE(tktp, prefix, infix, precedence) [TOKEN##tktp] = { prefix, infix, PREC##precedence }

ParserRule tkprec_rules[] = {
  TKPREC_RULE(_LEFT_PAREN,     expr_grouping,  NULL,         _NONE),
  TKPREC_RULE(_RIGHT_PAREN,    NULL,           NULL,         _NONE),
  TKPREC_RULE(_LEFT_BRACE,     NULL,           NULL,         _NONE),
  TKPREC_RULE(_RIGHT_BRACE,    NULL,           NULL,         _NONE),
  TKPREC_RULE(_COMMA,          NULL,           NULL,         _NONE),
  TKPREC_RULE(_DOT,            NULL,           NULL,         _NONE),
  TKPREC_RULE(_MINUS,          expr_unary,     expr_binary,  _TERM),
  TKPREC_RULE(_PLUS,           NULL,           expr_binary,  _TERM),
  TKPREC_RULE(_SEMICOLON,      NULL,           NULL,         _NONE),
  TKPREC_RULE(_SLASH,          NULL,           expr_binary,  _FACTOR),
  TKPREC_RULE(_STAR,           NULL,           expr_binary,  _FACTOR),
  TKPREC_RULE(_BANG,           expr_unary,     NULL,         _NONE),
  TKPREC_RULE(_BANG_EQUAL,     NULL,           expr_binary,  _EQUALITY),
  TKPREC_RULE(_EQUAL,          NULL,           NULL,         _NONE),
  TKPREC_RULE(_EQUAL_EQUAL,    NULL,           expr_binary,  _EQUALITY),
  TKPREC_RULE(_GREATER,        NULL,           expr_binary,  _COMPARISON),
  TKPREC_RULE(_GREATER_EQUAL,  NULL,           expr_binary,  _COMPARISON),
  TKPREC_RULE(_LESS,           NULL,           expr_binary,  _COMPARISON),
  TKPREC_RULE(_LESS_EQUAL,     NULL,           expr_binary,  _COMPARISON),
  TKPREC_RULE(_IDENTIFIER,     NULL,           NULL,         _NONE),
  TKPREC_RULE(_STRING,         expr_string,    NULL,         _NONE),
  TKPREC_RULE(_NUMBER,         expr_number,    NULL,         _NONE),
  TKPREC_RULE(_AND,            NULL,           NULL,         _NONE),
  TKPREC_RULE(_CLASS,          NULL,           NULL,         _NONE),
  TKPREC_RULE(_ELSE,           NULL,           NULL,         _NONE),
  TKPREC_RULE(_FALSE,          literal,        NULL,         _NONE),
  TKPREC_RULE(_FOR,            NULL,           NULL,         _NONE),
  TKPREC_RULE(_FUN,            NULL,           NULL,         _NONE),
  TKPREC_RULE(_IF,             NULL,           NULL,         _NONE),
  TKPREC_RULE(_NIL,            literal,        NULL,         _NONE),
  TKPREC_RULE(_OR,             NULL,           NULL,         _NONE),
  TKPREC_RULE(_PRINT,          NULL,           NULL,         _NONE),
  TKPREC_RULE(_RETURN,         NULL,           NULL,         _NONE),
  TKPREC_RULE(_SUPER,          NULL,           NULL,         _NONE),
  TKPREC_RULE(_THIS,           NULL,           NULL,         _NONE),
  TKPREC_RULE(_TRUE,           literal,        NULL,         _NONE),
  TKPREC_RULE(_VAR,            NULL,           NULL,         _NONE),
  TKPREC_RULE(_WHILE,          NULL,           NULL,         _NONE),
  TKPREC_RULE(_ERROR,          NULL,           NULL,         _NONE),
  TKPREC_RULE(_EOF,            NULL,           NULL,         _NONE)
};

#undef TKPREC_RULE

ParserRule *get_rule(TokenType tktype) {
  return &tkprec_rules[tktype];
}

void parse_precedence(Precedence);

Parser parser;
Chunk *active_chunk;

void _error_at(Token *token, const char *message) {
  if (parser.panic_mode) return;
  parser.panic_mode = true;
  parser.had_error = true;
  fprintf(stderr, "[line %d] Error", token->line);
  switch (token->type) {
  case TOKEN_EOF: fprintf(stderr, " at end"); break;
  case TOKEN_ERROR: break;
  default: fprintf(stderr, " at '%.*s'", token->length, token->start);
  }
  fprintf(stderr, ": %s\n", message);
}

void error(const char *message) {
  _error_at(&parser.previous, message);
}

void error_at(const char *message) {
  _error_at(&parser.current, message);
}

void compiler_advance() {
  parser.previous = parser.current;
  for (;;) {
    parser.current = scan();
#ifdef CLOX_SCAN_TRACE
    if (!is_at_end() || parser.previous.type != TOKEN_EOF)
      token_print(&parser.current);
#endif
    if (parser.current.type != TOKEN_ERROR) break;
    error_at(parser.current.start);
  }
}

void consume(TokenType type, const char *message) {
  if (parser.current.type == type) compiler_advance();
  else error_at(message);
}

Chunk *current_chunk() {
  return active_chunk;
}

void emit_byte(uint8_t byte) {
  chunk_append(current_chunk(), byte, parser.previous.line);
}

void emit_bytes(uint8_t b1, uint8_t b2) {
  emit_byte(b1); emit_byte(b2);
}

void emit_return() {
  emit_byte(OP_RETURN);
}

uint8_t make_constant(Value constant) {
  int location = chunk_cappend(current_chunk(), constant);
  if (location <= UINT8_MAX) return (uint8_t)location;
  error("Too many constants in one chunk.");
  return 0;
}

void emit_constant(Value constant) {
  emit_bytes(OP_CONSTANT, make_constant(constant));
}

void literal() {
  switch (parser.previous.type) {
  case TOKEN_NIL: emit_byte(OP_NIL);     break;
  case TOKEN_TRUE: emit_byte(OP_TRUE);   break;
  case TOKEN_FALSE: emit_byte(OP_FALSE); break;
  }
}

void expr_number() {
  double constant = strtod(parser.previous.start, NULL);
  emit_constant(NUMBER_VAL(constant));
}

void expr_string() {
  emit_constant(OBJECT_VAL(copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

void expr_grouping() {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void expr_binary() {
  TokenType optype = parser.previous.type;
  ParserRule *rule = get_rule(optype);
  parse_precedence((Precedence)(rule->precedence + 1));
  switch (optype) {
  case TOKEN_LESS: emit_byte(OP_LESS);                   break;
  case TOKEN_PLUS:  emit_byte(OP_ADD);                   break;
  case TOKEN_SLASH: emit_byte(OP_DIVIDE);                break;
  case TOKEN_STAR:  emit_byte(OP_MULTIPLY);              break;
  case TOKEN_MINUS: emit_byte(OP_SUBTRACT);              break;
  case TOKEN_GREATER: emit_byte(OP_GREATER);             break;
  case TOKEN_EQUAL_EQUAL: emit_byte(OP_EQUAL);           break;
  case TOKEN_BANG_EQUAL: emit_bytes(OP_EQUAL, OP_NOT);   break;
  case TOKEN_LESS_EQUAL: emit_bytes(OP_GREATER, OP_NOT); break;
  case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
  }
}

void expr_unary() {
  TokenType optype = parser.previous.type;
  parse_precedence(PREC_UNARY);
  switch (optype) {
  case TOKEN_BANG: emit_byte(OP_NOT);     break;
  case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
  }
}

void parse_precedence(Precedence precedence) {
  compiler_advance();
  ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
  if (prefix_rule != NULL) prefix_rule();
  else error("Expect expression.");
  while (precedence <= get_rule(parser.current.type)->precedence) {
    compiler_advance();
    get_rule(parser.previous.type)->infix();
  }
}

void expression() {
  parse_precedence(PREC_ASSIGNMENT);
}

void compiler_init(Chunk *chunk) {
  parser.panic_mode = false;
  parser.had_error = false;
  active_chunk = chunk;
}

void compiler_delete() {
  emit_return();
}

bool compile(const char *source, Chunk *chunk) {
  compiler_init(chunk);
  scanner_init(source);
  compiler_advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
  compiler_delete();
  return !parser.had_error;
}

CLOX_END_DECLS

#endif //_CLOX_COMPILER_H
