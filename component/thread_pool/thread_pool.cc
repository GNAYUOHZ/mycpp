#include "thread_pool.h"

#include <iostream>

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t thread_size) : stop_(false) {
  for (size_t i = 0; i < thread_size; ++i) {
    workers_.push_back(std::thread(&ThreadPool::ThreadLoop, this));
  }
}

// the destructor joins all threads
ThreadPool::~ThreadPool() {
  if (!stop_) {
    WaitAndClose();
  }
}

void ThreadPool::ThreadLoop() {
  while (true) {
    TaskFunc task;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
      if (stop_ && tasks_.empty()) return;
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    task();
  }
}

void ThreadPool::WaitAndClose() {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    stop_ = true;
  }
  cond_.notify_all();
  for (auto& worker : workers_) {
    worker.join();
  }
  std::cout << "ThreadPool closed" << std::endl;
}