all: main decode


main: main.o
	gcc main.o -o main

decode: decode.o
	gcc decode.o -o decode

%.o: %.c
	gcc -c $<

clean:
	rm main decode *.o

distclean: clean
	rm key.txt *.enc *.dec
