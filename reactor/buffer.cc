#pragma once

#include "buffer.h"

#include <endian.h>
#include <sys/uio.h>

#include <algorithm>
#include <cassert>
#include <cstring>

namespace reactor {

const char Buffer::kCRLF[] = "\r\n";
const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

size_t Buffer::readableBytes() const { return writerIndex_ - readerIndex_; }
size_t Buffer::writableBytes() const { return buffer_.size() - writerIndex_; }
size_t Buffer::prependableBytes() const { return readerIndex_; }

const char* Buffer::peek() const { return begin() + readerIndex_; }
const char* Buffer::beginWrite() const { return begin() + writerIndex_; }
char* Buffer::beginWrite() { return begin() + writerIndex_; }

const char* Buffer::findCRLF() const {
  const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
  return crlf == beginWrite() ? NULL : crlf;
}
const char* Buffer::findEOL() const {
  const void* eol = std::memchr(peek(), '\n', readableBytes());
  return static_cast<const char*>(eol);
}

void Buffer::retrieve(size_t len) {
  if (len < readableBytes()) {
    readerIndex_ += len;
  } else {
    retrieveAll();
  }
}
void Buffer::retrieveAll() {
  readerIndex_ = kCheapPrepend;
  writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAllAsString() { return retrieveAsString(readableBytes()); }
std::string Buffer::retrieveAsString(size_t len) {
  std::string result(peek(), len);
  retrieve(len);
  return result;
}

void Buffer::append(const char* /*restrict*/ data, size_t len) {
  ensureWritableBytes(len);
  std::copy(data, data + len, beginWrite());
  writerIndex_ += len;
}
void Buffer::append(const std::string_view& str) { append(str.data(), str.size()); }
void Buffer::append(const void* /*restrict*/ data, size_t len) {
  append(static_cast<const char*>(data), len);
}

void Buffer::ensureWritableBytes(size_t len) {
  if (writableBytes() >= len) {
    return;
  }

  if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
    buffer_.resize(writerIndex_ + len);
  } else {
    // move readable data to the front, make space inside buffer
    size_t readable = readableBytes();
    std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
    readerIndex_ = kCheapPrepend;
    writerIndex_ = readerIndex_ + readable;
  }
}

void Buffer::appendInt64(int64_t x) {
  int64_t be64 = htobe64(x);
  append(&be64, sizeof be64);
}
void Buffer::appendInt32(int32_t x) {
  int32_t be32 = htobe32(x);
  append(&be32, sizeof be32);
}
void Buffer::appendInt16(int16_t x) {
  int16_t be16 = htobe16(x);
  append(&be16, sizeof be16);
}
void Buffer::appendInt8(int8_t x) { append(&x, sizeof x); }

int64_t Buffer::peekInt64() const {
  int64_t be64 = 0;
  ::memcpy(&be64, peek(), sizeof be64);
  return be64toh(be64);
}
int32_t Buffer::peekInt32() const {
  int32_t be32 = 0;
  ::memcpy(&be32, peek(), sizeof be32);
  return be32toh(be32);
}
int16_t Buffer::peekInt16() const {
  int16_t be16 = 0;
  ::memcpy(&be16, peek(), sizeof be16);
  return be16toh(be16);
}
int8_t Buffer::peekInt8() const {
  int8_t x = *peek();
  return x;
}

void Buffer::prependInt64(int64_t x) {
  int64_t be64 = htobe64(x);
  prepend(&be64, sizeof be64);
}
void Buffer::prependInt32(int32_t x) {
  int32_t be32 = htobe32(x);
  prepend(&be32, sizeof be32);
}
void Buffer::prependInt16(int16_t x) {
  int16_t be16 = htobe16(x);
  prepend(&be16, sizeof be16);
}
void Buffer::prependInt8(int8_t x) { prepend(&x, sizeof x); }
void Buffer::prepend(const void* /*restrict*/ data, size_t len) {
  assert(len <= prependableBytes());
  readerIndex_ -= len;
  const char* d = static_cast<const char*>(data);
  std::copy(d, d + len, begin() + readerIndex_);
}

void Buffer::shrink(size_t reserve) {
  Buffer other;
  other.ensureWritableBytes(readableBytes() + reserve);
  other.append(std::string_view(peek(), static_cast<int>(readableBytes())));
  std::swap(*this, other);
}

ssize_t Buffer::readFd(int fd, int* savedErrno) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = ::readv(fd, vec, iovcnt);
  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    writerIndex_ += n;
  } else {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  return n;
}

char* Buffer::begin() { return &*buffer_.begin(); }
const char* Buffer::begin() const { return &*buffer_.begin(); }

}  // namespace reactor