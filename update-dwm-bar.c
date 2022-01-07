#include "bar.h"

unsigned hash(char *s)
{
	unsigned hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + 31*hashval;
	return hashval % HASHSIZE;
}

void print_hash()
{
	char * list[NFLAG] = {"date", "battery", "network", "volume",
		"temp", "disk", "mail", "music", "mic", "backlight", "reload",
		"up", "down", "toggle"};
	for(int i = 0; i < NFLAG; i++)
	{
		printf("case(%u): return %s;\n", hash(list[i]), list[i]);
	}
}

int hash_to_code(unsigned hash, unsigned flag)
{
	switch(hash)
	{
		case(4014): return DATE;
		case(3597): return BATTERY;
		case(2606): return NETWORK;
		case(1818): 
			    if(flag & TOGGLE)
			    {
				    char * arg[] = {"pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL};
				    system_pipe("/usr/bin/pactl", arg, NULL);
			    }
			    if(flag & UP)
			    {
				    char * arg[] = {"pactl", "set-sink-volume", "@DEFAULT_SINK@", "+1%", NULL};
				    system_pipe("/usr/bin/pactl", arg, NULL);
			    }
			    if(flag & DOWN)
			    {
				    char * arg[] = {"pactl", "set-sink-volume", "@DEFAULT_SINK@", "-1%", NULL};
				    system_pipe("/usr/bin/pactl", arg, NULL);
			    }
			    return VOLUME;
		case(980): return TEMP;
		case(3485): return DISK;
		case(1463): return MAIL;
		case(3621): return MUSIC;
		case(1607): 
			    printf("mute sound\n");
			    char * arg[] = {"pactl", "set-source-mute", "@DEFAULT_SOURCE@", "toggle", NULL};
			    system_pipe("/usr/bin/pactl", arg, NULL);
			    return MIC;
		case(2191):
			    /* do the backlight magic */
			    return BACKLIGHT;
		case(409): return RELOAD;
		case(3739): return UP;
		case(1186): return DOWN;
		case(2900): return TOGGLE;
		default: 
			    printf("hash %d not found\n", hash);
			    exit(1);
	}
}


int main(int argc, char ** argv)
{
	if(argc < 2)
	{
		print_hash();
		return 1;
	}
	unsigned update_code = 0;
	if(argc > 2)
		update_code |= hash_to_code(hash(argv[2]), update_code);
	update_code |= hash_to_code(hash(argv[1]), update_code);
	int fd = open(FIFO, O_WRONLY);
	if(fd < 0)
	{
		perror("open");
		exit(1);
	}
	ssize_t size = write(fd, &update_code, sizeof(unsigned short));
	if(size <= 0)
	{
		perror("write");
		exit(1);
	}
	close(fd);
}
