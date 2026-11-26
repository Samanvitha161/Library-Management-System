#include "../include/library.h"

static Book g_books[MAX_BOOKS];
static int g_book_count = 0;
static int g_next_book_id = 1;

static Member g_members[MAX_MEMBERS];
static int g_member_count = 0;
static int g_next_member_id = 1;

static Transaction g_transactions[MAX_TRANSACTIONS];
static int g_transaction_count = 0;
static int g_next_transaction_id = 1;

static char g_data_dir[256] = "data";

// Helper: current date in YYYY-MM-DD format
void library_get_current_date(char* buffer, size_t size) {
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    strftime(buffer, size, "%Y-%m-%d", tm_info);
}

// Helper: add days to a YYYY-MM-DD date string
void library_add_days_to_date(const char* start_date, int days, char* buffer, size_t size) {
    struct tm tm_val = {0};
    int y, m, d;
    if (sscanf(start_date, "%d-%d-%d", &y, &m, &d) == 3) {
        tm_val.tm_year = y - 1900;
        tm_val.tm_mon = m - 1;
        tm_val.tm_mday = d + days;
        tm_val.tm_isdst = -1;
        mktime(&tm_val);
        strftime(buffer, size, "%Y-%m-%d", &tm_val);
    } else {
        library_get_current_date(buffer, size);
    }
}

// Helper: calculate days difference (date2 - date1)
int library_calculate_days_diff(const char* date1, const char* date2) {
    struct tm tm1 = {0};
    struct tm tm2 = {0};
    int y1, m1, d1, y2, m2, d2;
    if (sscanf(date1, "%d-%d-%d", &y1, &m1, &d1) != 3) return 0;
    if (sscanf(date2, "%d-%d-%d", &y2, &m2, &d2) != 3) return 0;

    tm1.tm_year = y1 - 1900;
    tm1.tm_mon = m1 - 1;
    tm1.tm_mday = d1;
    tm1.tm_isdst = -1;

    tm2.tm_year = y2 - 1900;
    tm2.tm_mon = m2 - 1;
    tm2.tm_mday = d2;
    tm2.tm_isdst = -1;

    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);

    if (t1 == (time_t)-1 || t2 == (time_t)-1) return 0;
    double seconds = difftime(t2, t1);
    return (int)(seconds / (60 * 60 * 24));
}

// Update overdue status and compute current fines for active loans
void library_update_overdue_statuses(void) {
    char today[DATE_LEN];
    library_get_current_date(today, sizeof(today));

    for (int i = 0; i < g_transaction_count; i++) {
        if (strcmp(g_transactions[i].status, "Issued") == 0 || strcmp(g_transactions[i].status, "Overdue") == 0) {
            int days_past_due = library_calculate_days_diff(g_transactions[i].due_date, today);
            if (days_past_due > 0) {
                safe_copy(g_transactions[i].status, "Overdue", sizeof(g_transactions[i].status));
                g_transactions[i].fine_amount = days_past_due * DAILY_FINE_RATE;
            } else {
                safe_copy(g_transactions[i].status, "Issued", sizeof(g_transactions[i].status));
                g_transactions[i].fine_amount = 0.0;
            }
        }
    }
}

// Sanitize string for pipe-delimited storage
static void sanitize_field(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '|' || str[i] == '\n' || str[i] == '\r') {
            str[i] = ' ';
        }
    }
}

