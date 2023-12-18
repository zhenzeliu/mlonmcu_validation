# Makefile for varification of mlonmcu

CC = clang

CFLAG = -Wall

INCLUDE_PATH = inc/
SOURCE_PATH = src/

OBJECTS = test.o mlif.o

test: $(OBJECTS)
	$(CC) $(CFLAG) -o test $(OBJECTS)

test.o: test.c
	$(CC) $(CFLAG) -I$(INCLUDE_PATH) -c test.c

mlif.o: $(SOURCE_PATH)mlif.c $(INCLUDE_PATH)mlif.h
	$(CC) $(CFLAG) -I$(INCLUDE_PATH) -c $(SOURCE_PATH)mlif.c

clean:
	del *.exe
	del *.o
