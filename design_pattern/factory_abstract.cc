// 抽象工厂模式主要用来实现生产一系列的产品。

// g++ factory_abstract.cc -std=c++17

#include <iostream>

// 抽象产品类A
class AbstractProductA {
 public:
  virtual std::string FunctionA() const = 0;
};

// 产品A1的具体实现
class ProductA1 : public AbstractProductA {
 public:
  std::string FunctionA() const override { return "ProductA1"; }
};

// 产品A2的具体实现
class ProductA2 : public AbstractProductA {
  std::string FunctionA() const override { return "ProductA2"; }
};

// 抽象产品类B
class AbstractProductB {
 public:
  virtual std::string FunctionB() const = 0;
};

// 产品B1的具体实现
class ProductB1 : public AbstractProductB {
 public:
  std::string FunctionB() const override { return "ProductB1"; }
};

// 产品B2的具体实现
class ProductB2 : public AbstractProductB {
  std::string FunctionB() const override { return "ProductB2"; }
};

// 抽象工厂类
class AbstractFactory {
 public:
  virtual AbstractProductA *CreateProductA() const = 0;
  virtual AbstractProductB *CreateProductB() const = 0;
};

// 工厂1和2具体实现
class Factory1 : public AbstractFactory {
 public:
  AbstractProductA *CreateProductA() const override { return new ProductA1(); }
  AbstractProductB *CreateProductB() const override { return new ProductB1(); }
};

class Factory2 : public AbstractFactory {
 public:
  AbstractProductA *CreateProductA() const override { return new ProductA2(); }
  AbstractProductB *CreateProductB() const override { return new ProductB2(); }
};

int main() {
  AbstractFactory *factory1 = new Factory1();
  const AbstractProductA *productA = factory1->CreateProductA();
  const AbstractProductB *productB = factory1->CreateProductB();

  std::cout << productA->FunctionA() << std::endl;
  std::cout << productB->FunctionB() << std::endl;

  return 0;
}
