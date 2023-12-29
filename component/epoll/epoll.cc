#include "epoll.h"

#include <unistd.h>

#include <iostream>
#include <stdexcept>

AeEventLoop::AeEventLoop(int size)
    : event_size_(size),
      events(size),
      fired(size),
      stop(0),
      maxfd_(-1),
      beforesleep_(nullptr),
      aftersleep_(nullptr),
      flags(0),
      epfd_(epoll_create(size)),
      epoll_events(size) {
  if (epfd_ == -1) {
    throw std::runtime_error("Failed to create epoll instance.");
  }
  /* Events with mask == AE_NONE are not set. So let's initialize the
   * vector with it. */
  for (int i = 0; i < size; i++) events[i].mask = AE_NONE;
  std::cout << "AeEventLoop start" << std::endl;
}

AeEventLoop::~AeEventLoop() {
  close(epfd_);
  epoll_events.clear();
  events.clear();
  fired.clear();
  std::cout << "AeEventLoop stop" << std::endl;
}

int AeEventLoop::aeResizeSetSize(int size) {
  if (size == event_size_) return AE_OK;
  if (maxfd_ >= event_size_) return AE_ERR;
  epoll_events.resize(size);
  events.resize(size);
  fired.resize(size);
  event_size_ = size;

  /* Make sure that if we created new slots, they are initialized with
   * an AE_NONE mask. */
  for (int i = maxfd_ + 1; i < size; i++) events[i].mask = AE_NONE;
  return AE_OK;
}

int AeEventLoop::aeCreateFileEvent(int fd, int mask, aeFileProc *proc,
                                   void *clientData) {
  if (fd >= event_size_) {
    errno = ERANGE;
    return AE_ERR;
  }
  aeFileEvent *fe = &events[fd];

  if (addEvent(fd, mask) == -1) return AE_ERR;
  fe->mask |= mask;
  if (mask & AE_READABLE) fe->rfileProc = proc;
  if (mask & AE_WRITABLE) fe->wfileProc = proc;
  fe->clientData = clientData;
  if (fd > maxfd_) maxfd_ = fd;
  return AE_OK;
}

void AeEventLoop::aeDeleteFileEvent(int fd, int mask) {
  if (fd >= event_size_) return;
  aeFileEvent *fe = &events[fd];
  if (fe->mask == AE_NONE) return;

  /* We want to always remove AE_BARRIER if set when AE_WRITABLE
   * is removed. */
  if (mask & AE_WRITABLE) mask |= AE_BARRIER;

  delEvent(fd, mask);
  fe->mask = fe->mask & (~mask);
  if (fd == maxfd_ && fe->mask == AE_NONE) {
    /* Update the max fd */
    int j;
    for (j = maxfd_ - 1; j >= 0; j--)
      if (events[j].mask != AE_NONE) break;
    maxfd_ = j;
  }
}

void *AeEventLoop::aeGetFileClientData(int fd) {
  if (fd >= event_size_) return NULL;
  aeFileEvent *fe = &events[fd];
  if (fe->mask == AE_NONE) return NULL;
  return fe->clientData;
}

int AeEventLoop::aeGetFileEvents(int fd) {
  if (fd >= event_size_) return 0;
  aeFileEvent *fe = &events[fd];
  return fe->mask;
}

