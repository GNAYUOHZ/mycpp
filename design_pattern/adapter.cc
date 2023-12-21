// 适配器模式将一个类的接口转换成客户希望的另一个接口。
// Adapter模式使得原本由于接口不兼容而不能一起工作的那些类可以一起工作。

// g++ adapter.cc -std=c++17

#include <iostream>

// 被适配的类
class Adaptee {
 public:
  void SpecificRequest() {
    std::cout << "Specific Request is called." << std::endl;
  }
};

// 目标接口
class Target {
 public:
  virtual void Request() = 0;
};

// 适配器
class Adapter : public Target {
 private:
  Adaptee* adaptee_;

 public:
  Adapter(Adaptee* adaptee) : adaptee_(adaptee) {}
  void Request() override { adaptee_->SpecificRequest(); }
};

int main() {
  Adaptee* adaptee = new Adaptee();
  Target* target = new Adapter(adaptee);
  target->Request();
  return 0;
}
