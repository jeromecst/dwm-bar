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

#define FIFO "/tmp/bar2.fifo"
#define HASHSIZE 4096
#define SIZE 1024

#define RELOAD 0
#define VOLUME 1
#define MUTE 2
#define BATTERY 3
#define NETWORK 4
#define MIC 5
#define MAIL 6
#define MUSIC 7

#endif
