#include "bar.h"

int fd;
struct tm * tm_time;
time_t rtime;
char * mon[] = {"Jan",  "Feb",  "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char * bar[RELOAD+1];

void init_strings()
{
	for(int i = 0; i < RELOAD+1; i++)
	{
		bar[i] = (char *) malloc(SIZE*sizeof(char));
	}
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
	sprintf(bar[0], "%02d-%s %02d:%02d", tm_time->tm_mday, mon[tm_time->tm_mon], tm_time->tm_hour, tm_time->tm_min);
}

void update_network()
{
	char * arg[] = {"bar_helper.sh", "update_network", NULL};
	system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[NETWORK]);
}

void update_battery()
{
	char * arg[] = {"which", "acpi", NULL};
	if(system_pipe("/usr/bin/which", arg, NULL) == 0)
	{
		char * arg2[] = {"bar_helper.sh", "update_battery", NULL};
		system_pipe("/usr/local/bin/bar_helper.sh", arg2, bar[BATTERY]);
	}
}

void update_bar()
{
	update_battery();
	update_date();
	update_network();
}


void make_bar(char * buf)
{
	memset(buf, '\0', BAR_SIZE);
	for(int i = RELOAD; i >= 0; i--)
	{
		if(strlen(bar[i]) > 0)
		{
			strcat(buf, bar[i]);
			if(i != 0) strcat(buf, " | ");
		}
	}
}


void display_bar(char * buf)
{
	char *arg_xsetroot[] = {"xsetroot", "-name", buf, NULL};
	system_pipe("/usr/bin/xsetroot", arg_xsetroot, NULL);
}

int main()
{
	char bar_buf[BAR_SIZE];
	init_strings();
	signal(SIGINT, close_handler);
	struct timeval tval = {0, 0};
	unsigned short reload;
	unlink(FIFO);
	fd_set fds;
	if (mkfifo(FIFO, 0600)<0)
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
		update_bar();
		make_bar(bar_buf);
		display_bar(bar_buf);
		int ss = timeout(fd, &fds, &tval, &rtime);
		if( ss < 0 )
		{
			perror("select");
		}
		else if( ss == 0)
		{
			/* timeout */
			printf("there is a timeout\n");
		}
		else if( FD_ISSET(fd, &fds) != 0 )
		{
			/* manage reload */
			reload = -1;
			read(fd, &reload, sizeof(reload));
			printf("reload code number: %d\n", reload);
		}
	}
	close(fd);
}
