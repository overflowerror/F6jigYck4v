
CFLAGS = -Wall -fPIC

all: libsfuid.so example

libsfuid.so: lib/sfuid.o
	@echo Linking shared object...
	@gcc -shared -o $@ $^

example: example.o libsfuid.so
	@echo Linking example program...
	@gcc -L. -lsfuid -lm -o $@ $<

example.o: lib/sfuid.h
lib/sfuid.o: lib/sfuid.h

%.o: %.c
	@echo Compiling $<...
	@gcc ${CFLAGS} -c $< -o $@

benchmark: CFLAGS += -DBENCHMARK
benchmark: clean example
	@mv example benchmark
	@rm -f lib/*.o

clean:
	@rm -f *.o
	@rm -f lib/*.o
	@rm -f *.so
	@rm -f example
	@rm -f benchmark
