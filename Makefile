# Makefile for varification of mlonmcu

CC = clang

CFLAG = -Wall

INCLUDE_PATH = inc/
SOURCE_PATH = src/
OBJECT_PATH = build/

OBJECTS = test.o mlif.o

test: $(OBJECTS)
	$(CC) $(CFLAG) -o test $(OBJECT_PATH)*.o

test.o: test.c
	$(CC) $(CFLAG) -I$(INCLUDE_PATH) -c test.c -o $(OBJECT_PATH)test.o

mlif.o: $(SOURCE_PATH)mlif.c $(INCLUDE_PATH)mlif.h
	$(CC) $(CFLAG) -I$(INCLUDE_PATH) -c $(SOURCE_PATH)mlif.c -o $(OBJECT_PATH)mlif.o

clean:
	rm test $(OBJECT_PATH)*.o
