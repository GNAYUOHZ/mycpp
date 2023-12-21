// g++ mergesort.cc --std=c++17

#include <iostream>
#include <vector>

// 将两个已排序的部分 arr[l..m] 和 arr[m+1..r] 合并
void merge(std::vector<int>& arr, int l, int m, int r) {
  std::vector<int> temp(r - l + 1);
  int i = l;      // i指向左子序列的第一位置
  int j = m + 1;  // j指向右子序列的第一位置
  int k = 0;      // 指向temp的第一位置

  while (i <= m && j <= r) {
    if (arr[i] < arr[j])  // 永远都是 i 和 j 指向的数进行比较
      temp[k++] = arr[i++];
    else
      temp[k++] = arr[j++];
  }
  while (i <= m) {
    temp[k++] = arr[i++];
  }
  while (j <= r) {
    temp[k++] = arr[j++];
  }
  // 排序后的数组拷贝回原数组
  for (k = 0; k < temp.size(); ++k) {
    arr[l + k] = temp[k];
  }
}

// 归并排序算法
void MergeSort(std::vector<int>& arr, int l, int r) {
  if (l < r) {
    // 找到中间点来分割数组
    int m = l + (r - l) / 2;

    // 递归调用分为两部分的子序列
    MergeSort(arr, l, m);
    MergeSort(arr, m + 1, r);

    // 合并子序列
    merge(arr, l, m, r);
  }
}

int main() {
  std::vector<int> arr = {10, 7, 8, 9, 1, 5};
  int n = arr.size();

  MergeSort(arr, 0, n - 1);

  std::cout << "Sorted array: ";
  for (int i = 0; i < n; i++) {
    std::cout << arr[i] << " ";
  }
  std::cout << std::endl;

  return 0;
}
