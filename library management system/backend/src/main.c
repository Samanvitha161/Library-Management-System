#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/library.h"
#include "../include/http_server.h"

static void print_banner(void) {
    printf("\n");
    printf("  ==============================================================\n");
    printf("   LIBRARY MANAGEMENT SYSTEM - PURE C BACKEND CORE\n");
    printf("  ==============================================================\n");
}

static void run_cli_mode(void) {
    int choice = 0;
    char buffer[256];

    while (1) {
        print_banner();
        printf("  [1] List All Books\n");
        printf("  [2] Add New Book\n");
        printf("  [3] Search Book\n");
        printf("  [4] List All Members\n");
        printf("  [5] Add New Member\n");
        printf("  [6] Issue a Book\n");
        printf("  [7] Return a Book\n");
        printf("  [8] View Active Loans & Overdue Fines\n");
        printf("  [9] View System Statistics\n");
        printf("  [10] Launch Web Server (HTTP REST API)\n");
        printf("  [0] Exit\n");
        printf("  --------------------------------------------------------------\n");
        printf("  Enter choice (0-10): ");

        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        choice = atoi(buffer);

        if (choice == 0) {
            printf("\nExiting Library Management System. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1: {
                int count = 0;
                Book* books = library_get_books(&count);
                printf("\n--- Book Catalog (%d titles) ---\n", count);
                printf("%-4s | %-32s | %-24s | %-16s | %-5s | %-6s\n",
                       "ID", "Title", "Author", "Category", "Total", "Avail");
                printf("---------------------------------------------------------------------------------------\n");
                for (int i = 0; i < count; i++) {
                    printf("%-4d | %-32.32s | %-24.24s | %-16.16s | %-5d | %-6d\n",
                           books[i].id, books[i].title, books[i].author,
                           books[i].category, books[i].total_copies, books[i].available_copies);
                }
                break;
            }
            case 2: {
                Book b = {0};
                printf("\nEnter Book Title: ");
                fgets(b.title, sizeof(b.title), stdin);
                b.title[strcspn(b.title, "\r\n")] = '\0';

                printf("Enter Author: ");
                fgets(b.author, sizeof(b.author), stdin);
                b.author[strcspn(b.author, "\r\n")] = '\0';

                printf("Enter ISBN: ");
                fgets(b.isbn, sizeof(b.isbn), stdin);
                b.isbn[strcspn(b.isbn, "\r\n")] = '\0';

                printf("Enter Category: ");
                fgets(b.category, sizeof(b.category), stdin);
                b.category[strcspn(b.category, "\r\n")] = '\0';

                printf("Enter Publication Year: ");
                fgets(buffer, sizeof(buffer), stdin);
                b.pub_year = atoi(buffer);

                printf("Enter Total Copies: ");
                fgets(buffer, sizeof(buffer), stdin);
                b.total_copies = atoi(buffer);
                b.available_copies = b.total_copies;

                printf("Enter Shelf Location: ");
                fgets(b.shelf, sizeof(b.shelf), stdin);
                b.shelf[strcspn(b.shelf, "\r\n")] = '\0';

                int id = library_add_book(&b);
                if (id > 0) printf("[+] Book added successfully with ID: %d\n", id);
                else printf("[-] Failed to add book.\n");
                break;
            }
            case 3: {
                printf("\nEnter title, author, or ISBN keyword: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\r\n")] = '\0';

                int count = 0;
                Book* books = library_get_books(&count);
                printf("\nSearch Results for '%s':\n", buffer);
                int found = 0;
                for (int i = 0; i < count; i++) {
                    if (strstr(books[i].title, buffer) || strstr(books[i].author, buffer) || strstr(books[i].isbn, buffer)) {
                        printf("ID: %d | Title: %s | Author: %s | Available: %d/%d | Shelf: %s\n",
                               books[i].id, books[i].title, books[i].author,
                               books[i].available_copies, books[i].total_copies, books[i].shelf);
                        found++;
                    }
                }
                if (!found) printf("No books matched the search keyword.\n");
                break;
            }
            case 4: {
                int count = 0;
                Member* members = library_get_members(&count);
                printf("\n--- Registered Members (%d members) ---\n", count);
                printf("%-4s | %-20s | %-28s | %-10s | %-10s | %-5s\n",
                       "ID", "Name", "Email", "Type", "Status", "Loans");
                printf("----------------------------------------------------------------------------------------\n");
                for (int i = 0; i < count; i++) {
                    printf("%-4d | %-20.20s | %-28.28s | %-10.10s | %-10.10s | %-5d\n",
                           members[i].id, members[i].name, members[i].email,
                           members[i].type, members[i].status, members[i].books_borrowed_count);
                }
                break;
            }
            case 5: {
                Member m = {0};
                printf("\nEnter Member Name: ");
                fgets(m.name, sizeof(m.name), stdin);
                m.name[strcspn(m.name, "\r\n")] = '\0';

                printf("Enter Email: ");
                fgets(m.email, sizeof(m.email), stdin);
                m.email[strcspn(m.email, "\r\n")] = '\0';

                printf("Enter Phone: ");
                fgets(m.phone, sizeof(m.phone), stdin);
                m.phone[strcspn(m.phone, "\r\n")] = '\0';

                printf("Enter Membership Type (Student/Faculty/Staff/Guest): ");
                fgets(m.type, sizeof(m.type), stdin);
                m.type[strcspn(m.type, "\r\n")] = '\0';

                strncpy(m.status, "Active", sizeof(m.status) - 1);
                int id = library_add_member(&m);
                if (id > 0) printf("[+] Member registered with ID: %d\n", id);
                else printf("[-] Failed to register member.\n");
                break;
            }
            case 6: {
                printf("\nEnter Book ID to issue: ");
                fgets(buffer, sizeof(buffer), stdin);
                int bid = atoi(buffer);

                printf("Enter Member ID: ");
                fgets(buffer, sizeof(buffer), stdin);
                int mid = atoi(buffer);

                printf("Enter loan duration in days (e.g. 14): ");
                fgets(buffer, sizeof(buffer), stdin);
                int days = atoi(buffer);

                char err[256] = {0};
                int tid = library_issue_book(bid, mid, days, err, sizeof(err));
                if (tid > 0) {
                    printf("[+] Book issued successfully! Transaction ID: %d\n", tid);
                } else {
                    printf("[-] Issue failed: %s\n", err);
                }
                break;
            }
            case 7: {
                printf("\nEnter Transaction ID to return: ");
                fgets(buffer, sizeof(buffer), stdin);
                int tid = atoi(buffer);

                char err[256] = {0};
                int res = library_return_book(tid, -1.0, err, sizeof(err));
                if (res == 0) {
                    printf("[+] Book marked returned successfully!\n");
                } else {
                    printf("[-] Return failed: %s\n", err);
                }
                break;
            }
            case 8: {
                int count = 0;
                Transaction* t = library_get_transactions(&count);
                printf("\n--- Active and Overdue Loans ---\n");
                printf("%-4s | %-28s | %-18s | %-10s | %-10s | %-6s | %-8s\n",
                       "TID", "Book Title", "Borrower", "Issued", "Due", "Status", "Fine ($)");
                printf("------------------------------------------------------------------------------------------\n");
                int active_count = 0;
                for (int i = 0; i < count; i++) {
                    if (strcmp(t[i].status, "Returned") != 0) {
                        printf("%-4d | %-28.28s | %-18.18s | %-10s | %-10s | %-6s | $%.2f\n",
                               t[i].id, t[i].book_title, t[i].member_name,
                               t[i].issue_date, t[i].due_date, t[i].status, t[i].fine_amount);
                        active_count++;
                    }
                }
                if (active_count == 0) printf("No currently active loans.\n");
                break;
            }
            case 9: {
                LibraryStats stats;
                library_get_stats(&stats);
                printf("\n--- Library System Statistics ---\n");
                printf("  Total Book Titles:       %d\n", stats.total_titles);
                printf("  Total Book Copies:       %d\n", stats.total_books);
                printf("  Available Book Copies:   %d\n", stats.available_copies);
                printf("  Currently Borrowed:      %d\n", stats.borrowed_copies);
                printf("  Registered Members:      %d\n", stats.total_members);
                printf("  Active Loans:            %d\n", stats.active_loans);
                printf("  Overdue Loans:           %d\n", stats.overdue_loans);
                printf("  Total Outstanding Fines: $%.2f\n", stats.total_fines);
                break;
            }
            case 10: {
                printf("\nStarting HTTP Server mode on port 8080...\n");
                http_server_start(8080, "frontend");
                break;
            }
            default:
                printf("Invalid selection.\n");
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    const char* web_root = "frontend";
    const char* data_dir = "data";
    bool cli_mode = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--web-root") == 0 && i + 1 < argc) {
            web_root = argv[++i];
        } else if (strcmp(argv[i], "--data-dir") == 0 && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (strcmp(argv[i], "--cli") == 0) {
            cli_mode = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Library Management System - Pure C Backend\n");
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --port <PORT>        Specify HTTP server port (default: 8080)\n");
            printf("  --web-root <DIR>     Specify frontend directory (default: frontend)\n");
            printf("  --data-dir <DIR>     Specify data directory (default: data)\n");
            printf("  --cli                Launch interactive CLI console mode\n");
            printf("  --help, -h           Show this help message\n");
            return 0;
        }
    }

    // Initialize database
    library_init(data_dir);

    if (cli_mode) {
        run_cli_mode();
        return 0;
    }

    // Default: Start high-performance HTTP REST Server
    return http_server_start(port, web_root);
}
