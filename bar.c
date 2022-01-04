#include "bar.h"

int fd;
struct tm * tm_time;
time_t rtime;
char * mon[] = {"Jan",  "Feb",  "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char * bar[7];
/*
date
network
volume
disk
mic
temp
battery
*/

int system_pipe(char* file, char *argv[], char * return_buffer)
{
	int fd[2];
	pipe(fd);
	if(fork() == 0)
	{
		if (dup2(STDOUT_FILENO, fd[0]) == -1)
		{
			perror("dup2");
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
		memset(return_buffer, '\0', SIZE);
		read(fd[1], return_buffer, SIZE);
	}
	close(fd[0]);
	return return_value;
}

void close_handler()
{
	close(fd);
	unlink(FIFO);
	exit(0);
}

void update_date()
{
	tm_time = localtime(&rtime);
	sprintf(buf, "%02d-%s %02d:%02d", tm_time->tm_mday, mon[tm_time->tm_mon], tm_time->tm_hour, tm_time->tm_min);
}

void make_bar()
{
	get_date(buffer);
	printf("%s\n", buffer);
}

void display_bar(char * buf)
{
	char *arg_xsetroot[] = {"xsetroot", "-name", buf, NULL};
	system_pipe("/usr/bin/xsetroot", arg_xsetroot, NULL);
}

int timeout(int fd, fd_set * fds, struct timeval * tval)
{
	rtime = time(NULL);
	tval->tv_sec = R_INTERVAL - rtime%R_INTERVAL;
	printf("timeout set for.. %ld\n", tval->tv_sec);
	FD_ZERO(fds);
	FD_SET(fd, fds);
	return select(fd + 1, fds, NULL, NULL, tval);
}

int main()
{
	signal(SIGINT, close_handler);
	struct timeval tval = {0, 0};
	unsigned short reload;
	unlink(FIFO);
	fd_set fds;
	if (mkfifo(FIFO, 0640)<0)
	{
		exit(1);
		perror("mkfifo");
	}
	if ((fd = open(FIFO, O_RDWR)) < 0)
	{
		perror("open");
	}
	while(1)
	{
		int ss = timeout(fd, &fds, &tval);
		if( ss < 0 )
		{
			perror("select");
		}
		if( ss == 0)
		{
			/* timeout */
			printf("there is a timeout\n");
		}
		if( FD_ISSET(fd, &fds) )
		{
			/* manage reload */
			reload = -1;
			read(fd, &reload, sizeof(reload));
			printf("reload code number: %d\n", reload);
		}
		make_bar();
		display_bar();
	}
	close(fd);
}
