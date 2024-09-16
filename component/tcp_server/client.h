#include <sys/types.h>

#include <cstdint>
#include <list>

class connection;

class client {
  uint64_t id;  /* Client incremental unique ID. */
  time_t ctime; /* Client creation time. */
  connection *conn;
  char *querybuf;          /* Buffer we use to accumulate client queries. */
  size_t qb_pos;           /* The position we have read in querybuf. */
  std::list<void *> reply; /* List of reply objects to send to the client. */
  unsigned long long reply_bytes; /* Tot bytes of objects in reply list. */
};