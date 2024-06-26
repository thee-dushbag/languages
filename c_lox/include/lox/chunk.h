#ifndef _CLOX_CHUNK_H
#define _CLOX_CHUNK_H

#include "memory.h"
#include "common.h"
#include "value.h"

CLOX_BEG_DECLS

typedef enum {
  OP_CLOSE_UPVALUE,
  OP_JUMP_IF_FALSE,
  OP_DEFINE_GLOBAL,
  OP_SUPER_INVOKE,
  OP_SET_PROPERTY,
  OP_GET_PROPERTY,
  OP_SET_UPVALUE,
  OP_GET_UPVALUE,
  OP_SET_GLOBAL,
  OP_GET_GLOBAL,
  OP_GET_SUPER,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_CONSTANT,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_INHERIT,
  OP_CLOSURE,
  OP_GREATER,
  OP_INVOKE,
  OP_METHOD,
  OP_DIVIDE,
  OP_RETURN,
  OP_NEGATE,
  OP_FALSE,
  OP_EQUAL,
  OP_PRINT,
  OP_CLASS,
  OP_TRUE,
  OP_LESS,
  OP_JUMP,
  OP_LOOP,
  OP_CALL,
  OP_NIL,
  OP_ADD,
  OP_NOT,
  OP_POP,
} OpCode; // Instruction Bytes

#define INSTCS(inst) case OP##inst: return #inst + 1

const char* inst_print(uint8_t byte) {
  switch ( byte ) {
    INSTCS(_CLOSE_UPVALUE);
    INSTCS(_JUMP_IF_FALSE);
    INSTCS(_DEFINE_GLOBAL);
    INSTCS(_SUPER_INVOKE);
    INSTCS(_SET_PROPERTY);
    INSTCS(_GET_PROPERTY);
    INSTCS(_SET_UPVALUE);
    INSTCS(_GET_UPVALUE);
    INSTCS(_SET_GLOBAL);
    INSTCS(_GET_GLOBAL);
    INSTCS(_GET_SUPER);
    INSTCS(_GET_LOCAL);
    INSTCS(_SET_LOCAL);
    INSTCS(_CONSTANT);
    INSTCS(_SUBTRACT);
    INSTCS(_MULTIPLY);
    INSTCS(_INHERIT);
    INSTCS(_GREATER);
    INSTCS(_CLOSURE);
    INSTCS(_INVOKE);
    INSTCS(_METHOD);
    INSTCS(_DIVIDE);
    INSTCS(_RETURN);
    INSTCS(_NEGATE);
    INSTCS(_FALSE);
    INSTCS(_EQUAL);
    INSTCS(_PRINT);
    INSTCS(_CLASS);
    INSTCS(_TRUE);
    INSTCS(_LESS);
    INSTCS(_JUMP);
    INSTCS(_LOOP);
    INSTCS(_CALL);
    INSTCS(_NIL);
    INSTCS(_ADD);
    INSTCS(_NOT);
    INSTCS(_POP);
  }
  return "<UNKOWN_INST>";
}

#undef INSTCS

typedef struct {
  ValueArray constants;
  uint8_t* code; // Compiled Bytecode: from compile
  int capacity;
  int* lines;
  int count;
} Chunk;

void chunk_init(Chunk* chunk) {
  value_init(&chunk->constants);
  chunk->capacity = 0;
  chunk->lines = NULL;
  chunk->code = NULL;
  chunk->count = 0;
}

void chunk_append(Chunk* chunk, uint8_t byte, int line) {
  if ( chunk->capacity < chunk->count + 1 ) {
    int capacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(capacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, capacity, chunk->capacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, capacity, chunk->capacity);
  }
  chunk->lines[chunk->count] = line;
  chunk->code[chunk->count] = byte;
  chunk->count++;
}

void chunk_delete(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  value_delete(&chunk->constants);
}

int chunk_cappend(Chunk* chunk, Value value) {
  value_append(&chunk->constants, value);
  return chunk->constants.count - 1;
}

CLOX_END_DECLS

#endif //_CLOX_CHUNK_H
