# variables
CXX = g++
CLIENT = poopie
SERVER = poopied

# source directories
CLIENT_SOURCE = clientsrc
SERVER_SOURCE = serversrc

# compiler options
BASE_CFLAGS = -Wall -Winline -D_REENTRANT
CFLAGS = $(BASE_CFLAGS) -DDEBUG -g -gstabs+ -fno-default-inline
#CFLAGS = $(BASE_CFLAGS) -DNDEBUG -O3 -finline-functions
CLIENT_LD_FLAGS = -lpthread -ldl
SERVER_LD_FLAGS =


# targets
all: nobin server client
	@echo ""
	mkdir bin/
	mkdir lib/
	cp $(CLIENT_SOURCE)/$(CLIENT) bin/
	cp $(SERVER_SOURCE)/$(SERVER) bin/
	cp $(CLIENT_SOURCE)/consoleui/console.so lib/

nobin:
	rm -rf bin/
	rm -rf lib/
	@echo ""

server:
	$(MAKE) -C $(SERVER_SOURCE) CXX=$(CXX) PROGNAME=$(SERVER) CFLAGS="$(CFLAGS)" LD_FLAGS="$(SERVER_LD_FLAGS)"

client:
	$(MAKE) -C $(CLIENT_SOURCE) CXX=$(CXX) PROGNAME=$(CLIENT) CFLAGS="$(CFLAGS)" LD_FLAGS="$(CLIENT_LD_FLAGS)"

strip: all
	strip -sv bin/$(CLIENT)
	strip -sv bin/$(SERVER)
	strip -sv lib/console.so

clean: nobin
	$(MAKE) -C $(SERVER_SOURCE) PROGNAME=$(SERVER) clean
	$(MAKE) -C $(CLIENT_SOURCE) PROGNAME=$(CLIENT) clean

install: strip
	@echo "not done"
