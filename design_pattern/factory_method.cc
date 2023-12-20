// 工厂方法（Factory Method）是一种创建型设计模式，
// 它定义了一个创建对象的接口，将实例化的类的选择延迟到子类中进行。

// 通过工厂接口将客户代码和创建具体产品的代码解耦，提高了系统的灵活性和可维护性。
// 如果新增一个具体产品，只需要添加一个相应的具体工厂，不需修改现有代码，符合开闭原则。

// g++ factory_method.cc -pthread -std=c++17

#include <iostream>

// 产品基类
class Product {
 public:
  virtual std::string Name() = 0;
  virtual ~Product() = default;
};

// 产品类A
class ProductA : public Product {
 public:
  std::string Name() override { return "ProductA"; }
};

// 产品类B
class ProductB : public Product {
 public:
  std::string Name() override { return "ProductB"; }
};

// 工厂基类
class Factory {
 public:
  virtual Product* CreateProduct() = 0;
  virtual ~Factory() = default;
};

// 创建产品A的工厂
class FactoryA : public Factory {
 public:
  Product* CreateProduct() override { return new ProductA(); }
};

// 创建产品B的工厂
class FactoryB : public Factory {
 public:
  Product* CreateProduct() override { return new ProductB(); }
};

int main() {
  Factory* factoryA = new FactoryA();
  Product* productA = factoryA->CreateProduct();
  std::cout << productA->Name() << std::endl;

  Factory* factoryB = new FactoryB();
  Product* productB = factoryB->CreateProduct();
  std::cout << productB->Name() << std::endl;
}
