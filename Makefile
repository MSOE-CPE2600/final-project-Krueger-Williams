CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=
SOURCES=server.c client.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLES=server client

all: $(SOURCES) $(EXECUTABLES)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

server: server.o
	$(CC) server.o $(LDFLAGS) -o $@

client: client.o
	$(CC) client.o $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $< > $*.d

clean:
	rm -rf $(OBJECTS) $(EXECUTABLES) *.d
