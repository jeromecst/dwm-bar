CFLAGS=-Wall -Wextra -O3

all: dwm-bar update-dwm-bar

dwm-bar: dwm-bar.o

update-dwm-bar: update-dwm-bar.o

clean:
	rm *.o

install: bar update-dwm-bar
	install -m 555 update-dwm-bar /usr/local/bin/
	install -m 555 dwm-bar /usr/local/bin/
	install -m 555 bar-helper.sh /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/update-dwm-bar
	rm -f /usr/local/bin/dwm-bar
	rm -f /usr/local/bin/bar-helper.sh
