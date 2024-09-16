#include <iostream>

using namespace std;

class Basic {
 public:
  int a;
  double b;

  void setB(double b) {
    this->b = b;
    return;
  }

  // 暴露私有成员方法的地址
  static void (Basic::*getSecretPtr())(int) { return &Basic::setC; }

 private:
  int c;
  double d;
  void setC(int c) {
    this->c = c;
    return;
  }
};

int main() {
  Basic tmp;
  tmp.a = 10;
  tmp.setB(3.14);
  int* pC = reinterpret_cast<int*>(reinterpret_cast<char*>(&tmp) + 16);
  *pC = 12;  // 直接修改c的值
  cout << tmp.a << endl;

  void (Basic::*funcPtr)(int) = Basic::getSecretPtr();
  // 调用私有成员函数
  (tmp.*funcPtr)(10);

  return 0;
}