CFLAGS=-Wall -Wextra -O3

all: bar update_bar

bar: bar.o

update_bar: update_bar.o

clean:
	rm *.o

install: bar update_bar
	install -m 555 update_bar /usr/local/bin/
	install -m 555 bar /usr/local/bin/
	install -m 555 bar_helper.sh /usr/local/bin/

