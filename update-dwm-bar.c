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
	char * list[RELOAD+1] = {"date", "battery", "network", "volume", "temp", "disk", "mail", "music", "mic", "reload"};
	for(int i = 0; i < RELOAD; i++)
	{
		printf("%s -> %u\n", list[i], hash(list[i]));
	}
}

int hash_to_code(unsigned hash)
{
	switch(hash)
	{

		case(4014): return DATE;
		case(3597): return BATTERY;
		case(2606): return NETWORK;
		case(1818): return VOLUME;
		case(980): return TEMP;
		case(3485): return DISK;
		case(1463): return MAIL;
		case(3621): return MUSIC;
		case(1607): return MIC;
		default: exit(1);
	}
}


int main(int argc, char ** argv)
{
	printf("%d\n", RELOAD);
	if(argc < 2)
	{
		print_hash();
		return 1;
	}
	int update_code = hash_to_code(hash(argv[1]));
	printf("%s → %d(%d)\n", argv[1], hash(argv[1]), update_code);
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
