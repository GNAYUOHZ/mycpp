#include <sys/epoll.h>

#include <functional>
#include <vector>

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0     /* No events registered. */
#define AE_READABLE 1 /* Fire when descriptor is readable. */
#define AE_WRITABLE 2 /* Fire when descriptor is writable. */
#define AE_BARRIER                                     \
  4 /* With WRITABLE, never fire the event if the      \
       READABLE event already fired in the same event  \
       loop iteration. Useful when you want to persist \
       things to disk before sending replies, and want \
       to do that in a group fashion. */

#define AE_DONT_WAIT (1 << 2)

class AeEventLoop {
 public:
  using aeFileProc = std::function<void(int fd, void *clientData, int mask)>;

 public:
  AeEventLoop(int size);
  ~AeEventLoop();

  void aeMain() {
    while (!stop_) {
      aeProcessEvents();
    }
  }

  /* Process every pending time event, then every pending file event
   * (that may be registered by time event callbacks just processed).
   * Without special flags the function sleeps until some file event
   * fires.
   *
   * The function returns the number of events processed. */
  int aeProcessEvents();

  int aeCreateFileEvent(int fd, int mask, aeFileProc proc, void *clientData);

  void aeDeleteFileEvent(int fd, int mask);

  void aeSetBeforeSleepProc(std::function<void()> beforesleep) {
    beforesleep_ = std::move(beforesleep);
  }

  void aeSetAfterSleepProc(std::function<void()> aftersleep) {
    aftersleep_ = std::move(aftersleep);
  }

  /* Resize the maximum set size of the event loop.
   * If the requested set size is smaller than the current set size, but
   * there is already a file descriptor in use that is >= the requested
   * set size minus one, AE_ERR is returned and the operation is not
   * performed at all.
   *
   * Otherwise AE_OK is returned and the operation is successful. */
  int aeResizeSetSize(int size);

  void aeStop() { stop_ = 1; }

  /* Tells the next iteration/s of the event processing to set timeout of 0. */
  void aeSetDontWait(int no_wait) {
    if (no_wait)
      flags_ |= AE_DONT_WAIT;
    else
      flags_ &= ~AE_DONT_WAIT;
  }

 private:
  // epoll api
  int aeApiAddEvent(int fd, int mask);
  void aeApiDelEvent(int fd, int delmask);
  int aeApiPoll(struct timeval *tvp);

 private:
  /* File event structure */
  typedef struct aeFileEvent {
    int mask; /* one of AE_(READABLE|WRITABLE|BARRIER) */
    aeFileProc rfileProc;
    aeFileProc wfileProc;
    void *clientData;
  } aeFileEvent;

  /* A fired event */
  typedef struct aeFiredEvent {
    int fd;
    int mask;
  } aeFiredEvent;

  int epfd_;
  int maxfd_;      /* highest file descriptor currently registered */
  int event_size_; /* max number of file descriptors tracked */
  std::vector<struct epoll_event> epoll_events_;
  std::vector<aeFileEvent> events_; /* Registered events */
  std::vector<aeFiredEvent> fired_; /* Fired events */
  std::function<void()> beforesleep_;
  std::function<void()> aftersleep_;
  int flags_;
  int stop_;
};
