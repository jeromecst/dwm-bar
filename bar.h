#ifndef BARH
#define BARH
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

#define FIFO "/tmp/bar.fifo"
#define HASHSIZE 4096
#define SIZE 128
#define BAR_SIZE 512
#define R_INTERVAL 60
#define FBCK 30
#define NFLAG 14
#define DATE 0x1
#define BATTERY 0x2
#define NETWORK 0x4
#define VOLUME 0x8
#define TEMP 0x10
#define DISK 0x20
#define MAIL 0x40
#define MUSIC 0x80
#define MIC 0x100
#define BACKLIGHT 0x200
#define RELOAD 0x400
#define UP 0x800
#define DOWN 0x1000
#define TOGGLE 0x2000

#define NORMAL ''
#define RED ''
#define GREEN ''
#define YELLOW ''
#define BLUE ''
#define MAGENTA ''
#define CYAN ''

#define get_bar(a) (bar[flag_to_idx(a)])

unsigned int flag_to_idx(unsigned short a)
{
	for (unsigned int i = 0; i < sizeof(unsigned short) * 8; i++) {
		if(a & 1) return i;
		a >>= 1;
	}
	return 0;
}

unsigned short idx_to_flag(unsigned a)
{
	static unsigned short flag;
	flag = 1;
	for (unsigned i = 0; i  < a; i++)
		flag <<= 1;
	return flag;
}

unsigned short get_first_flag(unsigned a)
{
	for (unsigned int i = 0; i < sizeof(unsigned short) * 8; i++) {
		if(a & 1) return i;
		a >>= 1;
	}
	return 0;
}

int system_pipe(const char *file, char *argv[], char *return_buffer)
{
	static int fd[2];
	if (return_buffer != NULL) {
		if(pipe(fd) != 0)
			perror("pipe");
	}
	if (fork() == 0) {
		if (return_buffer != NULL) {
			close(fd[0]);
			if (dup2(fd[1], STDOUT_FILENO) == -1)
				perror("dup2");
		}
		else {
			close(0);
			close(1);
			close(2);
		}
		if (execv(file, argv) == -1)
			perror("execv");
	}
	int return_value;
	wait(&return_value);
	if (return_buffer != NULL) {
		close(fd[1]);
		memset(return_buffer, '\0', SIZE);
		if (read(fd[0], return_buffer, SIZE) < 0)
			perror("read");
		close(fd[0]);
	}
	return return_value;
}

int timeout(int fd, fd_set *fds, struct timeval *tval, time_t *rtime)
{
	tval->tv_sec = R_INTERVAL - *rtime % R_INTERVAL;
	tval->tv_usec = 0;
	FD_ZERO(fds);
	FD_SET(fd, fds);
	return select(fd + 1, fds, NULL, NULL, tval);
}

#endif
