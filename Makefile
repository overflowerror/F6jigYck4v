
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
	@gcc -Wall -fPIC -c $< -o $@

clean:
	@rm *.o
	@rm lib/*.o
	@rm *.so example
