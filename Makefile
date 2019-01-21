
all: F6jigYck4v

benchmark: main.c
	gcc -lm -DBENCHMARK -o $@ $<

F6jigYck4v: main.c
	gcc -lm -o $@ $<

clean:
	rm F6jigYck4v
