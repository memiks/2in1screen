# compiler to use
CC = g++

# flags to pass compiler
CFLAGS = -Wall -Werror

# name for executable
BIN = enable2in1screen

# space-separated list of source files
SRCS = config.cpp enable2in1screen.cpp

LIBS = -ljsoncpp -liio

# automatically generated list of object files
OBJS = $(SRCS:.cpp=.o)

all: $(BIN)
	chmod a+x $(BIN)

# binarie
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
	
# dependencies
$(OBJS): Makefile

clean: 
	rm -f $(OBJS)

mrproper: clean
	rm -f $(BIN)