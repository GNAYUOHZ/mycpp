#pragma once

#include <sys/uio.h>

#include <cstdint>
#include <string>
#include <vector>

namespace reactor {

/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
 public:
  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kCheapPrepend + initialSize),
        readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend) {}

  size_t readableBytes() const;
  size_t writableBytes() const;
  size_t prependableBytes() const;

  const char* peek() const;
  const char* beginWrite() const;
  char* beginWrite();

  const char* findCRLF() const;
  const char* findEOL() const;
  void retrieve(size_t len);
  void retrieveAll();
  std::string retrieveAllAsString();
  std::string retrieveAsString(size_t len);

  void append(const char* /*restrict*/ data, size_t len);
  void append(const std::string_view& str);
  void append(const void* /*restrict*/ data, size_t len);
  void ensureWritableBytes(size_t len);

  /// Append using network endian
  void appendInt64(int64_t x);
  void appendInt32(int32_t x);
  void appendInt16(int16_t x);
  void appendInt8(int8_t x);

  /// Peek from network endian
  int64_t peekInt64() const;
  int32_t peekInt32() const;
  int16_t peekInt16() const;
  int8_t peekInt8() const;

  /// Prepend using network endian
  void prependInt64(int64_t x);
  void prependInt32(int32_t x);
  void prependInt16(int16_t x);
  void prependInt8(int8_t x);
  void prepend(const void* /*restrict*/ data, size_t len);

  void shrink(size_t reserve);

  /// Read data directly into buffer.
  ssize_t readFd(int fd, int* savedErrno);

 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

 private:
  char* begin();
  const char* begin() const;

  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
  static const char kCRLF[];
};

}  // namespace reactor