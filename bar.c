#include "bar.h"

int fd;
struct tm * tm_time;
time_t rtime;
char * mon[] = {"Jan",  "Feb",  "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

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

void get_date(char * buf)
{
	tm_time = localtime(&rtime);
	sprintf(buf, "%02d-%s %02d:%02d", tm_time->tm_mday, mon[tm_time->tm_mon], tm_time->tm_hour, tm_time->tm_min);
}

int main()
{
	signal(SIGINT, close_handler);
	char buffer[SIZE];
	unsigned short reload;
	unlink(FIFO);
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
		rtime = time(NULL);
		reload = -1;
		get_date(buffer);
		printf("%s\n", buffer);
		int s = read(fd, &reload, sizeof(reload));
		if(s < 0)
		{
			perror("read");
		}
		printf("%d\n", reload);
		{
			char *arg_xsetroot[] = {"xsetroot", "-name", "bar this is me", NULL};
			system_pipe("/usr/bin/xsetroot", arg_xsetroot, NULL);
		}
	}
	close(fd);
}
