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
	memset(bar[DATE], '\0', SIZE);
	tm_time = localtime(&rtime);
	sprintf(bar[0], "%02d-%s %02d:%02d", tm_time->tm_mday, mon[tm_time->tm_mon], tm_time->tm_hour, tm_time->tm_min);
}

void update_network()
{
	memset(bar[NETWORK], '\0', SIZE);
	char * arg[] = {"bar_helper.sh", "network", NULL};
	system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[NETWORK]);
}

void update_battery()
{
	memset(bar[BATTERY], '\0', SIZE);
	char * arg[] = {"which", "acpi", NULL};
	if(system_pipe("/usr/bin/which", arg, NULL) == 0)
	{
		char * arg2[] = {"bar_helper.sh", "battery", NULL};
		system_pipe("/usr/local/bin/bar_helper.sh", arg2, bar[BATTERY]);
	}
}

void update_volume()
{
	memset(bar[VOLUME], '\0', SIZE);
	char * arg[] = {"bar_helper.sh", "volume", NULL};
	system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[VOLUME]);
}

void update_temp()
{
	memset(bar[TEMP], '\0', SIZE);
	char * arg[] = {"bar_helper.sh", "temp", NULL};
	system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[TEMP]);
}

void update_disk()
{
	memset(bar[DISK], '\0', SIZE);
	char * arg[] = {"bar_helper.sh", "disk", NULL};
	system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[DISK]);
}

void update_mail()
{
	memset(bar[MAIL], '\0', SIZE);
	char * arg[] = {"bar_helper.sh", "mail", NULL};
	system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[MAIL]);
}

void update_music()
{
	memset(bar[MUSIC], '\0', SIZE);
	char * arg2[] = {"cmus-remote", "-Q", NULL};
	if(system_pipe("/usr/bin/cmus-remote", arg2, NULL) == 0)
	{
		char * arg[] = {"bar_helper.sh", "music", NULL};
		system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[MUSIC]);
	}
}

void update_mic()
{
	char * arg[] = {"bar_helper.sh", "mic", NULL};
	system_pipe("/usr/local/bin/bar_helper.sh", arg, bar[MIC]);
}

void update_bar()
{
	update_mic();
	update_music();
	update_mail();
	update_disk();
	update_temp();
	update_volume();
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
			if(i != 0) strcat(buf, " ");
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
	update_bar();
	while(1)
	{
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
			update_bar();
		}
		else if( FD_ISSET(fd, &fds) != 0 )
		{
			read(fd, &reload, sizeof(reload));
			switch(reload)
			{
				case(DATE): update_date(); break;
				case(BATTERY): update_battery(); break;
				case(NETWORK): update_network(); break;
				case(VOLUME): update_volume(); break;
				case(TEMP): update_temp(); break;
				case(DISK): update_disk(); break;
				case(MAIL): update_mail(); break;
				case(MUSIC): update_music(); break;
				case(MIC): update_mic(); break;
				case(RELOAD): update_bar(); break;
			} 
		}
	}
	close(fd);
}
