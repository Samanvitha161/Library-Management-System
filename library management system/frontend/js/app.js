/**
 * Library Management System - Frontend Controller (Standalone Mini Project)
 * Runs directly in any web browser without needing any backend server!
 * Stores data in browser localStorage and synchronizes with books.txt format.
 */

const DEFAULT_SAMPLE_BOOKS = [
  { id: 1, title: "The C Programming Language", author: "Brian W. Kernighan, Dennis Ritchie", category: "Computer Science", isbn: "978-0131103627", pub_year: 1988, total_copies: 5, available_copies: 4, shelf: "A-101" },
  { id: 2, title: "Introduction to Algorithms (CLRS)", author: "Thomas H. Cormen, Charles Leiserson", category: "Computer Science", isbn: "978-0262033848", pub_year: 2009, total_copies: 4, available_copies: 3, shelf: "A-102" },
  { id: 3, title: "Design Patterns", author: "Erich Gamma, Richard Helm", category: "Software Engineering", isbn: "978-0201633610", pub_year: 1994, total_copies: 3, available_copies: 2, shelf: "A-103" },
  { id: 4, title: "Clean Code", author: "Robert C. Martin", category: "Software Engineering", isbn: "978-0132350884", pub_year: 2008, total_copies: 6, available_copies: 5, shelf: "A-104" },
  { id: 5, title: "Operating System Concepts", author: "Abraham Silberschatz, Peter Galvin", category: "Operating Systems", isbn: "978-1118063330", pub_year: 2012, total_copies: 4, available_copies: 4, shelf: "B-201" },
  { id: 6, title: "Computer Systems: A Programmer's Perspective", author: "Randal E. Bryant, David O'Hallaron", category: "Computer Science", isbn: "978-0134092669", pub_year: 2015, total_copies: 3, available_copies: 2, shelf: "B-202" },
  { id: 7, title: "Artificial Intelligence: A Modern Approach", author: "Stuart Russell, Peter Norvig", category: "Artificial Intelligence", isbn: "978-0136042594", pub_year: 2020, total_copies: 5, available_copies: 4, shelf: "B-203" },
  { id: 8, title: "Deep Learning", author: "Ian Goodfellow, Yoshua Bengio", category: "Artificial Intelligence", isbn: "978-0262035613", pub_year: 2016, total_copies: 4, available_copies: 3, shelf: "B-204" },
  { id: 9, title: "The Pragmatic Programmer", author: "David Thomas, Andrew Hunt", category: "Software Engineering", isbn: "978-0135957059", pub_year: 2019, total_copies: 5, available_copies: 5, shelf: "C-301" },
  { id: 10, title: "Database System Concepts", author: "Abraham Silberschatz, Henry Korth", category: "Database Systems", isbn: "978-0078022159", pub_year: 2019, total_copies: 4, available_copies: 4, shelf: "C-302" },
  { id: 11, title: "Structure and Interpretation of Computer Programs", author: "Harold Abelson, Gerald Sussman", category: "Computer Science", isbn: "978-0262510875", pub_year: 1996, total_copies: 3, available_copies: 3, shelf: "C-303" },
  { id: 12, title: "To Kill a Mockingbird", author: "Harper Lee", category: "Literature", isbn: "978-0061120084", pub_year: 1960, total_copies: 4, available_copies: 3, shelf: "D-401" }
];

