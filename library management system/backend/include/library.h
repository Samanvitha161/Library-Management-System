#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_BOOKS 1000
#define MAX_MEMBERS 1000
#define MAX_TRANSACTIONS 5000

#define TITLE_LEN 128
#define AUTHOR_LEN 128
#define ISBN_LEN 32
#define CATEGORY_LEN 64
#define SHELF_LEN 32

#define NAME_LEN 128
#define EMAIL_LEN 128
#define PHONE_LEN 32
#define TYPE_LEN 32
#define DATE_LEN 32
#define STATUS_LEN 16

#define DAILY_FINE_RATE 1.00 // $1.00 per day overdue

static inline void safe_copy(char* dest, const char* src, size_t dest_size) {
    if (!dest || dest_size == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    size_t len = strlen(src);
    if (len >= dest_size) len = dest_size - 1;
    memcpy(dest, src, len);
    dest[len] = '\0';
}

// Book structure
typedef struct {
    int id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    char isbn[ISBN_LEN];
    char category[CATEGORY_LEN];
    int pub_year;
    int total_copies;
    int available_copies;
    char shelf[SHELF_LEN];
} Book;

// Member structure
typedef struct {
    int id;
    char name[NAME_LEN];
    char email[EMAIL_LEN];
    char phone[PHONE_LEN];
    char type[TYPE_LEN]; // "Student", "Faculty", "Guest", "Staff"
    char joined_date[DATE_LEN];
    char status[STATUS_LEN]; // "Active", "Suspended"
    int books_borrowed_count;
} Member;

// Transaction structure
typedef struct {
    int id;
    int book_id;
    int member_id;
    char book_title[TITLE_LEN];
    char member_name[NAME_LEN];
    char issue_date[DATE_LEN];
    char due_date[DATE_LEN];
    char return_date[DATE_LEN];
    double fine_amount;
    char status[STATUS_LEN]; // "Issued", "Returned", "Overdue"
} Transaction;

// Overall statistics
typedef struct {
    int total_books;
    int total_titles;
    int available_copies;
    int borrowed_copies;
    int total_members;
    int active_loans;
    int overdue_loans;
    double total_fines;
} LibraryStats;

// Core library engine functions
int library_init(const char* data_dir);
int library_save_all(void);
int library_load_all(void);
void library_seed_sample_data(void);
void library_reset_database(void);

// Book Operations
Book* library_get_books(int* count);
Book* library_find_book_by_id(int id);
int library_add_book(Book* b);
int library_update_book(int id, Book* b);
int library_delete_book(int id);

// Member Operations
Member* library_get_members(int* count);
Member* library_find_member_by_id(int id);
int library_add_member(Member* m);
int library_update_member(int id, Member* m);
int library_delete_member(int id);

// Circulation Operations
Transaction* library_get_transactions(int* count);
Transaction* library_find_transaction_by_id(int id);
int library_issue_book(int book_id, int member_id, int loan_days, char* err_msg, size_t err_size);
int library_return_book(int transaction_id, double custom_fine, char* err_msg, size_t err_size);
int library_renew_book(int transaction_id, int extra_days, char* err_msg, size_t err_size);

// Analytics & Utilities
void library_get_stats(LibraryStats* stats);
void library_get_current_date(char* buffer, size_t size);
void library_add_days_to_date(const char* start_date, int days, char* buffer, size_t size);
int library_calculate_days_diff(const char* date1, const char* date2);
void library_update_overdue_statuses(void);

#endif // LIBRARY_H
