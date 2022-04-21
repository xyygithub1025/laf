// LAF Base Library
// Copyright (c) 2020-2022 Igara Studio S.A.
// Copyright (c) 2001-2017 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_STRING_H_INCLUDED
#define BASE_STRING_H_INCLUDED
#pragma once

#include <cstdarg>
#include <iterator>
#include <string>

namespace base {

  std::string string_printf(const char* format, ...);
  std::string string_vprintf(const char* format, std::va_list ap);

  std::string string_to_lower(const std::string& original);
  std::string string_to_upper(const std::string& original);

  std::string to_utf8(const wchar_t* src, const int n);

  inline std::string to_utf8(const std::wstring& widestring) {
   return to_utf8(widestring.c_str(), (int)widestring.size());
  }

  std::wstring from_utf8(const std::string& utf8string);

  int utf8_length(const std::string& utf8string);
  int utf8_icmp(const std::string& a, const std::string& b, int n = 0);

  template<typename SubIterator>
  class utf8_iteratorT : public std::iterator<std::forward_iterator_tag,
                                              std::string::value_type,
                                              std::string::difference_type,
                                              typename SubIterator::pointer,
                                              typename SubIterator::reference> {
  public:
    typedef typename SubIterator::pointer pointer; // Needed for GCC

    utf8_iteratorT() {
    }

    explicit utf8_iteratorT(const SubIterator& it)
      : m_internal(it) {
    }

    // Based on Allegro Unicode code (allegro/src/unicode.c)
    utf8_iteratorT& operator++() {
      int c = *m_internal;
      ++m_internal;

      if (c & 0x80) {
        int n = 1;
        while (c & (0x80>>n))
          n++;

        c &= (1<<(8-n))-1;

        while (--n > 0) {
          int t = *m_internal;
          ++m_internal;

          if ((!(t & 0x80)) || (t & 0x40)) {
            --m_internal;
            return *this;
          }

          c = (c<<6) | (t & 0x3F);
        }
      }

      return *this;
    }

    utf8_iteratorT& operator+=(int i) {
      while (i--)
        operator++();
      return *this;
    }

    utf8_iteratorT operator+(int i) {
      utf8_iteratorT it(*this);
      it += i;
      return it;
    }

    const int operator*() const {
      SubIterator it = m_internal;
      int c = *it;
      ++it;

      if (c & 0x80) {
        int n = 1;
        while (c & (0x80>>n))
          n++;

        c &= (1<<(8-n))-1;

        while (--n > 0) {
          int t = *it;
          ++it;

          if ((!(t & 0x80)) || (t & 0x40))
            return '^';

          c = (c<<6) | (t & 0x3F);
        }
      }

      return c;
    }

    bool operator==(const utf8_iteratorT& it) const {
      return m_internal == it.m_internal;
    }

    bool operator!=(const utf8_iteratorT& it) const {
      return m_internal != it.m_internal;
    }

    pointer operator->() {
      return m_internal.operator->();
    }

    std::string::difference_type operator-(const utf8_iteratorT& it) const {
      return m_internal - it.m_internal;
    }

  private:
    SubIterator m_internal;
  };

  class utf8_iterator : public utf8_iteratorT<std::string::iterator> {
  public:
    utf8_iterator() { }
    utf8_iterator(const utf8_iteratorT<std::string::iterator>& it)
      : utf8_iteratorT<std::string::iterator>(it) {
    }
    explicit utf8_iterator(const std::string::iterator& it)
      : utf8_iteratorT<std::string::iterator>(it) {
    }
  };

  class utf8_const_iterator : public utf8_iteratorT<std::string::const_iterator> {
  public:
    utf8_const_iterator() { }
    utf8_const_iterator(const utf8_iteratorT<std::string::const_iterator>& it)
      : utf8_iteratorT<std::string::const_iterator>(it) {
    }
    explicit utf8_const_iterator(const std::string::const_iterator& it)
      : utf8_iteratorT<std::string::const_iterator>(it) {
    }
  };

  class utf8 {
  public:
    utf8(std::string& s) : m_begin(utf8_iterator(s.begin())),
                           m_end(utf8_iterator(s.end())) {
    }
    const utf8_iterator& begin() const { return m_begin; }
    const utf8_iterator& end() const { return m_end; }
  private:
    utf8_iterator m_begin;
    utf8_iterator m_end;
  };

  class utf8_const {
  public:
    utf8_const(const std::string& s) : m_begin(utf8_const_iterator(s.begin())),
                                       m_end(utf8_const_iterator(s.end())) {
    }
    const utf8_const_iterator& begin() const { return m_begin; }
    const utf8_const_iterator& end() const { return m_end; }
  private:
    utf8_const_iterator m_begin;
    utf8_const_iterator m_end;
  };

  class utf8_decode {
  public:
    using string = std::string;
    using string_ref = const std::string&;
    using iterator = std::string::const_iterator;

    utf8_decode() { }
    utf8_decode(const utf8_decode&) = default;
    utf8_decode& operator=(const utf8_decode&) = default;

    explicit utf8_decode(string_ref str)
      : m_it(str.begin())
      , m_end(str.end()) {
    }

    bool is_end() const {
      return m_it == m_end;
    }

    bool is_valid() const {
      return m_valid;
    }

    int next() {
      if (m_it == m_end)
        return 0;

      int c = *m_it;
      ++m_it;

      // UTF-8 escape bit 0x80 to encode larger code points
      if (c & 0b1000'0000) {
        // Get the number of bytes following the first one 0b1xxx'xxxx.
        //
        // This is like "number of leading ones", similar to a
        // __builtin_clz(~x)-24 (for 8 bits), anyway doing some tests,
        // the CLZ intrinsic is not faster than this code in x86_64.
        int n = 0;
        int f = 0b0100'0000;
        while (c & f) {
          ++n;
          f >>= 1;
        }

        if (n == 0) {
          // Invalid UTF-8: 0b10xx'xxxx alone, i.e. not inside a
          // escaped sequence (e.g. after 0b110xx'xxx
          m_valid = false;
          return 0;
        }

        // Keep only the few initial data bits from the first byte (6
        // first bits if we have only one extra char, then for each
        // extra char we have less useful data in this first byte).
        c &= (0b0001'1111 >> (n-1));

        while (n--) {
          if (m_it == m_end) {
            // Invalid UTF-8: missing 0b10xx'xxxx bytes
            m_valid = false;
            return 0;
          }
          const int chr = *m_it;
          ++m_it;
          if ((chr & 0b1100'0000) != 0b1000'0000) {
            // Invalid UTF-8: Extra byte doesn't contain 0b10xx'xxxx
            m_valid = false;
            return 0;
          }
          c = (c << 6) | (chr & 0b0011'1111);
        }
      }

      return c;
    }

  private:
    iterator m_it;
    iterator m_end;
    bool m_valid = true;
  };

}

#endif