const app = {
  books: [],
  selectedCategory: 'All',
  searchQuery: '',
  searchField: 'all',

  init() {
    this.loadBooksFromStorage();
    this.setupSearch();
    this.setupCategoryFilters();
    this.render();
  },

  // Load books from localStorage or initialize with sample books
  loadBooksFromStorage() {
    const stored = localStorage.getItem('library_mini_books');
    if (stored) {
      try {
        this.books = JSON.parse(stored);
      } catch (e) {
        this.books = [...DEFAULT_SAMPLE_BOOKS];
      }
    } else {
      this.books = [...DEFAULT_SAMPLE_BOOKS];
      this.saveBooksToStorage();
    }
  },

  // Save current books state to localStorage
  saveBooksToStorage() {
    localStorage.setItem('library_mini_books', JSON.stringify(this.books));
  },

  // Search input and controls handling
  setupSearch() {
    const input = document.getElementById('searchInput');
    const btnClear = document.getElementById('btnClearSearch');

    if (input) {
      // Live search as user types
      input.addEventListener('input', (e) => {
        this.searchQuery = e.target.value.trim().toLowerCase();
        if (btnClear) {
          btnClear.style.display = this.searchQuery ? 'block' : 'none';
        }
        this.render();
      });

      // Pressing Enter executes search
      input.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
          e.preventDefault();
          this.executeSearch();
        }
      });
    }
  },

  // Explicit Search button click
  executeSearch() {
    const input = document.getElementById('searchInput');
    const btnClear = document.getElementById('btnClearSearch');
    if (input) {
      this.searchQuery = input.value.trim().toLowerCase();
      if (btnClear) {
        btnClear.style.display = this.searchQuery ? 'block' : 'none';
      }
    }
    this.render();
  },

  // Handle Search By Field dropdown change
  handleSearchFieldChange() {
    const select = document.getElementById('searchField');
    if (select) {
      this.searchField = select.value;
    }
    const input = document.getElementById('searchInput');
    if (input) {
      const placeholders = {
        all: "Type keyword to search all fields (Title, Author, Category, ISBN, Shelf)...",
        title: "Search by book title (e.g. C Programming, Clean Code)...",
        author: "Search by author name (e.g. Ritchie, Martin, Cormen)...",
        category: "Search by category (e.g. Computer Science, AI)...",
        isbn: "Search by ISBN (e.g. 978-0131103627)...",
        shelf: "Search by shelf location (e.g. A-101, B-201)..."
      };
      input.placeholder = placeholders[this.searchField] || "Type keyword to search books...";
    }
    this.render();
  },

  // Get readable label for selected search field
  getSearchFieldLabel() {
    const labels = {
      all: "All Fields",
      title: "Title",
      author: "Author",
      category: "Category",
      isbn: "ISBN",
      shelf: "Shelf Location"
    };
    return labels[this.searchField] || "All Fields";
  },

  // Clear search query and reset view
  clearSearch() {
    const input = document.getElementById('searchInput');
    const btnClear = document.getElementById('btnClearSearch');
    if (input) input.value = '';
    if (btnClear) btnClear.style.display = 'none';
    this.searchQuery = '';
    this.selectedCategory = 'All';
    document.querySelectorAll('#categoryFilters .pill').forEach(p => {
      p.classList.toggle('active', p.getAttribute('data-category') === 'All');
    });
    this.render();
  },

  // Category filter pills
  setupCategoryFilters() {
    document.querySelectorAll('#categoryFilters .pill').forEach(pill => {
      pill.addEventListener('click', () => {
        document.querySelectorAll('#categoryFilters .pill').forEach(p => p.classList.remove('active'));
        pill.classList.add('active');
        this.selectedCategory = pill.getAttribute('data-category');
        this.render();
      });
    });
  },

  // Filter books based on search query, search field, and category
  getFilteredBooks() {
    return this.books.filter(b => {
      const matchCategory = this.selectedCategory === 'All' || b.category === this.selectedCategory;
      if (!this.searchQuery) return matchCategory;

      const q = this.searchQuery;
      let matchSearch = false;

      switch (this.searchField) {
        case 'title':
          matchSearch = b.title && b.title.toLowerCase().includes(q);
          break;
        case 'author':
          matchSearch = b.author && b.author.toLowerCase().includes(q);
          break;
        case 'category':
          matchSearch = b.category && b.category.toLowerCase().includes(q);
          break;
        case 'isbn':
          matchSearch = (b.isbn && b.isbn.toLowerCase().includes(q)) || String(b.id) === q;
          break;
        case 'shelf':
          matchSearch = b.shelf && b.shelf.toLowerCase().includes(q);
          break;
        case 'all':
        default:
          matchSearch =
            (b.title && b.title.toLowerCase().includes(q)) ||
            (b.author && b.author.toLowerCase().includes(q)) ||
            (b.category && b.category.toLowerCase().includes(q)) ||
            (b.isbn && b.isbn.toLowerCase().includes(q)) ||
            String(b.id) === q ||
            (b.shelf && b.shelf.toLowerCase().includes(q));
          break;
      }

      return matchCategory && matchSearch;
    });
  },

  // Main render function
  render() {
    const filtered = this.getFilteredBooks();
    this.renderTable(filtered);
    this.renderStats();
    this.renderSearchStatus(filtered);
  },

  // Render search results status bar
  renderSearchStatus(filtered) {
    const bar = document.getElementById('searchStatusBar');
    const text = document.getElementById('searchStatusText');
    if (!bar || !text) return;

    if (this.searchQuery || this.selectedCategory !== 'All') {
      bar.style.display = 'flex';
      let msg = `Showing <strong>${filtered.length}</strong> ${filtered.length === 1 ? 'book' : 'books'}`;
      if (this.searchQuery) {
        msg += ` matching "<strong>${this.escapeHtml(this.searchQuery)}</strong>" in <em>${this.getSearchFieldLabel()}</em>`;
      }
      if (this.selectedCategory !== 'All') {
        msg += ` (Category: <strong>${this.escapeHtml(this.selectedCategory)}</strong>)`;
      }
      text.innerHTML = msg;
    } else {
      bar.style.display = 'none';
    }
  },

  // Render stats bar
  renderStats() {
    const totalTitles = this.books.length;
    let totalCopies = 0;
    let availableCopies = 0;

    this.books.forEach(b => {
      totalCopies += parseInt(b.total_copies, 10) || 0;
      availableCopies += parseInt(b.available_copies, 10) || 0;
    });

    const borrowedCopies = Math.max(0, totalCopies - availableCopies);

    document.getElementById('statTitles').textContent = totalTitles;
    document.getElementById('statTotalCopies').textContent = totalCopies;
    document.getElementById('statAvailableCopies').textContent = availableCopies;
    document.getElementById('statBorrowedCopies').textContent = borrowedCopies;
  },

  // Render books table
  renderTable(books) {
    const tbody = document.getElementById('booksTableBody');
    const badge = document.getElementById('booksCountBadge');
    if (badge) badge.textContent = `${books.length} ${books.length === 1 ? 'book' : 'books'}`;

    if (!tbody) return;

    if (books.length === 0) {
      tbody.innerHTML = `
        <tr>
          <td colspan="10" class="text-center" style="padding: 2.5rem; color: var(--text-muted);">
            No books found matching the search or category filter.
            <div style="margin-top: 0.75rem;">
              <button class="btn btn-outline btn-sm" onclick="app.clearSearch()">Clear Filters</button>
            </div>
          </td>
        </tr>
      `;
      return;
    }

    tbody.innerHTML = books.map(b => {
      const isAvailable = b.available_copies > 0;
      const statusBadge = isAvailable
        ? `<span class="badge-status badge-available">Available (${b.available_copies})</span>`
        : `<span class="badge-status badge-out">Out of Stock</span>`;

      return `
        <tr>
          <td style="color: var(--text-light); font-weight: 500;">#${b.id}</td>
          <td>
            <strong style="color: var(--text-main); font-size: 0.92rem;">${this.escapeHtml(b.title)}</strong>
          </td>
          <td>${this.escapeHtml(b.author)}</td>
          <td><span class="category-tag">${this.escapeHtml(b.category)}</span></td>
          <td style="font-family: monospace; font-size: 0.85rem; color: var(--text-muted);">${this.escapeHtml(b.isbn)}</td>
          <td>${b.pub_year}</td>
          <td><strong>${b.available_copies}</strong> / ${b.total_copies}</td>
          <td style="color: var(--text-muted);">${this.escapeHtml(b.shelf)}</td>
          <td>${statusBadge}</td>
          <td>
            <div class="actions-cell">
              <button class="btn btn-outline btn-sm" onclick="app.handleIssueBook(${b.id})" title="Issue one copy" ${b.available_copies <= 0 ? 'disabled style="opacity:0.5; cursor:not-allowed;"' : ''}>Issue</button>
              <button class="btn btn-outline btn-sm" onclick="app.handleReturnBook(${b.id})" title="Return copy and edit">Return</button>
              <button class="btn btn-edit btn-sm" onclick="app.openEditBookModal(${b.id})" title="Edit details">Edit</button>
              <button class="btn btn-delete btn-sm" onclick="app.handleDeleteBook(${b.id})" title="Delete book">Delete</button>
            </div>
          </td>
        </tr>
      `;
    }).join('');
  },

  // Open Add Book Modal
  openAddBookModal() {
    document.getElementById('modalTitle').textContent = 'Add New Book';
    document.getElementById('btnSaveSubmit').textContent = 'Add Book';
    document.getElementById('bookId').value = '';
    document.getElementById('bookForm').reset();
    document.getElementById('bookYear').value = new Date().getFullYear();
    document.getElementById('bookCopies').value = 3;
    document.getElementById('bookShelf').value = 'A-101';
    this.openModal('bookModal');
  },

  // Open Edit Book Modal
  openEditBookModal(id) {
    const book = this.books.find(b => b.id === id);
    if (!book) return;

    document.getElementById('modalTitle').textContent = 'Edit Book Details';
    document.getElementById('btnSaveSubmit').textContent = 'Save Changes';
    document.getElementById('bookId').value = book.id;
    document.getElementById('bookTitle').value = book.title;
    document.getElementById('bookAuthor').value = book.author;
    document.getElementById('bookCategory').value = book.category;
    document.getElementById('bookIsbn').value = book.isbn;
    document.getElementById('bookYear').value = book.pub_year;
    document.getElementById('bookCopies').value = book.total_copies;
    document.getElementById('bookShelf').value = book.shelf;

    this.openModal('bookModal');
  },

  // Save Book (Add or Update)
  handleSaveBook(e) {
    e.preventDefault();
    const id = document.getElementById('bookId').value;
    const title = document.getElementById('bookTitle').value.trim();
    const author = document.getElementById('bookAuthor').value.trim();
    const category = document.getElementById('bookCategory').value;
    const isbn = document.getElementById('bookIsbn').value.trim();
    const pub_year = parseInt(document.getElementById('bookYear').value, 10) || 2024;
    const total_copies = parseInt(document.getElementById('bookCopies').value, 10) || 1;
    const shelf = document.getElementById('bookShelf').value.trim() || 'General';

    if (!title || !author) {
      this.showToast('Title and Author are required', 'error');
      return;
    }

    if (id) {
      // Update existing book
      const bookId = parseInt(id, 10);
      const book = this.books.find(b => b.id === bookId);
      if (book) {
        const diff = total_copies - book.total_copies;
        book.title = title;
        book.author = author;
        book.category = category;
        book.isbn = isbn;
        book.pub_year = pub_year;
        book.total_copies = total_copies;
        book.available_copies = Math.max(0, Math.min(total_copies, book.available_copies + diff));
        book.shelf = shelf;
        this.saveBooksToStorage();
        this.showToast(`Updated "${title}" successfully`, 'success');
      }
    } else {
      // Add new book with next sequential ID
      const maxId = this.books.reduce((max, b) => (b.id > max ? b.id : max), 0);
      const newBook = {
        id: maxId + 1,
        title,
        author,
        category,
        isbn,
        pub_year,
        total_copies,
        available_copies: total_copies,
        shelf
      };
      this.books.push(newBook);
      this.saveBooksToStorage();
      this.showToast(`Added "${title}" to catalog`, 'success');
    }

    this.closeModal('bookModal');
    this.render();
  },

  // Delete Book
  handleDeleteBook(id) {
    const book = this.books.find(b => b.id === id);
    const title = book ? `"${book.title}"` : 'this book';

    if (!confirm(`Are you sure you want to delete ${title}?`)) {
      return;
    }

    this.books = this.books.filter(b => b.id !== id);
    this.saveBooksToStorage();
    this.showToast(`Deleted ${title}`, 'info');
    this.render();
  },

  // Issue Book (checkout one copy)
  handleIssueBook(id) {
    const book = this.books.find(b => b.id === id);
    if (!book) return;
    if (book.available_copies <= 0) {
      this.showToast(`No copies available for "${book.title}"`, 'error');
      return;
    }
    book.available_copies--;
    this.saveBooksToStorage();
    this.showToast(`Issued one copy of "${book.title}"`, 'success');
    this.render();
  },

  // Return Book (check in one copy and give option to edit it)
  handleReturnBook(id) {
    const book = this.books.find(b => b.id === id);
    if (!book) return;

    if (book.available_copies < book.total_copies) {
      book.available_copies++;
    } else {
      // If already at total copies, returning an extra copy increments both
      book.total_copies++;
      book.available_copies++;
    }

    this.saveBooksToStorage();
    this.render();

    this.lastReturnedBookId = id;
    const titleEl = document.getElementById('returnSuccessBookTitle');
    if (titleEl) {
      titleEl.textContent = `"${book.title}" (Available: ${book.available_copies} of ${book.total_copies} on shelf)`;
    }
    this.openModal('returnSuccessModal');
    this.showToast(`Returned copy of "${book.title}"`, 'success');
  },

  // Confirm edit book after return
  confirmEditAfterReturn() {
    this.closeModal('returnSuccessModal');
    if (this.lastReturnedBookId) {
      this.openEditBookModal(this.lastReturnedBookId);
    }
  },

  // Reset to default sample books
  resetSampleBooks() {
    if (!confirm('Reset the catalog to the default 12 sample books?')) {
      return;
    }
    this.books = JSON.parse(JSON.stringify(DEFAULT_SAMPLE_BOOKS));
    this.saveBooksToStorage();
    this.showToast('Reset to default sample books', 'success');
    this.clearSearch();
  },

  // Export current books as books.txt (pipe-delimited format for C program)
  exportBooksTxt() {
    if (this.books.length === 0) {
      this.showToast('No books to export', 'error');
      return;
    }

    const lines = this.books.map(b => 
      `${b.id}|${b.title}|${b.author}|${b.isbn}|${b.category}|${b.pub_year}|${b.total_copies}|${b.available_copies}|${b.shelf}`
    );
    const content = lines.join('\n') + '\n';
    this.downloadFile('books.txt', content, 'text/plain');
    this.showToast('Exported books.txt for C backend', 'success');
  },

  // Import books.txt file selected by user
  importBooksTxt(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
      const text = e.target.result;
      const lines = text.split(/\r?\n/).filter(line => line.trim().length > 0);
      const parsed = [];

      lines.forEach(line => {
        const parts = line.split('|');
        if (parts.length >= 8) {
          parsed.push({
            id: parseInt(parts[0], 10),
            title: parts[1] || 'Untitled',
            author: parts[2] || 'Unknown',
            isbn: parts[3] || '',
            category: parts[4] || 'General',
            pub_year: parseInt(parts[5], 10) || 2024,
            total_copies: parseInt(parts[6], 10) || 1,
            available_copies: parseInt(parts[7], 10) || 1,
            shelf: parts[8] || 'General'
          });
        }
      });

      if (parsed.length > 0) {
        this.books = parsed;
        this.saveBooksToStorage();
        this.showToast(`Imported ${parsed.length} books from books.txt`, 'success');
        this.render();
      } else {
        this.showToast('Invalid books.txt format', 'error');
      }
    };
    reader.readAsText(file);
    event.target.value = ''; // reset file input
  },

  // Download file helper
  downloadFile(filename, content, type) {
    const blob = new Blob([content], { type: `${type};charset=utf-8;` });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = filename;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
  },

  // Modal helpers
  openModal(id) {
    const el = document.getElementById(id);
    if (el) el.classList.add('open');
  },

  closeModal(id) {
    const el = document.getElementById(id);
    if (el) el.classList.remove('open');
  },

  // Toast notification
  showToast(message, type = 'info') {
    const box = document.getElementById('toastBox');
    if (!box) return;

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.textContent = message;
    box.appendChild(toast);

    setTimeout(() => {
      toast.style.opacity = '0';
      setTimeout(() => toast.remove(), 300);
    }, 3000);
  },

  // Escape HTML helper
  escapeHtml(str) {
    if (!str) return '';
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#039;');
  }
};

// Start application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
  app.init();
});
