#
# Makefile
#
# Licença: GPLv3
# Autores: Higor Euripedes
#          Francilene Coelho
#
#
SOURCES=$(wildcard *.c)
SOURCES:=$(sort $(SOURCES))
DAEMON=daemon
CLIENT=cliente
ALL_OBJECTS=$(SOURCES:.c=.o)
ALL_OBJECTS:=$(filter-out wilson.o, $(ALL_OBJECTS))
DAEMON_OBJECTS=$(filter-out cliente.o, $(ALL_OBJECTS))
CLIENT_OBJECTS=$(filter-out daemon.o, $(ALL_OBJECTS))
HEADERS=$(wildcard *.h)
CC=gcc
CFLAGS:=$(CFLAGS) -Wall
LIBS:=$(LIBS)

all: $(ALL_OBJECTS) $(DAEMON) $(CLIENT)
	
$(CLIENT): $(CLIENT_OBJECTS)
	$(CC) -o $(CLIENT) $(CFLAGS) $(LIBS) $(CLIENT_OBJECTS)

$(DAEMON): $(DAEMON_OBJECTS)
	$(CC) -o $(DAEMON)  $(CFLAGS) $(LIBS) $(DAEMON_OBJECTS)

clean:
	-rm -f $(ALL_OBJECTS) $(CLIENT) $(DAEMON) *.trava *.fifo *.log *~

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $<

