#include "bar.h"

int fd;
time_t rtime;
char * bar[RELOAD+1];
#define get_bar(a) (bar[flag_to_idx(a)])

static void init_strings()
{
	for(int i = 0; i < RELOAD+1; i++)
	{
		bar[i] = (char *) malloc(SIZE*sizeof(char));
	}
}

static void close_handler()
{
	close(fd);
	unlink(FIFO);
	exit(0);
}

static void update_date()
{
	static const char * mon[] = {"Jan",  "Feb",  "Mar", "Apr", "May",
		"Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	memset(get_bar(DATE), '\0', SIZE);
	static struct tm * tm_time;
	tm_time = localtime(&rtime);
	sprintf(bar[0], "%02d-%s %02d:%02d", tm_time->tm_mday,
			mon[tm_time->tm_mon], tm_time->tm_hour,
			tm_time->tm_min);
}

static void update_network()
{
	memset(get_bar(NETWORK), '\0', SIZE);
	static char * arg[] = {"bar-helper.sh", "network", NULL};
	system_pipe("/usr/local/bin/bar-helper.sh", arg, get_bar(NETWORK));
}

static void update_battery()
{
	memset(get_bar(BATTERY), '\0', SIZE);
	static char * arg[] = {"which", "acpi", NULL};
	if(system_pipe("/usr/bin/which", arg, NULL) == 0)
	{
		static char * arg2[] = {"bar-helper.sh", "battery", NULL};
		system_pipe("/usr/local/bin/bar-helper.sh", arg2, get_bar(BATTERY));
	}
}

static void update_volume(unsigned short action)
{
	static short volume_number = -1; 
	static int counter = FBCK;
	static int len = 0;
	if(counter == 0) counter = FBCK;
	/* if volume is not set, fallback to default volume handler */
	if(!(action & (UP | DOWN)) || (len = strlen(get_bar(VOLUME))) < 2 || --counter == 0)
	{
		volume_number = -1;
		memset(get_bar(VOLUME), '\0', SIZE);
		static char * arg[] = {"bar-helper.sh", "volume", NULL};
		system_pipe("/usr/local/bin/bar-helper.sh", arg, get_bar(VOLUME));
		return;
	}
	if(volume_number < 0)
	{
		get_bar(VOLUME)[len-1] = '\0';
		volume_number = atoi(get_bar(VOLUME));
	}
	if(action & UP)
		++volume_number;
	else if(action & DOWN)
		volume_number == 0 ? 0 : --volume_number;
	memset(get_bar(VOLUME), '\0', SIZE);
	sprintf(get_bar(VOLUME), "%u%%", volume_number);
}

static void update_temp()
{
	memset(get_bar(TEMP), '\0', SIZE);
	static char * arg[] = {"bar-helper.sh", "temp", NULL};
	system_pipe("/usr/local/bin/bar-helper.sh", arg, get_bar(TEMP));
}

static void update_disk()
{
	memset(get_bar(DISK), '\0', SIZE);
	static char * arg[] = {"bar-helper.sh", "disk", NULL};
	system_pipe("/usr/local/bin/bar-helper.sh", arg, get_bar(DISK));
}

static void update_mail()
{
	memset(get_bar(MAIL), '\0', SIZE);
	static char * arg[] = {"bar-helper.sh", "mail", NULL};
	system_pipe("/usr/local/bin/bar-helper.sh", arg, get_bar(MAIL));
}

static void update_music()
{
	memset(get_bar(MUSIC), '\0', SIZE);
	static char * arg2[] = {"cmus-remote", "-Q", NULL};
	if(system_pipe("/usr/bin/cmus-remote", arg2, NULL) == 0)
	{
		static char * arg[] = {"bar-helper.sh", "music", NULL};
		system_pipe("/usr/local/bin/bar-helper.sh", arg, get_bar(MUSIC));
	}
}

static void update_mic()
{
	static char * arg[] = {"bar-helper.sh", "mic", NULL};
	system_pipe("/usr/local/bin/bar-helper.sh", arg, get_bar(MIC));
}

static void update_bar()
{
	update_date();
	update_mic();
	update_music();
	update_mail();
	update_disk();
	update_temp();
	update_volume(RELOAD);
	update_battery();
	update_network();
}


static void make_bar(char * buf)
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


static void display_bar(char * buf)
{
	static char final_buffer[BAR_SIZE];
	sprintf(final_buffer, " %s ", buf);
	static char *arg_xsetroot[] = {"xsetroot", "-name", final_buffer, NULL};
	system_pipe("/usr/bin/xsetroot", arg_xsetroot, NULL);
}

int main()
{
	rtime = time(NULL);
	char bar_buf[BAR_SIZE];
	init_strings();
	signal(SIGINT, close_handler);
	struct timeval tval = {0, 100*1000000};
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
		rtime = time(NULL);
		make_bar(bar_buf);
		display_bar(bar_buf);
		int ss = timeout(fd, &fds, &tval, &rtime);
		if( ss < 0 )
		{
			perror("select");
		}
		/* timeout */
		else if( ss == 0)
		{
			rtime = time(NULL);
			update_bar();
		}
		/* reload specific item */
		else if( FD_ISSET(fd, &fds) != 0 )
		{
			if(read(fd, &reload, sizeof(reload)) < 0)
			{
				perror("read");
			}
			printf("received flag %d", reload);
			int first_flag = idx_to_flag(flag_to_idx(reload));
			printf(" first flag is %d\n", first_flag);
			switch(first_flag)
			{
				case(DATE): update_date(); break;
				case(BATTERY): update_battery(); break;
				case(NETWORK): update_network(); break;
				case(VOLUME): update_volume(reload); break;
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
