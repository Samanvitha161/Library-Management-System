#include "../include/http_server.h"
#include "../include/cJSON_mini.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int g_port = DEFAULT_PORT;
static char g_web_root[256] = "frontend";
static volatile bool g_running = true;

typedef struct {
    SOCKET client_sock;
} ClientContext;

// Helper to URL-decode strings in-place
static void url_decode(char* src, char* dst, size_t dst_size) {
    size_t d = 0;
    for (size_t s = 0; src[s] != '\0' && d < dst_size - 1; s++) {
        if (src[s] == '+') {
            dst[d++] = ' ';
        } else if (src[s] == '%' && src[s+1] && src[s+2]) {
            int val;
            char hex[3] = {src[s+1], src[s+2], '\0'};
            if (sscanf(hex, "%x", &val) == 1) {
                dst[d++] = (char)val;
                s += 2;
            } else {
                dst[d++] = src[s];
            }
        } else {
            dst[d++] = src[s];
        }
    }
    dst[d] = '\0';
}

// Case-insensitive substring search
static bool str_contains_nocase(const char* haystack, const char* needle) {
    if (!needle || !needle[0]) return true;
    if (!haystack) return false;

    size_t nlen = strlen(needle);
    size_t hlen = strlen(haystack);
    if (nlen > hlen) return false;

    for (size_t i = 0; i <= hlen - nlen; i++) {
        bool match = true;
        for (size_t j = 0; j < nlen; j++) {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j])) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

// Send HTTP response
static void send_response(SOCKET sock, int status_code, const char* status_text,
                          const char* content_type, const char* body, size_t body_len) {
    char header[1024];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, content_type, body_len);

    send(sock, header, header_len, 0);
    if (body && body_len > 0) {
        send(sock, body, (int)body_len, 0);
    }
}

static void send_json_response(SOCKET sock, int status_code, const char* json_str) {
    size_t len = json_str ? strlen(json_str) : 0;
    send_response(sock, status_code, status_code == 200 ? "OK" : (status_code == 400 ? "Bad Request" : (status_code == 404 ? "Not Found" : "Error")),
                  "application/json", json_str, len);
}

// Serve static files from web root
static void serve_static_file(SOCKET sock, const char* requested_path) {
    char safe_path[1024];
    if (strcmp(requested_path, "/") == 0 || strcmp(requested_path, "") == 0) {
        snprintf(safe_path, sizeof(safe_path), "%s/index.html", g_web_root);
    } else {
        // Prevent directory traversal
        if (strstr(requested_path, "..")) {
            send_response(sock, 403, "Forbidden", "text/plain", "Forbidden", 9);
            return;
        }
        // Skip leading slash
        const char* sub = requested_path;
        if (sub[0] == '/') sub++;
        snprintf(safe_path, sizeof(safe_path), "%s/%.500s", g_web_root, sub);
    }

    FILE* f = fopen(safe_path, "rb");
    if (!f) {
        // SPA Fallback: if not an asset, try index.html
        if (!strchr(requested_path, '.')) {
            char index_fallback[1024];
            snprintf(index_fallback, sizeof(index_fallback), "%s/index.html", g_web_root);
            f = fopen(index_fallback, "rb");
        }
    }

    if (!f) {
        const char* not_found = "404 Not Found";
        send_response(sock, 404, "Not Found", "text/plain", not_found, strlen(not_found));
        return;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* file_content = (char*)malloc(fsize);
    if (!file_content) {
        fclose(f);
        send_response(sock, 500, "Internal Server Error", "text/plain", "Out of memory", 13);
        return;
    }

    fread(file_content, 1, fsize, f);
    fclose(f);

    // Determine MIME type
    const char* mime = "application/octet-stream";
    const char* dot = strrchr(safe_path, '.');
    if (dot) {
        if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) mime = "text/html";
        else if (strcmp(dot, ".css") == 0) mime = "text/css";
        else if (strcmp(dot, ".js") == 0) mime = "application/javascript";
        else if (strcmp(dot, ".json") == 0) mime = "application/json";
        else if (strcmp(dot, ".svg") == 0) mime = "image/svg+xml";
        else if (strcmp(dot, ".png") == 0) mime = "image/png";
        else if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) mime = "image/jpeg";
        else if (strcmp(dot, ".ico") == 0) mime = "image/x-icon";
    }

    send_response(sock, 200, "OK", mime, file_content, fsize);
    free(file_content);
}

