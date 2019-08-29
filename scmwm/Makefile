CC=gcc
SOURCES = scmwm.c list.c log.c
CLIENT_SOURCES = client.c
FLAGS = -Wall -Werror -Ofast
DEBUG_FLAGS = -g -Wall -lxcb -DDEBUG_BUILD -DDEBUG
LINK_FLAGS = -lxcb -pthread -lxcb-keysyms
BINARIES = scmwm scmwm-debug-build list-tests scmwmclient scmwmclient-debug

all: $(SOURCES)
	$(CC) $(FLAGS) $(SOURCES) -o scmwm $(LINK_FLAGS) -DLOGGING_RELEASE
	$(CC) $(FLAGS) $(CLIENT_SOURCES) -o scmwmclient -DLOGGING_RELEASE
	$(CC) $(DEBUG_FLAGS) $(SOURCES) -o scmwm-debug-build $(LINK_FLAGS)
	$(CC) $(DEBUG_FLAGS) $(CLIENT_SOURCES) -o scmwmclient-debug

release: $(SOURCES)
	$(CC) $(FLAGS) $(SOURCES) -o scmwm $(LINK_FLAGS) -DLOGGING_RELEASE

debug: $(SOURCES) 
	$(CC) $(DEBUG_FLAGS) $(SOURCES) -o scmwm-debug-build $(LINK_FLAGS)

install: $(SOURCES) 
	$(CC) $(FLAGS) $(SOURCES) -o /usr/bin/scmwm $(LINK_FLAGS) -DLOGGING_RELEASE
	$(CC) $(FLAGS) $(CLIENT_SOURCES) -o /usr/bin/scmwmclient -DLOGGING_RELEASE
	cp scmwm-session /usr/bin/scmwm-session
	chmod +x /usr/bin/scmwm-session
	cp scmwm.desktop /usr/share/xsessions/scmwm.desktop

debug-install: $(SOURCES) 
	$(CC) $(DEBUG_FLAGS) $(SOURCES) -o /usr/bin/scmwm $(LINK_FLAGS)
	$(CC) $(DEBUG_FLAGS) $(CLIENT_SOURCES) -o /usr/bin/scmwmclient
	cp scmwm-session /usr/bin/scmwm-session
	chmod +x /usr/bin/scmwm-session
	cp scmwm.desktop /usr/share/xsessions/scmwm.desktop

tests: list.c
	$(CC) list.c -o list-tests -Wall -g $(LINK_FLAGS) -DDEBUG

clean:
	rm -f $(BINARIES)
