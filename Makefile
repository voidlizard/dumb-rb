all:
	cc ./tests.c ./ringbuf.c -o ./tests

clean:
	git clean -f -d

