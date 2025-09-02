
#include "os_stubs.h"
#include "main.h"

/* Assume ctx->s holds UART handle cast to int.
   In production you should store UART_HandleTypeDef* instead. */

/* Dummy UART pointer – replace with your actual UART handle */
extern UART_HandleTypeDef huart2;

int read(int fd, void *buf, size_t count) {
    (void)fd;
    if (HAL_UART_Receive(&huart2, (uint8_t*)buf, count, HAL_MAX_DELAY) == HAL_OK) {
        return (int)count;
    }
    return -1;
}

int write(int fd, const void *buf, size_t count) {
    (void)fd;
    if (HAL_UART_Transmit(&huart2, (uint8_t*)buf, count, HAL_MAX_DELAY) == HAL_OK) {
        return (int)count;
    }
    return -1;
}

int close(int fd) {
    (void)fd;
    return 0;   /* nothing to do on baremetal */
}

/* Simple select stub: just wait until data is available */
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout) {
    (void)nfds; (void)readfds; (void)writefds; (void)exceptfds;
    /* On STM32 you’d check UART RXNE flag or use interrupt/RTOS queue */
    HAL_Delay(timeout ? timeout->tv_sec * 1000 : 1);
    return 1; /* pretend something is ready */
}

void usleep(unsigned int usec) {
    HAL_Delay(usec / 1000);
}

unsigned int sleep(unsigned int seconds) {
    HAL_Delay(seconds * 1000);
    return 0;
}
