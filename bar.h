#ifndef BARH
#define BARH

#include <stdlib.h>

#define FIFO "/tmp/dwm-bar.fifo"
#define HASHSIZE 4096
#define SIZE 128
#define BAR_SIZE 512
#define R_INTERVAL 60
#define FBCK 30
#define NFLAG 15

/* index of elements inside the bar array char*[NFLAG] */
#define DATE 0
#define BATTERY 1
#define NETWORK 2
#define VOLUME 3
#define TEMP 4
#define DISK 5
#define MAIL 6
#define MEM 7
#define MUSIC 8
#define MIC 9
#define BACKLIGHT 10
#define RELOAD 11
#define UP 12
#define DOWN 13
#define TOGGLE 14

unsigned int flag_to_idx(unsigned short a);
int system_pipe(const char *file, char *const argv[], char *return_buffer);
int timeout(int fd, fd_set *fds, struct timeval *tval, time_t *rtime);
int xsetroot(const char *bar_name);

#endif
