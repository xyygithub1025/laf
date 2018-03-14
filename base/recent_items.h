// LAF Base Library
// Copyright (c) 2001-2018 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_RECENT_ITEMS_H_INCLUDED
#define BASE_RECENT_ITEMS_H_INCLUDED
#pragma once

#include <list>
#include <algorithm>

#include "base/debug.h"

namespace base {

template<typename T>
class RecentItems {
public:
  typedef std::list<T> Items;
  typedef typename Items::iterator iterator;
  typedef typename Items::const_iterator const_iterator;

  RecentItems(std::size_t limit) : m_limit(limit) { }

  const_iterator begin() { return m_items.begin(); }
  const_iterator end() { return m_items.end(); }

  bool empty() const { return m_items.empty(); }
  std::size_t size() const { return m_items.size(); }
  std::size_t limit() const { return m_limit; }
  void clear() { m_items.clear(); }

  template<typename T2, typename Predicate>
  void addItem(const T2& item, Predicate p) {
    iterator it = std::find_if(m_items.begin(), m_items.end(), p);

    // If the item already exist in the list...
    if (it != m_items.end()) {
      // Move it to the first position
      m_items.erase(it);
      m_items.insert(m_items.begin(), item);
      return;
    }

    if (m_limit > 0)
      m_items.insert(m_items.begin(), item);

    while (m_items.size() > m_limit)
      m_items.erase(--m_items.end());
  }

  template<typename T2, typename Predicate>
  void removeItem(const T2& item, Predicate p) {
    iterator it = std::find_if(m_items.begin(), m_items.end(), p);
    if (it != m_items.end())
      m_items.erase(it);
  }

  void setLimit(const int newLimit) {
    ASSERT(newLimit >= 0);

    if (newLimit < m_items.size()) {
      auto it = m_items.begin();
      std::advance(it, newLimit);
      m_items.erase(it, m_items.end());
    }

    m_limit = newLimit;
  }

private:
  Items m_items;
  std::size_t m_limit;
};

} // namespace base

#endif
