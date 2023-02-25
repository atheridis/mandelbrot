PROG = mandelbrot
DESTPREFIX = /usr/local

CC = gcc
CFLAGS = -pedantic -Wall -Wno-deprecated-declarations -O3

SRC = mandelbrot.c
OBJ = ${SRC:.c=.o}

all: ${PROG}

%.o: %.c
	${CC} -c ${CFLAGS} $<

$(PROG): ${OBJ}
	${CC} -o $@ ${OBJ}

clean:
	rm -f ${PROG} ${OBJ}

install: all
	mkdir -p ${DESTPREFIX}/bin/
	cp -f ${PROG} ${DESTPREFIX}/bin/
	chmod 755 ${DESTPREFIX}/bin/${PROG}

uninstall:
	rm -f ${DESTPREFIX}/bin/${PROG}

.PHONY: all clean install uninstall
