#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  ThreadPool(size_t);
  ~ThreadPool();

  template <class F, class... Args>
  auto Enqueue(F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>;

  void WaitAndClose();

 private:
  void ThreadLoop();
  typedef std::function<void()> TaskFunc;

  std::vector<std::thread> workers_;
  std::queue<TaskFunc> tasks_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool stop_;
};

// add new work item to the pool
// F&& f 表示一个函数或可调用对象
// Args&&... args 表示该函数对象的参数列表
template <class F, class... Args>
auto ThreadPool::Enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
  // 用于推断函数F在参数Args...下的执行结果的类型
  using return_type = typename std::result_of<F(Args...)>::type;

  // 创建一个std::packaged_task对象，它包装了一个可调用对象，并允许异步获取该对象的调用结果
  // 这里将函数f和参数args...绑定为一个packaged_task
  auto task = std::make_shared<std::packaged_task<return_type()> >(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (stop_) throw std::runtime_error("enqueue on stopped ThreadPool");
    tasks_.emplace([task]() { (*task)(); });
  }
  cond_.notify_one();

  // 从packaged_task中获取一个std::future<return_type>对象，用于之后获取函数f的执行结果
  return task->get_future();
}

// std::packaged_task将一个可调用对象和一个承诺（promise）绑定在一起。
// 当std::packaged_task对象被调用时，它会调用与之关联的可调用对象，
// 并将返回值（或异常）传递到一个std::future对象中。
