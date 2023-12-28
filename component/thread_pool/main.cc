// g++ main.cc thread_pool.cc -lpthread -std=c++17
#include <chrono>
#include <iostream>

#include "thread_pool.h"

std::atomic<int> g_succ_count;

int test_get_result(int val) { return val; }

void test_sleep_for(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  g_succ_count++;
}

int main() {
  ThreadPool pool(10);

  // test one
  auto result = pool.Enqueue(test_get_result, 10086);
  std::cout << "get result: " << result.get() << std::endl;

  // benchmark
  const int num_tasks = 100000;  // 任务数量
  const int work_ms = 1;         // 每项任务的工作时长（毫秒）
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_tasks; ++i) {
    pool.Enqueue(test_sleep_for, work_ms);  // 提交任务到线程池
  }
  pool.WaitAndClose();  // 等待所有任务完成

  // result = pool.Enqueue(test_get_result, 10086); // push after close: aborted

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Succ count: " << g_succ_count << std::endl;
  std::cout << "Total time taken: " << elapsed.count() << " seconds"
            << std::endl;
  std::cout << "Throughput: " << num_tasks / elapsed.count() << " tasks/second"
            << std::endl;
  return 0;
}