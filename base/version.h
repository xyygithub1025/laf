// LAF Base Library
// Copyright (C) 2019  Igara Studio S.A.
// Copyright (c) 2001-2016 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_VERSION_H_INCLUDED
#define BASE_VERSION_H_INCLUDED
#pragma once

#include <string>
#include <vector>

namespace base {

  class Version {
  public:
    typedef std::vector<int> Digits;

    Version();
    explicit Version(const std::string& from);

    bool operator<(const Version& other) const;
    bool operator==(const Version& other) const;
    bool operator!=(const Version& other) const { return !operator==(other); }

    const Digits& digits() const { return m_digits; }
    const std::string& prerelease() const { return m_prerelease; }
    const int prereleaseDigit() const { return m_prereleaseDigit; }

    std::string str() const;

  private:
    Digits m_digits;
    std::string m_prerelease; // alpha, beta, dev, rc (empty if it's official release)
    int m_prereleaseDigit = 0;
  };

}

#endif
