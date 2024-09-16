#include "logger.h"

std::mutex Logger::m_mutex;
std::vector<std::string> Logger::m_level_strs = {"DEBUG", "INFO", "ERROR", "FATAL"};

void Logger::endLine() {
  // 保护对 std::cout 的访问
  std::lock_guard<std::mutex> lock(m_mutex);

  // 输出日志级别和消息
  std::cout << m_level_strs[m_level] << ": " << m_stream.str();
  if (m_level == FATAL) std::cout << " Aborting...";
  std::cout << std::endl;

  // 清空流以便下次使用
  m_stream.str("");
  m_stream.clear();
  if (m_level == FATAL) exit(1);
}
