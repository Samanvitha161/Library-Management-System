/**
 * LibraryOS REST API Client
 * Interfaces directly with the native C backend HTTP server
 */
const API = {
  baseUrl: '/api',

  async request(endpoint, options = {}) {
    const url = `${this.baseUrl}${endpoint}`;
    const defaultHeaders = {
      'Content-Type': 'application/json',
      'Accept': 'application/json'
    };

    const config = {
      ...options,
      headers: {
        ...defaultHeaders,
        ...options.headers
      }
    };

    try {
      const response = await fetch(url, config);
      const data = await response.json();

      if (!response.ok) {
        throw new Error(data.message || `Request failed with status ${response.status}`);
      }
      return data;
    } catch (err) {
      console.error(`[API Error] ${endpoint}:`, err);
      throw err;
    }
  },

  // Stats
  async getStats() {
    return this.request('/stats');
  },

  // Books
  async getBooks(search = '', category = '') {
    const params = new URLSearchParams();
    if (search) params.append('search', search);
    if (category && category !== 'All') params.append('category', category);
    const qs = params.toString() ? `?${params.toString()}` : '';
    return this.request(`/books${qs}`);
  },

  async getBook(id) {
    return this.request(`/books/${id}`);
  },

  async addBook(bookData) {
    return this.request('/books', {
      method: 'POST',
      body: JSON.stringify(bookData)
    });
  },

  async updateBook(id, bookData) {
    return this.request(`/books/${id}`, {
      method: 'PUT',
      body: JSON.stringify(bookData)
    });
  },

  async deleteBook(id) {
    return this.request(`/books/${id}`, {
      method: 'DELETE'
    });
  },

  // Members
  async getMembers() {
    return this.request('/members');
  },

  async addMember(memberData) {
    return this.request('/members', {
      method: 'POST',
      body: JSON.stringify(memberData)
    });
  },

  async updateMember(id, memberData) {
    return this.request(`/members/${id}`, {
      method: 'PUT',
      body: JSON.stringify(memberData)
    });
  },

  async deleteMember(id) {
    return this.request(`/members/${id}`, {
      method: 'DELETE'
    });
  },

  // Circulation
  async getTransactions() {
    return this.request('/transactions');
  },

  async issueBook(bookId, memberId, loanDays = 14) {
    return this.request('/issue', {
      method: 'POST',
      body: JSON.stringify({
        book_id: parseInt(bookId, 10),
        member_id: parseInt(memberId, 10),
        loan_days: parseInt(loanDays, 10)
      })
    });
  },

  async returnBook(transactionId, fine = -1.0) {
    return this.request('/return', {
      method: 'POST',
      body: JSON.stringify({
        transaction_id: parseInt(transactionId, 10),
        fine: parseFloat(fine)
      })
    });
  },

  async renewBook(transactionId, extraDays = 14) {
    return this.request('/renew', {
      method: 'POST',
      body: JSON.stringify({
        transaction_id: parseInt(transactionId, 10),
        extra_days: parseInt(extraDays, 10)
      })
    });
  },

  // Database Reset
  async resetData() {
    return this.request('/reset', {
      method: 'POST'
    });
  }
};