// Extract query parameter value
static bool get_query_param(const char* query, const char* param, char* out_val, size_t max_len) {
    if (!query || !param || !out_val || max_len == 0) return false;
    out_val[0] = '\0';

    char search[128];
    snprintf(search, sizeof(search), "%s=", param);
    const char* p = strstr(query, search);
    if (!p) return false;

    p += strlen(search);
    char raw[256] = {0};
    size_t i = 0;
    while (*p && *p != '&' && i < sizeof(raw) - 1) {
        raw[i++] = *p++;
    }
    raw[i] = '\0';
    url_decode(raw, out_val, max_len);
    return true;
}

// Dispatch API routes
static void handle_api_request(SOCKET sock, HttpRequest* req) {
    // 1. GET /api/stats
    if (strcmp(req->method, "GET") == 0 && strcmp(req->path, "/api/stats") == 0) {
        LibraryStats stats;
        library_get_stats(&stats);
        char* json = json_serialize_stats(&stats);
        send_json_response(sock, 200, json);
        free(json);
        return;
    }

    // 2. GET /api/books (with ?search= & ?category=)
    if (strcmp(req->method, "GET") == 0 && strcmp(req->path, "/api/books") == 0) {
        int count = 0;
        Book* books = library_get_books(&count);

        char search_query[128] = {0};
        char category_filter[128] = {0};
        get_query_param(req->query, "search", search_query, sizeof(search_query));
        get_query_param(req->query, "category", category_filter, sizeof(category_filter));

        if (strlen(search_query) == 0 && (strlen(category_filter) == 0 || strcmp(category_filter, "All") == 0)) {
            char* json = json_serialize_books(books, count);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            // Filter books
            Book filtered[MAX_BOOKS];
            int f_count = 0;
            for (int i = 0; i < count; i++) {
                bool cat_match = (strlen(category_filter) == 0 || strcmp(category_filter, "All") == 0 ||
                                  strcmp(books[i].category, category_filter) == 0);
                bool search_match = (strlen(search_query) == 0 ||
                                     str_contains_nocase(books[i].title, search_query) ||
                                     str_contains_nocase(books[i].author, search_query) ||
                                     str_contains_nocase(books[i].isbn, search_query));
                if (cat_match && search_match) {
                    filtered[f_count++] = books[i];
                }
            }
            char* json = json_serialize_books(filtered, f_count);
            send_json_response(sock, 200, json);
            free(json);
        }
        return;
    }

    // 3. GET /api/books/:id
    if (strcmp(req->method, "GET") == 0 && strncmp(req->path, "/api/books/", 11) == 0) {
        int id = atoi(req->path + 11);
        Book* b = library_find_book_by_id(id);
        if (b) {
            char* json = json_serialize_book(b);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, "Book not found", 0);
            send_json_response(sock, 404, json);
            free(json);
        }
        return;
    }

    // 4. POST /api/books (Create new book)
    if (strcmp(req->method, "POST") == 0 && strcmp(req->path, "/api/books") == 0) {
        Book b = {0};
        json_get_string(req->body, "title", b.title, sizeof(b.title));
        json_get_string(req->body, "author", b.author, sizeof(b.author));
        json_get_string(req->body, "isbn", b.isbn, sizeof(b.isbn));
        json_get_string(req->body, "category", b.category, sizeof(b.category));
        json_get_string(req->body, "shelf", b.shelf, sizeof(b.shelf));
        b.pub_year = json_get_int(req->body, "pub_year", 2024);
        b.total_copies = json_get_int(req->body, "total_copies", 1);
        b.available_copies = b.total_copies;

        if (strlen(b.title) == 0 || strlen(b.author) == 0) {
            char* json = json_serialize_message(false, "Title and Author are required", 0);
            send_json_response(sock, 400, json);
            free(json);
            return;
        }

        int new_id = library_add_book(&b);
        if (new_id > 0) {
            char* json = json_serialize_message(true, "Book added successfully", new_id);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, "Failed to add book (storage full)", 0);
            send_json_response(sock, 500, json);
            free(json);
        }
        return;
    }

    // 5. PUT /api/books/:id (Update book)
    if (strcmp(req->method, "PUT") == 0 && strncmp(req->path, "/api/books/", 11) == 0) {
        int id = atoi(req->path + 11);
        Book* target = library_find_book_by_id(id);
        if (!target) {
            char* json = json_serialize_message(false, "Book not found", 0);
            send_json_response(sock, 404, json);
            free(json);
            return;
        }

        Book updated = *target;
        json_get_string(req->body, "title", updated.title, sizeof(updated.title));
        json_get_string(req->body, "author", updated.author, sizeof(updated.author));
        json_get_string(req->body, "isbn", updated.isbn, sizeof(updated.isbn));
        json_get_string(req->body, "category", updated.category, sizeof(updated.category));
        json_get_string(req->body, "shelf", updated.shelf, sizeof(updated.shelf));
        updated.pub_year = json_get_int(req->body, "pub_year", updated.pub_year);
        updated.total_copies = json_get_int(req->body, "total_copies", updated.total_copies);

        if (library_update_book(id, &updated) == 0) {
            char* json = json_serialize_message(true, "Book updated successfully", id);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, "Failed to update book", 0);
            send_json_response(sock, 500, json);
            free(json);
        }
        return;
    }

    // 6. DELETE /api/books/:id (Delete book)
    if (strcmp(req->method, "DELETE") == 0 && strncmp(req->path, "/api/books/", 11) == 0) {
        int id = atoi(req->path + 11);
        int res = library_delete_book(id);
        if (res == 0) {
            char* json = json_serialize_message(true, "Book deleted successfully", id);
            send_json_response(sock, 200, json);
            free(json);
        } else if (res == -2) {
            char* json = json_serialize_message(false, "Cannot delete book: Copies are currently borrowed", id);
            send_json_response(sock, 400, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, "Book not found", id);
            send_json_response(sock, 404, json);
            free(json);
        }
        return;
    }

    // 7. GET /api/members
    if (strcmp(req->method, "GET") == 0 && strcmp(req->path, "/api/members") == 0) {
        int count = 0;
        Member* members = library_get_members(&count);
        char* json = json_serialize_members(members, count);
        send_json_response(sock, 200, json);
        free(json);
        return;
    }

    // 8. POST /api/members (Add member)
    if (strcmp(req->method, "POST") == 0 && strcmp(req->path, "/api/members") == 0) {
        Member m = {0};
        json_get_string(req->body, "name", m.name, sizeof(m.name));
        json_get_string(req->body, "email", m.email, sizeof(m.email));
        json_get_string(req->body, "phone", m.phone, sizeof(m.phone));
        json_get_string(req->body, "type", m.type, sizeof(m.type));
        json_get_string(req->body, "status", m.status, sizeof(m.status));
        if (!m.type[0]) safe_copy(m.type, "Student", sizeof(m.type));
        if (!m.status[0]) safe_copy(m.status, "Active", sizeof(m.status));

        if (strlen(m.name) == 0 || strlen(m.email) == 0) {
            char* json = json_serialize_message(false, "Name and Email are required", 0);
            send_json_response(sock, 400, json);
            free(json);
            return;
        }

        int new_id = library_add_member(&m);
        if (new_id > 0) {
            char* json = json_serialize_message(true, "Member registered successfully", new_id);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, "Failed to add member", 0);
            send_json_response(sock, 500, json);
            free(json);
        }
        return;
    }

    // 9. PUT /api/members/:id (Update member)
    if (strcmp(req->method, "PUT") == 0 && strncmp(req->path, "/api/members/", 13) == 0) {
        int id = atoi(req->path + 13);
        Member* target = library_find_member_by_id(id);
        if (!target) {
            char* json = json_serialize_message(false, "Member not found", 0);
            send_json_response(sock, 404, json);
            free(json);
            return;
        }

        Member updated = *target;
        json_get_string(req->body, "name", updated.name, sizeof(updated.name));
        json_get_string(req->body, "email", updated.email, sizeof(updated.email));
        json_get_string(req->body, "phone", updated.phone, sizeof(updated.phone));
        json_get_string(req->body, "type", updated.type, sizeof(updated.type));
        json_get_string(req->body, "status", updated.status, sizeof(updated.status));

        if (library_update_member(id, &updated) == 0) {
            char* json = json_serialize_message(true, "Member updated successfully", id);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, "Failed to update member", 0);
            send_json_response(sock, 500, json);
            free(json);
        }
        return;
    }

    // 10. DELETE /api/members/:id
    if (strcmp(req->method, "DELETE") == 0 && strncmp(req->path, "/api/members/", 13) == 0) {
        int id = atoi(req->path + 13);
        int res = library_delete_member(id);
        if (res == 0) {
            char* json = json_serialize_message(true, "Member removed successfully", id);
            send_json_response(sock, 200, json);
            free(json);
        } else if (res == -2) {
            char* json = json_serialize_message(false, "Cannot remove member with active borrowed books", id);
            send_json_response(sock, 400, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, "Member not found", id);
            send_json_response(sock, 404, json);
            free(json);
        }
        return;
    }

    // 11. GET /api/transactions
    if (strcmp(req->method, "GET") == 0 && strcmp(req->path, "/api/transactions") == 0) {
        int count = 0;
        Transaction* trans = library_get_transactions(&count);
        char* json = json_serialize_transactions(trans, count);
        send_json_response(sock, 200, json);
        free(json);
        return;
    }

    // 12. POST /api/issue (Issue book)
    if (strcmp(req->method, "POST") == 0 && strcmp(req->path, "/api/issue") == 0) {
        int book_id = json_get_int(req->body, "book_id", 0);
        int member_id = json_get_int(req->body, "member_id", 0);
        int loan_days = json_get_int(req->body, "loan_days", 14);

        char err[256] = {0};
        int trans_id = library_issue_book(book_id, member_id, loan_days, err, sizeof(err));
        if (trans_id > 0) {
            char* json = json_serialize_message(true, "Book issued successfully", trans_id);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, err[0] ? err : "Failed to issue book", 0);
            send_json_response(sock, 400, json);
            free(json);
        }
        return;
    }

    // 13. POST /api/return (Return book)
    if (strcmp(req->method, "POST") == 0 && strcmp(req->path, "/api/return") == 0) {
        int transaction_id = json_get_int(req->body, "transaction_id", 0);
        double fine = json_get_double(req->body, "fine", -1.0);

        char err[256] = {0};
        int res = library_return_book(transaction_id, fine, err, sizeof(err));
        if (res == 0) {
            char* json = json_serialize_message(true, "Book returned successfully", transaction_id);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, err[0] ? err : "Failed to return book", 0);
            send_json_response(sock, 400, json);
            free(json);
        }
        return;
    }

    // 14. POST /api/renew (Renew loan)
    if (strcmp(req->method, "POST") == 0 && strcmp(req->path, "/api/renew") == 0) {
        int transaction_id = json_get_int(req->body, "transaction_id", 0);
        int extra_days = json_get_int(req->body, "extra_days", 14);

        char err[256] = {0};
        int res = library_renew_book(transaction_id, extra_days, err, sizeof(err));
        if (res == 0) {
            char* json = json_serialize_message(true, "Loan renewed successfully", transaction_id);
            send_json_response(sock, 200, json);
            free(json);
        } else {
            char* json = json_serialize_message(false, err[0] ? err : "Failed to renew loan", 0);
            send_json_response(sock, 400, json);
            free(json);
        }
        return;
    }

    // 15. POST /api/reset (Reset database to defaults)
    if (strcmp(req->method, "POST") == 0 && strcmp(req->path, "/api/reset") == 0) {
        library_reset_database();
        char* json = json_serialize_message(true, "Database reset to initial sample data", 0);
        send_json_response(sock, 200, json);
        free(json);
        return;
    }

    // Unknown API route
    char* json = json_serialize_message(false, "API route not found", 0);
    send_json_response(sock, 404, json);
    free(json);
}

