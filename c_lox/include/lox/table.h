#ifndef _CLOX_TABLE_H
#define _CLOX_TABLE_H

#include "common.h"
#include "value.h"
#include "memory.h"

#define TABLE_MAX_LOAD 0.75f

typedef struct {
  ObjectString* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;

#include "object.h"

void table_print(Table*);
void entry_print(Entry*);

void table_init(Table* table) {
  table->entries = NULL;
  table->capacity = 0;
  table->count = 0;
}

void table_delete(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
}

uint64_t fnv1a_algo(const char* chars, int length) {
  uint64_t hash = 2'166'136'261u;
  for ( int idx = 0; idx < length; ++idx )
    hash *= (hash ^ chars[idx]);
  return hash;
}

uint64_t my_hash(const char* chars, int length) {
  uint64_t hash = length;
  // printf("Hashing: \"%.*s\"\n", length, chars);
  for ( int idx = 0; idx < length; ++idx ) {
    // printf(" hash: %u\n", hash);
    hash *= (hash << 1) ^ chars[idx];
  }
  return hash;
}

uint64_t hash_string(const char* chars, int length) {
#ifndef HASH_FUNCTION
# define HASH_FUNCTION my_hash
#endif
  return HASH_FUNCTION(chars, length);
}

Entry* entry_find(Entry* const entries, int cap, ObjectString* const key) {
  uint64_t idx = key->hash % cap;
  Entry* tombstone = NULL, * entry = entries + idx;
  for ( ;; idx = (idx + 1) % cap, entry = entries + idx )
    if ( entry->key == key ) return entry;
    else if (entry->key && !memcmp(key->chars, entry->key->chars, key->length)) return entry;
    else if ( entry->key == NULL )
      if ( IS_NIL(entry->value) )
        return tombstone ? tombstone : entry;
      else if ( tombstone == NULL ) tombstone = entry;
  return NULL; // UNREACHABLE
}

ObjectString* table_find_string(Table* table, const char* chars, int length, uint64_t hash) {
  if ( table->count == 0 ) return NULL;
  uint64_t index = hash % table->capacity;
  Entry* entry = table->entries + index;
  for ( ;; index = (index + 1) % table->capacity,
    entry = table->entries + index )
    if ( entry->key == NULL && IS_NIL(entry->value) ) return NULL;
    else if ( entry->key->length == length &&
      entry->key->hash == hash &&
      !memcmp(entry->key->chars, chars, length) )
      return entry->key;
  printf("DidNotFindInternFor: \"%.*s\"\n", length, chars);
  return NULL;
}

void table_adjust_cap(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity), * entry = table->entries;
  const int old_capacity = table->capacity;
  *table = (Table){ 0, capacity, entries };
  for ( ; capacity > 0; --capacity, ++entries )
    *entries = (Entry){ .key = NULL, .value = NIL_VAL };
  entries = entry;
  for ( capacity = old_capacity; capacity > 0; --capacity, ++entry )
    if ( entry->key != NULL ) {
      *entry_find(table->entries, old_capacity, entry->key)
        = (Entry){ .key = entry->key, .value = entry->value };
      ++table->count;
    }
  FREE_ARRAY(Entry, entries, old_capacity);
}

bool table_set(Table* table, ObjectString* key, Value value) {
  if ( table->count + 1 > table->capacity * TABLE_MAX_LOAD )
    table_adjust_cap(table, GROW_CAPACITY(table->capacity));
  Entry* entry = entry_find(table->entries, table->capacity, key);
  bool new_entry = entry->key == NULL;
  if ( new_entry && IS_NIL(entry->value) ) ++table->count;
  *entry = (Entry){ .key = key, .value = value };
  return new_entry;
}

void table_concat(Table* to, Table* from) {
  Entry* c = from->entries, * e = c + from->capacity;
  for ( ; c != e; ++c ) if ( c->key != NULL )
    table_set(to, c->key, c->value);
}

bool table_get(Table* table, ObjectString* key, Value* value) {
  if ( table->count == 0 ) return false;
  Entry* target = entry_find(table->entries, table->capacity, key);
  if ( !target->key ) return false;
  *value = target->value;
  return true;
}

bool table_del(Table* table, ObjectString* key) {
  if ( table->count == 0 ) return false;
  Entry* entry = entry_find(table->entries, table->capacity, key);
  if ( !entry->key ) return false;
  *entry = (Entry){ .key = NULL, .value = BOOL_VAL(true) };
  return true;
}

void entry_print(Entry* entry) {
  value_oprint(OBJECT_VAL(entry->key));
  printf(": ");
  value_print(entry->value);
}

void table_print(Table* table) {
  Entry* entry = table->entries,
    * const stop = entry + table->capacity;
  putchar('{');
  for ( ; entry != stop; ++entry )
    if ( entry->key != NULL )
      entry_print(entry), printf(", ");
  putchar('}');
}

#endif //_CLOX_TABLE_H
