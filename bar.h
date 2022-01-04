#ifndef BAR
#define BAR
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/select.h>

#define FIFO "/tmp/bar2.fifo"
#define HASHSIZE 4096
#define SIZE 128
#define R_INTERVAL 60

#define DATE 0
#define BATTERY 1
#define NETWORK 2
#define VOLUME 3
#define TEMP 4
#define DISK 5
#define MAIL 6
#define MUSIC 7
#define MIC 8
#define RELOAD 9

#endif
