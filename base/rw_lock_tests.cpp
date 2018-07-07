// LAF Base Library
// Copyright (c) 2001-2016 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gtest/gtest.h>

#include "base/rw_lock.h"

using namespace base;

TEST(RWLock, MultipleReaders)
{
  RWLock a;
  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_FALSE(a.lock(RWLock::WriteLock, 0));
  a.unlock();
  a.unlock();
  a.unlock();
  a.unlock();
}

TEST(RWLock, OneWriter)
{
  RWLock a;

  EXPECT_TRUE(a.lock(RWLock::WriteLock, 0));
  EXPECT_FALSE(a.lock(RWLock::ReadLock, 0));
  a.unlock();

  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_FALSE(a.lock(RWLock::WriteLock, 0));
  a.unlock();

  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_FALSE(a.lock(RWLock::WriteLock, 0));
  a.unlock();
  EXPECT_FALSE(a.lock(RWLock::WriteLock, 0));
  a.unlock();
  EXPECT_TRUE(a.lock(RWLock::WriteLock, 0));
  EXPECT_FALSE(a.lock(RWLock::WriteLock, 0));
  a.unlock();
}

TEST(RWLock, UpgradeToWrite)
{
  RWLock a;

  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_FALSE(a.lock(RWLock::WriteLock, 0));
  EXPECT_TRUE(a.upgradeToWrite(0));
  EXPECT_FALSE(a.lock(RWLock::ReadLock, 0));
  EXPECT_FALSE(a.lock(RWLock::WriteLock, 0));
  a.downgradeToRead();
  a.unlock();
}

TEST(RWLock, WeakLock)
{
  RWLock a;
  RWLock::WeakLock flag = RWLock::WeakUnlocked;

  EXPECT_TRUE(a.weakLock(&flag));
  EXPECT_EQ(RWLock::WeakLocked, flag);
  EXPECT_FALSE(a.lock(RWLock::ReadLock, 0));
  EXPECT_EQ(RWLock::WeakUnlocking, flag);
  a.weakUnlock();
  EXPECT_EQ(RWLock::WeakUnlocked, flag);

  EXPECT_TRUE(a.lock(RWLock::ReadLock, 0));
  EXPECT_FALSE(a.weakLock(&flag));
  a.unlock();
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
