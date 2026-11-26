#include "../include/cJSON_mini.h"
#include <ctype.h>

void json_buf_init(JsonBuffer* buf, size_t initial_cap) {
    if (initial_cap < 256) initial_cap = 256;
    buf->data = (char*)malloc(initial_cap);
    buf->data[0] = '\0';
    buf->size = 0;
    buf->capacity = initial_cap;
}

static void json_buf_ensure(JsonBuffer* buf, size_t needed) {
    if (buf->size + needed + 1 >= buf->capacity) {
        size_t new_cap = (buf->capacity * 2) + needed + 256;
        char* new_data = (char*)realloc(buf->data, new_cap);
        if (new_data) {
            buf->data = new_data;
            buf->capacity = new_cap;
        }
    }
}

void json_buf_append(JsonBuffer* buf, const char* str) {
    if (!str) return;
    size_t len = strlen(str);
    json_buf_ensure(buf, len);
    memcpy(buf->data + buf->size, str, len);
    buf->size += len;
    buf->data[buf->size] = '\0';
}

void json_buf_append_escaped(JsonBuffer* buf, const char* str) {
    if (!str) return;
    for (size_t i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        if (c == '"') {
            json_buf_append(buf, "\\\"");
        } else if (c == '\\') {
            json_buf_append(buf, "\\\\");
        } else if (c == '\b') {
            json_buf_append(buf, "\\b");
        } else if (c == '\f') {
            json_buf_append(buf, "\\f");
        } else if (c == '\n') {
            json_buf_append(buf, "\\n");
        } else if (c == '\r') {
            json_buf_append(buf, "\\r");
        } else if (c == '\t') {
            json_buf_append(buf, "\\t");
        } else if ((unsigned char)c < 32) {
            char hex[8];
            snprintf(hex, sizeof(hex), "\\u%04x", (unsigned char)c);
            json_buf_append(buf, hex);
        } else {
            char temp[2] = {c, '\0'};
            json_buf_append(buf, temp);
        }
    }
}

