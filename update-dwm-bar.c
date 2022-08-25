#include "bar.h"

#include <stdlib.h>
#include <fcntl.h>

unsigned int hash(char *s)
{
	unsigned int hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + 31*hashval;
	return hashval % HASHSIZE;
}

void print_hash(void)
{
	char *const list[NFLAG] = {"date", "battery", "network", "volume",
		"temp", "disk", "mem", "mail", "music", "mic", "backlight", "reload",
		"up", "down", "toggle"};

	for (int i = 0; i < NFLAG; i++)
		printf("case(%u): return %s;\n", hash(list[i]), list[i]);
}

int hash_to_code(unsigned int hash)
{
	switch (hash) {
	case(4014): return DATE;
	case(3597): return BATTERY;
	case(2606): return NETWORK;
	case(1818): return VOLUME;
	case(980): return TEMP;
	case(3485): return DISK;
	case(1463): return MAIL;
	case(3621): return MUSIC;
	case(1493): return MEM;
	case(1607): return MIC;
	case(2191): return BACKLIGHT;
	case(409): return RELOAD;
	case(3739): return UP;
	case(1186): return DOWN;
	case(2900): return TOGGLE;
	default:
				printf("hash %d not found\n", hash);
				exit(1);
	}
}

int main(int argc, char **argv)
{
	unsigned int update_code = 0;
	ssize_t size;
	int fd;

	if (argc < 2) {
		print_hash();
		return 1;
	}
	fd = open(FIFO, O_WRONLY);
	if (fd < 0) {
		perror("open");
		exit(1);
	}
	for (int i = 1; i < argc; i++) {
		update_code = hash_to_code(hash(argv[i]));
#ifdef DEBUG
		printf("update code is %d\n", update_code);
#endif
		size = write(fd, &update_code, sizeof(unsigned int));
		if (size <= 0)
			perror("write");
	}
	close(fd);
}