int library_save_all(void) {
    char filepath[300];

    // 1. Save Books
    snprintf(filepath, sizeof(filepath), "%s/books.txt", g_data_dir);
    FILE* fb = fopen(filepath, "w");
    if (fb) {
        for (int i = 0; i < g_book_count; i++) {
            Book* b = &g_books[i];
            fprintf(fb, "%d|%s|%s|%s|%s|%d|%d|%d|%s\n",
                b->id, b->title, b->author, b->isbn,
                b->category, b->pub_year, b->total_copies,
                b->available_copies, b->shelf);
        }
        fclose(fb);
    }

    // 2. Save Members
    snprintf(filepath, sizeof(filepath), "%s/members.txt", g_data_dir);
    FILE* fm = fopen(filepath, "w");
    if (fm) {
        for (int i = 0; i < g_member_count; i++) {
            Member* m = &g_members[i];
            fprintf(fm, "%d|%s|%s|%s|%s|%s|%s|%d\n",
                m->id, m->name, m->email, m->phone,
                m->type, m->joined_date, m->status,
                m->books_borrowed_count);
        }
        fclose(fm);
    }

    // 3. Save Transactions
    snprintf(filepath, sizeof(filepath), "%s/transactions.txt", g_data_dir);
    FILE* ft = fopen(filepath, "w");
    if (ft) {
        for (int i = 0; i < g_transaction_count; i++) {
            Transaction* t = &g_transactions[i];
            fprintf(ft, "%d|%d|%d|%s|%s|%s|%s|%s|%.2f|%s\n",
                t->id, t->book_id, t->member_id,
                t->book_title, t->member_name,
                t->issue_date, t->due_date,
                t->return_date[0] ? t->return_date : "-",
                t->fine_amount, t->status);
        }
        fclose(ft);
    }

    return 0;
}

int library_load_all(void) {
    char filepath[300];
    char line[1024];

    g_book_count = 0;
    g_member_count = 0;
    g_transaction_count = 0;

    // 1. Load Books
    snprintf(filepath, sizeof(filepath), "%s/books.txt", g_data_dir);
    FILE* fb = fopen(filepath, "r");
    if (fb) {
        while (fgets(line, sizeof(line), fb) && g_book_count < MAX_BOOKS) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) == 0) continue;

            Book* b = &g_books[g_book_count];
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
            b->pub_year = token ? atoi(token) : 2020;

            token = strtok(NULL, "|");
            b->total_copies = token ? atoi(token) : 1;

            token = strtok(NULL, "|");
            b->available_copies = token ? atoi(token) : 1;

            token = strtok(NULL, "|");
            safe_copy(b->shelf, token ? token : "General", sizeof(b->shelf));

            if (b->id >= g_next_book_id) g_next_book_id = b->id + 1;
            g_book_count++;
        }
        fclose(fb);
    }

    // 2. Load Members
    snprintf(filepath, sizeof(filepath), "%s/members.txt", g_data_dir);
    FILE* fm = fopen(filepath, "r");
    if (fm) {
        while (fgets(line, sizeof(line), fm) && g_member_count < MAX_MEMBERS) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) == 0) continue;

            Member* m = &g_members[g_member_count];
            char* token = strtok(line, "|");
            if (!token) continue;
            m->id = atoi(token);

            token = strtok(NULL, "|");
            safe_copy(m->name, token ? token : "", sizeof(m->name));

            token = strtok(NULL, "|");
            safe_copy(m->email, token ? token : "", sizeof(m->email));

            token = strtok(NULL, "|");
            safe_copy(m->phone, token ? token : "", sizeof(m->phone));

            token = strtok(NULL, "|");
            safe_copy(m->type, token ? token : "Student", sizeof(m->type));

            token = strtok(NULL, "|");
            safe_copy(m->joined_date, token ? token : "2024-01-01", sizeof(m->joined_date));

            token = strtok(NULL, "|");
            safe_copy(m->status, token ? token : "Active", sizeof(m->status));

            token = strtok(NULL, "|");
            m->books_borrowed_count = token ? atoi(token) : 0;

            if (m->id >= g_next_member_id) g_next_member_id = m->id + 1;
            g_member_count++;
        }
        fclose(fm);
    }

    // 3. Load Transactions
    snprintf(filepath, sizeof(filepath), "%s/transactions.txt", g_data_dir);
    FILE* ft = fopen(filepath, "r");
    if (ft) {
        while (fgets(line, sizeof(line), ft) && g_transaction_count < MAX_TRANSACTIONS) {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) == 0) continue;

            Transaction* t = &g_transactions[g_transaction_count];
            char* token = strtok(line, "|");
            if (!token) continue;
            t->id = atoi(token);

            token = strtok(NULL, "|");
            t->book_id = token ? atoi(token) : 0;

            token = strtok(NULL, "|");
            t->member_id = token ? atoi(token) : 0;

            token = strtok(NULL, "|");
            safe_copy(t->book_title, token ? token : "", sizeof(t->book_title));

            token = strtok(NULL, "|");
            safe_copy(t->member_name, token ? token : "", sizeof(t->member_name));

            token = strtok(NULL, "|");
            safe_copy(t->issue_date, token ? token : "", sizeof(t->issue_date));

            token = strtok(NULL, "|");
            safe_copy(t->due_date, token ? token : "", sizeof(t->due_date));

            token = strtok(NULL, "|");
            if (token && strcmp(token, "-") != 0) {
                safe_copy(t->return_date, token, sizeof(t->return_date));
            } else {
                t->return_date[0] = '\0';
            }

            token = strtok(NULL, "|");
            t->fine_amount = token ? atof(token) : 0.0;

            token = strtok(NULL, "|");
            safe_copy(t->status, token ? token : "Issued", sizeof(t->status));

            if (t->id >= g_next_transaction_id) g_next_transaction_id = t->id + 1;
            g_transaction_count++;
        }
        fclose(ft);
    }

    library_update_overdue_statuses();
    return (g_book_count > 0 || g_member_count > 0);
}

