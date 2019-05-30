// LAF Base Library
// Copyright (C) 2019  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_TASK_H_INCLUDED
#define BASE_TASK_H_INCLUDED
#pragma once

#include "base/thread_pool.h"

#include <atomic>
#include <functional>

namespace base {

  class thread_pool;

  class task_token {
  public:
    task_token() : m_canceled(false), m_progress(0.0f) { }

    bool canceled() const { return m_canceled; }
    float progress() const { return m_progress; }

    void cancel() { m_canceled = true; }
    void set_progress(float progress) { m_progress = progress; }

  private:
    std::atomic<bool> m_canceled;
    std::atomic<float> m_progress;
  };

  class task {
  public:
    typedef std::function<void(task_token&)> func_t;

    task();
    ~task();

    void on_execute(func_t&& f) { m_execute = std::move(f); }

    task_token& start(thread_pool& pool);

    bool running() const { return m_running; }
    bool completed() const { return m_completed; }

  private:
    void in_worker_thread();

    std::atomic<bool> m_running;
    std::atomic<bool> m_completed;
    task_token m_token;
    func_t m_execute;
  };

} // namespace base

#endif