// Client connection thread handler
static DWORD WINAPI handle_client_thread(LPVOID param) {
    ClientContext* ctx = (ClientContext*)param;
    SOCKET client_sock = ctx->client_sock;
    free(ctx);

    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        closesocket(client_sock);
        return 0;
    }
    buffer[bytes_received] = '\0';

    HttpRequest req = {0};

    // Parse method and path
    char raw_path[MAX_PATH_LEN] = {0};
    if (sscanf(buffer, "%15s %511s", req.method, raw_path) != 2) {
        closesocket(client_sock);
        return 0;
    }

    // Handle OPTIONS preflight request for CORS
    if (strcmp(req.method, "OPTIONS") == 0) {
        send_response(client_sock, 200, "OK", "text/plain", "", 0);
        closesocket(client_sock);
        return 0;
    }

    // Split path and query string
    char* q = strchr(raw_path, '?');
    if (q) {
        *q = '\0';
        safe_copy(req.path, raw_path, sizeof(req.path));
        safe_copy(req.query, q + 1, sizeof(req.query));
    } else {
        safe_copy(req.path, raw_path, sizeof(req.path));
        req.query[0] = '\0';
    }

    // Check Content-Length header
    int content_length = 0;
    char* cl_header = strstr(buffer, "Content-Length:");
    if (!cl_header) cl_header = strstr(buffer, "content-length:");
    if (cl_header) {
        cl_header += 15;
        while (*cl_header == ' ') cl_header++;
        content_length = atoi(cl_header);
    }

    // Locate HTTP Body
    char* body_start = strstr(buffer, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        int current_body_len = bytes_received - (int)(body_start - buffer);
        while (current_body_len < content_length && bytes_received < (int)sizeof(buffer) - 1) {
            int more = recv(client_sock, buffer + bytes_received, (int)sizeof(buffer) - 1 - bytes_received, 0);
            if (more <= 0) break;
            bytes_received += more;
            buffer[bytes_received] = '\0';
            current_body_len += more;
        }
        req.body = body_start;
        req.body_len = current_body_len;
    }

    // Route dispatch
    if (strncmp(req.path, "/api/", 5) == 0) {
        handle_api_request(client_sock, &req);
    } else {
        serve_static_file(client_sock, req.path);
    }

    closesocket(client_sock);
    return 0;
}