void library_seed_sample_data(void) {
    struct {
        const char* title;
        const char* author;
        const char* isbn;
        const char* category;
        int year;
        int total;
        int avail;
        const char* shelf;
    } sample_books[] = {
        {"The C Programming Language (2nd Ed)", "Brian W. Kernighan, Dennis M. Ritchie", "978-0131103627", "Computer Science", 1988, 5, 4, "A-101"},
        {"Introduction to Algorithms (CLRS)", "Thomas H. Cormen, Charles E. Leiserson", "978-0262033848", "Computer Science", 2009, 4, 3, "A-102"},
        {"Design Patterns: Elements of Reusable OO Software", "Erich Gamma, Richard Helm, Ralph Johnson", "978-0201633610", "Software Engineering", 1994, 3, 2, "A-103"},
        {"Clean Code: A Handbook of Agile Software Craftsmanship", "Robert C. Martin", "978-0132350884", "Software Engineering", 2008, 6, 5, "A-104"},
        {"Operating System Concepts", "Abraham Silberschatz, Peter B. Galvin", "978-1118063330", "Operating Systems", 2012, 4, 4, "B-201"},
        {"Computer Systems: A Programmer's Perspective", "Randal E. Bryant, David R. O'Hallaron", "978-0134092669", "Computer Science", 2015, 3, 2, "B-202"},
        {"Artificial Intelligence: A Modern Approach", "Stuart Russell, Peter Norvig", "978-0136042594", "Artificial Intelligence", 2020, 5, 4, "B-203"},
        {"Deep Learning", "Ian Goodfellow, Yoshua Bengio, Aaron Courville", "978-0262035613", "Artificial Intelligence", 2016, 4, 3, "B-204"},
        {"The Pragmatic Programmer: Your Journey to Mastery", "David Thomas, Andrew Hunt", "978-0135957059", "Software Engineering", 2019, 5, 5, "C-301"},
        {"Database System Concepts", "Abraham Silberschatz, Henry F. Korth", "978-0078022159", "Database Systems", 2019, 4, 4, "C-302"},
        {"Structure and Interpretation of Computer Programs", "Harold Abelson, Gerald Jay Sussman", "978-0262510875", "Computer Science", 1996, 3, 3, "C-303"},
        {"To Kill a Mockingbird", "Harper Lee", "978-0061120084", "Literature", 1960, 4, 3, "D-401"}
    };

    g_book_count = 0;
    g_next_book_id = 1;
    for (size_t i = 0; i < sizeof(sample_books)/sizeof(sample_books[0]); i++) {
        Book* b = &g_books[g_book_count++];
        b->id = g_next_book_id++;
        safe_copy(b->title, sample_books[i].title, sizeof(b->title));
        safe_copy(b->author, sample_books[i].author, sizeof(b->author));
        safe_copy(b->isbn, sample_books[i].isbn, sizeof(b->isbn));
        safe_copy(b->category, sample_books[i].category, sizeof(b->category));
        b->pub_year = sample_books[i].year;
        b->total_copies = sample_books[i].total;
        b->available_copies = sample_books[i].avail;
        safe_copy(b->shelf, sample_books[i].shelf, sizeof(b->shelf));
    }

    struct {
        const char* name;
        const char* email;
        const char* phone;
        const char* type;
        const char* date;
    } sample_members[] = {
        {"Alex Morgan", "alex.morgan@university.edu", "+1-555-0101", "Student", "2024-01-15"},
        {"Prof. David Mitchell", "d.mitchell@university.edu", "+1-555-0102", "Faculty", "2023-08-20"},
        {"Samantha Ray", "samantha.r@university.edu", "+1-555-0103", "Student", "2024-02-10"},
        {"Dr. Elena Rostova", "elena.r@research.org", "+1-555-0104", "Faculty", "2023-11-05"},
        {"Marcus Chen", "marcus.chen@tech.org", "+1-555-0105", "Guest", "2024-03-01"}
    };

    g_member_count = 0;
    g_next_member_id = 1;
    for (size_t i = 0; i < sizeof(sample_members)/sizeof(sample_members[0]); i++) {
        Member* m = &g_members[g_member_count++];
        m->id = g_next_member_id++;
        safe_copy(m->name, sample_members[i].name, sizeof(m->name));
        safe_copy(m->email, sample_members[i].email, sizeof(m->email));
        safe_copy(m->phone, sample_members[i].phone, sizeof(m->phone));
        safe_copy(m->type, sample_members[i].type, sizeof(m->type));
        safe_copy(m->joined_date, sample_members[i].date, sizeof(m->joined_date));
        safe_copy(m->status, "Active", sizeof(m->status));
        m->books_borrowed_count = 0;
    }

    char today[DATE_LEN];
    library_get_current_date(today, sizeof(today));
    char due_in_7[DATE_LEN], due_in_14[DATE_LEN], past_due_5[DATE_LEN], past_issue_20[DATE_LEN];
    library_add_days_to_date(today, 7, due_in_7, sizeof(due_in_7));
    library_add_days_to_date(today, 14, due_in_14, sizeof(due_in_14));
    library_add_days_to_date(today, -5, past_due_5, sizeof(past_due_5));
    library_add_days_to_date(today, -20, past_issue_20, sizeof(past_issue_20));

    g_transaction_count = 0;
    g_next_transaction_id = 1;

    Transaction* t1 = &g_transactions[g_transaction_count++];
    t1->id = g_next_transaction_id++;
    t1->book_id = 1;
    t1->member_id = 1;
    safe_copy(t1->book_title, g_books[0].title, sizeof(t1->book_title));
    safe_copy(t1->member_name, g_members[0].name, sizeof(t1->member_name));
    safe_copy(t1->issue_date, today, sizeof(t1->issue_date));
    safe_copy(t1->due_date, due_in_14, sizeof(t1->due_date));
    t1->return_date[0] = '\0';
    t1->fine_amount = 0.0;
    safe_copy(t1->status, "Issued", sizeof(t1->status));
    g_members[0].books_borrowed_count++;

    Transaction* t2 = &g_transactions[g_transaction_count++];
    t2->id = g_next_transaction_id++;
    t2->book_id = 2;
    t2->member_id = 2;
    safe_copy(t2->book_title, g_books[1].title, sizeof(t2->book_title));
    safe_copy(t2->member_name, g_members[1].name, sizeof(t2->member_name));
    safe_copy(t2->issue_date, today, sizeof(t2->issue_date));
    safe_copy(t2->due_date, due_in_7, sizeof(t2->due_date));
    t2->return_date[0] = '\0';
    t2->fine_amount = 0.0;
    safe_copy(t2->status, "Issued", sizeof(t2->status));
    g_members[1].books_borrowed_count++;

    Transaction* t3 = &g_transactions[g_transaction_count++];
    t3->id = g_next_transaction_id++;
    t3->book_id = 3;
    t3->member_id = 3;
    safe_copy(t3->book_title, g_books[2].title, sizeof(t3->book_title));
    safe_copy(t3->member_name, g_members[2].name, sizeof(t3->member_name));
    safe_copy(t3->issue_date, past_issue_20, sizeof(t3->issue_date));
    safe_copy(t3->due_date, past_due_5, sizeof(t3->due_date));
    t3->return_date[0] = '\0';
    t3->fine_amount = 5.0 * DAILY_FINE_RATE;
    safe_copy(t3->status, "Overdue", sizeof(t3->status));
    g_members[2].books_borrowed_count++;

    Transaction* t4 = &g_transactions[g_transaction_count++];
    t4->id = g_next_transaction_id++;
    t4->book_id = 4;
    t4->member_id = 4;
    safe_copy(t4->book_title, g_books[3].title, sizeof(t4->book_title));
    safe_copy(t4->member_name, g_members[3].name, sizeof(t4->member_name));
    safe_copy(t4->issue_date, "2024-02-01", sizeof(t4->issue_date));
    safe_copy(t4->due_date, "2024-02-15", sizeof(t4->due_date));
    safe_copy(t4->return_date, "2024-02-14", sizeof(t4->return_date));
    t4->fine_amount = 0.0;
    safe_copy(t4->status, "Returned", sizeof(t4->status));

    library_save_all();
}

