all: clean
	gcc *.c -o shell -m32 -Wall

clean:
	rm -f *.o

