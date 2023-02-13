#include "bar.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int fd;
time_t rtime;
char *bar[RELOAD + 1];

static void init_strings(void) {
	for (int i = 0; i < RELOAD + 1; i++)
		bar[i] = (char *)malloc(SIZE * sizeof(char));
}

static void close_handler(int sig) {
	fprintf(stdout, "closing: received %d...\n", sig);
	close(fd);
	unlink(FIFO);
	exit(0);
}

static void update_date(void) {
	static struct tm *tm_time;
	static char *const mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	memset(bar[DATE], '\0', SIZE);
	tm_time = localtime(&rtime);
	sprintf(bar[0], "%02d-%s %02d:%02d", tm_time->tm_mday, mon[tm_time->tm_mon],
	        tm_time->tm_hour, tm_time->tm_min);
}

static void update_network(void) {
	static char *const arg[] = {"bar-helper.sh", "network", NULL};

	memset(bar[NETWORK], '\0', SIZE);
	system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[NETWORK]);
}

static void update_mem(void) {
	static char *const arg[] = {"bar-helper.sh", "mem", NULL};

	memset(bar[MEM], '\0', SIZE);
	system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[MEM]);
}

static void update_battery(void) {
	static char *const arg[] = {"which", "acpi", NULL};

	memset(bar[BATTERY], '\0', SIZE);
	if (system_pipe("/usr/bin/which", arg, NULL) == 0) {
		static char *const arg2[] = {"bar-helper.sh", "battery", NULL};

		system_pipe("/usr/local/bin/bar-helper.sh", arg2, bar[BATTERY]);
	}
}

static void update_volume(unsigned short action) {
	static short volume_number = -1;
	static int counter = FBCK;
	static int len;

	if (counter == 0)
		counter = FBCK;
	/* if volume is not set, fallback to default volume handler */
	len = strlen(bar[VOLUME]);
	if (action == VOLUME || len < 2 || --counter == 0) {
		static char *const arg[] = {"bar-helper.sh", "volume", NULL};

		volume_number = -1;
		memset(bar[VOLUME], '\0', SIZE);
		system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[VOLUME]);
		return;
	}
	if (volume_number < 0) {
		bar[VOLUME][len - 1] = '\0';
		volume_number = atoi(bar[VOLUME]);
	}
	memset(bar[VOLUME], '\0', SIZE);
	switch (action) {
	case (TOGGLE):
		return;
	case (DOWN):
		volume_number == 0 ? 0 : --volume_number;
		break;
	case (UP):
		++volume_number;
		break;
	}
	sprintf(bar[VOLUME], "%u%%", volume_number);
}

static void update_temp(void) {
	static char *const arg[] = {"bar-helper.sh", "temp", NULL};

	memset(bar[TEMP], '\0', SIZE);
	system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[TEMP]);
}

static void update_disk(void) {
	static char *const arg[] = {"bar-helper.sh", "disk", NULL};

	memset(bar[DISK], '\0', SIZE);
	system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[DISK]);
}

static void update_mail(void) {
	static char *const arg[] = {"bar-helper.sh", "mail", NULL};

	memset(bar[MAIL], '\0', SIZE);
	system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[MAIL]);
}

static void update_music(void) {
	static char *const arg2[] = {"cmus-remote", "-Q", NULL};

	memset(bar[MUSIC], '\0', SIZE);
	if (system_pipe("/usr/bin/cmus-remote", arg2, NULL) == 0) {
		static char *const arg[] = {"bar-helper.sh", "music", NULL};

		system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[MUSIC]);
	}
}

static void update_mic(void) {
	static char *const arg[] = {"bar-helper.sh", "mic", NULL};

	system_pipe("/usr/local/bin/bar-helper.sh", arg, bar[MIC]);
}

static void update_bar(void) {
	update_date();
	update_mic();
	update_music();
	update_mail();
	update_disk();
	update_temp();
	update_volume(RELOAD);
	update_battery();
	update_network();
	update_mem();
}

static void make_bar(char *buf) {
	memset(buf, '\0', BAR_SIZE);
	for (int i = RELOAD; i >= 0; i--) {
		if (strlen(bar[i]) > 0) {
			strcat(buf, bar[i]);
			if (i != 0)
				strcat(buf, " ");
		}
	}
}

static void display_bar(char *buf) {
	static char final_buffer[BAR_SIZE];

	sprintf(final_buffer, " %s ", buf);
	xsetroot(final_buffer);
}

int main(void) {
	struct timeval tval = {0, 100 * 1000000};
	char bar_buf[BAR_SIZE];
	int ss;
	unsigned int reload;

	rtime = time(NULL);
	init_strings();
	signal(SIGINT, close_handler);
	unlink(FIFO);
	fd_set fds;

	if (mkfifo(FIFO, 0600) < 0) {
		exit(1);
		perror("mkfifo");
	}

	fd = open(FIFO, O_RDWR);
	if (fd < 0)
		perror("open");
	update_bar();

	while (1) {
		rtime = time(NULL);
		make_bar(bar_buf);
		display_bar(bar_buf);

		/* calls select */
		ss = timeout(fd, &fds, &tval, &rtime);

		if (ss < 0)
			perror("select");
		/* timeout */
		else if (ss == 0) {
			rtime = time(NULL);
			update_bar();
		}
		/* reload specific item */
		else if (FD_ISSET(fd, &fds) != 0) {
			if (read(fd, &reload, sizeof(reload)) < 0)
				perror("read");
			switch (reload) {
			case (DATE):
				update_date();
				break;
			case (BATTERY):
				update_battery();
				break;
			case (NETWORK):
				update_network();
				break;
			case (TEMP):
				update_temp();
				break;
			case (DISK):
				update_disk();
				break;
			case (MAIL):
				update_mail();
				break;
			case (MUSIC):
				update_music();
				break;
			case (MIC):
				update_mic();
				break;
			case (RELOAD):
				update_bar();
				break;
			case (MEM):
				update_mem();
				break;
			case (VOLUME):
				update_volume(VOLUME);
				break;
			case (UP):
				update_volume(UP);
				break;
			case (DOWN):
				update_volume(DOWN);
				break;
			case (TOGGLE):
				update_volume(TOGGLE);
				break;
			default:
				printf("not found %d\n", reload);
				break;
			}
		}
	}
	close(fd);
}