int AeEventLoop::aeProcessEvents() {
  std::cout << std::endl << "new epoch: aeProcessEvents run" << std::endl;
  int processed = 0, numevents;

  struct timeval tv, *tvp;

  /* If we have to check for events but need to return
   * ASAP because of AE_DONT_WAIT we need to set the timeout
   * to zero */
  if (flags & AE_DONT_WAIT) {
    tv.tv_sec = tv.tv_usec = 0;
    tvp = &tv;
  } else {
    // tv.tv_sec = 0;
    // tv.tv_usec = 1000000;
    // tvp = &tv;
    tvp = NULL; /* wait forever */
  }

  if (beforesleep_ != NULL) beforesleep_();

  /* Call the multiplexing API, will return only on timeout or when
   * some event fires. */
  numevents = poll(tvp);

  /* After sleep callback. */
  if (aftersleep_ != NULL) aftersleep_();

  for (int j = 0; j < numevents; j++) {
    int fd = fired[j].fd;
    aeFileEvent *fe = &events[fd];
    int mask = fired[j].mask;
    int fired_num = 0; /* Number of events fired for current fd. */

    /* Normally we execute the readable event first, and the writable
     * event later. This is useful as sometimes we may be able
     * to serve the reply of a query immediately after processing the
     * query.
     *
     * However if AE_BARRIER is set in the mask, our application is
     * asking us to do the reverse: never fire the writable event
     * after the readable. In such a case, we invert the calls.
     * This is useful when, for instance, we want to do things
     * in the beforeSleep() hook, like fsyncing a file to disk,
     * before replying to a client. */
    int invert = fe->mask & AE_BARRIER;

    /* Note the "fe->mask & mask & ..." code: maybe an already
     * processed event removed an element that fired and we still
     * didn't processed, so we check if the event is still valid.
     *
     * Fire the readable event if the call sequence is not
     * inverted. */
    if (!invert && fe->mask & mask & AE_READABLE) {
      fe->rfileProc(fd, fe->clientData, mask);
      fired_num++;
      fe = &events[fd]; /* Refresh in case of resize. */
    }

    /* Fire the writable event. */
    if (fe->mask & mask & AE_WRITABLE) {
      if (!fired_num || fe->wfileProc != fe->rfileProc) {
        fe->wfileProc(fd, fe->clientData, mask);
        fired_num++;
      }
    }

    /* If we have to invert the call, fire the readable event now
     * after the writable one. */
    if (invert) {
      fe = &events[fd]; /* Refresh in case of resize. */
      if ((fe->mask & mask & AE_READABLE) &&
          (!fired_num || fe->wfileProc != fe->rfileProc)) {
        fe->rfileProc(fd, fe->clientData, mask);
        fired_num++;
      }
    }

    processed++;
  }
  return processed; /* return the number of processed file/time events */
}

int AeEventLoop::addEvent(int fd, int mask) {
  struct epoll_event ee = {0}; /* avoid valgrind warning */
  /* If the fd was already monitored for some event, we need a MOD
   * operation. Otherwise we need an ADD operation. */
  int op = events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  ee.events = 0;
  mask |= events[fd].mask; /* Merge old events */
  if (mask & AE_READABLE) ee.events |= EPOLLIN;
  if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
  ee.data.fd = fd;
  if (epoll_ctl(epfd_, op, fd, &ee) == -1) return -1;
  return 0;
}

void AeEventLoop::delEvent(int fd, int delmask) {
  struct epoll_event ee = {0}; /* avoid valgrind warning */
  int mask = events[fd].mask & (~delmask);

  ee.events = 0;
  if (mask & AE_READABLE) ee.events |= EPOLLIN;
  if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
  ee.data.fd = fd;
  if (mask != AE_NONE) {
    epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ee);
  } else {
    /* Note, Kernel < 2.6.9 requires a non null event pointer even for
     * EPOLL_CTL_DEL. */
    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &ee);
  }
}

int AeEventLoop::poll(struct timeval *tvp) {
  int retval, numevents = 0;

  retval =
      epoll_wait(epfd_, &epoll_events[0], event_size_,
                 tvp ? (tvp->tv_sec * 1000 + (tvp->tv_usec + 999) / 1000) : -1);
  if (retval > 0) {
    numevents = retval;
    for (int j = 0; j < numevents; j++) {
      int mask = 0;
      struct epoll_event *e = &epoll_events[j];

      if (e->events & EPOLLIN) mask |= AE_READABLE;
      if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
      if (e->events & EPOLLERR) mask |= AE_WRITABLE | AE_READABLE;
      if (e->events & EPOLLHUP) mask |= AE_WRITABLE | AE_READABLE;
      fired[j].fd = e->data.fd;
      fired[j].mask = mask;
    }
  } else if (retval == -1 && errno != EINTR) {
    throw std::runtime_error("AeApiPoll: epoll_wait err");
  }

  return numevents;
}