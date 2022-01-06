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

#define FIFO "/tmp/bar.fifo"
#define HASHSIZE 4096
#define SIZE 128
#define BAR_SIZE 512
#define R_INTERVAL 60
#define FBCK 30

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
#define VOLUP 10
#define VOLDOWN 11
#define VOLTOGGLE 12

int system_pipe(char* file, char *argv[], char * return_buffer)
{
	int fd[2];
	if(return_buffer != NULL)
	{
		if(pipe(fd) != 0)
		{
			perror("pipe");
		}
	}
	if(fork() == 0)
	{
		if(return_buffer != NULL)
		{
			close(fd[0]);
			if (dup2(fd[1], STDOUT_FILENO) == -1)
			{
				perror("dup2");
			}
		}
		else
		{
			close(0);
			close(1);
			close(2);
		}
		if(execv(file, argv) == -1)
		{
			perror("execv");
		}
	}
	int return_value;
	wait(&return_value);
	if(return_buffer != NULL)
	{
		close(fd[1]);
		memset(return_buffer, '\0', SIZE);
		if(read(fd[0], return_buffer, SIZE) < 0)
		{
			perror("read");
		}
		close(fd[0]);
	}
	return return_value;
}

int timeout(int fd, fd_set * fds, struct timeval * tval, time_t * rtime)
{
	tval->tv_sec = R_INTERVAL - *rtime%R_INTERVAL;
	/* printf("timeout set for.. %ld\n", tval->tv_sec); */
	FD_ZERO(fds);
	FD_SET(fd, fds);
	return select(fd + 1, fds, NULL, NULL, tval);
}

#endif
