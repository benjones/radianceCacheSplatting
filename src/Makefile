CC = g++
OPT = -Wall -O2 -g 

all: radianceSplat

COMPILE = $(CC) $(OPT) -c

OBJFILES := $(pathsubst %.cpp, %.o, $(wildcard *.cpp))

radianceSplat: $(OBJFILES)
	$(CC) -o radianceSplat $(OBJFILES)

%.o: %.cpp
	$(COMPILE) -o $@ $<