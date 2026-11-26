#ifndef CJSON_MINI_H
#define CJSON_MINI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "library.h"

// Dynamic string buffer for JSON building
typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} JsonBuffer;

void json_buf_init(JsonBuffer* buf, size_t initial_cap);
void json_buf_append(JsonBuffer* buf, const char* str);
void json_buf_append_escaped(JsonBuffer* buf, const char* str);
void json_buf_free(JsonBuffer* buf);

// Serialization helpers
char* json_serialize_books(Book* books, int count);
char* json_serialize_book(Book* b);
char* json_serialize_members(Member* members, int count);
char* json_serialize_member(Member* m);
char* json_serialize_transactions(Transaction* trans, int count);
char* json_serialize_transaction(Transaction* t);
char* json_serialize_stats(LibraryStats* stats);
char* json_serialize_message(bool success, const char* message, int id);

// Parsing helpers
bool json_get_string(const char* json, const char* key, char* out_val, size_t max_len);
int json_get_int(const char* json, const char* key, int default_val);
double json_get_double(const char* json, const char* key, double default_val);

#endif // CJSON_MINI_H
