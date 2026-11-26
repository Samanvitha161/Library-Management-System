# Library Management System (Mini Project)

A Windows library management project with a browser interface and a C backend.

---

## 🚀 How to Run the Mini Project

### Windows quick start

1. Extract the complete ZIP file to a folder (do not run the app from inside the ZIP).
2. Double-click `run.bat`.
3. Choose option `1` to start the backend and open `http://localhost:8080`.
4. Keep the server window open while using the app. Close it to stop the server.

The ready-to-run Windows package includes the server executable; GCC is not
needed. Windows and a web browser are required. If port `8080` is already in
use, close the other app using it before starting this one.

To run the standalone browser demo without the backend, open `index.html`
instead. It uses browser local storage and does not share data with the C
backend.

### 2. C Program Console
Double-click:
```
library.exe
```
Or run from your terminal:
```cmd
.\library.exe
```

### 3. Quick Launcher Menu
Double-click `run.bat` to choose:
- Option `1`: Open Web Frontend in browser
- Option `2`: Launch C console program
- Option `3`: Compile C code (`library.c`)

---

## ✨ Features (Mini Project Scope)

1. **Pre-Seeded Sample Books**:
   - 12 classic computer science and literature books are pre-loaded out of the box.
2. **Add New Books**:
   - Add a new title with author, category, ISBN, publication year, copies, and shelf location.
3. **Update / Edit Books**:
   - Modify existing book details or total stock with automatic status recalculation.
4. **Delete Books**:
   - Delete any book with confirmation.
5. **Issue & Return Books (with Edit on Return)**:
   - Check out a copy (decrements available copies) or return a copy (restocks shelf).
   - **Edit on Return**: Immediately upon returning a book, you are offered an option to edit its details (e.g. update shelf location, inspect copy count, or edit condition) in both the Web UI and the C console program.
6. **Comprehensive Search Options & Category Filter**:
   - **Search By Field Selector**: Choose between *Search All Fields*, *By Title*, *By Author*, *By Category*, *By ISBN*, or *By Shelf Location*.
   - **Dedicated Search & Reset Controls**: Visible primary `Search` button, clear `X` button, and `Reset` button.
   - **Instant Live Filtering**: Results filter live as you type or upon pressing `Enter` or clicking `Search`.
   - **Active Search Status Bar**: Clearly displays the match count, search keyword, and active filter with a quick one-click clear link.
   - **Quick Category Filter Pills**: Instant one-click filtering (*All Books*, *Computer Science*, *Software Eng*, *AI*, *Operating Systems*, *Databases*, *Literature*).
   - **C Console Search Menu**: Sub-menu offering dedicated search options (All Fields, Title, Author, Category, ISBN, Book ID) with quick action shortcuts directly on found books.
7. **Clean White UI**:
   - Solid pure white background (`#ffffff`).
   - Zero gradients anywhere in the styling.
   - Simple text title: **Library Management System** (no graphical logo).
8. **File Syncing**:
   - **Export `books.txt`**: Generates the exact pipe-delimited file read by the C program.
   - **Import `books.txt`**: Import records edited in the C program directly into the Web UI.

---

## 📋 Comprehensive List of Details Used in Project

### 1. Architecture & Design
- **Mini-Project Architecture**: Serverless design. The Web UI runs entirely client-side without any backend HTTP server, while the C program runs as a standalone console executable.
- **Shared Data Specification**: Both the C backend and the web frontend read and write to the same standardized pipe-delimited text record format (`books.txt`).
- **Zero-Dependency Execution**: Can be run on any Windows machine without installing web servers, runtime frameworks, or database engines.

### 2. C Backend Implementation Details
- **Language Standard**: ISO/IEC 9899:1999 (C99 Standard C).
- **Compiler**: GCC 16.2.0 (w64devkit / MinGW-w64) with `-O2 -Wall -Wextra`.
- **C Header Files Used**:
  - `<stdio.h>`: File stream input/output (`fopen`, `fprintf`, `fgets`, `fclose`), standard I/O (`printf`, `fgets`).
  - `<stdlib.h>`: Memory management and string-to-integer conversion (`atoi`, `exit`).
  - `<string.h>`: String handling and tokenization (`strtok`, `strlen`, `strcspn`, `strstr`, `memcpy`).
  - `<ctype.h>`: Character case transformations (`tolower`) for case-insensitive keyword searching.
- **Data Model (`struct Book`)**:
  - `int id`: Unique sequential identifier.
  - `char title[100]`: Book title.
  - `char author[100]`: Author name.
  - `char isbn[30]`: ISBN identifier.
  - `char category[50]`: Genre / classification.
  - `int pub_year`: Publication year.
  - `int total_copies`: Total stock volume.
  - `int available_copies`: Quantity currently on the shelves.
  - `char shelf[30]`: Shelf bay code.
- **C Functions Implemented**:
  - `load_books_from_file()`: Reads `books.txt` on startup into in-memory array.
  - `save_books_to_file()`: Writes all records back to `books.txt`.
  - `seed_sample_books()`: Populates default 12 sample books if file is missing.
  - `display_all_books()`: Formatted tabular display of all books.
  - `add_book()`: Interactive console input for registering a book.
  - `search_book()`: Interactive sub-menu to search by all fields, title, author, category, ISBN, or ID, with quick actions (issue/return/edit) on matching results.
  - `update_book()`: Allows selective in-place field updates.
  - `delete_book()`: Shifts array elements and saves updated list to file.
  - `issue_book()`: Decrements copy count and records borrower.
  - `return_book()`: Increments copy count and prompts with an option to immediately edit book details.
  - `view_statistics()`: Calculates total titles, total copies, available copies, and loaned copies.

### 3. Frontend Implementation Details
- **Styling (`styles.css`)**:
  - Pure solid white background (`--bg-body: #ffffff;`).
  - **Zero gradients** anywhere in the code.
  - Solid border lines (`#e5e7eb`), crisp dark charcoal typography (`#111827`), clean status pills.
  - Responsive layout optimized for mobile and desktop screens.
- **Markup (`index.html`)**:
  - Clean semantic HTML5.
  - Plain typography title: **Library Management System** (no logo graphics).
  - High-visibility inventory table, stat boxes, search bar, and modal forms.
- **Scripting (`app.js`)**:
  - Vanilla JavaScript (ES6+): zero external libraries or frameworks.
  - Browser `localStorage` for instant local persistence.
  - Responsive event listeners for live search, category pill toggles, and modal forms.
  - Client-side File API (`FileReader` and `Blob`) for exporting and importing `books.txt`.

### 4. Data Storage Schema (`books.txt`)
Each record is stored as a single pipe-delimited line:
```
<id>|<title>|<author>|<isbn>|<category>|<pub_year>|<total_copies>|<available_copies>|<shelf>
```
*Example line:*
```
1|The C Programming Language|Brian W. Kernighan, Dennis Ritchie|978-0131103627|Computer Science|1988|5|4|A-101
```

---

## 📁 Directory Structure

```
library management system/
├── index.html            <- Main Web Frontend (Double-click to open!)
├── library.c             <- Standalone C Program source code (Mini Project)
├── library.exe           <- Compiled C Program executable
├── books.txt             <- Shared persistent database file (12 sample books)
├── build.bat             <- Compiles library.c using GCC
├── run.bat               <- Interactive launcher menu
├── Makefile              <- GNU Makefile for GCC / MinGW
├── README.md             <- Full documentation
└── frontend/
    ├── index.html        <- Web Frontend
    ├── css/
    │   └── styles.css    <- Clean white stylesheet (zero gradients)
    └── js/
        └── app.js        <- Standalone client-side application controller
```
