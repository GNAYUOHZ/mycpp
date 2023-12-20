// g++ quicksort.cc --std=c++17

#include <algorithm>  // std::swap
#include <iostream>
#include <vector>

// 分区函数
int partition(std::vector<int>& arr, int low, int high) {
  // 选择最后一个元素作为基准
  int pivot = arr[high];

  for (int i = low; i <= high - 1; ++i) {
    // 如果当前元素小于或等于基准，就交换到low位置
    if (arr[i] <= pivot) {
      std::swap(arr[low], arr[i]);
      ++low;
    }
  }
  // 将基准放到正确的位置
  std::swap(arr[low], arr[high]);
  return low;  // 返回基准所在的位置
}

// 快速排序的递归函数
void QuickSort(std::vector<int>& arr, int low, int high) {
  if (low < high) {
    // pi 是基准的索引，arr[pi]在排序后的正确位置
    int pi = partition(arr, low, high);

    // 分别对基准前后的子数组进行递归排序
    QuickSort(arr, low, pi - 1);
    QuickSort(arr, pi + 1, high);
  }
}

int main() {
  std::vector<int> arr = {10, 7, 8, 9, 1, 5};
  int n = arr.size();
  QuickSort(arr, 0, n - 1);

  std::cout << "Sorted array: ";
  for (int i = 0; i < n; i++) {
    std::cout << arr[i] << " ";
  }
  std::cout << std::endl;

  return 0;
}