void library_reset_database(void) {
    library_seed_sample_data();
}

int library_init(const char* data_dir) {
    if (data_dir) {
        safe_copy(g_data_dir, data_dir, sizeof(g_data_dir));
    }
    if (!library_load_all()) {
        library_seed_sample_data();
    }
    return 0;
}

Book* library_get_books(int* count) {
    if (count) *count = g_book_count;
    return g_books;
}

Book* library_find_book_by_id(int id) {
    for (int i = 0; i < g_book_count; i++) {
        if (g_books[i].id == id) return &g_books[i];
    }
    return NULL;
}

int library_add_book(Book* b) {
    if (g_book_count >= MAX_BOOKS) return -1;
    b->id = g_next_book_id++;
    sanitize_field(b->title);
    sanitize_field(b->author);
    sanitize_field(b->isbn);
    sanitize_field(b->category);
    sanitize_field(b->shelf);
    if (b->available_copies > b->total_copies) b->available_copies = b->total_copies;
    g_books[g_book_count++] = *b;
    library_save_all();
    return b->id;
}

int library_update_book(int id, Book* b) {
    Book* target = library_find_book_by_id(id);
    if (!target) return -1;

    sanitize_field(b->title);
    sanitize_field(b->author);
    sanitize_field(b->isbn);
    sanitize_field(b->category);
    sanitize_field(b->shelf);

    int diff = b->total_copies - target->total_copies;
    target->total_copies = b->total_copies;
    target->available_copies += diff;
    if (target->available_copies < 0) target->available_copies = 0;
    if (target->available_copies > target->total_copies) target->available_copies = target->total_copies;

    safe_copy(target->title, b->title, sizeof(target->title));
    safe_copy(target->author, b->author, sizeof(target->author));
    safe_copy(target->isbn, b->isbn, sizeof(target->isbn));
    safe_copy(target->category, b->category, sizeof(target->category));
    safe_copy(target->shelf, b->shelf, sizeof(target->shelf));
    target->pub_year = b->pub_year;

    library_save_all();
    return 0;
}