void json_buf_free(JsonBuffer* buf) {
    if (buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    buf->size = 0;
    buf->capacity = 0;
}

static void append_book_json(JsonBuffer* buf, Book* b) {
    char temp[128];
    json_buf_append(buf, "{\"id\":");
    snprintf(temp, sizeof(temp), "%d,", b->id);
    json_buf_append(buf, temp);

    json_buf_append(buf, "\"title\":\"");
    json_buf_append_escaped(buf, b->title);
    json_buf_append(buf, "\",\"author\":\"");
    json_buf_append_escaped(buf, b->author);
    json_buf_append(buf, "\",\"isbn\":\"");
    json_buf_append_escaped(buf, b->isbn);
    json_buf_append(buf, "\",\"category\":\"");
    json_buf_append_escaped(buf, b->category);
    json_buf_append(buf, "\",\"pub_year\":");
    snprintf(temp, sizeof(temp), "%d,\"total_copies\":%d,\"available_copies\":%d,\"shelf\":\"",
             b->pub_year, b->total_copies, b->available_copies);
    json_buf_append(buf, temp);
    json_buf_append_escaped(buf, b->shelf);
    json_buf_append(buf, "\"}");
}

char* json_serialize_books(Book* books, int count) {
    JsonBuffer buf;
    json_buf_init(&buf, 4096);
    json_buf_append(&buf, "[");
    for (int i = 0; i < count; i++) {
        if (i > 0) json_buf_append(&buf, ",");
        append_book_json(&buf, &books[i]);
    }
    json_buf_append(&buf, "]");
    return buf.data;
}

char* json_serialize_book(Book* b) {
    JsonBuffer buf;
    json_buf_init(&buf, 512);
    append_book_json(&buf, b);
    return buf.data;
}

static void append_member_json(JsonBuffer* buf, Member* m) {
    char temp[128];
    json_buf_append(buf, "{\"id\":");
    snprintf(temp, sizeof(temp), "%d,", m->id);
    json_buf_append(buf, temp);

    json_buf_append(buf, "\"name\":\"");
    json_buf_append_escaped(buf, m->name);
    json_buf_append(buf, "\",\"email\":\"");
    json_buf_append_escaped(buf, m->email);
    json_buf_append(buf, "\",\"phone\":\"");
    json_buf_append_escaped(buf, m->phone);
    json_buf_append(buf, "\",\"type\":\"");
    json_buf_append_escaped(buf, m->type);
    json_buf_append(buf, "\",\"joined_date\":\"");
    json_buf_append_escaped(buf, m->joined_date);
    json_buf_append(buf, "\",\"status\":\"");
    json_buf_append_escaped(buf, m->status);
    json_buf_append(buf, "\",\"books_borrowed_count\":");
    snprintf(temp, sizeof(temp), "%d}", m->books_borrowed_count);
    json_buf_append(buf, temp);
}

char* json_serialize_members(Member* members, int count) {
    JsonBuffer buf;
    json_buf_init(&buf, 2048);
    json_buf_append(&buf, "[");
    for (int i = 0; i < count; i++) {
        if (i > 0) json_buf_append(&buf, ",");
        append_member_json(&buf, &members[i]);
    }
    json_buf_append(&buf, "]");
    return buf.data;
}

char* json_serialize_member(Member* m) {
    JsonBuffer buf;
    json_buf_init(&buf, 512);
    append_member_json(&buf, m);
    return buf.data;
}

static void append_transaction_json(JsonBuffer* buf, Transaction* t) {
    char temp[256];
    json_buf_append(buf, "{\"id\":");
    snprintf(temp, sizeof(temp), "%d,\"book_id\":%d,\"member_id\":%d,",
             t->id, t->book_id, t->member_id);
    json_buf_append(buf, temp);

    json_buf_append(buf, "\"book_title\":\"");
    json_buf_append_escaped(buf, t->book_title);
    json_buf_append(buf, "\",\"member_name\":\"");
    json_buf_append_escaped(buf, t->member_name);
    json_buf_append(buf, "\",\"issue_date\":\"");
    json_buf_append_escaped(buf, t->issue_date);
    json_buf_append(buf, "\",\"due_date\":\"");
    json_buf_append_escaped(buf, t->due_date);
    json_buf_append(buf, "\",\"return_date\":\"");
    json_buf_append_escaped(buf, t->return_date);
    json_buf_append(buf, "\",\"fine_amount\":");
    snprintf(temp, sizeof(temp), "%.2f,\"status\":\"", t->fine_amount);
    json_buf_append(buf, temp);
    json_buf_append_escaped(buf, t->status);
    json_buf_append(buf, "\"}");
}

char* json_serialize_transactions(Transaction* trans, int count) {
    JsonBuffer buf;
    json_buf_init(&buf, 4096);
    json_buf_append(&buf, "[");
    for (int i = 0; i < count; i++) {
        if (i > 0) json_buf_append(&buf, ",");
        append_transaction_json(&buf, &trans[i]);
    }
    json_buf_append(&buf, "]");
    return buf.data;
}

char* json_serialize_transaction(Transaction* t) {
    JsonBuffer buf;
    json_buf_init(&buf, 512);
    append_transaction_json(&buf, t);
    return buf.data;
}

char* json_serialize_stats(LibraryStats* stats) {
    JsonBuffer buf;
    json_buf_init(&buf, 512);
    char temp[512];
    snprintf(temp, sizeof(temp),
        "{\"total_books\":%d,\"total_titles\":%d,\"available_copies\":%d,"
        "\"borrowed_copies\":%d,\"total_members\":%d,\"active_loans\":%d,"
        "\"overdue_loans\":%d,\"total_fines\":%.2f}",
        stats->total_books, stats->total_titles, stats->available_copies,
        stats->borrowed_copies, stats->total_members, stats->active_loans,
        stats->overdue_loans, stats->total_fines);
    json_buf_append(&buf, temp);
    return buf.data;
}

char* json_serialize_message(bool success, const char* message, int id) {
    JsonBuffer buf;
    json_buf_init(&buf, 256);
    char temp[128];
    json_buf_append(&buf, "{\"success\":");
    json_buf_append(&buf, success ? "true," : "false,");
    json_buf_append(&buf, "\"message\":\"");
    json_buf_append_escaped(&buf, message ? message : "");
    json_buf_append(&buf, "\",\"id\":");
    snprintf(temp, sizeof(temp), "%d}", id);
    json_buf_append(&buf, temp);
    return buf.data;
}

static const char* find_json_key(const char* json, const char* key) {
    if (!json || !key) return NULL;
    char quoted[128];
    snprintf(quoted, sizeof(quoted), "\"%s\"", key);
    const char* p = strstr(json, quoted);
    if (p) return p + strlen(quoted);

    // Also support unquoted key e.g. key:
    p = strstr(json, key);
    if (p) return p + strlen(key);

    return NULL;
}

// Lightweight JSON field extraction
bool json_get_string(const char* json, const char* key, char* out_val, size_t max_len) {
    if (!json || !key || !out_val || max_len == 0) return false;
    out_val[0] = '\0';

    const char* p = find_json_key(json, key);
    if (!p) return false;

    while (*p && (*p == ' ' || *p == '\t' || *p == ':' || *p == '\r' || *p == '\n')) p++;

    if (*p == '"' || *p == '\'') {
        char quote = *p++;
        size_t i = 0;
        while (*p && *p != quote && i < max_len - 1) {
            if (*p == '\\' && *(p + 1)) {
                p++;
                if (*p == 'n') out_val[i++] = '\n';
                else if (*p == 'r') out_val[i++] = '\r';
                else if (*p == 't') out_val[i++] = '\t';
                else out_val[i++] = *p;
            } else {
                out_val[i++] = *p;
            }
            p++;
        }
        out_val[i] = '\0';
        return true;
    } else {
        // Unquoted string (until comma or closing brace)
        size_t i = 0;
        while (*p && *p != ',' && *p != '}' && *p != '\r' && *p != '\n' && i < max_len - 1) {
            out_val[i++] = *p++;
        }
        while (i > 0 && isspace((unsigned char)out_val[i - 1])) i--;
        out_val[i] = '\0';
        return i > 0;
    }
}

int json_get_int(const char* json, const char* key, int default_val) {
    if (!json || !key) return default_val;

    const char* p = find_json_key(json, key);
    if (!p) return default_val;

    while (*p && (*p == ' ' || *p == '\t' || *p == ':' || *p == '\r' || *p == '\n' || *p == '"' || *p == '\'')) p++;

    if (*p == '-' || isdigit((unsigned char)*p)) {
        return atoi(p);
    }
    return default_val;
}

double json_get_double(const char* json, const char* key, double default_val) {
    if (!json || !key) return default_val;

    const char* p = find_json_key(json, key);
    if (!p) return default_val;

    while (*p && (*p == ' ' || *p == '\t' || *p == ':' || *p == '\r' || *p == '\n' || *p == '"' || *p == '\'')) p++;

    if (*p == '-' || isdigit((unsigned char)*p)) {
        return atof(p);
    }
    return default_val;
}
