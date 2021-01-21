# compiler to use
CC = clang

# flags to pass compiler
CFLAGS = -Wall -Werror

# name for executable
BIN = 2in1screen

# space-separated list of source files
SRCS = 2in1screen.c

# automatically generated list of object files
OBJS = $(SRCS:.c=.o)

all: $(BIN)

# binarie
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	
# dependencies
$(OBJS): Makefile

clean: 
	rm -f $(OBJS)

mrproper: clean
	rm -f $(BIN)