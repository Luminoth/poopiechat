# variables
OBJECTS = main.o
INCLUDE = .
LIB = .


# targets
all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) -L$(LIB) $(LD_FLAGS)

strip: all
	strip -sv $(LIBNAME)

clean:
	rm -f *.o
	rm -f $(LIBNAME)
	rm -f core
	rm -f a.out
	rm -f *.log


# objects
main.o: main.cpp main.h
	$(CXX) -I$(INCLUDE) $(CFLAGS) -fPIC -c $<
