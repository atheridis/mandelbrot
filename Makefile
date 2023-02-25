PROG = mandelbrot
DESTPREFIX = /usr/local/bin/

CC = gcc
VERSION = -std=c99
WARNINGS = -Wpedantic -Wall -Wextra -Wunused-parameter -Wunused-variable \
	   -Wunused-function
INC =-lm -pthread  
OO = -O3
CFLAGS = ${VERSION} ${WARNINGS} ${INC} ${OO}

SRC = mandelbrot.c
OBJ = ${SRC:.c=.o}

all: ${PROG}

%.o: %.c
	${CC} -c $< ${CFLAGS}

$(PROG): ${OBJ}
	${CC} -o $@ ${OBJ} ${CFLAGS}

clean:
	rm -f ${PROG} ${OBJ}

install: all
	mkdir -p ${DESTPREFIX}
	cp -f ${PROG} ${DESTPREFIX}
	chmod 755 ${DESTPREFIX}/${PROG}

uninstall:
	rm -f ${DESTPREFIX}/${PROG}

.PHONY: all clean install uninstall
