// 单例模式可以确保一个类只有一个实例，并提供一个全局访问点。
//
// 线程不安全版本：适用于单线程环境，但是在多线程环境下，可能创建多个类的实例。
// 线程安全版本：使用互斥锁来确保在多线程环境下只创建一个类的实例。但是，每次调用获取实例的方法时都会进行锁定和解锁，这会对性能产生影响。
// 双检查锁版本：这是一种改进的线程安全版本。在调用获取实例的方法时，首先检查实例是否已经创建，只有在实例未创建的情况下才进行锁定和实例化。
//             这种方法既保证了线程安全，又能在大多数情况下避免锁定，从而提高性能。

// g++ singleton.cc -std=c++17

#include <iostream>
#include <mutex>

// 线程不安全版本-----------------------------------
class Singleton {
 private:
  Singleton() = default;
  Singleton(const Singleton& other);
  static Singleton* instance_;

 public:
  static Singleton* GetInstance();
};

Singleton* Singleton::instance_ = nullptr;

Singleton* Singleton::GetInstance() {
  if (instance_ == nullptr) {
    instance_ = new Singleton();
  }
  return instance_;
}

// 线程安全版本-----------------------------------
class Singleton1 {
 private:
  Singleton1() = default;
  Singleton1(const Singleton1& other);
  static Singleton1* instance_;
  static std::mutex mutex_;

 public:
  static Singleton1* GetInstance();
};

Singleton1* Singleton1::instance_ = nullptr;
std::mutex Singleton1::mutex_;

Singleton1* Singleton1::GetInstance() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (instance_ == nullptr) {
    instance_ = new Singleton1();
  }
  return instance_;
}

// 双检查锁版本-----------------------------------
class Singleton2 {
 private:
  Singleton2() = default;
  Singleton2(const Singleton2& other);
  static Singleton2* instance_;
  static std::mutex mutex_;

 public:
  static Singleton2* GetInstance();
};

Singleton2* Singleton2::instance_ = nullptr;
std::mutex Singleton2::mutex_;

Singleton2* Singleton2::GetInstance() {
  if (instance_ == nullptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance_ == nullptr) {
      instance_ = new Singleton2();
    }
  }
  return instance_;
}

int main() {
  std::cout << "Singleton Test:" << std::endl;
  Singleton* singleton1 = Singleton::GetInstance();
  Singleton* singleton2 = Singleton::GetInstance();

  std::cout << (singleton1 == singleton2 ? "Pass" : "Fail") << std::endl;

  std::cout << "Singleton1 (Thread-safe) Test:" << std::endl;
  Singleton1* singleton1_1 = Singleton1::GetInstance();
  Singleton1* singleton1_2 = Singleton1::GetInstance();

  std::cout << (singleton1_1 == singleton1_2 ? "Pass" : "Fail") << std::endl;

  std::cout << "Singleton2 (Double-checked Locking) Test:" << std::endl;
  Singleton2* singleton2_1 = Singleton2::GetInstance();
  Singleton2* singleton2_2 = Singleton2::GetInstance();

  std::cout << (singleton2_1 == singleton2_2 ? "Pass" : "Fail") << std::endl;

  return 0;
}
