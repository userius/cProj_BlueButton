
#ifndef TERMIOS_STUB_H
#define TERMIOS_STUB_H

/* Dummy termios structure for non-POSIX systems (e.g., STM32) */
struct termios {
    unsigned int c_cflag;  /* control mode flags (stub) */
};

/* Dummy macros to avoid build errors */
#define PARENB  0x0001
#define PARODD  0x0002
#define CSTOPB  0x0004
#define CS8     0x0008

/* No-op functions to replace POSIX ones */
static inline int tcgetattr(int fd, struct termios *termios_p) {
    (void)fd; (void)termios_p;
    return 0;
}
static inline int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    (void)fd; (void)optional_actions; (void)termios_p;
    return 0;
}
static inline int cfsetispeed(struct termios *termios_p, unsigned int speed) {
    (void)termios_p; (void)speed;
    return 0;
}
static inline int cfsetospeed(struct termios *termios_p, unsigned int speed) {
    (void)termios_p; (void)speed;
    return 0;
}

#endif /* TERMIOS_STUB_H */