int library_delete_book(int id) {
    int idx = -1;
    for (int i = 0; i < g_book_count; i++) {
        if (g_books[i].id == id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return -1;

    // Clean up any transactions referencing this book so deletion always succeeds
    int write_idx = 0;
    for (int i = 0; i < g_transaction_count; i++) {
        if (g_transactions[i].book_id != id) {
            g_transactions[write_idx++] = g_transactions[i];
        }
    }
    g_transaction_count = write_idx;

    for (int i = idx; i < g_book_count - 1; i++) {
        g_books[i] = g_books[i + 1];
    }
    g_book_count--;
    library_save_all();
    return 0;
}

Member* library_get_members(int* count) {
    if (count) *count = g_member_count;
    return g_members;
}

Member* library_find_member_by_id(int id) {
    for (int i = 0; i < g_member_count; i++) {
        if (g_members[i].id == id) return &g_members[i];
    }
    return NULL;
}

int library_add_member(Member* m) {
    if (g_member_count >= MAX_MEMBERS) return -1;
    m->id = g_next_member_id++;
    sanitize_field(m->name);
    sanitize_field(m->email);
    sanitize_field(m->phone);
    sanitize_field(m->type);
    sanitize_field(m->status);
    if (!m->joined_date[0]) {
        library_get_current_date(m->joined_date, sizeof(m->joined_date));
    }
    m->books_borrowed_count = 0;
    g_members[g_member_count++] = *m;
    library_save_all();
    return m->id;
}

int library_update_member(int id, Member* m) {
    Member* target = library_find_member_by_id(id);
    if (!target) return -1;

    sanitize_field(m->name);
    sanitize_field(m->email);
    sanitize_field(m->phone);
    sanitize_field(m->type);
    sanitize_field(m->status);

    safe_copy(target->name, m->name, sizeof(target->name));
    safe_copy(target->email, m->email, sizeof(target->email));
    safe_copy(target->phone, m->phone, sizeof(target->phone));
    safe_copy(target->type, m->type, sizeof(target->type));
    safe_copy(target->status, m->status, sizeof(target->status));

    library_save_all();
    return 0;
}

int library_delete_member(int id) {
    int idx = -1;
    for (int i = 0; i < g_member_count; i++) {
        if (g_members[i].id == id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return -1;

    if (g_members[idx].books_borrowed_count > 0) {
        return -2;
    }

    for (int i = idx; i < g_member_count - 1; i++) {
        g_members[i] = g_members[i + 1];
    }
    g_member_count--;
    library_save_all();
    return 0;
}

Transaction* library_get_transactions(int* count) {
    library_update_overdue_statuses();
    if (count) *count = g_transaction_count;
    return g_transactions;
}

Transaction* library_find_transaction_by_id(int id) {
    for (int i = 0; i < g_transaction_count; i++) {
        if (g_transactions[i].id == id) return &g_transactions[i];
    }
    return NULL;
}

int library_issue_book(int book_id, int member_id, int loan_days, char* err_msg, size_t err_size) {
    Book* book = library_find_book_by_id(book_id);
    if (!book) {
        if (err_msg) snprintf(err_msg, err_size, "Book with ID %d not found.", book_id);
        return -1;
    }

    Member* member = library_find_member_by_id(member_id);
    if (!member) {
        if (err_msg) snprintf(err_msg, err_size, "Member with ID %d not found.", member_id);
        return -2;
    }

    if (strcmp(member->status, "Active") != 0) {
        if (err_msg) snprintf(err_msg, err_size, "Member account is %s.", member->status);
        return -3;
    }

    if (book->available_copies <= 0) {
        if (err_msg) snprintf(err_msg, err_size, "No available copies of '%s'.", book->title);
        return -4;
    }

    if (member->books_borrowed_count >= 5) {
        if (err_msg) snprintf(err_msg, err_size, "Member has reached the maximum borrowing limit of 5 books.");
        return -5;
    }

    if (g_transaction_count >= MAX_TRANSACTIONS) {
        if (err_msg) snprintf(err_msg, err_size, "Transaction table is full.");
        return -6;
    }

    if (loan_days <= 0 || loan_days > 90) loan_days = 14;

    char today[DATE_LEN];
    library_get_current_date(today, sizeof(today));
    char due[DATE_LEN];
    library_add_days_to_date(today, loan_days, due, sizeof(due));

    Transaction* t = &g_transactions[g_transaction_count++];
    t->id = g_next_transaction_id++;
    t->book_id = book->id;
    t->member_id = member->id;
    safe_copy(t->book_title, book->title, sizeof(t->book_title));
    safe_copy(t->member_name, member->name, sizeof(t->member_name));
    safe_copy(t->issue_date, today, sizeof(t->issue_date));
    safe_copy(t->due_date, due, sizeof(t->due_date));
    t->return_date[0] = '\0';
    t->fine_amount = 0.0;
    safe_copy(t->status, "Issued", sizeof(t->status));

    book->available_copies--;
    member->books_borrowed_count++;

    library_save_all();
    return t->id;
}

int library_return_book(int transaction_id, double custom_fine, char* err_msg, size_t err_size) {
    Transaction* t = library_find_transaction_by_id(transaction_id);
    if (!t) {
        if (err_msg) snprintf(err_msg, err_size, "Transaction ID %d not found.", transaction_id);
        return -1;
    }

    if (strcmp(t->status, "Returned") == 0) {
        if (err_msg) snprintf(err_msg, err_size, "Book is already recorded as returned.");
        return -2;
    }

    char today[DATE_LEN];
    library_get_current_date(today, sizeof(today));
    safe_copy(t->return_date, today, sizeof(t->return_date));
    safe_copy(t->status, "Returned", sizeof(t->status));

    if (custom_fine >= 0.0) {
        t->fine_amount = custom_fine;
    } else {
        int overdue_days = library_calculate_days_diff(t->due_date, today);
        if (overdue_days > 0) {
            t->fine_amount = overdue_days * DAILY_FINE_RATE;
        } else {
            t->fine_amount = 0.0;
        }
    }

    Book* book = library_find_book_by_id(t->book_id);
    if (book) {
        if (book->available_copies < book->total_copies) {
            book->available_copies++;
        }
    }

    Member* member = library_find_member_by_id(t->member_id);
    if (member) {
        if (member->books_borrowed_count > 0) {
            member->books_borrowed_count--;
        }
    }

    library_save_all();
    return 0;
}

int library_renew_book(int transaction_id, int extra_days, char* err_msg, size_t err_size) {
    Transaction* t = library_find_transaction_by_id(transaction_id);
    if (!t) {
        if (err_msg) snprintf(err_msg, err_size, "Transaction ID %d not found.", transaction_id);
        return -1;
    }

    if (strcmp(t->status, "Returned") == 0) {
        if (err_msg) snprintf(err_msg, err_size, "Cannot renew an already returned book.");
        return -2;
    }

    if (extra_days <= 0 || extra_days > 30) extra_days = 14;

    char new_due[DATE_LEN];
    library_add_days_to_date(t->due_date, extra_days, new_due, sizeof(new_due));
    safe_copy(t->due_date, new_due, sizeof(t->due_date));

    library_update_overdue_statuses();
    library_save_all();
    return 0;
}

void library_get_stats(LibraryStats* stats) {
    if (!stats) return;
    memset(stats, 0, sizeof(LibraryStats));

    library_update_overdue_statuses();

    stats->total_titles = g_book_count;
    for (int i = 0; i < g_book_count; i++) {
        stats->total_books += g_books[i].total_copies;
        stats->available_copies += g_books[i].available_copies;
    }
    stats->borrowed_copies = stats->total_books - stats->available_copies;
    stats->total_members = g_member_count;

    for (int i = 0; i < g_transaction_count; i++) {
        if (strcmp(g_transactions[i].status, "Issued") == 0) {
            stats->active_loans++;
        } else if (strcmp(g_transactions[i].status, "Overdue") == 0) {
            stats->active_loans++;
            stats->overdue_loans++;
        }
        stats->total_fines += g_transactions[i].fine_amount;
    }
}
