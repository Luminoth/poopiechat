# variables
OBJECTS = Module.o \
          Client.o \
          CLArgs.o \
          main.o
INCLUDE = .
LIB = .


# targets
all: $(PROGNAME) consoleui

$(PROGNAME): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) -L$(LIB) $(LD_FLAGS)
	$(MAKE) -C consoleui CXX=$(CXX) LIBNAME=console.so CFLAGS="$(CFLAGS)" LD_FLAGS=-shared

strip: all
	strip -sv $(PROGNAME)
	$(MAKE) -C consoleui LIBNAME=console.so strip

clean:
	-rm -f *.o
	-rm -f $(PROGNAME)
	-rm -f core
	-rm -f a.out
	-rm -f *.log
	$(MAKE) -C consoleui LIBNAME=console.so clean


# objects
main.o: main.cpp main.h ../shared.h Module.o Client.o consoleui/main.cpp consoleui/main.h
	$(CXX) -I$(INCLUDE) $(CFLAGS) -c $<

Module.o: Module.cpp Module.h
	$(CXX) -I$(INCLUDE) $(CFLAGS) -c $<

Client.o: Client.cpp Client.h ../shared.h Module.o consoleui/main.cpp consoleui/main.h
	$(CXX) -I$(INCLUDE) $(CFLAGS) -c $<

CLArgs.o: CLArgs.cpp CLArgs.h
	$(CXX) -I$(INCLUDE) $(CFLAGS) -c $<
