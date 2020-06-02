CC=gcc
CFLAGS=  
LDFLAGS=
DEBUG= -g

all: minls minget

minls: minls.o min.o 
	${CC} ${CFLAGS} ${DEBUG} -o minls minls.o min.o 

minget: minget.o min.o
	${CC} ${CFLAGS} ${DEBUG} -o minget minget.o min.o

minls.o: minls.c 
	${CC} ${CFLAGS} ${DEBUG} -c -o minls.o minls.c 

minget.o: minget.c 
	${CC} ${CFLAGS} ${DEBUG} -c -o minget.o minget.c

min.o: min.c
	${CC} ${CFLAGS} ${DEBUG} -c -o min.o min.c

clean:
	rm *.o




