# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra	-Wno-implicit-function-declaration	-std=c99	-g

# Libraries
LIBS = -lrt -lpthread

# Targets
all:	oss	worker

oss:	oss.o
	$(CC)	$(CFLAGS)	-o	oss	oss.o	$(LIBS)

worker:	worker.o
	$(CC)	$(CFLAGS)	-o	worker	worker.o $(LIBS)

oss.o:	oss.c
	$(CC)	$(CFLAGS)	-c	oss.c

worker.o:	worker.c
	$(CC)	$(CFLAGS)	-c	worker.c

clean:
	rm	-f	oss	worker	oss.o	worker.o

