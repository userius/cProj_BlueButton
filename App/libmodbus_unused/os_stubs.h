
#ifndef OS_STUBS_H
#define OS_STUBS_H

#include <stdint.h>
#include <stddef.h>

#define EBADF      101
#define ECONNRESET 102
#define EPIPE      103
#define ETIMEDOUT      104
#define ECONNREFUSED      105
#define ENOPROTOOPT      106

/* File descriptor type stub (libmodbus expects int) */
typedef int fd_t;
typedef int speed_t;

/* Minimal timeval replacement (for select stub) */
struct timeval {
  long tv_sec;  /* seconds */
  long tv_usec; /* microseconds */
};

struct timespec {
  long tv_sec;  /* seconds */
  long tv_nsec; /* nanoseconds */
};

/* fd_set stub */
typedef struct {
  uint32_t fds_bits[ 1 ]; /* only one UART supported, dummy */
} fd_set;

/* FD_SET macros (no real meaning here) */
#define FD_ZERO( set )      ( (void) ( set ) )
#define FD_SET( fd, set )   ( (void) ( fd ), (void) ( set ) )
#define FD_ISSET( fd, set ) ( 1 )

/* Stubs for POSIX I/O */
int          read( int fd, void *buf, size_t count );
int          write( int fd, const void *buf, size_t count );
int          close( int fd );
int          select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                     struct timeval *timeout );
void         usleep( unsigned int usec );
unsigned int sleep( unsigned int seconds );

#endif /* OS_STUBS_H */