int http_server_start(int port, const char* web_root) {
    g_port = port;
    if (web_root) {
        safe_copy(g_web_root, web_root, sizeof(g_web_root));
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "[-] WSAStartup failed\n");
        return -1;
    }

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        fprintf(stderr, "[-] Failed to create socket\n");
        WSACleanup();
        return -1;
    }

    BOOL reuse = TRUE;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((u_short)g_port);

    if (bind(listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "[-] Failed to bind socket on port %d (Error: %d)\n", g_port, WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return -1;
    }

    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "[-] Failed to listen on socket\n");
        closesocket(listen_sock);
        WSACleanup();
        return -1;
    }

    printf("============================================================\n");
    printf("   LIBRARY MANAGEMENT SYSTEM - C BACKEND SERVER READY\n");
    printf("============================================================\n");
    printf("   [+] Architecture: 100%% Pure C Backend Engine\n");
    printf("   [+] HTTP REST API: http://localhost:%d/api\n", g_port);
    printf("   [+] Web UI Dashboard: http://localhost:%d\n", g_port);
    printf("   [+] Static Web Root: ./%s\n", g_web_root);
    printf("   [+] Persistence: Active (data/ folder)\n");
    printf("   [+] Press Ctrl+C in terminal to stop server\n");
    printf("============================================================\n\n");

    while (g_running) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        SOCKET client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &client_len);

        if (client_sock == INVALID_SOCKET) {
            if (!g_running) break;
            continue;
        }

        ClientContext* ctx = (ClientContext*)malloc(sizeof(ClientContext));
        if (ctx) {
            ctx->client_sock = client_sock;
            HANDLE thread = CreateThread(NULL, 0, handle_client_thread, ctx, 0, NULL);
            if (thread) {
                CloseHandle(thread);
            } else {
                closesocket(client_sock);
                free(ctx);
            }
        } else {
            closesocket(client_sock);
        }
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}

void http_server_stop(void) {
    g_running = false;
}
