CC=gcc

all: final

final: final.c libmyapi.a
	$(CC) -o $@ $< -L. -lmyapi -pthread

libmyapi.a: myapi.o
	ar rcs $@ $<

myapi.o: myapi.c myapi.h
	$(CC) -c $<

clean:
	rm -f final libmyapi.a myapi.o example.txt
