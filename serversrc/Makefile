# variables
OBJECTS = Server.o \
          main.o
INCLUDE = .
LIB = .


# targets
all: $(PROGNAME)

$(PROGNAME): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) -L$(LIB) $(LD_FLAGS)

strip: all
	strip -sv $(PROGNAME)

clean:
	-rm -f *.o
	-rm -f $(PROGNAME)
	-rm -f core
	-rm -f a.out
	-rm -f *.log


# objects
main.o: main.cpp main.h Server.o
	$(CXX) -I$(INCLUDE) $(CFLAGS) -c $<

Server.o: Server.cpp Server.h ../shared.h
	$(CXX) -I$(INCLUDE) $(CFLAGS) -c $<
