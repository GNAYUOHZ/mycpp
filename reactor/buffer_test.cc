#include "buffer.h"
#include <fcntl.h>
#include <unistd.h>
#include "gtest/gtest.h"

using namespace reactor;

class BufferTest : public testing::Test {
  void SetUp() override {}
  void TearDown() override {}
};

TEST(BufferTest, AppendAndRetrieve) {
  Buffer buffer;

  const std::string testData = "Hello, Buffer!";
  buffer.append(testData.data(), testData.size());
  EXPECT_EQ(buffer.readableBytes(), testData.size());

  std::string result = buffer.retrieveAllAsString();
  EXPECT_EQ(buffer.readableBytes(), 0);
  EXPECT_EQ(result, testData);
}

TEST(BufferTest, AppendMultipleTimes) {
  Buffer buffer;

  buffer.append("Hello, ", 7);
  buffer.append("Buffer!", 7);
  printf("\n%d\n", (int)buffer.readableBytes());
  EXPECT_EQ(buffer.readableBytes(), 14);
  EXPECT_EQ(std::string(buffer.peek(), buffer.readableBytes()), "Hello, Buffer!");

  buffer.retrieveAll();
  EXPECT_EQ(buffer.readableBytes(), 0);
}

TEST(BufferTest, PeekAndRetrievePartial) {
  Buffer buffer;

  const std::string testData = "Hello, Buffer!";
  buffer.append(testData.data(), testData.size());

  EXPECT_EQ(std::string(buffer.peek(), 5), "Hello");
  std::string partial = buffer.retrieveAsString(5);
  EXPECT_EQ(partial, "Hello");
  EXPECT_EQ(std::string(buffer.peek(), buffer.readableBytes()), ", Buffer!");
}

TEST(BufferTest, ReadFd) {
  int pipefd[2];
  ASSERT_EQ(pipe(pipefd), 0);

  pid_t pid = fork();
  if (pid == 0) {  // child
    close(pipefd[0]);
    const char* msg = "Hello from pipe!";
    ssize_t written = write(pipefd[1], msg, strlen(msg));
    ASSERT_EQ(written, strlen(msg));
    close(pipefd[1]);
    _exit(0);
  } else {
    close(pipefd[1]);

    Buffer buffer;
    int savedErrno;
    ssize_t bytesRead = buffer.readFd(pipefd[0], &savedErrno);
    ASSERT_GE(bytesRead, 0);

    std::string result = buffer.retrieveAllAsString();
    EXPECT_EQ(result, "Hello from pipe!");

    close(pipefd[0]);
    wait(nullptr);
  }
}
