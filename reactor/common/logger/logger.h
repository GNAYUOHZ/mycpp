#pragma once
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

enum LogLevel { DEBUG, INFO, ERROR, FATAL };
// Logger 类
class Logger {
 public:
  explicit Logger(LogLevel level) : m_level(level), m_stream() {}
  ~Logger() { endLine(); }

  // 将 std::ostream 操作符重载为输出到 m_stream
  template <typename T>
  Logger& operator<<(T&& value) {
    m_stream << std::forward<T>(value);
    return *this;
  }

 private:
  // 结束日志条目并实际写入
  void endLine();

 private:
  LogLevel m_level;
  std::ostringstream m_stream;
  static std::mutex m_mutex;
};

// 日志宏定义
#define LOG_DEBUG Logger(DEBUG)
#define LOG_INFO Logger(INFO)
#define LOG_ERROR Logger(ERROR)
#define LOG_FATAL Logger(FATAL)