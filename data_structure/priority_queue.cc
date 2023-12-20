/**
 * 优先级队列利用堆来维护优先级队列，其中堆顶的元素（即数组的第一个元素）是优先级最高的元素。
 * 可以通过提供自定义比较函数来定义元素的优先级。
 * 
 * g++ priority_queue.cc --std=c++17
 */

#include <algorithm>
#include <iostream>
#include <vector>

template <typename T, typename Container = std::vector<T>,
          typename Comparator = std::less<typename Container::value_type>>
class PriorityQueue {
 private:
  Container heap;
  Comparator comp;

  void heapifyDown(size_t idx) {
    size_t left = 2 * idx + 1;
    size_t right = 2 * idx + 2;
    size_t smallest = idx;

    if (left < heap.size() && !comp(heap[left], heap[idx])) {
      smallest = left;
    }
    if (right < heap.size() && !comp(heap[right], heap[smallest])) {
      smallest = right;
    }
    if (smallest != idx) {
      std::swap(heap[idx], heap[smallest]);
      heapifyDown(smallest);
    }
  }

  void heapifyUp(size_t idx) {
    while (idx > 0) {
      size_t parent = (idx - 1) / 2;
      if (comp(heap[idx], heap[parent])) {
        return;
      }
      std::swap(heap[parent], heap[idx]);
      idx = parent;
    }
  }

 public:
  // Insert element into the priority queue
  void push(const T& value) {
    heap.push_back(value);
    heapifyUp(heap.size() - 1);
  }

  // Remove the top element from the priority queue
  void pop() {
    if (heap.empty()) {
      throw std::runtime_error("Priority queue is empty!");
    }
    heap[0] = heap.back();
    heap.pop_back();
    if (!heap.empty()) {
      heapifyDown(0);
    }
  }

  // Return the top element from the priority queue
  const T& top() const {
    if (heap.empty()) {
      throw std::runtime_error("Priority queue is empty!");
    }
    return heap[0];
  }

  // Check if the priority queue is empty
  bool empty() const { return heap.empty(); }

  // Get the number of elements in the priority queue
  size_t size() const { return heap.size(); }

  // Helper function to print the priority queue
  void print() {
    auto temp(*this);  // Create a copy of the priority queue
    std::cout << "[";
    while (!temp.empty()) {
      std::cout << temp.top();
      temp.pop();
      if (!temp.empty()) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
  }

  PriorityQueue() {}
  PriorityQueue(Comparator comp) : comp(comp) {}  // for lambda
};

int main() {
  // 创建一个 int 类型的优先级队列，多种初始化方式
  PriorityQueue<int, std::vector<int>, std::greater<int>> pq;

  //   PriorityQueue<int> pq;

  //   struct ObjectComparator {
  //     bool operator()(int a, int b) { return a < b; }
  //   };
  //   PriorityQueue<int, std::vector<int>, ObjectComparator> pq;

  //   auto LambdaComparator = [](int a, int b) { return a < b; };
  //   PriorityQueue<int, decltype(LambdaComparator)> pq1(LambdaComparator);

  // 测试 push 操作
  std::cout << "Inserting elements into the priority queue: ";
  pq.push(10);
  pq.push(5);
  pq.push(20);
  pq.push(1);
  pq.print();  // 预期结果: [1, 5, 20, 10]

  // 查看顶部元素
  std::cout << "Top element: " << pq.top() << std::endl;  // 预期结果: 1

  // 测试 pop 操作
  std::cout << "Removing the top element (1): ";
  pq.pop();
  pq.print();  // 预期结果: [5, 10, 20]

  // 查看优先级队列是否为空
  std::cout << "Priority queue empty? " << (pq.empty() ? "Yes" : "No")
            << std::endl;  // 预期结果: No

  // 输出队列中的元素数量
  std::cout << "Priority queue size: " << pq.size()
            << std::endl;  // 预期结果: 3

  // 清空队列，并查看是否为空
  std::cout << "Emptying the priority queue: " << std::endl;
  while (!pq.empty()) pq.pop();
  std::cout << "Priority queue empty? " << (pq.empty() ? "Yes" : "No")
            << std::endl;  // 预期结果: Yes

  return 0;
}
