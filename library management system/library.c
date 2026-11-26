/*
 * ============================================================================
 *   LIBRARY MANAGEMENT SYSTEM - C BACKEND (MINI PROJECT)
 * ============================================================================
 *   Language: Pure C (C99 Standard)
 *   Storage:  File-based persistence (books.txt)
 *   Features: Pre-seeded sample books, Add, Display, Search, Update, Delete,
 *             Issue, Return, Statistics, and File Persistence.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOOKS 500
#define FILE_NAME "books.txt"

// Book Structure
typedef struct {
    int id;
    char title[100];
    char author[100];
    char isbn[30];
    char category[50];
    int pub_year;
    int total_copies;
    int available_copies;
    char shelf[30];
} Book;

// Global array of books and count
static Book books[MAX_BOOKS];
static int book_count = 0;
static int next_id = 1;

// Function Prototypes
void load_books_from_file(void);
void save_books_to_file(void);
void seed_sample_books(void);
void display_all_books(void);
void add_book(void);
void search_book(void);
void update_book(void);
void delete_book(void);
void issue_book(void);
void return_book(void);
void view_statistics(void);
void clean_newline(char* str);
void print_header(const char* title);
void edit_book_by_index(int idx);

// Safe string copy helper
static void safe_copy(char* dest, const char* src, size_t dest_size) {
    if (!dest || dest_size == 0) return;
    if (!src) { dest[0] = '\0'; return; }
    size_t len = strlen(src);
    if (len >= dest_size) len = dest_size - 1;
    memcpy(dest, src, len);
    dest[len] = '\0';
}

int main(void) {
    int choice = 0;
    char input[50];

    // Initialize database from file or sample books
    load_books_from_file();

    while (1) {
        printf("\n============================================================\n");
        printf("               LIBRARY MANAGEMENT SYSTEM                    \n");
        printf("============================================================\n");
        printf("  [1] Display All Books\n");
        printf("  [2] Add New Book\n");
        printf("  [3] Search Books (Multiple Search Options)\n");
        printf("  [4] Update Book Details\n");
        printf("  [5] Delete Book\n");
        printf("  [6] Issue a Book\n");
        printf("  [7] Return a Book\n");
        printf("  [8] View Library Statistics\n");
        printf("  [9] Reset to Default Sample Books\n");
        printf("  [0] Exit\n");
        printf("------------------------------------------------------------\n");
        printf("  Enter choice (0-9): ");

        if (!fgets(input, sizeof(input), stdin)) break;
        choice = atoi(input);

        if (choice == 0) {
            printf("\nSaving data and exiting. Thank you!\n");
            save_books_to_file();
            break;
        }

        switch (choice) {
            case 1: display_all_books(); break;
            case 2: add_book(); break;
            case 3: search_book(); break;
            case 4: update_book(); break;
            case 5: delete_book(); break;
            case 6: issue_book(); break;
            case 7: return_book(); break;
            case 8: view_statistics(); break;
            case 9:
                printf("\nResetting to default 12 sample books...\n");
                seed_sample_books();
                save_books_to_file();
                printf("[+] Sample books restored successfully!\n");
                break;
            default:
                printf("\n[!] Invalid choice. Please enter a number between 0 and 9.\n");
                break;
        }
    }

    return 0;
}

// Remove trailing newline or carriage return characters
void clean_newline(char* str) {
    if (!str) return;
    str[strcspn(str, "\r\n")] = '\0';
}

// Print formatted header
void print_header(const char* title) {
    printf("\n------------------------------------------------------------\n");
    printf("  %s\n", title);
    printf("------------------------------------------------------------\n");
}

// Pre-seed 12 sample books into database
void seed_sample_books(void) {
    struct {
        const char* title;
        const char* author;
        const char* isbn;
        const char* category;
        int year;
        int total;
        int avail;
        const char* shelf;
    } samples[] = {
        {"The C Programming Language", "Brian W. Kernighan, Dennis Ritchie", "978-0131103627", "Computer Science", 1988, 5, 4, "A-101"},
        {"Introduction to Algorithms (CLRS)", "Thomas H. Cormen, Charles Leiserson", "978-0262033848", "Computer Science", 2009, 4, 3, "A-102"},
        {"Design Patterns", "Erich Gamma, Richard Helm", "978-0201633610", "Software Engineering", 1994, 3, 2, "A-103"},
        {"Clean Code", "Robert C. Martin", "978-0132350884", "Software Engineering", 2008, 6, 5, "A-104"},
        {"Operating System Concepts", "Abraham Silberschatz, Peter Galvin", "978-1118063330", "Operating Systems", 2012, 4, 4, "B-201"},
        {"Computer Systems: A Programmer's Perspective", "Randal E. Bryant, David O'Hallaron", "978-0134092669", "Computer Science", 2015, 3, 2, "B-202"},
        {"Artificial Intelligence: A Modern Approach", "Stuart Russell, Peter Norvig", "978-0136042594", "Artificial Intelligence", 2020, 5, 4, "B-203"},
        {"Deep Learning", "Ian Goodfellow, Yoshua Bengio", "978-0262035613", "Artificial Intelligence", 2016, 4, 3, "B-204"},
        {"The Pragmatic Programmer", "David Thomas, Andrew Hunt", "978-0135957059", "Software Engineering", 2019, 5, 5, "C-301"},
        {"Database System Concepts", "Abraham Silberschatz, Henry Korth", "978-0078022159", "Database Systems", 2019, 4, 4, "C-302"},
        {"Structure and Interpretation of Computer Programs", "Harold Abelson, Gerald Sussman", "978-0262510875", "Computer Science", 1996, 3, 3, "C-303"},
        {"To Kill a Mockingbird", "Harper Lee", "978-0061120084", "Literature", 1960, 4, 3, "D-401"}
    };

    book_count = 0;
    next_id = 1;
    size_t count = sizeof(samples) / sizeof(samples[0]);

    for (size_t i = 0; i < count && i < MAX_BOOKS; i++) {
        Book* b = &books[book_count++];
        b->id = next_id++;
        safe_copy(b->title, samples[i].title, sizeof(b->title));
        safe_copy(b->author, samples[i].author, sizeof(b->author));
        safe_copy(b->isbn, samples[i].isbn, sizeof(b->isbn));
        safe_copy(b->category, samples[i].category, sizeof(b->category));
        b->pub_year = samples[i].year;
        b->total_copies = samples[i].total;
        b->available_copies = samples[i].avail;
        safe_copy(b->shelf, samples[i].shelf, sizeof(b->shelf));
    }
}

// Load book records from file
void load_books_from_file(void) {
    FILE* fp = fopen(FILE_NAME, "r");
    if (!fp) {
        // File doesn't exist yet, seed with sample books
        seed_sample_books();
        save_books_to_file();
        return;
    }

    char line[512];
    book_count = 0;
    next_id = 1;

    while (fgets(line, sizeof(line), fp) && book_count < MAX_BOOKS) {
        clean_newline(line);
        if (strlen(line) == 0) continue;

        Book* b = &books[book_count];
        char* token = strtok(line, "|");
        if (!token) continue;
        b->id = atoi(token);

        token = strtok(NULL, "|");
        safe_copy(b->title, token ? token : "", sizeof(b->title));

        token = strtok(NULL, "|");
        safe_copy(b->author, token ? token : "", sizeof(b->author));

        token = strtok(NULL, "|");
        safe_copy(b->isbn, token ? token : "", sizeof(b->isbn));

        token = strtok(NULL, "|");
        safe_copy(b->category, token ? token : "", sizeof(b->category));

        token = strtok(NULL, "|");
        b->pub_year = token ? atoi(token) : 2024;

        token = strtok(NULL, "|");
        b->total_copies = token ? atoi(token) : 1;

        token = strtok(NULL, "|");
        b->available_copies = token ? atoi(token) : 1;

        token = strtok(NULL, "|");
        safe_copy(b->shelf, token ? token : "General", sizeof(b->shelf));

        if (b->id >= next_id) next_id = b->id + 1;
        book_count++;
    }

    fclose(fp);

    if (book_count == 0) {
        seed_sample_books();
        save_books_to_file();
    }
}

// Save book records to file
void save_books_to_file(void) {
    FILE* fp = fopen(FILE_NAME, "w");
    if (!fp) {
        printf("[!] Error saving data to %s\n", FILE_NAME);
        return;
    }

    for (int i = 0; i < book_count; i++) {
        Book* b = &books[i];
        fprintf(fp, "%d|%s|%s|%s|%s|%d|%d|%d|%s\n",
                b->id, b->title, b->author, b->isbn,
                b->category, b->pub_year, b->total_copies,
                b->available_copies, b->shelf);
    }

    fclose(fp);
}

// Display all books
void display_all_books(void) {
    if (book_count == 0) {
        printf("\n[i] No books currently in the library.\n");
        return;
    }

    print_header("BOOK CATALOG");
    printf("%-4s | %-30s | %-22s | %-16s | %-6s | %-6s\n",
           "ID", "Title", "Author", "Category", "Copies", "Shelf");
    printf("---------------------------------------------------------------------------------------\n");

    for (int i = 0; i < book_count; i++) {
        char copies_str[20];
        snprintf(copies_str, sizeof(copies_str), "%d/%d", books[i].available_copies, books[i].total_copies);
        printf("%-4d | %-30.30s | %-22.22s | %-16.16s | %-6s | %-6s\n",
               books[i].id, books[i].title, books[i].author,
               books[i].category, copies_str, books[i].shelf);
    }
    printf("---------------------------------------------------------------------------------------\n");
    printf("Total: %d book titles\n", book_count);
}

// Add a new book
void add_book(void) {
    if (book_count >= MAX_BOOKS) {
        printf("\n[!] Library catalog is full! Cannot add more books.\n");
        return;
    }

    Book b = {0};
    char buffer[100];

    print_header("ADD NEW BOOK");

    printf("Enter Book Title: ");
    if (!fgets(b.title, sizeof(b.title), stdin)) return;
    clean_newline(b.title);
    if (strlen(b.title) == 0) {
        printf("[!] Title cannot be empty.\n");
        return;
    }

    printf("Enter Author Name: ");
    if (!fgets(b.author, sizeof(b.author), stdin)) return;
    clean_newline(b.author);

    printf("Enter Category (e.g. Computer Science, Literature): ");
    if (!fgets(b.category, sizeof(b.category), stdin)) return;
    clean_newline(b.category);

    printf("Enter ISBN: ");
    if (!fgets(b.isbn, sizeof(b.isbn), stdin)) return;
    clean_newline(b.isbn);

    printf("Enter Publication Year: ");
    if (fgets(buffer, sizeof(buffer), stdin)) b.pub_year = atoi(buffer);
    if (b.pub_year <= 0) b.pub_year = 2024;

    printf("Enter Total Copies: ");
    if (fgets(buffer, sizeof(buffer), stdin)) b.total_copies = atoi(buffer);
    if (b.total_copies <= 0) b.total_copies = 1;
    b.available_copies = b.total_copies;

    printf("Enter Shelf Location (e.g. A-101): ");
    if (!fgets(b.shelf, sizeof(b.shelf), stdin)) return;
    clean_newline(b.shelf);
    if (strlen(b.shelf) == 0) strcpy(b.shelf, "General");

    b.id = next_id++;
    books[book_count++] = b;
    save_books_to_file();

    printf("\n[+] SUCCESS: Book added successfully with ID #%d!\n", b.id);
}

// Case-insensitive substring search helper
static int str_contains_ignore_case(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    char h[128], n[128];
    size_t i;
    for (i = 0; haystack[i] && i < sizeof(h) - 1; i++) h[i] = (char)tolower((unsigned char)haystack[i]);
    h[i] = '\0';
    for (i = 0; needle[i] && i < sizeof(n) - 1; i++) n[i] = (char)tolower((unsigned char)needle[i]);
    n[i] = '\0';
    return strstr(h, n) != NULL;
}

// Search for a book with multiple search options
void search_book(void) {
    char opt_str[20];
    char query[100];
    print_header("SEARCH BOOKS");

    printf("  [1] Search All Fields (Title, Author, Category, ISBN, Shelf, ID)\n");
    printf("  [2] Search by Title\n");
    printf("  [3] Search by Author\n");
    printf("  [4] Search by Category\n");
    printf("  [5] Search by ISBN\n");
    printf("  [6] Search by Book ID\n");
    printf("  [0] Return to Main Menu\n");
    printf("------------------------------------------------------------\n");
    printf("  Select search option (0-6): ");

    if (!fgets(opt_str, sizeof(opt_str), stdin)) return;
    int opt = atoi(opt_str);
    if (opt == 0) return;
    if (opt < 1 || opt > 6) {
        printf("[!] Invalid search option.\n");
        return;
    }

    const char* field_name = "Keyword";
    switch (opt) {
        case 1: field_name = "Keyword (All Fields)"; break;
        case 2: field_name = "Book Title"; break;
        case 3: field_name = "Author Name"; break;
        case 4: field_name = "Category"; break;
        case 5: field_name = "ISBN"; break;
        case 6: field_name = "Book ID"; break;
    }

    printf("Enter %s to search: ", field_name);
    if (!fgets(query, sizeof(query), stdin)) return;
    clean_newline(query);

    if (strlen(query) == 0) {
        printf("[!] Search query cannot be empty.\n");
        return;
    }

    int search_id = atoi(query);
    int found_indices[MAX_BOOKS];
    int found = 0;

    for (int i = 0; i < book_count; i++) {
        int match = 0;
        switch (opt) {
            case 1: // All fields
                if ((search_id > 0 && books[i].id == search_id) ||
                    str_contains_ignore_case(books[i].title, query) ||
                    str_contains_ignore_case(books[i].author, query) ||
                    str_contains_ignore_case(books[i].category, query) ||
                    str_contains_ignore_case(books[i].isbn, query) ||
                    str_contains_ignore_case(books[i].shelf, query)) {
                    match = 1;
                }
                break;
            case 2: // Title
                if (str_contains_ignore_case(books[i].title, query)) match = 1;
                break;
            case 3: // Author
                if (str_contains_ignore_case(books[i].author, query)) match = 1;
                break;
            case 4: // Category
                if (str_contains_ignore_case(books[i].category, query)) match = 1;
                break;
            case 5: // ISBN
                if (str_contains_ignore_case(books[i].isbn, query)) match = 1;
                break;
            case 6: // ID
                if (search_id > 0 && books[i].id == search_id) match = 1;
                break;
        }

        if (match) {
            found_indices[found++] = i;
        }
    }

    printf("\n---------------------------------------------------------------------------------------\n");
    printf("SEARCH RESULTS for \"%s\" (%s):\n", query, field_name);
    printf("%-4s | %-30s | %-22s | %-16s | %-6s | %-6s\n",
           "ID", "Title", "Author", "Category", "Copies", "Shelf");
    printf("---------------------------------------------------------------------------------------\n");

    if (found == 0) {
        printf("No books matched your search query '%s'.\n", query);
        printf("---------------------------------------------------------------------------------------\n");
        return;
    }

    for (int j = 0; j < found; j++) {
        int i = found_indices[j];
        char copies_str[20];
        snprintf(copies_str, sizeof(copies_str), "%d/%d", books[i].available_copies, books[i].total_copies);
        printf("%-4d | %-30.30s | %-22.22s | %-16.16s | %-6s | %-6s\n",
               books[i].id, books[i].title, books[i].author,
               books[i].category, copies_str, books[i].shelf);
    }
    printf("---------------------------------------------------------------------------------------\n");
    printf("Found %d matching book(s).\n", found);

    // If one book is found, offer quick actions
    if (found == 1) {
        int idx = found_indices[0];
        printf("\nQuick action for Book #%d (\"%s\"):\n", books[idx].id, books[idx].title);
        printf("  [1] Edit Book Details\n");
        printf("  [2] Issue Book\n");
        printf("  [3] Return Book\n");
        printf("  [0] Done / Back to Menu\n");
        printf("Select action (0-3): ");
        char act_str[20];
        if (fgets(act_str, sizeof(act_str), stdin)) {
            int a = atoi(act_str);
            if (a == 1) {
                edit_book_by_index(idx);
            } else if (a == 2) {
                if (books[idx].available_copies <= 0) {
                    printf("[!] Book is OUT OF STOCK.\n");
                } else {
                    printf("Borrower Name: ");
                    char bname[100];
                    if (fgets(bname, sizeof(bname), stdin)) {
                        clean_newline(bname);
                        books[idx].available_copies--;
                        save_books_to_file();
                        printf("\n[+] SUCCESS: Book \"%s\" issued to %s! Available: %d\n",
                               books[idx].title, bname, books[idx].available_copies);
                    }
                }
            } else if (a == 3) {
                books[idx].available_copies++;
                if (books[idx].available_copies > books[idx].total_copies) {
                    books[idx].total_copies = books[idx].available_copies;
                }
                save_books_to_file();
                printf("\n[+] SUCCESS: Book \"%s\" returned! Available: %d of %d\n",
                       books[idx].title, books[idx].available_copies, books[idx].total_copies);
                printf("Edit book details now? (y/n): ");
                char yn[10];
                if (fgets(yn, sizeof(yn), stdin) && (yn[0] == 'y' || yn[0] == 'Y')) {
                    edit_book_by_index(idx);
                }
            }
        }
    }
}

// Edit book details by array index
void edit_book_by_index(int idx) {
    char buffer[100];
    Book* b = &books[idx];
    printf("\nCurrently editing: \"%s\" by %s\n", b->title, b->author);
    printf("(Press Enter to keep current value)\n\n");

    printf("New Title [%s]: ", b->title);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        clean_newline(buffer);
        if (strlen(buffer) > 0) safe_copy(b->title, buffer, sizeof(b->title));
    }

    printf("New Author [%s]: ", b->author);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        clean_newline(buffer);
        if (strlen(buffer) > 0) safe_copy(b->author, buffer, sizeof(b->author));
    }

    printf("New Category [%s]: ", b->category);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        clean_newline(buffer);
        if (strlen(buffer) > 0) safe_copy(b->category, buffer, sizeof(b->category));
    }

    printf("New Total Copies [%d]: ", b->total_copies);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        clean_newline(buffer);
        if (strlen(buffer) > 0) {
            int new_total = atoi(buffer);
            if (new_total > 0) {
                int diff = new_total - b->total_copies;
                b->total_copies = new_total;
                b->available_copies += diff;
                if (b->available_copies < 0) b->available_copies = 0;
                if (b->available_copies > b->total_copies) b->available_copies = b->total_copies;
            }
        }
    }

    printf("New Shelf Location [%s]: ", b->shelf);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        clean_newline(buffer);
        if (strlen(buffer) > 0) safe_copy(b->shelf, buffer, sizeof(b->shelf));
    }

    save_books_to_file();
    printf("\n[+] SUCCESS: Book #%d updated successfully!\n", b->id);
}

// Update book details
void update_book(void) {
    char buffer[100];
    print_header("UPDATE BOOK");

    printf("Enter Book ID to update: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    int id = atoi(buffer);

    int idx = -1;
    for (int i = 0; i < book_count; i++) {
        if (books[i].id == id) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("[!] Book with ID #%d not found.\n", id);
        return;
    }

    edit_book_by_index(idx);
}

// Delete book
void delete_book(void) {
    char buffer[100];
    print_header("DELETE BOOK");

    printf("Enter Book ID to delete: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    int id = atoi(buffer);

    int idx = -1;
    for (int i = 0; i < book_count; i++) {
        if (books[i].id == id) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("[!] Book with ID #%d not found.\n", id);
        return;
    }

    printf("Are you sure you want to delete \"%s\"? (y/n): ", books[idx].title);
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    clean_newline(buffer);

    if (buffer[0] == 'y' || buffer[0] == 'Y') {
        for (int i = idx; i < book_count - 1; i++) {
            books[i] = books[i + 1];
        }
        book_count--;
        save_books_to_file();
        printf("\n[+] SUCCESS: Book #%d deleted successfully.\n", id);
    } else {
        printf("\n[-] Deletion cancelled.\n");
    }
}

// Issue a book (decrease available copy)
void issue_book(void) {
    char buffer[100];
    print_header("ISSUE BOOK");

    printf("Enter Book ID to issue: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    int id = atoi(buffer);

    int idx = -1;
    for (int i = 0; i < book_count; i++) {
        if (books[i].id == id) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("[!] Book with ID #%d not found.\n", id);
        return;
    }

    if (books[idx].available_copies <= 0) {
        printf("[!] Sorry, \"%s\" is currently OUT OF STOCK.\n", books[idx].title);
        return;
    }

    printf("Enter Borrower / Student Name: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    clean_newline(buffer);

    books[idx].available_copies--;
    save_books_to_file();

    printf("\n[+] SUCCESS: Book \"%s\" issued to %s!\n", books[idx].title, buffer);
    printf("    Remaining copies available: %d\n", books[idx].available_copies);
}

// Return a book (increase available copy)
void return_book(void) {
    char buffer[100];
    print_header("RETURN BOOK");

    printf("Enter Book ID to return: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    int id = atoi(buffer);

    int idx = -1;
    for (int i = 0; i < book_count; i++) {
        if (books[i].id == id) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("[!] Book with ID #%d not found.\n", id);
        return;
    }

    if (books[idx].available_copies >= books[idx].total_copies) {
        books[idx].total_copies++;
        books[idx].available_copies++;
        save_books_to_file();
        printf("\n[+] SUCCESS: Extra copy of \"%s\" registered into inventory!\n", books[idx].title);
        printf("    Available copies on shelf: %d of %d\n", books[idx].available_copies, books[idx].total_copies);
    } else {
        books[idx].available_copies++;
        save_books_to_file();
        printf("\n[+] SUCCESS: Book \"%s\" returned successfully!\n", books[idx].title);
        printf("    Total available copies now: %d of %d\n", books[idx].available_copies, books[idx].total_copies);
    }

    printf("\n>>> Would you like to edit this book's details now? (y/n): ");
    if (fgets(buffer, sizeof(buffer), stdin)) {
        clean_newline(buffer);
        if (buffer[0] == 'y' || buffer[0] == 'Y') {
            edit_book_by_index(idx);
        }
    }
}

// View library statistics
void view_statistics(void) {
    int total_books_count = 0;
    int total_avail_count = 0;

    for (int i = 0; i < book_count; i++) {
        total_books_count += books[i].total_copies;
        total_avail_count += books[i].available_copies;
    }

    int borrowed_count = total_books_count - total_avail_count;

    print_header("LIBRARY SYSTEM STATISTICS");
    printf("  Total Book Titles in Catalog:  %d\n", book_count);
    printf("  Total Physical Volumes/Copies: %d\n", total_books_count);
    printf("  Available Copies on Shelves:   %d\n", total_avail_count);
    printf("  Currently Borrowed Books:      %d\n", borrowed_count);
    printf("  Data File:                     %s\n", FILE_NAME);
    printf("------------------------------------------------------------\n");
}
