VERSION=1.0

include ../../rules.mk

HOST        := arm-fsl-linux-gnueabi

#TOOLS_DIR   := ../tools

target  = keypad_test

objects = $(patsubst %.c, %.o, $(wildcard *.c))


all:$(objects)
	$(CC) -o $(target) *.o $(LIBS)
	
clean:
	rm -rf *.o $(target)
